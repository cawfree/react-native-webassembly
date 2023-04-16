import { useWasmUri } from './useWasmUri';

export const useWasmHelloWorld = () =>
  useWasmUri<{
    readonly add: (a: number, b: number) => number;
  }>(
    'https://github.com/torch2424/wasm-by-example/raw/master/examples/hello-world/demo/assemblyscript/hello-world.wasm'
  );
