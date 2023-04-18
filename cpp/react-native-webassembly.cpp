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
#include <future>

#include "m3_bind.h"
#include "m3_env.h"
#include "wasm3_cpp.h"

using namespace facebook::jsi;
using namespace std;

// TODO: Copied from m3_info, use alternative.
cstr_t  GetTypeName  (u8 i_m3Type)
{
    if (i_m3Type < 5)
        return c_waTypes [i_m3Type];
    else
        return "?";
}

// TODO: Copied from m3_info, use alternative.
cstr_t  SPrintFuncTypeSignature  (IM3FuncType i_funcType)
{
    static char string [256];

    sprintf (string, "(");

    for (u32 i = 0; i < i_funcType->numArgs; ++i)
    {
        if (i != 0)
            strcat (string, ", ");

        strcat (string, GetTypeName (d_FuncArgType(i_funcType, i)));
    }

    strcat (string, ") -> ");

    for (u32 i = 0; i < i_funcType->numRets; ++i)
    {
        if (i != 0)
            strcat (string, ", ");

        strcat (string, GetTypeName (d_FuncRetType(i_funcType, i)));
    }

    return string;
}

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

struct UserData {
  std::shared_ptr<facebook::jsi::Function> fn;
  facebook::jsi::Runtime*                  rt;
};

std::map<std::string, wasm3::runtime> RUNTIMES;
std::map<std::string, UserData>       USER_DATAS;

// TODO: Fix potential for collisions.
std::string ConvertM3FunctionToIdentifier(IM3Function f) {
  std::string signature = transform_signature(f);
  return std::string(f->import.moduleUtf8) + std::string(f->import.fieldUtf8) + signature;
}

facebook::jsi::Array ConvertStringArrayToJSIArray(
  Runtime &runtime,
  std::string* strings,
  size_t num_strings
) {
  facebook::jsi::Array jsi_array(runtime, num_strings);
    
  for (size_t i = 0; i < num_strings; i += 1)
    jsi_array.setValueAtIndex(runtime, i, facebook::jsi::String::createFromUtf8(runtime, strings[i]));
  
  return jsi_array;
}

m3ApiRawFunction(_doSomethingWithFunction)
{
    
    uint8_t length = m3_GetArgCount(_ctx->function);
    
    std::map<std::string, UserData>::iterator it = USER_DATAS.find(ConvertM3FunctionToIdentifier(_ctx->function).data());

    if (it == USER_DATAS.end()) throw std::runtime_error("Unable to invoke.");

    UserData context = it->second;

    facebook::jsi::Function& originalFunction = *context.fn.get();
    
    // The order here matters: m3ApiReturnType should go before calling get_args_from_stack,
    // since both modify `_sp`, and the return value on the stack is reserved before the arguments.
    m3ApiReturnType(int32_t)
    
    // Let's convert these into strings for JS.
    std::string* result = new std::string[length];
    
    for (uint32_t i = 0; i < length ; i += 1) {
      M3ValueType type = m3_GetArgType(_ctx->function, i);
        
      if (type == c_m3Type_i32) {
        m3ApiGetArg (int32_t, param)
        result[i] = std::to_string(param);
      }
    }
    
    std::string functionName = m3_GetFunctionName(_ctx->function);
    
    Object resultDict(*context.rt);
    
    resultDict.setProperty(*context.rt, "module", facebook::jsi::String::createFromUtf8(*context.rt, std::string(_ctx->function->import.moduleUtf8)));
    resultDict.setProperty(*context.rt,   "func", facebook::jsi::String::createFromUtf8(*context.rt, functionName));
    resultDict.setProperty(*context.rt,   "args", ConvertStringArrayToJSIArray(*context.rt, result, length));
    
    Value callResult = originalFunction.call(*context.rt, resultDict);
    
//    iprintf(str);
    m3ApiSuccess();
}

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

      Function callback = params.getProperty(runtime, "callback").asObject(runtime).asFunction(runtime);
        
      std::shared_ptr<facebook::jsi::Function> fn = std::make_shared<facebook::jsi::Function>(std::move(callback));

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
          const IM3Function f = & io_module->functions[i];

          const char* moduleName = f->import.moduleUtf8;
          const char* fieldName = f->import.fieldUtf8;

          // TODO: is this valid?
          if (!moduleName || !fieldName) continue;

          // TODO: remove this in favour of wasm3::detail::m3_signature
          // TODO: look at m3_type_to_sig
          std::string signature = transform_signature(f);
            
          UserData userData;
          userData.rt = &runtime;
          userData.fn = fn;
            
          std::string functionId = ConvertM3FunctionToIdentifier(f);
           
          // TODO: Make function identifiers unique w/ uid.
          // HACK: Allow fast refresh to purge old callbacks.
          USER_DATAS.erase(functionId);
            
          USER_DATAS.insert(std::make_pair(functionId.data(), userData));
            
          // TODO: The callback implementation is erroneous?
          // TODO: Generate signature.
          // TODO: Remove raw function links.
          m3_LinkRawFunction(io_module, moduleName, fieldName, signature.data(), &_doSomethingWithFunction);
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
    });

    jsiRuntime.global().setProperty(jsiRuntime, "RNWebassembly_invoke", move(RNWebassembly_invoke));
  }
}
