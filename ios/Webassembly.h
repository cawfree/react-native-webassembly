#import <React/RCTBridgeModule.h>

@interface Webassembly : NSObject <RCTBridgeModule>

@property (nonatomic, assign) BOOL setBridgeOnMainQueue;

@end

//#ifdef __cplusplus
//#import "react-native-webassembly.h"
//#endif
//
//#ifdef RCT_NEW_ARCH_ENABLED
//#import "RNWebassemblySpec.h"
//
//@interface Webassembly : NSObject <NativeWebassemblySpec>
//#else
//#import <React/RCTBridgeModule.h>
//
//@interface Webassembly : NSObject <RCTBridgeModule>
//#endif
//
//@end
//
