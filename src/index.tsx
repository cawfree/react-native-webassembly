const { Image, NativeModules } = require('react-native');
const { Buffer } = require('buffer');
const { nanoid } = require('nanoid/non-secure');

type InstantiateParams = {
  readonly iid: string;
  readonly bufferSource: string;
  readonly stackSizeInBytes: number;
};

type InvokeParams = {
  readonly iid: string;
  readonly func: string;
  readonly args: readonly string[];
};

// @ts-expect-error synthesized
const reactNativeWebAssembly: {
  readonly RNWebassembly_instantiate: (params: InstantiateParams) => number;
  readonly RNWebassembly_invoke: (params: InvokeParams) => readonly string[];
} = global;

if (
  typeof reactNativeWebAssembly.RNWebassembly_instantiate !== 'function' &&
  !NativeModules.Webassembly?.install?.()
)
  throw new Error('Unable to bind Webassembly to React Native JSI.');

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

type ImportsMap = Omit<
  {
    readonly [key: string]: Imports | WebAssemblyEnv;
  },
  'env'
>;

export type WebAssemblyImportObject = ImportsMap & {
  readonly env?: WebAssemblyEnv;
};

export type WebassemblyInstantiateResult<Exports extends object> = {
  readonly instance: WebassemblyInstance<Exports>;
};

const fetchRequireAsBase64 = async (moduleId: number): Promise<string> => {
  const maybeAssetSource = Image.resolveAssetSource(moduleId);

  const maybeUri = maybeAssetSource?.uri;

  if (typeof maybeUri !== 'string' || !maybeUri.length)
    throw new Error(
      `Expected non-empty string uri, encountered "${String(maybeUri)}".`
    );

  const base64EncodedString = String(
    await new Promise(async (resolve, reject) => {
      const reader = new FileReader();
      reader.onload = () => resolve(reader.result);
      reader.onerror = reject;
      reader.readAsDataURL(await (await fetch(maybeUri)).blob());
    })
  );

  const maybeBase64EncodedString = base64EncodedString
    .substring(base64EncodedString.indexOf(','))
    .slice(1);

  if (
    typeof maybeBase64EncodedString !== 'string' ||
    !maybeBase64EncodedString.length
  )
    throw new Error(
      `Expected non-empty string base64EncodedString, encountered "${maybeBase64EncodedString}".`
    );

  return maybeBase64EncodedString;
};

// https://developer.mozilla.org/en-US/docs/WebAssembly/JavaScript_interface/instantiate
export async function instantiate<Exports extends object>(
  bufferSource: Uint8Array | ArrayBuffer | number,
  maybeImportObject: WebAssemblyImportObject | undefined = undefined
): Promise<WebassemblyInstantiateResult<Exports>> {
  const iid = nanoid();

  const importObject = maybeImportObject || {};
  const { env: maybeEnv } = importObject;

  const memory = maybeEnv?.memory || DEFAULT_MEMORY;

  const stackSizeInBytes = memory?.__initial ?? DEFAULT_STACK_SIZE_IN_BYTES;

  const instanceResult = reactNativeWebAssembly.RNWebassembly_instantiate({
    iid,
    bufferSource:
      typeof bufferSource === 'number'
        ? await fetchRequireAsBase64(bufferSource)
        : Buffer.from(bufferSource).toString('base64'),
    stackSizeInBytes,
  });

  if (instanceResult !== 0)
    throw new Error(`Failed to instantiate WebAssembly. (${instanceResult})`);

  const exports = new Proxy({} as Exports, {
    get(_, func) {
      if (typeof func !== 'string')
        throw new Error(`Expected string, encountered ${typeof func}.`);

      return (...args: readonly number[]) => {
        const res = reactNativeWebAssembly.RNWebassembly_invoke({
          iid,
          func,
          args: args.map((e) => e.toString()),
        });

        if (!res.length) return undefined;

        const num = res.map(parseFloat);

        // TODO: This is incorrect. We need to check if the return type is truly
        //       a scalar or not.
        if (res.length !== 1) return num;

        return num[0];
      };
    },
  });

  return { instance: { exports } };
}
