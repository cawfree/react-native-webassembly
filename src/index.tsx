//@ts-ignore we want to ignore everything
// else in global except what we need to access.
// Maybe there is a better way to do this.
import { NativeModules } from 'react-native';

// Installing JSI Bindings as done by
// https://github.com/mrousavy/react-native-mmkv

//@ts-ignore
const simpleJsiModule: {
  helloWorld(): string;
  multiplyWithCallback(
    x: number,
    y: number,
    callback: (z: number) => void
  ): void;
  multiply(x: number, y: number): number;
  getDeviceName(): string;
  setItem(key: string, value: string): boolean;
  getItem(key: string): string;
  foo(callback:(error:string | undefined,value:string | undefined) => void):void
  //@ts-ignore
} = global;

export function isLoaded() {
  return typeof simpleJsiModule.getItem === 'function';
}

if (!isLoaded()) {
  const result = NativeModules.Webassembly?.install();

  if (!result || !isLoaded()) throw new Error('JSI bindings were not installed for: SimpleJsi Module');
}

export default simpleJsiModule;

//const { Image } = require('react-native');
//const { Buffer } = require('buffer');
//const { nanoid } = require('nanoid/non-secure');
//
//const Webassembly = require('./NativeWebassembly').default;
//
//export type { InstantiateParams } from './NativeWebassembly';
//
//export type WebassemblyInstance<Exports extends object> = {
//  readonly exports: Exports;
//};
//
//type WebAssemblyMemoryParams = {
//  readonly initial: number;
//};
//
//export class Memory {
//  readonly __initial: WebAssemblyMemoryParams['initial'] | undefined;
//
//  constructor(params: WebAssemblyMemoryParams) {
//    this.__initial = params?.initial;
//  }
//}
//
//export type WebAssemblyEnv = {
//  readonly memory?: Memory;
//};
//
//const DEFAULT_STACK_SIZE_IN_BYTES = 8192;
//
//const DEFAULT_MEMORY = new Memory({
//  initial: DEFAULT_STACK_SIZE_IN_BYTES,
//});
//
//export type Imports = Record<string, Function>;
//
//type ImportsMap = Omit<
//  {
//    readonly [key: string]: Imports | WebAssemblyEnv;
//  },
//  'env'
//>;
//
//export type WebAssemblyImportObject = ImportsMap & {
//  readonly env?: WebAssemblyEnv;
//};
//
//export type WebassemblyInstantiateResult<Exports extends object> = {
//  readonly instance: WebassemblyInstance<Exports>;
//};
//
//type ScopedFunction = {
//  readonly functionName: string;
//  readonly scope: string;
//};
//
//const getScopedFunctions = (
//  importsMap: ImportsMap
//): readonly ScopedFunction[] => {
//  if (!importsMap) return [];
//
//  return Object.entries(importsMap).flatMap(([scope, imports]) =>
//    Object.keys(imports).map((functionName) => ({ scope, functionName }))
//  );
//};
//
//const fetchRequireAsBase64 = async (moduleId: number): Promise<string> => {
//  const maybeAssetSource = Image.resolveAssetSource(moduleId);
//
//  const maybeUri = maybeAssetSource?.uri;
//
//  if (typeof maybeUri !== 'string' || !maybeUri.length)
//    throw new Error(
//      `Expected non-empty string uri, encountered "${String(maybeUri)}".`
//    );
//
//  const base64EncodedString = String(
//    await new Promise(async (resolve, reject) => {
//      const reader = new FileReader();
//      reader.onload = () => resolve(reader.result);
//      reader.onerror = reject;
//      reader.readAsDataURL(await (await fetch(maybeUri)).blob());
//    })
//  );
//
//  const maybeBase64EncodedString = base64EncodedString
//    .substring(base64EncodedString.indexOf(','))
//    .slice(1);
//
//  if (
//    typeof maybeBase64EncodedString !== 'string' ||
//    !maybeBase64EncodedString.length
//  )
//    throw new Error(
//      `Expected non-empty string base64EncodedString, encountered "${maybeBase64EncodedString}".`
//    );
//
//  return maybeBase64EncodedString;
//};
//
//// https://developer.mozilla.org/en-US/docs/WebAssembly/JavaScript_interface/instantiate
//export async function instantiate<Exports extends object>(
//  bufferSource: Uint8Array | ArrayBuffer | number,
//  maybeImportObject: WebAssemblyImportObject | undefined = undefined
//): Promise<WebassemblyInstantiateResult<Exports>> {
//  const iid = nanoid();
//
//  const importObject = maybeImportObject || {};
//  const { env: maybeEnv, ...extras } = importObject;
//
//  const memory = maybeEnv?.memory || DEFAULT_MEMORY;
//
//  const stackSizeInBytes = memory?.__initial ?? DEFAULT_STACK_SIZE_IN_BYTES;
//
//  const scopedFunctions = getScopedFunctions(extras);
//
//  const rawFunctions = scopedFunctions.map(({ functionName }) => functionName);
//  const rawFunctionScopes = scopedFunctions.map(({ scope }) => scope);
//
//  const instanceResult = Webassembly.instantiate({
//    iid,
//    bufferSource:
//      typeof bufferSource === 'number'
//        ? await fetchRequireAsBase64(bufferSource)
//        : Buffer.from(bufferSource).toString('base64'),
//    stackSizeInBytes,
//    rawFunctions,
//    rawFunctionScopes,
//  });
//
//  if (instanceResult !== 0)
//    throw new Error('Failed to instantiate WebAssembly.');
//
//  const exports = new Proxy({} as Exports, {
//    get(_, func) {
//      if (typeof func !== 'string')
//        throw new Error(`Expected string, encountered ${typeof func}.`);
//
//      return (...args: readonly number[]) => {
//        const res = Webassembly.invoke({
//          iid,
//          func,
//          args: args.map((e) => e.toString()),
//        });
//
//        if (!res.length) return undefined;
//
//        const num = res.map(parseFloat);
//
//        if (res.length !== 1) return num;
//
//        return num[0];
//      };
//    },
//  });
//
//  return { instance: { exports } };
//}
//
