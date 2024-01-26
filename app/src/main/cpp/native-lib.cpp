// inspired by https://android.googlesource.com/platform/frameworks/base/+/c779752/core/jni/android/graphics/CreateJavaOutputStreamAdaptor.cpp

#include <jni.h>
#include <string>
#include <cassert>
#include <array>

#include "InputStreamAdapter.h"

extern "C" JNIEXPORT jstring

JNICALL
Java_com_example_myapplication_FileReader_read(JNIEnv *env, jobject, jobject input_stream, jbyteArray mem)
{
    std::string hello;
    hello.resize(1024);

    auto inputStreamAdapter = CreateJavaInputStreamAdaptor(env, input_stream, mem);

    inputStreamAdapter->read(hello.data(),hello.size());

    return env->NewStringUTF(hello.c_str());
}
