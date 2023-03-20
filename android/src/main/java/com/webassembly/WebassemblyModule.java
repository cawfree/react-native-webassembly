package com.webassembly;

import android.util.Base64;

import androidx.annotation.NonNull;

import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReadableArray;
import com.facebook.react.bridge.ReadableMap;
import com.facebook.react.bridge.WritableArray;
import com.facebook.react.bridge.WritableNativeArray;
import com.facebook.react.module.annotations.ReactModule;

import java.util.ArrayList;

@ReactModule(name = WebassemblyModule.NAME)
public class WebassemblyModule extends NativeWebassemblySpec {
  public static final String NAME = "Webassembly";

  public WebassemblyModule(ReactApplicationContext reactContext) {
    super(reactContext);
  }

  @Override
  @NonNull
  public String getName() {
    return NAME;
  }

  static {
    System.loadLibrary("cpp");
  }

  private static native double nativeInstantiate(String iid, byte[] bufferSource, long bufferSourceLength, double stackSizeInBytes, String[] rawFunctions);

  private static native double nativeInvoke(String iid, String func, String[] args, ArrayList<String> result);

  @Override public final double instantiate(final ReadableMap pParams) {
    final String iid = pParams.getString("iid");
    final String bufferSource = pParams.getString("bufferSource");
    final int stackSizeInBytes = pParams.getInt("stackSizeInBytes");
    final ReadableArray rawFunctionsArray = pParams.getArray("rawFunctions");

    final String[] rawFunctions = new String[rawFunctionsArray.size()];

    for (int i = 0; i < rawFunctionsArray.size(); i += 1) {
      rawFunctions[i] = rawFunctionsArray.getString(i);
    }

    final byte[] bufferSourceBytes = Base64.decode(bufferSource, Base64.DEFAULT);

    return nativeInstantiate(
      iid,
      bufferSourceBytes,
      bufferSourceBytes.length,
      stackSizeInBytes,
      rawFunctions
    );
  }

  @Override public final WritableArray invoke(final ReadableMap pParams) {
    final String iid = pParams.getString("iid");
    final String func = pParams.getString("func");
    final ReadableArray args = pParams.getArray("args");

    final ArrayList<String> result = new ArrayList<>();

    final String[] args2 = new String[args.size()];

    for (int i = 0; i < args.size(); i += 1)
      args2[i] = args.getString(i);

    if (nativeInvoke(iid, func, args2, result) != 0)
      throw new RuntimeException("Invocation of " + func + " failed.");

    final WritableArray writableArray = new WritableNativeArray();

    for (int i = 0; i < result.size(); i += 1)
      writableArray.pushString(result.get(i));

    return writableArray;
  }

}
