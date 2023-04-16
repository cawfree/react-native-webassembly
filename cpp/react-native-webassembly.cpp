#include "react-native-webassembly.h"

#include <jsi/jsi.h>
#include <thread>
#include <map>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <regex>

#include "m3_info.h"
#include "m3_bind.h"
#include "m3_env.h"
#include "wasm3_cpp.h"

using namespace facebook::jsi;
using namespace std;

// https://stackoverflow.com/a/34571089
static std::string base64_decode(const std::string &in) {

    std::string out;

    std::vector<int> T(256,-1);
    for (int i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

    int val=0, valb=-8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val>>valb)&0xFF));
            valb -= 8;
        }
    }
    return out;
}

static void stringToBytes(const char* str, unsigned char* bytes) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i) {
        bytes[i] = static_cast<unsigned char>(str[i]);
    }
}


char* double_to_c_string(double value)
{
  std::stringstream stream;
  stream << std::fixed << std::setprecision(16) << value;
  std::string s = stream.str();
  return s.data();
}

static std::string trim(std::string str) {
  return std::regex_replace(str, std::regex("\\s+"), "");
}

static std::string transform_parentheses_arguments(std::string input_string) {
  std::stringstream ss(input_string);
  std::vector<std::string> contents;

  std::string content;

  while (std::getline(ss, content, ',')) {

    content = trim(content);

    if (content == "i32") {
      contents.push_back("i");
    } else if (content == "i64") {
      contents.push_back("I");
    } else if (content == "f32") {
      contents.push_back("f");
    } else if (content == "f64") {
      contents.push_back("F");
    } else {
      std::cout << "Encountered unexpected argument, " << content << "." << "\n";
      throw std::runtime_error("Encountered unexpected argument.");
    }
  }

  std::string result;

  for (const auto& s : contents)
    result += s;

  return result;

}

static std::string transform_parentheses(IM3Function f)
{
  std::string funcTypeSignature = std::string(SPrintFuncTypeSignature(f->funcType));

  std::regex  re("\\((.*?)\\)");
  std::smatch match;

  std::regex_search(funcTypeSignature, match, re);

  // TODO: A second match may exist for the return type if using multiple args.
  std::string inner = match[1];

  if (inner.length() == 0) return "()";

  return "(" + transform_parentheses_arguments(inner) + ")";
}

static std::string transform_return(IM3Function f)
{
  if (f->funcType->numRets == 0) return "v";

  if (f->funcType->numRets >  1) {
    std::cout << "Encountered unexpected number of return types, " << f->funcType->numRets << "." << "\n";
    throw std::runtime_error("Encountered unexpected number of return types.");
  }

  std::string funcTypeSignature = std::string(SPrintFuncTypeSignature(f->funcType));

  std::string delimiter = "->";
  size_t pos = funcTypeSignature.find(delimiter);

  if (pos == std::string::npos) {
    std::cout << "Unable to find delimiter in: " << funcTypeSignature << "." << "\n";
    throw std::runtime_error("Unable to find delimiter.");
  }

  std::string returnType = trim(funcTypeSignature.substr(pos + delimiter.length()));

  if (returnType == "i32") return "i";
  if (returnType == "i64") return "I";
  if (returnType == "f32") return "f";
  if (returnType == "f64") return "F";

  std::cout << "Encountered unsupported return type \"" << returnType << "\" in " << funcTypeSignature << "." << "\n";
  throw std::runtime_error("Unsupported return type.");
}

static std::string transform_signature(IM3Function f)
{
  return transform_return(f) + transform_parentheses(f);
}

typedef const void *(*t_callback)(struct M3Runtime *, struct M3ImportContext *, unsigned long long *, void *);

std::map<std::string, wasm3::runtime> RUNTIMES;

