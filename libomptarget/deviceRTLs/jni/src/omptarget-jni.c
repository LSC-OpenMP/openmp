//===--- omptarget-nvptx.cu - NVPTX OpenMP GPU initialization ---- CUDA -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the wrapper for JNI functions
//
//===----------------------------------------------------------------------===//

#include "omptarget-jni.h"

#include <stdio.h>
#include <string.h>

void __jni_ReleaseByteArrayElements(JNIEnv *env, jbyteArray array,
                                    jbyte *carray, jint mode) {
  (*env)->ReleaseByteArrayElements(env, array, carray, mode);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(
        stderr,
        "omptarget-jni: Failed to call __jni_ReleasePrimitiveArrayCritical!\n");
    return;
  }
  // fprintf(stderr, "Run __jni_ReleasePrimitiveArrayCritical\n");
}

void *__jni_GetByteArrayElements(JNIEnv *env, jbyteArray array,
                                 jboolean *isCopy) {
  void *carray = (*env)->GetByteArrayElements(env, array, isCopy);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr,
            "omptarget-jni: Failed to call __jni_GetPrimitiveArrayCritical!\n");
    return 0;
  }
  // fprintf(stderr, "Run __jni_GetPrimitiveArrayCritical\n");
  return carray;
}

void __jni_ReleasePrimitiveArrayCritical(JNIEnv *env, jarray array,
                                         void *carray, jint mode) {
  (*env)->ReleasePrimitiveArrayCritical(env, array, carray, mode);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(
        stderr,
        "omptarget-jni: Failed to call __jni_ReleasePrimitiveArrayCritical!\n");
    return;
  }
  // fprintf(stderr, "Run __jni_ReleasePrimitiveArrayCritical\n");
}

void *__jni_GetPrimitiveArrayCritical(JNIEnv *env, jarray array,
                                      jboolean *isCopy) {
  void *carray = (*env)->GetPrimitiveArrayCritical(env, array, isCopy);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
    fprintf(stderr,
            "omptarget-jni: Failed to call __jni_GetPrimitiveArrayCritical!\n");
    return 0;
  }
  // fprintf(stderr, "Run __jni_GetPrimitiveArrayCritical\n");
  return carray;
}

jobject __jni_CreateNewTuple(JNIEnv *env, jint size, jbyteArray *arrays) {

  if (size > 22) {
    fprintf(stderr, "omptarget-jni: Failed to call __jni_CreateNewTuple: the "
                    "size is too big!\n");
    return 0;
  }

  if (size <= 0) {
    fprintf(stderr, "omptarget-jni: Failed to call __jni_CreateNewTuple: the "
                    "size is wrong!\n");
    return 0;
  }

  jclass cls;
  jobject obj;
  jmethodID mid;

  char className[16];
  snprintf(className, sizeof(className), "scala/Tuple%d", size);

  cls = (*env)->FindClass(env, className);
  if (cls == 0) {
    fprintf(stderr, "JNI Class lookup failed: %s\n", className);
    return 0;
  }

  char argTypeList[512] = "(";
  for (int i = 0; i < size; i++)
    strncat(argTypeList, "Ljava/lang/Object;", 19);
  strncat(argTypeList, ")V", 3);

  mid = (*env)->GetMethodID(env, cls, "<init>", argTypeList);
  if (mid == 0) {
    fprintf(stderr, "JNI Method lookup failed: <init> %s \n", argTypeList);
    return 0;
  }

  // cast from jobject* to jvalue*
  jvalue newArrays[22];
  for (int i = 0; i < size; i++)
    newArrays[i].l = arrays[i];

  obj = (*env)->NewObjectA(env, cls, mid, newArrays);

  return obj;
}
