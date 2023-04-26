# react-native-webassembly

This package enables [__WebAssembly__](https://webassembly.org/) for [__React Native__](https://reactnative.dev) powered by C++ [__TurboModules__](https://reactnative.dev/docs/next/the-new-architecture/cxx-cxxturbomodules) and [__Wasm3__](https://github.com/wasm3/wasm3), a fast and universal WebAssembly runtime.

[`react-native-webassembly`](https://github.com/cawfree/react-native-webassembly) provides React Native applications with the capability to execute universal [__Wasm__](https://webassembly.org/) binaries with native speed.

> ✏️ This project is still in __active development__. The following tasks are still remaining to be completed:
>
> - Sanitize C++ memory management practices.
> - Normalize execution and result handling of userland `export` functions.
> - Test framework implementation.
>
> [__Pull Requests are welcome!__](https://github.com/cawfree/react-native-webassembly/pulls) 🙏

### 📡 Installation

1. First, ensure your React Native application supports the [__New Architecture__](https://reactnative.dev/docs/new-architecture-intro):
   - [__iOS__](https://reactnative.dev/docs/new-architecture-library-ios)
   - [__Android__](https://reactnative.dev/docs/new-architecture-library-android)
2. Install `react-native-webassembly`:

   ```shell
   yarn add react-native-webassembly # React Native
   npx expo install react-native-webassembly # Expo
   ```
3. If you're using [__Expo__](https://expo.dev/), don't forget to run `npx expo prebuild` after installing.

### ✍️ Usage

The goal of [`react-native-webassembly`](https://github.com/cawfree/react-native-webassembly) is to export a [__browser-equivalent interface__](https://developer.mozilla.org/en-US/docs/WebAssembly) to the WebAssembly API.

To initialize a new WebAssembly module, we'll need to `instantiate` an module using a buffer populated with a `.wasm` binary:

```typescript
import axios from 'axios';
import * as WebAssembly from 'react-native-webassembly';

import HelloWorld from './hello-world.wasm';

const module = await WebAssembly.instantiate<{
  add: (a: number, b: number) => number;
}>(HelloWorld);
```

> **Note**
>
> To import `.wasm` files directly, you will need to [update your `metro.config.js`](https://github.com/cawfree/react-native-webassembly/blob/d9d950e47277e899371a85cd430336a84d96c369/example/metro.config.js#L32).

Alternatively, in the snippet below, we show how to download and instantiate the reference [__Hello World__](https://github.com/torch2424/wasm-by-example) example stored at a remote location:

```typescript
import axios from 'axios';
import * as WebAssembly from 'react-native-webassembly';

const {
  data: bufferSource,
} = await axios({
  url: 'https://github.com/torch2424/wasm-by-example/raw/master/examples/hello-world/demo/assemblyscript/hello-world.wasm',
  method: 'get',
  responseType: 'arraybuffer',
});

const module = await WebAssembly.instantiate<{
  add: (a: number, b: number) => number;
}>(bufferSource);
```

You'll notice that in our call to `instantiate`, we can also pass typing information for the `Exports` of the module. In this case, the `hello-world.wasm` binary exports a function to add two numbers, `add`.

Once configured, we can execute the compiled `wasm` module from our JavaScript code, using the type-safe exported interface:

```typescript
module.instance.exports.add(1, 2); // 3.
```

It's also possible to declare an `importObject` to receive callbacks from the compiled module, which declares a list of callback function implementations which can be invoked by the WebAssembly runtime.

> **Warning**
>
> Some native modules __require__ the presence of certain function implementations. Without specifying module-specific required dependencies, instantiation will fail.

For example, the [__Circom__](https://github.com/iden3/circom) library converts arithmetic circuits used for generating, evaluating and verifying [__SNARK__](https://consensys.net/blog/developers/introduction-to-zk-snarks/)s are expressed as WASM modules which require the runtime to define an `exceptionHandler` function belonging to the namespace `runtime`.

It's simple to define an `importObject`:

```typescript
const module = await WebAssembly.instantiate<{
  getVersion: () => number;
  getFieldNumLen32: () => number;
  // ...
}>(bufferSource, {
  // Declare custom memory implementation.
  env: {
    memory: new WebAssembly.Memory({ initial: 32767 }),
  },
  // Define the scope of the import functions.
  runtime: {
    exceptionHandler: (value: number) => console.error(value),
  },
});
```

Here, we declare an `exceptionHandler` as `runtime` imports to the compiled module. Without declaring this required dependency, the module would fail to compile.

You can find a working implementation of this process in the [__Example App__](example/src/App.tsx).

### ✌️ License
[__MIT__](LICENSE)
