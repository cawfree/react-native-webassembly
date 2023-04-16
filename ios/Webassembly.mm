#import <React/RCTUtils.h>
#import <React/RCTBridge+Private.h>
#import <jsi/jsi.h>
#import <sys/utsname.h>

#import "YeetJSIUtils.h"

#import "Webassembly.h"
#import "react-native-webassembly.h"

using namespace facebook::jsi;
using namespace std;

@implementation Webassembly

@synthesize bridge = _bridge;
@synthesize methodQueue = _methodQueue;

RCT_EXPORT_MODULE()

+ (BOOL)requiresMainQueueSetup {
  return YES;
}

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install)
{
  RCTBridge* bridge = [RCTBridge currentBridge];
  RCTCxxBridge* cxxBridge = (RCTCxxBridge*)bridge;
  
  if (cxxBridge == nil) return @false;

  auto jsiRuntime = (jsi::Runtime*) cxxBridge.runtime;
  
  if (jsiRuntime == nil) return @false;

  webassembly::install(*(facebook::jsi::Runtime *)jsiRuntime);

  return @true;
}

@end

//#import "Webassembly.h"
//
//@implementation Webassembly
//RCT_EXPORT_MODULE()
//
//- (NSNumber *)instantiate:(JS::NativeWebassembly::InstantiateParams &)a {
//
//    NSData  *data         = [[NSData alloc] initWithBase64EncodedString:a.bufferSource() options:0];
//    uint8_t *bufferSource = (uint8_t *)[data bytes];
//
//    std::vector<std::string> rawFunctions;
//
//    for (const auto& rawFunction : a.rawFunctions())
//      rawFunctions.push_back([rawFunction UTF8String]);
//
//    std::vector<std::string> rawFunctionScopes;
//
//    for (const auto& rawFunctionScope : a.rawFunctionScopes())
//      rawFunctionScopes.push_back([rawFunctionScope UTF8String]);
//
//    return @(webassembly::instantiate(new RNWebassemblyInstantiateParams{[a.iid() UTF8String], bufferSource, (size_t)data.length, a.stackSizeInBytes(), &rawFunctions, &rawFunctionScopes}));
//}
//
//- (NSArray<NSString *> *)invoke:(JS::NativeWebassembly::InvokeParams &)a {
//
//    std::vector<std::string> args;
//    args.reserve(a.args().size());
//
//    for (NSString *str : a.args()) {
//      args.emplace_back([str UTF8String]);
//    }
//
//    std::vector<std::string> res;
//
//    NSNumber* result = @(webassembly::invoke(new RNWebassemblyInvokeParams{[a.iid() UTF8String], [a.func() UTF8String], &args, &res}));
//
//    if (result.intValue != 0) throw std::runtime_error("Failed to invoke.");
//
//    NSMutableArray<NSString *> *array = [NSMutableArray arrayWithCapacity:res.size()];
//
//    for (const auto& str : res)
//      [array addObject:[NSString stringWithUTF8String:str.c_str()]];
//
//    return [NSArray arrayWithArray:array];
//}
//
//// Don't compile this code when we build for the old architecture.
//#ifdef RCT_NEW_ARCH_ENABLED
//- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
//    (const facebook::react::ObjCTurboModule::InitParams &)params
//{
//    return std::make_shared<facebook::react::NativeWebassemblySpecJSI>(params);
//}
//#endif
//
//@end
//
