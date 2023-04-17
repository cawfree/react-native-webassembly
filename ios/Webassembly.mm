#import <React/RCTUtils.h>
#import <React/RCTBridge+Private.h>
#import <jsi/jsi.h>
#import <sys/utsname.h>

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

  auto jsiRuntime = (facebook::jsi::Runtime*) cxxBridge.runtime;
  
  if (jsiRuntime == nil) return @false;

  webassembly::install(*(facebook::jsi::Runtime *)jsiRuntime);

  return @true;
}

@end
