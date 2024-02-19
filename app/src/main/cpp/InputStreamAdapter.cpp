#include "InputStreamAdapter.h"

InputStreamPtr InputStreamAdaptorFactory::CreateJavaInputStreamAdaptor(JNIEnv* env, jobject stream) {
    static bool gInited;
    if (!gInited) {
        jclass inputStream_Clazz = env->FindClass("java/io/InputStream");
        RETURN_NULL_IF_NULL(inputStream_Clazz);
        gInputStream_resetMethodID      = env->GetMethodID(inputStream_Clazz,
                                                           "reset", "()V");
        gInputStream_markMethodID       = env->GetMethodID(inputStream_Clazz,
                                                           "mark", "(I)V");
        gInputStream_markSupportedMethodID = env->GetMethodID(inputStream_Clazz,
                                                              "markSupported", "()Z");
        gInputStream_readMethodID       = env->GetMethodID(inputStream_Clazz,
                                                           "read", "([BII)I");
        gInputStream_skipMethodID       = env->GetMethodID(inputStream_Clazz,
                                                           "skip", "(J)J");
        RETURN_NULL_IF_NULL(gInputStream_resetMethodID);
        RETURN_NULL_IF_NULL(gInputStream_markMethodID);
        RETURN_NULL_IF_NULL(gInputStream_markSupportedMethodID);
        RETURN_NULL_IF_NULL(gInputStream_readMethodID);
        RETURN_NULL_IF_NULL(gInputStream_skipMethodID);
        gInited = true;
    }
    return std::make_shared<InputStreamAdaptor>(env, stream);
}
