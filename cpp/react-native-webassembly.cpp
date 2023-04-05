#include "react-native-webassembly.h"

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

char* double_to_c_string(double value)
{
  std::stringstream stream;
  stream << std::fixed << std::setprecision(16) << value;
  std::string s = stream.str();
  return s.data();
}

std::string trim(std::string str) {
  return std::regex_replace(str, std::regex("\\s+"), "");
}

std::string transform_parentheses_arguments(std::string input_string) {
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

std::string transform_parentheses(IM3Function f)
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

std::string transform_return(IM3Function f)
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

std::string transform_signature(IM3Function f)
{
  return transform_return(f) + transform_parentheses(f);
}

typedef const void *(*t_callback)(struct M3Runtime *, struct M3ImportContext *, unsigned long long *, void *);

std::map<std::string, wasm3::runtime> RUNTIMES;

namespace webassembly {
  double instantiate(RNWebassemblyInstantiateParams* a) {

    std::string iid = a->iid;
    uint32_t stackSizeInBytes = (uint32_t)a->stackSizeInBytes;

    /* Wasm module can also be loaded from an array */
    try {
      wasm3::environment env;
      wasm3::runtime runtime = env.new_runtime(stackSizeInBytes);

      wasm3::module mod = env.parse_module(a->bufferSource, a->bufferSourceLength);

      runtime.load(mod);
        
      // First iterate the functions and manually determine interface.
      // Then define the appropriate export.
        
      IM3Module io_module = mod.m_module.get();
      
      for (u32 i = 0; i < io_module->numFunctions; ++i)
      {
        const IM3Function f = & io_module->functions [i];
          
        const char* moduleName = f->import.moduleUtf8;
        const char* fieldName = f->import.fieldUtf8;
          
        // TODO: is this valid?
        if (!moduleName || !fieldName) continue;
        
        std::cout << "Linking " << moduleName << ":" << fieldName << "!" << "\n";
          
        std::string signature = transform_signature(f);
          
        M3FuncType * funcType = f->funcType;
          
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

//      std::vector<std::string>::value_type *rawFunctions = a->rawFunctions->data();
//      std::vector<std::string>::value_type *rawFunctionScopes = a->rawFunctionScopes->data();

      mod.compile();

      RUNTIMES.insert(std::make_pair(iid, runtime));

      return 0;
    } catch(wasm3::error &e) {
      std::cerr << e.what() << std::endl;
      return 1;
    }
  }

  double invoke(RNWebassemblyInvokeParams* a) {

    std::string               iid  = a->iid;
    std::string               func = a->func;
    std::vector<std::string>* args = a->args;
    std::vector<std::string>* res  = a->res;

    std::map<std::string, wasm3::runtime>::iterator it = RUNTIMES.find(iid);

    if (it == RUNTIMES.end())
      throw std::runtime_error("Unable to invoke.");

    wasm3::runtime  runtime = it->second;
    wasm3::function fn      = runtime.find_function(func.data());

    std::vector<std::string> vec;

    for (uint32_t i = 0; i < fn.GetArgCount(); i += 1) {
      vec.push_back((*args)[i]);
    }

    if (fn.GetRetCount() > 1) throw std::runtime_error("Unable to invoke.");

    if (fn.GetRetCount() == 0) {
      fn.call_argv<int>(vec);
      return 0;
    }

    M3ValueType type = fn.GetRetType(0);

    if (type == c_m3Type_i32) {
      res->push_back(std::to_string(fn.call_argv<int32_t>(vec)));
      return 0;
    } else if (type == c_m3Type_i64) {
      res->push_back(std::to_string(fn.call_argv<int64_t>(vec)));
      return 0;
    } else if (type == c_m3Type_f32) {
      res->push_back(std::string(double_to_c_string(fn.call_argv<float>(vec))));
      return 0;
    } else if (type == c_m3Type_f64) {
      res->push_back(std::string(double_to_c_string(fn.call_argv<double>(vec))));
      return 0;
    }

    throw std::runtime_error("Failed to invoke.");
  }
}
