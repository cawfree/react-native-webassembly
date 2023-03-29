const { Buffer } = require('buffer');
const { nanoid } = require('nanoid/non-secure');

const Webassembly = require('./NativeWebassembly').default;

export type { InstantiateParams } from './NativeWebassembly';

export type WebassemblyInstance<Exports extends object> = {
  readonly exports: Exports;
};

type WebAssemblyMemoryParams = {
  readonly initial: number;
};

export class Memory {
  readonly __initial: WebAssemblyMemoryParams['initial'] | undefined;

  constructor(params: WebAssemblyMemoryParams) {
    this.__initial = params?.initial;
  }
}

export type WebAssemblyEnv = {
  readonly memory?: Memory;
};

const DEFAULT_STACK_SIZE_IN_BYTES = 8192;

const DEFAULT_MEMORY = new Memory({
  initial: DEFAULT_STACK_SIZE_IN_BYTES,
});

export type Imports = Record<string, Function>;

export type WebAssemblyImportObject = {
  readonly env?: WebAssemblyEnv;
  readonly imports: Imports;
};

export type WebassemblyInstantiateResult<Exports extends object> = {
  readonly instance: WebassemblyInstance<Exports>;
};

// https://developer.mozilla.org/en-US/docs/WebAssembly/JavaScript_interface/instantiate
export async function instantiate<Exports extends object>(
  bufferSource: Uint8Array | ArrayBuffer,
  importObject: WebAssemblyImportObject | undefined = undefined
): Promise<WebassemblyInstantiateResult<Exports>> {
  const iid = nanoid();

  const memory = importObject?.env?.memory || DEFAULT_MEMORY;

  const stackSizeInBytes = memory?.__initial ?? DEFAULT_STACK_SIZE_IN_BYTES;

  const rawFunctions = Object.keys(importObject?.imports || {});

  const instanceResult = Webassembly.instantiate({
    iid,
    bufferSource: Buffer.from(bufferSource).toString('base64'),
    stackSizeInBytes,
    rawFunctions,
    rawFunctionScopes: rawFunctions.map(() => '*'),
  });

  if (instanceResult !== 0)
    throw new Error('Failed to instantiate WebAssembly.');

  const exports = new Proxy({} as Exports, {
    get(_, func) {
      if (typeof func !== 'string')
        throw new Error(`Expected string, encountered ${typeof func}.`);

      return (...args: readonly number[]) => {
        const res = Webassembly.invoke({
          iid,
          func,
          args: args.map((e) => e.toString()),
        });

        if (!res.length) return undefined;

        const num = res.map(parseFloat);

        if (res.length !== 1) return num;

        return num[0];
      };
    },
  });

  return { instance: { exports } };
}
