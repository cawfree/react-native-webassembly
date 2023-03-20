#import "Webassembly.h"

@implementation Webassembly
RCT_EXPORT_MODULE()

- (NSNumber *)instantiate:(JS::NativeWebassembly::InstantiateParams &)a {
    
    NSData  *data         = [[NSData alloc] initWithBase64EncodedString:a.bufferSource() options:0];
    uint8_t *bufferSource = (uint8_t *)[data bytes];
    
    std::vector<std::string> rawFunctions;
    
    for (const auto& rawFunction : a.rawFunctions())
      rawFunctions.push_back([rawFunction UTF8String]);
    
    return @(webassembly::instantiate(new RNWebassemblyInstantiateParams{[a.iid() UTF8String], bufferSource, (size_t)data.length, a.stackSizeInBytes(), &rawFunctions}));
}

- (NSArray<NSString *> *)invoke:(JS::NativeWebassembly::InvokeParams &)a {
    
    std::vector<std::string> args;
    args.reserve(a.args().size());

    for (NSString *str : a.args()) {
      args.emplace_back([str UTF8String]);
    }
    
    std::vector<std::string> res;
    
    @(webassembly::invoke(new RNWebassemblyInvokeParams{[a.iid() UTF8String], [a.func() UTF8String], &args, &res}));
    
    NSMutableArray<NSString *> *array = [NSMutableArray arrayWithCapacity:res.size()];
    
    for (const auto& str : res)
      [array addObject:[NSString stringWithUTF8String:str.c_str()]];
    
    return [NSArray arrayWithArray:array];
}

// Don't compile this code when we build for the old architecture.
#ifdef RCT_NEW_ARCH_ENABLED
- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
    return std::make_shared<facebook::react::NativeWebassemblySpecJSI>(params);
}
#endif

@end
