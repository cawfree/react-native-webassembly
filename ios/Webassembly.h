#import <React/RCTBridgeModule.h>

#import "react-native-webassembly.h"

@interface Webassembly : NSObject <RCTBridgeModule>
  @property (nonatomic, assign) BOOL setBridgeOnMainQueue;
@end
