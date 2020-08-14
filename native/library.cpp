#include "library.h"

#include <iostream>
#include "io_github_karlatemp_openagent_OpenAgent.h"
#include "load_ins.h"
#include <jvmti.h>
#include <cstring>

//static jmethodID transformMethod = nullptr;
//static jclass transformMethodOwner = nullptr;
//static jobject transformAgent = nullptr;
static JavaVM *JAVA_VM = nullptr;
static jvmtiEnv *JVM_TI = nullptr;

void initialize(JavaVM *vm, jstring, JNIEnv *env);
//
//void JNICALL eventHandlerClassFileLoadHook(
//        jvmtiEnv *jvmtienv,
//        JNIEnv *jnienv,
//        jclass class_being_redefined,
//        jobject loader,
//        const char *name,
//        jobject protectionDomain,
//        jint class_data_len,
//        const unsigned char *class_data,
//        jint *new_class_data_len,
//        unsigned char **new_class_data
//);

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JAVA_VM = vm;
    return JNI_VERSION_1_8;
}

JNIEXPORT void JNICALL Java_io_github_karlatemp_openagent_OpenAgent_initialize
        (JNIEnv *env, jclass class_, jstring path) {
// region
//    transformMethodOwner = class_;
//    // transform(Ljava/lang/ClassLoader;Ljava/lang/String;Ljava/lang/Class;Ljava/security/ProtectionDomain;[B)[B
//    transformMethod = env->GetStaticMethodID(
//            class_, "transform0",
//            "(Ljava/lang/ClassLoader;Ljava/lang/String;Ljava/lang/Class;Ljava/security/ProtectionDomain;[B)[B");
//    transformAgent = env->GetStaticObjectField(class_, env->GetStaticFieldID(
//            class_, "INSTANCE",
//            "Lio/github/karlatemp/openagent/OpenAgent;"
//    ));
// std::cout << "Agent: " << transformAgent << std::endl;
// endregion
    initialize(JAVA_VM, path, env);
}

JNIEXPORT jlong JNICALL Java_io_github_karlatemp_openagent_OpenAgent_getObjectSize
        (JNIEnv *, jobject, jobject obj) {
    jlong size;
    JVM_TI->GetObjectSize(obj, &size);
    return size;
}

JNIEXPORT jobjectArray JNICALL Java_io_github_karlatemp_openagent_OpenAgent_getAllLoadedClasses
        (JNIEnv *env, jobject) {
    jint class_count;
    jclass *classes = nullptr;
    JVM_TI->GetLoadedClasses(&class_count, &classes);
    jclass class_class = env->FindClass("java/lang/Class");
    auto localArray = env->NewObjectArray(class_count, class_class, nullptr);
    /* now copy refs to all the classes and put them into the array */
    for (auto classIndex = 0; classIndex < class_count; classIndex++) {
        /* put class into array */
        env->SetObjectArrayElement(localArray, classIndex, classes[classIndex]);
    }
    JVM_TI->Deallocate((unsigned char *) classes);
    return localArray;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
}

void initialize(JavaVM *vm, jstring path, JNIEnv *env) {
    jvmtiEnv *retransformerEnv = nullptr;
    jint jnierror = vm->GetEnv((void **) &retransformerEnv,
                               JVMTI_VERSION_1_1);
    if (jnierror != JNI_OK) {
        return;
    }
    auto c_path = env->GetStringUTFChars(path, nullptr);
    startIns(vm, c_path, const_cast<char *>("cache/OpenAgent.jar"));
//    free((void *) c_path);
//    dlopen()
//    jvmtiEventCallbacks callbacks;
//    memset(&callbacks, 0, sizeof(callbacks));
//    callbacks.ClassFileLoadHook = &eventHandlerClassFileLoadHook;
//    retransformerEnv->SetEventCallbacks(&callbacks, sizeof(callbacks));
//
//    /* turn on ClassFileLoadHook */
//    retransformerEnv->SetEventNotificationMode(
//            JVMTI_ENABLE,
//            JVMTI_EVENT_CLASS_FILE_LOAD_HOOK,
//            nullptr /* all threads */);
    JVM_TI = retransformerEnv;

}


