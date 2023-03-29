import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export type InstantiateParams = {
  readonly iid: string;
  readonly bufferSource: string;
  readonly stackSizeInBytes: number;
  readonly rawFunctions: readonly string[];
  readonly rawFunctionScopes: readonly string[];
};

export type InvokeParams = {
  readonly iid: string;
  readonly func: string;
  readonly args: readonly string[];
};

export interface Spec extends TurboModule {
  instantiate(params: InstantiateParams): number;
  invoke(params: InvokeParams): readonly string[];
}

export default TurboModuleRegistry.getEnforcing<Spec>('Webassembly');
