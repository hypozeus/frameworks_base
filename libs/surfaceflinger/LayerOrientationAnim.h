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

#ifndef ANDROID_LAYER_ORIENTATION_ANIM_H
#define ANDROID_LAYER_ORIENTATION_ANIM_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>
#include <utils/Parcel.h>

#include "LayerBase.h"
#include "LayerBitmap.h"

namespace android {

// ---------------------------------------------------------------------------
class OrientationAnimation;


class LayerOrientationAnimBase : public LayerBase
{
public:
    LayerOrientationAnimBase(SurfaceFlinger* flinger, DisplayID display)
        : LayerBase(flinger, display) {
    }
    virtual void onOrientationCompleted() = 0;
};

// ---------------------------------------------------------------------------

class LayerOrientationAnim : public LayerOrientationAnimBase
{
public:    
    static const uint32_t typeInfo;
    static const char* const typeID;
    virtual char const* getTypeID() const { return typeID; }
    virtual uint32_t getTypeInfo() const { return typeInfo; }
    
                LayerOrientationAnim(SurfaceFlinger* flinger, DisplayID display,
                        OrientationAnimation* anim, 
                        const LayerBitmap& bitmapIn,
                        const LayerBitmap& bitmapOut);
        virtual ~LayerOrientationAnim();

            void onOrientationCompleted();

    virtual void onDraw(const Region& clip) const;
    virtual Point getPhysicalSize() const;
    virtual void validateVisibility(const Transform& globalTransform);
    virtual bool needsBlending() const;
    virtual bool isSecure() const       { return false; }
private:
    void drawScaled(float scale, float alphaIn, float alphaOut) const;

    class Lerp {
        float in;
        float outMinusIn;
    public:
        Lerp() : in(0), outMinusIn(0) { }
        Lerp(float in, float out) : in(in), outMinusIn(out-in) { }
        float getIn() const { return in; };
        float getOut() const { return in + outMinusIn; }
        void set(float in, float out) { 
            this->in = in; 
            this->outMinusIn = out-in; 
        }
        void setIn(float in) { 
            this->in = in; 
        }
        void setOut(float out) { 
            this->outMinusIn = out - this->in; 
        }
        float operator()(float t) const { 
            return outMinusIn*t + in; 
        }
    };
    
    OrientationAnimation* mAnim;
    LayerBitmap mBitmapIn;
    LayerBitmap mBitmapOut;
    nsecs_t mStartTime;
    nsecs_t mFinishTime;
    bool mOrientationCompleted;
    mutable bool mFirstRedraw;
    mutable float mLastNormalizedTime;
    mutable GLuint  mTextureName;
    mutable GLuint  mTextureNameIn;
    mutable bool mNeedsBlending;
    
    mutable Lerp mAlphaInLerp;
    mutable Lerp mAlphaOutLerp;
};

// ---------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_LAYER_ORIENTATION_ANIM_H
