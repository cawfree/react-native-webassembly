#include <jni.h>
#include <android/log.h>
#include <string>

#include "react-native-webassembly.h"

extern "C" JNIEXPORT jdouble JNICALL
Java_com_webassembly_WebassemblyModule_nativeInstantiate(
  JNIEnv *env,
  jclass type,
  jstring iid,
  jbyteArray bufferSource,
  jlong bufferSourceLength,
  jdouble stackSizeInBytes,
  jobjectArray rawFunctions,
  jobjectArray rawFunctionScopes
) {

  std::string iid_str = std::string(env->GetStringUTFChars(iid, NULL));

  uint8_t* uint8Buffer = new uint8_t[bufferSourceLength];
  memcpy(uint8Buffer, env->GetByteArrayElements(bufferSource, NULL), bufferSourceLength);

  std::vector<std::string> rawFunctionsVec;

  for (jsize i = 0; i < env->GetArrayLength(rawFunctions); i++) {
    jstring str = (jstring) env->GetObjectArrayElement(rawFunctions, i);
    const char* rawString = env->GetStringUTFChars(str, 0);
    rawFunctionsVec.push_back(std::string(rawString));
  }

  std::vector<std::string> rawFunctionScopesVec;

  for (jsize i = 0; i < env->GetArrayLength(rawFunctionScopes); i++) {
    jstring str = (jstring) env->GetObjectArrayElement(rawFunctionScopes, i);
    const char* rawString = env->GetStringUTFChars(str, 0);
    rawFunctionScopesVec.push_back(std::string(rawString));
  }

  RNWebassemblyInstantiateParams params = {
    iid_str,
    uint8Buffer,
    static_cast<size_t>(bufferSourceLength),
    static_cast<double>(stackSizeInBytes),
    &rawFunctionsVec,
    &rawFunctionScopesVec,
  };

  return webassembly::instantiate(&params);
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_webassembly_WebassemblyModule_nativeInvoke(
  JNIEnv *env,
  jclass cls,
  jstring iid,
  jstring func,
  jobjectArray args,
  jobject result
) {

  std::string iid_str = std::string(env->GetStringUTFChars(iid, NULL));
  std::string func_str = std::string(env->GetStringUTFChars(func, NULL));

  std::vector<std::string> argsVec;

  for (jsize i = 0; i < env->GetArrayLength(args); i++) {
      jstring str = (jstring) env->GetObjectArrayElement(args, i);
      const char* rawString = env->GetStringUTFChars(str, 0);
      argsVec.push_back(std::string(rawString));
  }

  std::vector<std::string> resultVec;

  RNWebassemblyInvokeParams params = {
    iid_str,
    func_str,
    &argsVec,
    &resultVec
  };

  double xxx = webassembly::invoke(&params);

  jclass arrayListClass = env->FindClass("java/util/ArrayList");
  jmethodID arrayListAddMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");

  for (const auto& elem : resultVec) {
    jstring jElem = env->NewStringUTF(elem.c_str());
    env->CallBooleanMethod(result, arrayListAddMethod, jElem);
    env->DeleteLocalRef(jElem);
  }

  return xxx;
}
