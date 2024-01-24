// inspired by https://android.googlesource.com/platform/frameworks/base/+/c779752/core/jni/android/graphics/CreateJavaOutputStreamAdaptor.cpp

#include <jni.h>
#include <string>
#include <cassert>
#include <array>

#define RETURN_NULL_IF_NULL(value) \
    do { if (!(value)) { assert(0); return NULL; } } while (false)
#define RETURN_ZERO_IF_NULL(value) \
    do { if (!(value)) { assert(0); return 0; } } while (false)

static jmethodID    gInputStream_resetMethodID;
static jmethodID    gInputStream_markMethodID;
static jmethodID    gInputStream_markSupportedMethodID;
static jmethodID    gInputStream_readMethodID;
static jmethodID    gInputStream_skipMethodID;

class JavaInputStreamAdaptor {
public:
    JavaInputStreamAdaptor(JNIEnv* env, jobject js)
            : fEnv(env), fJavaInputStream(js) {
        auto len = 1024;
        fJavaByteArray = env->NewByteArray( len);
        fCapacity = env->GetArrayLength(fJavaByteArray);
        assert(fCapacity > 0);
        fBytesRead = 0;
        fIsAtEnd = false;
    }
    virtual size_t read(void* buffer, size_t size) {
        JNIEnv* env = fEnv;
        if (NULL == buffer) {
            if (0 == size) {
                return 0;
            } else {
                /*  InputStream.skip(n) can return <=0 but still not be at EOF
                    If we see that value, we need to call read(), which will
                    block if waiting for more data, or return -1 at EOF
                 */
                size_t amountSkipped = 0;
                do {
                    size_t amount = this->doSkip(size - amountSkipped);
                    if (0 == amount) {
                        char tmp;
                        amount = this->doRead(&tmp, 1);
                        if (0 == amount) {
                            // if read returned 0, we're at EOF
                            fIsAtEnd = true;
                            break;
                        }
                    }
                    amountSkipped += amount;
                } while (amountSkipped < size);
                return amountSkipped;
            }
        }
        return this->doRead(buffer, size);
    }
    virtual bool isAtEnd() const {
        return fIsAtEnd;
    }
private:
    // Does not override rewind, since a JavaInputStreamAdaptor's interface
    // does not support rewinding. RewindableJavaStream, which is a friend,
    // will be able to call this method to rewind.
    bool doRewind() {
        JNIEnv* env = fEnv;
        fBytesRead = 0;
        fIsAtEnd = false;
        env->CallVoidMethod(fJavaInputStream, gInputStream_resetMethodID);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return false;
        }
        return true;
    }
    size_t doRead(void* buffer, size_t size) {
        JNIEnv* env = fEnv;
        size_t bytesRead = 0;
        // read the bytes
        do {
            size_t requested = size;
            if (requested > fCapacity)
                requested = fCapacity;
            jint n = env->CallIntMethod(fJavaInputStream,
                                        gInputStream_readMethodID, fJavaByteArray, 0, requested);
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                return 0;
            }
            if (n < 0) { // n == 0 should not be possible, see InputStream read() specifications.
                fIsAtEnd = true;
                break;  // eof
            }
            env->GetByteArrayRegion(fJavaByteArray, 0, n,
                                    reinterpret_cast<jbyte*>(buffer));
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                return 0;
            }
            buffer = (void*)((char*)buffer + n);
            bytesRead += n;
            size -= n;
            fBytesRead += n;
        } while (size != 0);
        return bytesRead;
    }
    size_t doSkip(size_t size) {
        JNIEnv* env = fEnv;
        jlong skipped = env->CallLongMethod(fJavaInputStream,
                                            gInputStream_skipMethodID, (jlong)size);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return 0;
        }
        if (skipped < 0) {
            skipped = 0;
        }
        return (size_t)skipped;
    }
    JNIEnv*     fEnv;
    jobject     fJavaInputStream;   // the caller owns this object
    jbyteArray  fJavaByteArray;     // the caller owns this object
    size_t      fCapacity;
    size_t      fBytesRead;
    bool        fIsAtEnd;
    // Allows access to doRewind and fBytesRead.
    friend class RewindableJavaStream;
};

std::shared_ptr<JavaInputStreamAdaptor> CreateJavaInputStreamAdaptor(JNIEnv* env, jobject stream) {
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
    return std::make_shared<JavaInputStreamAdaptor>(env, stream);
}

extern "C" JNIEXPORT jstring

JNICALL
Java_com_example_myapplication_InputStreamNative_read(JNIEnv *env, jobject, jobject input_stream)
{
    std::string hello;
    hello.resize(1024);

    auto inputStreamAdapter = CreateJavaInputStreamAdaptor(env, input_stream);

    inputStreamAdapter->read(hello.data(),hello.size());

    return env->NewStringUTF(hello.c_str());
}
