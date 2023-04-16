import * as React from 'react';
import * as WebAssembly from 'react-native-webassembly';
import type { WebassemblyInstantiateResult } from 'react-native-webassembly';

import { fetchWasm } from '../utils';

type State<Exports extends object> = Readonly<
  | { loading: true }
  | { loading: false; result: WebassemblyInstantiateResult<Exports> }
  | { loading: false; error: Error }
>;

export function useWasmUri<Exports extends object>(
  uri: string,
  importObject: WebAssembly.WebAssemblyImportObject | undefined = undefined
): State<Exports> {
  const [state, setState] = React.useState<State<Exports>>({
    loading: true,
  });

  React.useEffect(
    () =>
      void (async () => {
        try {
          setState({
            loading: false,
            result: await WebAssembly.instantiate<Exports>(
              await fetchWasm(uri),
              importObject
            ),
          });
        } catch (cause) {
          setState({
            loading: false,
            error: new Error(
              `Failed to instantiate "${uri}".`,
              cause as ErrorOptions
            ),
          });
        }
      })(),
    [uri, importObject]
  );

  return state;
}