namespace webassembly {

void install(Runtime &jsiRuntime) {

  auto RNWebassembly_instantiate = Function::createFromHostFunction(
    jsiRuntime,
    PropNameID::forAscii(jsiRuntime, "RNWebassembly_instantiate"),
    0,
    [](Runtime &runtime, const Value &thisValue, const Value *arguments, size_t count) -> Value {
      
      Object params = arguments->getObject(runtime);
        
      String iid = params.getProperty(runtime, "iid").asString(runtime);
      String bufferSource = params.getProperty(runtime, "bufferSource").asString(runtime);
      uint32_t stackSizeInBytes = (uint32_t)params.getProperty(runtime, "stackSizeInBytes").asNumber();
        
      /* Wasm module can also be loaded from an array */
      try {
        wasm3::environment env;
        wasm3::runtime     m3runtime = env.new_runtime(stackSizeInBytes);
        
        std::string decoded = base64_decode(bufferSource.utf8(runtime));
        
        unsigned char* buffer = new unsigned char[decoded.length()];
        memcpy(buffer, decoded.data(), decoded.length());
      
        wasm3::module mod = env.parse_module(buffer, decoded.length());

        m3runtime.load(mod);

        IM3Module io_module = mod.m_module.get();

        for (u32 i = 0; i < io_module->numFunctions; ++i)
        {
          const IM3Function f = & io_module->functions [i];

          const char* moduleName = f->import.moduleUtf8;
          const char* fieldName = f->import.fieldUtf8;

          // TODO: is this valid?
          if (!moduleName || !fieldName) continue;

          std::string signature = transform_signature(f);

//          M3FuncType * funcType = f->funcType;

          t_callback callback = [](
            struct M3Runtime *runtime,
            struct M3ImportContext *context,
            unsigned long long *args,
            void *userData
          ) -> const void * {
            return NULL;
          };

          // TODO: Generate signature.
          // TODO: Remove raw function links.
          m3_LinkRawFunctionEx(io_module, moduleName, fieldName, signature.data(), callback, NULL);
        }

        mod.compile();
          
        RUNTIMES.insert(std::make_pair(iid.utf8(runtime), m3runtime));
      
        return Value(0);
      } catch(wasm3::error &e) {
        std::cerr << e.what() << std::endl;
        return Value(1);
      }
    }
  );

  jsiRuntime.global().setProperty(jsiRuntime, "RNWebassembly_instantiate", move(RNWebassembly_instantiate));
  
  auto RNWebassembly_invoke = Function::createFromHostFunction(
    jsiRuntime,
    PropNameID::forAscii(jsiRuntime, "RNWebassembly_invoke"),
    0,
    [](Runtime &runtime, const Value &thisValue, const Value *arguments, size_t count) -> Value {
      
      Object params = arguments->getObject(runtime);
        
      String iid  = params.getProperty(runtime, "iid").asString(runtime);
      String func = params.getProperty(runtime, "func").asString(runtime);
      Array  args = params.getProperty(runtime, "args").asObject(runtime).asArray(runtime);
        
      std::map<std::string, wasm3::runtime>::iterator it = RUNTIMES.find(iid.utf8(runtime));
      
      if (it == RUNTIMES.end()) throw std::runtime_error("Unable to invoke.");
      
      wasm3::runtime  m3runtime = it->second;
      wasm3::function fn        = m3runtime.find_function(func.utf8(runtime).data());
        
      std::vector<std::string> vec;
      std::vector<std::string> res;
      
      for (uint32_t i = 0; i < fn.GetArgCount(); i += 1)
        vec.push_back(args.getValueAtIndex(runtime, i).asString(runtime).utf8(runtime));
        
      if (fn.GetRetCount() > 1) throw std::runtime_error("Unable to invoke.");
      
      if (fn.GetRetCount() == 0) {
        fn.call_argv<int>(vec);
        return 0;
      }
      
      M3ValueType type = fn.GetRetType(0);
      
      if (type == c_m3Type_i32) {
        res.push_back(std::to_string(fn.call_argv<int32_t>(vec)));
      } else if (type == c_m3Type_i64) {
        res.push_back(std::to_string(fn.call_argv<int64_t>(vec)));
      } else if (type == c_m3Type_f32) {
        res.push_back(std::string(double_to_c_string(fn.call_argv<float>(vec))));
      } else if (type == c_m3Type_f64) {
        res.push_back(std::string(double_to_c_string(fn.call_argv<double>(vec))));
      } else {
        throw std::runtime_error("Failed to invoke.");
      }
        
      auto array = Array(runtime, res.size());
        
      for (uint32_t i = 0; i < fn.GetRetCount(); i += 1)
        array.setValueAtIndex(runtime, i, res[i]);
        
      return array;
    }
  );

  jsiRuntime.global().setProperty(jsiRuntime, "RNWebassembly_invoke", move(RNWebassembly_invoke));

//    auto multiply = Function::createFromHostFunction(jsiRuntime,
//                                                     PropNameID::forAscii(jsiRuntime,
//                                                                          "multiply"),
//                                                     2,
//                                                     [](Runtime &runtime,
//                                                        const Value &thisValue,
//                                                        const Value *arguments,
//                                                        size_t count) -> Value {
//        int x = arguments[0].getNumber();
//        int y = arguments[1].getNumber();
//
//        return Value(x * y);
//
//    });
//
//    jsiRuntime.global().setProperty(jsiRuntime, "multiply", move(multiply));
//
//    auto multiplyWithCallback = Function::createFromHostFunction(jsiRuntime,
//                                                                 PropNameID::forAscii(jsiRuntime,
//                                                                                      "multiplyWithCallback"),
//                                                                 3,
//                                                                 [](Runtime &runtime,
//                                                                    const Value &thisValue,
//                                                                    const Value *arguments,
//                                                                    size_t count) -> Value {
//        int x = arguments[0].getNumber();
//        int y = arguments[1].getNumber();
//
//        arguments[2].getObject(runtime).getFunction(runtime).call(runtime, x * y);
//
//        return Value();
//
//    });
//
//    jsiRuntime.global().setProperty(jsiRuntime, "multiplyWithCallback", move(multiplyWithCallback));
//
//
//    auto foo = Function::createFromHostFunction(
//            jsiRuntime,
//            PropNameID::forAscii(jsiRuntime, "foo"),
//            1,
//            [](Runtime& runtime, const Value& thisValue, const Value* arguments, size_t count) -> Value {
//
//                auto userCallbackRef = std::make_shared<Object>(arguments[0].getObject(runtime));
//
//                auto f = [&runtime](shared_ptr<Object> userCallbackRef) {
//                    auto val = String::createFromUtf8(runtime, "hello world");
//                    auto error = Value::undefined();
//                    userCallbackRef->asFunction(runtime).call(runtime, error, val);
//                };
//
//                std::thread thread_object(f,userCallbackRef);
//                thread_object.detach();
//
//                return Value::undefined();
//            }
//    );
//    jsiRuntime.global().setProperty(jsiRuntime, "foo", std::move(foo));
  }
}

