import * as React from 'react';
import { Button, StyleSheet, View } from 'react-native';

import * as WebAssembly from 'react-native-webassembly';

import Local from './Local.wasm';

import { useWasmCircomRuntime, useWasmHelloWorld } from './hooks';
import { fetchWasm } from './utils';

export default function App() {
  const helloWorld = useWasmHelloWorld();
  const helloWorldResult =
    'result' in helloWorld ? helloWorld.result : undefined;
  const helloWorldError = 'error' in helloWorld ? helloWorld.error : undefined;

  const { calculateWTNSBin, error: circomError } = useWasmCircomRuntime();

  React.useEffect(
    () => void (helloWorldError && console.error(helloWorldError)),
    [helloWorldError]
  );

  React.useEffect(
    () => void (circomError && console.error(circomError)),
    [circomError]
  );

  React.useEffect(() => {
    if (!helloWorldResult) return;

    const result = helloWorldResult.instance.exports.add(103, 202);

    if (result !== 305) throw new Error('Failed to add.');
  }, [helloWorldResult]);

  React.useEffect(
    () =>
      void (async () => {
        try {
          const localModule = await WebAssembly.instantiate<{
            readonly add: (a: number, b: number) => number;
          }>(Local);

          const result = localModule.instance.exports.add(1000, 2000);

          if (result !== 3000) throw new Error('Failed to add. (Local)');
        } catch (e) {
          console.error(e);
        }
      })(),
    []
  );

  React.useEffect(
    () =>
      void (async () => {
        try {
          /* complex */
          await WebAssembly.instantiate(
            await fetchWasm(
              'https://github.com/tact-lang/ton-wasm/raw/main/output/wasm/emulator-emscripten.wasm'
            ),
            {}
          );
        } catch (e) {
          console.error(e);
        }
      })(),
    []
  );

  return (
    <View style={styles.container}>
      <Button
        title="Circom"
        onPress={() => {
          try {
            // https://github.com/cawfree/zk-starter
            const witnessBuffer = calculateWTNSBin({ a: 3, b: 11 });
            console.log(JSON.stringify(witnessBuffer));
          } catch (e) {
            // Ignore instantiation errors.
            if (String(e) === 'Not ready to calculate.') return;

            console.error(e);
          }
        }}
      />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
});