/*

#define JPLIS_CURRENTLY_INSIDE_TOKEN                ((void *) 0x7EFFC0BB)
#define JPLIS_CURRENTLY_OUTSIDE_TOKEN               ((void *) 0)


jvmtiError
confirmingTLSSet(jvmtiEnv *jvmtienv,
                 jthread thread,
                 const void *newValue) {
    jvmtiError error;

    error = jvmtienv->SetThreadLocalStorage(
            thread,
            newValue);
    return error;
}

jboolean tryToAcquireReentrancyToken(jvmtiEnv *jvmtienv,
                                     jthread thread) {
    jboolean result = JNI_FALSE;
    jvmtiError error = JVMTI_ERROR_NONE;
    void *storedValue = nullptr;

    error = jvmtienv->GetThreadLocalStorage(
            thread,
            &storedValue);
//    check_phase_ret_false(error);
//    jplis_assert(error == JVMTI_ERROR_NONE);
    if (error == JVMTI_ERROR_NONE) {
        // if this thread is already inside, just return false and short-circuit
        if (storedValue == JPLIS_CURRENTLY_INSIDE_TOKEN) {
            result = JNI_FALSE;
        } else {
            error = confirmingTLSSet(jvmtienv,
                                     thread,
                                     JPLIS_CURRENTLY_INSIDE_TOKEN);
            if (error != JVMTI_ERROR_NONE) {
                result = JNI_FALSE;
            } else {
                result = JNI_TRUE;
            }
        }
    }
    return result;
}


void releaseReentrancyToken(jvmtiEnv *jvmtienv,
                            jthread thread) {
    jvmtiError error = JVMTI_ERROR_NONE;

    confirmingTLSSet(jvmtienv,
                     thread,
                     JPLIS_CURRENTLY_OUTSIDE_TOKEN);

}
*/
/*
#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-use-auto"

void JNICALL eventHandlerClassFileLoadHook(
        jvmtiEnv *jvmtienv,
        JNIEnv *jnienv,
        jclass class_being_redefined,
        jobject loader,
        const char *name,
        jobject protectionDomain,
        jint class_data_len,
        const unsigned char *class_data,
        jint *new_class_data_len,
        unsigned char **new_class_data
) {
//    if (!tryToAcquireReentrancyToken(jvmtienv, nullptr)) {
//        return;
//    }
    auto jvmName = jnienv->NewStringUTF(name);
    auto jvmBytecode = jnienv->NewByteArray(class_data_len);
//    std::cout << "S 1" << std::endl;

    jbyte *typedBuffer = (jbyte *) class_data; // nasty cast, dumb JNI interface, const missing
    // The sign cast is safe. The const cast is dumb.
    jnienv->SetByteArrayRegion(jvmBytecode, 0, class_data_len, typedBuffer);
//    std::cout << "S 2" << std::endl;

    // (Ljava/lang/ClassLoader;Ljava/lang/String;Ljava/lang/Class;Ljava/security/ProtectionDomain;[B)[B
    jbyteArray transformed = nullptr;
    try {
        transformed = (jbyteArray) jnienv->CallStaticObjectMethod(
                transformMethodOwner, transformMethod,
                loader, jvmName, class_being_redefined,
                protectionDomain, jvmBytecode
        );
    } catch (jobject je) {
        if (je != nullptr)
            throw je;
    } catch (...) {
        jnienv->ExceptionClear();
        return;
    }
    // std::cout << "S 3" << std::endl;

    if (transformed != nullptr) {
        // std::cout << "S 4" << std::endl;

        auto size = jnienv->GetArrayLength(transformed);
        unsigned char *result = nullptr;
        // std::cout << "S 5" << std::endl;

        jvmtienv->Allocate(size, &result);
        // std::cout << "S 6" << std::endl;

        jnienv->GetByteArrayRegion(transformed, 0, size, (jbyte *) result);
        // std::cout << "S 7" << std::endl;

        *new_class_data_len = size;
        *new_class_data = result;
    }
//    releaseReentrancyToken(jvmtienv, nullptr);
}
#pragma clang diagnostic pop
*/


