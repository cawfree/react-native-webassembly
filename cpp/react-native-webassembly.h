#ifndef WEBASSEMBLY_H
#define WEBASSEMBLY_H

namespace facebook {
  namespace jsi {
    class Runtime;
  }
}

namespace webassembly {
  void install(facebook::jsi::Runtime &jsiRuntime);
}

#endif /* WEBASSEMBLY_H */
