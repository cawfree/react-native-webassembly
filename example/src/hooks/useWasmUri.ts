import * as React from 'react';
import axios from 'axios';
import * as WebAssembly from 'react-native-webassembly';
import type { WebassemblyInstantiateResult } from 'react-native-webassembly';

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
          const { data: bufferSource } = await axios({
            url: uri,
            method: 'get',
            responseType: 'arraybuffer',
          });

          setState({
            loading: false,
            result: await WebAssembly.instantiate<Exports>(
              bufferSource,
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
