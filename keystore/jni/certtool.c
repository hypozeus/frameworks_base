/*
**
** Copyright 2009, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#define LOG_TAG "CertTool"

#include <string.h>
#include <jni.h>
#include <cutils/log.h>
#include <openssl/pkcs12.h>
#include <openssl/x509v3.h>

#include "cert.h"

typedef int PKCS12_KEYSTORE_FUNC(PKCS12_KEYSTORE *store, char *buf, int size);

jstring
android_security_CertTool_generateCertificateRequest(JNIEnv* env,
                                                     jobject thiz,
                                                     jint bits,
                                                     jstring jChallenge)

{
    int ret = -1;
    jboolean bIsCopy;
    char csr[REPLY_MAX];
    const char* challenge = (*env)->GetStringUTFChars(env, jChallenge, &bIsCopy);

    ret = gen_csr(bits, challenge , csr);
    (*env)->ReleaseStringUTFChars(env, jChallenge, challenge);
    if (ret == 0) return (*env)->NewStringUTF(env, csr);
    return NULL;
}

jboolean
android_security_CertTool_isPkcs12Keystore(JNIEnv* env,
                                           jobject thiz,
                                           jbyteArray data)
{
    int len = (*env)->GetArrayLength(env, data);

    if (len > 0) {
        PKCS12 *handle = NULL;
        char buf[len];

        (*env)->GetByteArrayRegion(env, data, 0, len, (jbyte*)buf);
        return (jboolean)is_pkcs12(buf, len);
    } else {
        return 0;
    }
}

jint
android_security_CertTool_getPkcs12Handle(JNIEnv* env,
                                          jobject thiz,
                                          jbyteArray data,
                                          jstring jPassword)
{
    jboolean bIsCopy;
    int len = (*env)->GetArrayLength(env, data);
    const char* passwd = (*env)->GetStringUTFChars(env, jPassword , &bIsCopy);

    if (len > 0) {
        PKCS12_KEYSTORE *handle = NULL;
        char buf[len];

        (*env)->GetByteArrayRegion(env, data, 0, len, (jbyte*)buf);
        handle = get_pkcs12_keystore_handle(buf, len, passwd);
        (*env)->ReleaseStringUTFChars(env, jPassword, passwd);
        return (jint)handle;
    } else {
        return 0;
    }
}

jstring call_pkcs12_ks_func(PKCS12_KEYSTORE_FUNC *func,
                            JNIEnv* env,
                            jobject thiz,
                            jint phandle)
{
    char buf[REPLY_MAX];

    if (phandle == 0) return NULL;
    if (func((PKCS12_KEYSTORE*)phandle, buf, sizeof(buf)) == 0) {
        return (*env)->NewStringUTF(env, buf);
    }
    return NULL;
}

jstring
android_security_CertTool_getPkcs12Certificate(JNIEnv* env,
                                               jobject thiz,
                                               jint phandle)
{
    return call_pkcs12_ks_func((PKCS12_KEYSTORE_FUNC *)get_pkcs12_certificate,
                               env, thiz, phandle);
}

jstring
android_security_CertTool_getPkcs12PrivateKey(JNIEnv* env,
                                              jobject thiz,
                                              jint phandle)
{
    return call_pkcs12_ks_func((PKCS12_KEYSTORE_FUNC *)get_pkcs12_private_key,
                               env, thiz, phandle);
}

jstring
android_security_CertTool_popPkcs12CertificateStack(JNIEnv* env,
                                                    jobject thiz,
                                                    jint phandle)
{
    return call_pkcs12_ks_func((PKCS12_KEYSTORE_FUNC *)pop_pkcs12_certs_stack,
                               env, thiz, phandle);
}

void android_security_CertTool_freePkcs12Handle(JNIEnv* env,
                                                jobject thiz,
                                                jint handle)
{
    if (handle != 0) free_pkcs12_keystore((PKCS12_KEYSTORE*)handle);
}

jint
android_security_CertTool_generateX509Certificate(JNIEnv* env,
                                                  jobject thiz,
                                                  jbyteArray data)
{
    char buf[REPLY_MAX];
    int len = (*env)->GetArrayLength(env, data);

    if (len > REPLY_MAX) return 0;
    (*env)->GetByteArrayRegion(env, data, 0, len, (jbyte*)buf);
    return (jint) parse_cert(buf, len);
}

jboolean android_security_CertTool_isCaCertificate(JNIEnv* env,
                                                   jobject thiz,
                                                   jint handle)
{
    return (handle == 0) ? (jboolean)0 : (jboolean) is_ca_cert((X509*)handle);
}

jstring android_security_CertTool_getIssuerDN(JNIEnv* env,
                                              jobject thiz,
                                              jint handle)
{
    char issuer[MAX_CERT_NAME_LEN];

    if (handle == 0) return NULL;
    if (get_issuer_name((X509*)handle, issuer, MAX_CERT_NAME_LEN)) return NULL;
    return (*env)->NewStringUTF(env, issuer);
}

jstring android_security_CertTool_getCertificateDN(JNIEnv* env,
                                                   jobject thiz,
                                                   jint handle)
{
    char name[MAX_CERT_NAME_LEN];
    if (handle == 0) return NULL;
    if (get_cert_name((X509*)handle, name, MAX_CERT_NAME_LEN)) return NULL;
    return (*env)->NewStringUTF(env, name);
}

jstring android_security_CertTool_getPrivateKeyPEM(JNIEnv* env,
                                                   jobject thiz,
                                                   jint handle)
{
    char pem[MAX_PEM_LENGTH];
    if (handle == 0) return NULL;
    if (get_private_key_pem((X509*)handle, pem, MAX_PEM_LENGTH)) return NULL;
    return (*env)->NewStringUTF(env, pem);
}

void android_security_CertTool_freeX509Certificate(JNIEnv* env,
                                                   jobject thiz,
                                                   jint handle)
{
    if (handle != 0) X509_free((X509*)handle);
}

/*
 * Table of methods associated with the CertTool class.
 */
