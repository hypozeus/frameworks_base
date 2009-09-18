/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "SurfaceFlinger"

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/StopWatch.h>

#include <core/SkBitmap.h>

#include <ui/EGLDisplaySurface.h>

#include "BlurFilter.h"
#include "LayerBase.h"
#include "LayerOrientationAnim.h"
#include "SurfaceFlinger.h"
#include "DisplayHardware/DisplayHardware.h"
#include "OrientationAnimation.h"

namespace android {
// ---------------------------------------------------------------------------

const uint32_t LayerOrientationAnim::typeInfo = LayerBase::typeInfo | 0x80;
const char* const LayerOrientationAnim::typeID = "LayerOrientationAnim";

// ---------------------------------------------------------------------------

// Animation...
const float DURATION = ms2ns(200);
const float BOUNCES_PER_SECOND = 0.5f;
const float DIM_TARGET = 0.40f;
#define INTERPOLATED_TIME(_t)   (_t)

// ---------------------------------------------------------------------------

LayerOrientationAnim::LayerOrientationAnim(
        SurfaceFlinger* flinger, DisplayID display, 
        OrientationAnimation* anim, 
        const LayerBitmap& bitmapIn,
        const LayerBitmap& bitmapOut)
    : LayerOrientationAnimBase(flinger, display), mAnim(anim), 
      mBitmapIn(bitmapIn), mBitmapOut(bitmapOut), 
      mTextureName(-1), mTextureNameIn(-1)
{
    // blur that texture. 
    mOrientationCompleted = false;
    mNeedsBlending = false;
}

LayerOrientationAnim::~LayerOrientationAnim()
{
    if (mTextureName != -1U) {
        LayerBase::deletedTextures.add(mTextureName);
    }
    if (mTextureNameIn != -1U) {
        LayerBase::deletedTextures.add(mTextureNameIn);
    }
}

bool LayerOrientationAnim::needsBlending() const 
{
    return mNeedsBlending; 
}

Point LayerOrientationAnim::getPhysicalSize() const
{
    const GraphicPlane& plane(graphicPlane(0));
    const DisplayHardware& hw(plane.displayHardware());
    return Point(hw.getWidth(), hw.getHeight());
}

void LayerOrientationAnim::validateVisibility(const Transform&)
{
    const Layer::State& s(drawingState());
    const Transform tr(s.transform);
    const Point size(getPhysicalSize());
    uint32_t w = size.x;
    uint32_t h = size.y;
    mTransformedBounds = tr.makeBounds(w, h);
    mLeft = tr.tx();
    mTop  = tr.ty();
    transparentRegionScreen.clear();
    mTransformed = true;
    mCanUseCopyBit = false;
    copybit_device_t* copybit = mFlinger->getBlitEngine();
    if (copybit) { 
        mCanUseCopyBit = true;
    }
}

void LayerOrientationAnim::onOrientationCompleted()
{
    mAnim->onAnimationFinished();
}

void LayerOrientationAnim::onDraw(const Region& clip) const
{
    float alphaIn =  DIM_TARGET;
    
    // clear screen
    // TODO: with update on demand, we may be able 
    // to not erase the screen at all during the animation 
    if (!mOrientationCompleted) {
        glDisable(GL_BLEND);
        glDisable(GL_DITHER);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    copybit_image_t dst;
    const GraphicPlane& plane(graphicPlane(0));
    const DisplayHardware& hw(plane.displayHardware());
    hw.getDisplaySurface(&dst);

    copybit_image_t src;
    mBitmapIn.getBitmapSurface(&src);

    copybit_image_t srcOut;
    mBitmapOut.getBitmapSurface(&srcOut);

    const int w = dst.w; 
    const int h = dst.h; 
    const int xc = uint32_t(dst.w-w)/2;
    const int yc = uint32_t(dst.h-h)/2;
    const copybit_rect_t drect = { xc, yc, xc+w, yc+h }; 
    const copybit_rect_t srect = { 0, 0, src.w, src.h };
    const Region reg(Rect( drect.l, drect.t, drect.r, drect.b ));

    int err = NO_ERROR;
    const int can_use_copybit = canUseCopybit();
    if (can_use_copybit)  {
        copybit_device_t* copybit = mFlinger->getBlitEngine();
        copybit->set_parameter(copybit, COPYBIT_TRANSFORM, 0);
        copybit->set_parameter(copybit, COPYBIT_DITHER, COPYBIT_ENABLE);
        
        if (alphaIn > 0) {
            region_iterator it(reg);
            copybit->set_parameter(copybit, COPYBIT_BLUR, COPYBIT_ENABLE);
            copybit->set_parameter(copybit, COPYBIT_PLANE_ALPHA, int(alphaIn*255));
            err = copybit->stretch(copybit, &dst, &src, &drect, &srect, &it);
            copybit->set_parameter(copybit, COPYBIT_BLUR, COPYBIT_DISABLE);
        }
        LOGE_IF(err != NO_ERROR, "copybit failed (%s)", strerror(err));
    }
    if (!can_use_copybit || err) {   
        GGLSurface t;
        t.version = sizeof(GGLSurface);
        t.width  = src.w;
        t.height = src.h;
        t.stride = src.w;
        t.vstride= src.h;
        t.format = src.format;
        t.data = (GGLubyte*)(intptr_t(src.base) + src.offset);

        Transform tr;
        tr.set(xc, yc);
        
        // FIXME: we should not access mVertices and mDrawingState like that,
        // but since we control the animation, we know it's going to work okay.
        // eventually we'd need a more formal way of doing things like this.
        LayerOrientationAnim& self(const_cast<LayerOrientationAnim&>(*this));
        tr.transform(self.mVertices[0], 0, 0);
        tr.transform(self.mVertices[1], 0, src.h);
        tr.transform(self.mVertices[2], src.w, src.h);
        tr.transform(self.mVertices[3], src.w, 0);
        if (!(mFlags & DisplayHardware::SLOW_CONFIG)) {
            // Too slow to do this in software
            self.mDrawingState.flags |= ISurfaceComposer::eLayerFilter;
        }

        if (alphaIn > 0.0f) {
            t.data = (GGLubyte*)(intptr_t(src.base) + src.offset);
            if (UNLIKELY(mTextureNameIn == -1LU)) {
                mTextureNameIn = createTexture();
                GLuint w=0, h=0;
                const Region dirty(Rect(t.width, t.height));
                loadTexture(dirty, mTextureNameIn, t, w, h);
            }
            self.mDrawingState.alpha = int(alphaIn*255);
            drawWithOpenGL(reg, mTextureNameIn, t);
        }
    }
}

// ---------------------------------------------------------------------------

}; // namespace android
