//===---- omptarget-nvptx.h - NVPTX OpenMP GPU initialization ---- CUDA -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of all library macros, types,
// and functions.
//
//===----------------------------------------------------------------------===//

#ifndef __OMPTARGET_JNI_H
#define __OMPTARGET_JNI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

void __jni_ReleasePrimitiveArrayCritical(JNIEnv *env, jarray array,
                                         void *carray, jint mode);
void *__jni_GetPrimitiveArrayCritical(JNIEnv *env, jarray array,
                                      jboolean *isCopy);
void __jni_ReleaseByteArrayElements(JNIEnv *env, jbyteArray array,
                                    jbyte *carray, jint mode);
void *__jni_GetByteArrayElements(JNIEnv *env, jbyteArray array,
                                 jboolean *isCopy);
jobject __jni_CreateNewTuple(JNIEnv *env, jint size, jbyteArray *arrays);

#ifdef __cplusplus
}
#endif

#endif
