#ifndef MY_APPLICATION_INPUTSTREAMADAPTER_H
#define MY_APPLICATION_INPUTSTREAMADAPTER_H

#include <jni.h>
#include <cassert>
#include <memory>

#include "InputStream.h"

#define RETURN_NULL_IF_NULL(value) \
    do { if (!(value)) { assert(0); return NULL; } } while (false)

static jmethodID    gInputStream_resetMethodID;
static jmethodID    gInputStream_markMethodID;
static jmethodID    gInputStream_markSupportedMethodID;
static jmethodID    gInputStream_readMethodID;
static jmethodID    gInputStream_skipMethodID;

class InputStreamAdaptor : public InputStream {
public:
    InputStreamAdaptor(JNIEnv* env, jobject js)
            : fEnv(env), fJavaInputStream(js) {

        fJavaByteArray= env->NewByteArray(2000);
        fCapacity = env->GetArrayLength(fJavaByteArray);
        assert(fCapacity > 0);
        fBytesRead = 0;
        fIsAtEnd = false;
    }

    virtual std::streamsize read(char* buffer, std::streamsize size) override {
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

    virtual bool isBufferBased() const override
    {
        return false;
    }

    virtual bool isSeekable() const override
    {
        return false;
    }

    virtual std::streamoff seek(std::streamoff offset) override
    {
        return 0;
    }

private:
    // Does not override rewind, since a InputStreamAdaptor's interface
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
};

class InputStreamAdaptorFactory
{
public:
    static InputStreamPtr CreateJavaInputStreamAdaptor(JNIEnv* env, jobject stream);
};

#endif //MY_APPLICATION_INPUTSTREAMADAPTER_H