//
//char* double_to_c_string(double value)
//{
//  std::stringstream stream;
//  stream << std::fixed << std::setprecision(16) << value;
//  std::string s = stream.str();
//  return s.data();
//}
//
//std::string trim(std::string str) {
//  return std::regex_replace(str, std::regex("\\s+"), "");
//}
//
//
//namespace webassembly {
//  double instantiate(RNWebassemblyInstantiateParams* a) {
//
//    std::string iid = a->iid;
//    uint32_t stackSizeInBytes = (uint32_t)a->stackSizeInBytes;
//
//    /* Wasm module can also be loaded from an array */
//    try {
//      wasm3::environment env;
//      wasm3::runtime runtime = env.new_runtime(stackSizeInBytes);
//
//      wasm3::module mod = env.parse_module(a->bufferSource, a->bufferSourceLength);
//
//      runtime.load(mod);
//
//      // First iterate the functions and manually determine interface.
//      // Then define the appropriate export.
//
//      IM3Module io_module = mod.m_module.get();
//
//      for (u32 i = 0; i < io_module->numFunctions; ++i)
//      {
//        const IM3Function f = & io_module->functions [i];
//
//        const char* moduleName = f->import.moduleUtf8;
//        const char* fieldName = f->import.fieldUtf8;
//
//        // TODO: is this valid?
//        if (!moduleName || !fieldName) continue;
//
//        std::string signature = transform_signature(f);
//
//        M3FuncType * funcType = f->funcType;
//
//        t_callback callback = [](
//          struct M3Runtime *runtime,
//          struct M3ImportContext *context,
//          unsigned long long *args,
//          void *userData
//        ) -> const void * {
//          return NULL;
//        };
//
//        // TODO: Generate signature.
//        // TODO: Remove raw function links.
//        m3_LinkRawFunctionEx(io_module, moduleName, fieldName, signature.data(), callback, NULL);
//      }
//
////      std::vector<std::string>::value_type *rawFunctions = a->rawFunctions->data();
////      std::vector<std::string>::value_type *rawFunctionScopes = a->rawFunctionScopes->data();
//
//      mod.compile();
//
//      RUNTIMES.insert(std::make_pair(iid, runtime));
//
//      return 0;
//    } catch(wasm3::error &e) {
//      std::cerr << e.what() << std::endl;
//      return 1;
//    }
//  }
//
//  double invoke(RNWebassemblyInvokeParams* a) {
//
//    std::string               iid  = a->iid;
//    std::string               func = a->func;
//    std::vector<std::string>* args = a->args;
//    std::vector<std::string>* res  = a->res;
//
//    std::map<std::string, wasm3::runtime>::iterator it = RUNTIMES.find(iid);
//
//    if (it == RUNTIMES.end())
//      throw std::runtime_error("Unable to invoke.");
//
//    wasm3::runtime  runtime = it->second;
//    wasm3::function fn      = runtime.find_function(func.data());
//
//    std::vector<std::string> vec;
//
//    for (uint32_t i = 0; i < fn.GetArgCount(); i += 1) {
//      vec.push_back((*args)[i]);
//    }
//
//    if (fn.GetRetCount() > 1) throw std::runtime_error("Unable to invoke.");
//
//    if (fn.GetRetCount() == 0) {
//      fn.call_argv<int>(vec);
//      return 0;
//    }
//
//    M3ValueType type = fn.GetRetType(0);
//
//    if (type == c_m3Type_i32) {
//      res->push_back(std::to_string(fn.call_argv<int32_t>(vec)));
//      return 0;
//    } else if (type == c_m3Type_i64) {
//      res->push_back(std::to_string(fn.call_argv<int64_t>(vec)));
//      return 0;
//    } else if (type == c_m3Type_f32) {
//      res->push_back(std::string(double_to_c_string(fn.call_argv<float>(vec))));
//      return 0;
//    } else if (type == c_m3Type_f64) {
//      res->push_back(std::string(double_to_c_string(fn.call_argv<double>(vec))));
//      return 0;
//    }
//
//    throw std::runtime_error("Failed to invoke.");
//  }
//}
//