static JNINativeMethod gCertToolMethods[] = {
    /* name, signature, funcPtr */
    {"generateCertificateRequest", "(ILjava/lang/String;)Ljava/lang/String;",
        (void*)android_security_CertTool_generateCertificateRequest},
    {"isPkcs12Keystore", "([B)Z",
        (void*)android_security_CertTool_isPkcs12Keystore},
    {"getPkcs12Handle", "([BLjava/lang/String;)I",
        (void*)android_security_CertTool_getPkcs12Handle},
    {"getPkcs12Certificate", "(I)Ljava/lang/String;",
        (void*)android_security_CertTool_getPkcs12Certificate},
    {"getPkcs12PrivateKey", "(I)Ljava/lang/String;",
        (void*)android_security_CertTool_getPkcs12PrivateKey},
    {"popPkcs12CertificateStack", "(I)Ljava/lang/String;",
        (void*)android_security_CertTool_popPkcs12CertificateStack},
    {"freePkcs12Handle", "(I)V",
        (void*)android_security_CertTool_freePkcs12Handle},
    {"generateX509Certificate", "([B)I",
        (void*)android_security_CertTool_generateX509Certificate},
    {"isCaCertificate", "(I)Z",
        (void*)android_security_CertTool_isCaCertificate},
    {"getIssuerDN", "(I)Ljava/lang/String;",
        (void*)android_security_CertTool_getIssuerDN},
    {"getCertificateDN", "(I)Ljava/lang/String;",
        (void*)android_security_CertTool_getCertificateDN},
    {"getPrivateKeyPEM", "(I)Ljava/lang/String;",
        (void*)android_security_CertTool_getPrivateKeyPEM},
    {"freeX509Certificate", "(I)V",
        (void*)android_security_CertTool_freeX509Certificate},
};

/*
 * Register several native methods for one class.
 */
static int registerNatives(JNIEnv* env, const char* className,
                           JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = (*env)->FindClass(env, className);
    if (clazz == NULL) {
        LOGE("Can not find class %s\n", className);
        return JNI_FALSE;
    }

    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        LOGE("Can not RegisterNatives\n");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;


    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        goto bail;
    }

    if (!registerNatives(env, "android/security/CertTool",
                         gCertToolMethods, nelem(gCertToolMethods))) {
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}
