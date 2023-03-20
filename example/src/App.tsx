import * as React from 'react';
import { Button, StyleSheet, View } from 'react-native';

import { useWasmCircomRuntime, useWasmHelloWorld } from './hooks';

export default function App() {
  const helloWorld = useWasmHelloWorld();
  const helloWorldResult =
    'result' in helloWorld ? helloWorld.result : undefined;

  const { calculateWTNSBin } = useWasmCircomRuntime();

  React.useEffect(() => {
    if (!helloWorldResult) return;

    const result = helloWorldResult.instance.exports.add(103, 202);

    if (result !== 305) throw new Error('Failed to add.');
  }, [helloWorldResult]);

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
