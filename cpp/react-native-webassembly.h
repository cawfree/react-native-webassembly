#ifndef WEBASSEMBLY_H
#define WEBASSEMBLY_H

#include <string>
#include <optional>
#include <vector>

struct RNWebassemblyInstantiateParams {
  std::string iid;
  uint8_t*    bufferSource;
  size_t      bufferSourceLength;
  double      stackSizeInBytes;
    
  std::vector<std::string>* rawFunctions;
};

struct RNWebassemblyInvokeParams {
  std::string               iid;
  std::string               func;
  std::vector<std::string>* args;
  std::vector<std::string>* res;
};

namespace webassembly {
  double instantiate(RNWebassemblyInstantiateParams* a);
  double invoke(RNWebassemblyInvokeParams* a);
}

#endif /* WEBASSEMBLY_H */
