import * as React from 'react';
import {StyleSheet, View} from 'react-native';
import * as WebAssembly from 'react-native-webassembly';

export default function App() {

  React.useEffect(
    () => console.warn(WebAssembly.instantiate()),
    []
  );

  return <View style={styles.container} />;
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
});
