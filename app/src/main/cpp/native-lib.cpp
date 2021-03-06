#include <jni.h>
#include <string>
#include "Mp3Decoder.h"

jfieldID FIELD_ID_MP3_DECODER_NATIVEPTR;
jmethodID METHID_ON_DATA;

typedef struct CallBackData{
    JNIEnv *env;
    jobject instance;
    void *data;
}CallBackData;

extern "C"
JNIEXPORT jlong JNICALL
Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_onData(JNIEnv *env, jobject instance,
                                                                 jstring path_) {
}

static void callback(const void *buffer, ssize_t size, ssize_t count,const void *data){
    const CallBackData *pData = (const CallBackData *) data;
    JNIEnv *env = pData->env;
    jbyteArray  jbarray = env->NewByteArray(size*count);
    env->SetByteArrayRegion(jbarray, 0, size*count, (const jbyte *) buffer);
    pData->env->CallVoidMethod(pData->instance,METHID_ON_DATA,jbarray,size,count);
    env->DeleteLocalRef(jbarray);
}


static inline Mp3Decoder*  getNativeMp3Decoder(JNIEnv *env, jobject instance){
    long ptr = env->GetLongField(instance,FIELD_ID_MP3_DECODER_NATIVEPTR);
    return reinterpret_cast<Mp3Decoder*>(ptr);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_nativeInit(JNIEnv *env, jobject instance,
                                                                 jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    Mp3Decoder *mp3Decoder = new Mp3Decoder(path);
    if (mp3Decoder->checkFileOk()) {
        env->ReleaseStringUTFChars(path_, path);
       //抛出io异常
//        jniThrowIOException(env, errno);  // Will throw on return
        return 0;
    }
    env->ReleaseStringUTFChars(path_, path);
    return reinterpret_cast<long>(mp3Decoder);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_nativeRelease(JNIEnv *env, jobject instance) {

}


extern "C"
JNIEXPORT void JNICALL
Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_nativeStart(JNIEnv *env, jobject instance,jboolean useCallBack) {
    Mp3Decoder *mp3Decoder = getNativeMp3Decoder(env,instance);
    if (useCallBack) {
        CallBackData *data = new CallBackData;
        data->data = mp3Decoder;
        data->env = env;
        //TODO delete ref
        data->instance =env->NewWeakGlobalRef(instance);
        mp3Decoder->setPutBufferImpl(callback,data);
    }
    mp3Decoder->start();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_nativeRead(JNIEnv *env, jobject instance,
                                                                 jbyteArray buffer_, jint offset,
                                                                 jint length) {
    jbyte *buffer = env->GetByteArrayElements(buffer_, NULL);

    // TODO

    env->ReleaseByteArrayElements(buffer_, buffer, 0);
    return NULL;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_classInit(JNIEnv *env, jclass type) {
    FIELD_ID_MP3_DECODER_NATIVEPTR = env->GetFieldID(type,"nativePtr","J");
    METHID_ON_DATA = env->GetMethodID(type,"onData","([BII)V");
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_tlinux_mp3playffmpegandaudiotrack_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


extern "C"
JNIEXPORT bool JNICALL
Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_nativePrepare(
        JNIEnv *env,
        jobject /* this */instance) {
    Mp3Decoder* decoder = getNativeMp3Decoder(env,instance);
    if(!decoder->prepare()) {
        return true;
    }
    return false;
}


extern "C"
JNIEXPORT jobject JNICALL
Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_ngetAudioInfo(
        JNIEnv *env,
        jobject /* this */instance) {
    Mp3Decoder* decoder = getNativeMp3Decoder(env,instance);
    jclass caudioinfo = env->FindClass("com/tlinux/mp3playffmpegandaudiotrack/AudioInfo"); //或得Student类引用

    //获得得该类型的构造函数  函数名为 <init> 返回类型必须为 void 即 V
    jmethodID constrocMID = env->GetMethodID(caudioinfo,"<init>","()V");

    jobject ret = env->NewObject(caudioinfo,constrocMID);
    jfieldID fIdRate = env->GetFieldID(caudioinfo,"sampleRate","I");
    jfieldID fIdChannelCount = env->GetFieldID(caudioinfo,"channelCount","I");
    jfieldID fIdFormat = env->GetFieldID(caudioinfo,"encodformat","Ljava/lang/String;");

    env->SetIntField(ret,fIdRate,decoder->getSampleRate());
    env->SetIntField(ret,fIdChannelCount,decoder->getChannelCount());
    env->SetObjectField(ret,fIdFormat,env->NewStringUTF(decoder->getFmt()));

    return ret;

}

//
//static JNINativeMethod method_table[] = {
//        { "nativeInit",   "(Ljava/lang/String)J", (void*)Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_nativeInit },
//        { "nativeRelease", "()V", (void*)Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_nativeRelease },
//        { "nativeStart", "()V", (void*)Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_nativeStart },
//        { "nativeRead", "([BII)I", (void*)Java_com_tlinux_mp3playffmpegandaudiotrack_Mp3Decoder_nativeRead },
//};
//
//int register_android_Mp3Decoder(JNIEnv *env) {
//    return env->RegisterNatives(env->FindClass("com/tlinux/mp3playffmpegandaudiotrack/Mp3Decoder"),
//                                    method_table, (sizeof(method_table)/ sizeof(method_table[0])));
//}
//
//extern "C" jint JNI_OnLoad(JavaVM* vm, void* /* reserved */) {
//    JNIEnv* env;
//    jint result = -1;
//
//    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
//        return result;
//    }
//
//
//    register_android_Mp3Decoder(env);
//}