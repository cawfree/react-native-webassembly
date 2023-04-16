#include <jni.h>
#include <android/log.h>
#include <string>

#include "react-native-webassembly.h"


using namespace facebook::jsi;
using namespace std;

extern "C"
JNIEXPORT void JNICALL
Java_com_webassembly_WebassemblyModule_nativeInstall(JNIEnv *env, jobject thiz, jlong jsi) {

  auto runtime = reinterpret_cast<facebook::jsi::Runtime *>(jsi);

  if (runtime) {
    webassembly::install(*runtime);
  }

}
