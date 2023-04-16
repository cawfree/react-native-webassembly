package com.webassembly;

import androidx.annotation.NonNull;
import android.content.SharedPreferences;
import android.os.Build;
import android.preference.PreferenceManager;
import android.util.Log;

import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.module.annotations.ReactModule;

@ReactModule(name = WebassemblyModule.NAME)
public class WebassemblyModule extends ReactContextBaseJavaModule {
  public static final String NAME = "Webassembly";

  private native void nativeInstall(long jsiPtr, String docDir);

  public WebassemblyModule(ReactApplicationContext reactContext) {
    super(reactContext);
  }

  @Override
  @NonNull
  public String getName() {
    return NAME;
  }

  @ReactMethod(isBlockingSynchronousMethod = true)
  public boolean install() {

    Log.d("cawfree", "did load library attempt");
    try {
      System.loadLibrary("cpp");

      Log.d("cawfree", "did load library");

      ReactApplicationContext context = getReactApplicationContext();
      nativeInstall(
        context.getJavaScriptContextHolder().get(),
        context.getFilesDir().getAbsolutePath()
      );
      return true;
    } catch (Exception exception) {

      Log.d("cawfree", "did load library not");
      return false;
    }
  }

  public String getModel() {
    String manufacturer = Build.MANUFACTURER;
    String model = Build.MODEL;
    if (model.startsWith(manufacturer)) {
      return model;
    } else {
      return manufacturer + " " + model;
    }
  }

  public void setItem(final String key, final String value) {
    SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this.getReactApplicationContext());
    SharedPreferences.Editor editor = preferences.edit();
    editor.putString(key,value);
    editor.apply();
  }

  public String getItem(final String key) {
    SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this.getReactApplicationContext());
    String value = preferences.getString(key, "");
    return value;
  }

  //private static native double nativeInstantiate(
  //  String iid,
  //  byte[] bufferSource,
  //  long bufferSourceLength,
  //  double stackSizeInBytes,
  //  String[] rawFunctions,
  //  String[] rawFunctionScopes
  //);

  //private static native double nativeInvoke(String iid, String func, String[] args, ArrayList<String> result);

  //@Override public final double instantiate(final ReadableMap pParams) {
  //  final String iid = pParams.getString("iid");
  //  final String bufferSource = pParams.getString("bufferSource");
  //  final int stackSizeInBytes = pParams.getInt("stackSizeInBytes");

  //  final ReadableArray rawFunctionsArray = pParams.getArray("rawFunctions");
  //  final ReadableArray rawFunctionScopesArray = pParams.getArray("rawFunctionScopes");

  //  final String[] rawFunctions = new String[rawFunctionsArray.size()];
  //  final String[] rawFunctionScopes = new String[rawFunctionScopesArray.size()];

  //  for (int i = 0; i < rawFunctionsArray.size(); i += 1) {
  //    rawFunctions[i] = rawFunctionsArray.getString(i);
  //    rawFunctionScopes[i] = rawFunctionScopesArray.getString(i);
  //  }

  //  final byte[] bufferSourceBytes = Base64.decode(bufferSource, Base64.DEFAULT);

  //  return nativeInstantiate(
  //    iid,
  //    bufferSourceBytes,
  //    bufferSourceBytes.length,
  //    stackSizeInBytes,
  //    rawFunctions,
  //    rawFunctionScopes
  //  );
  //}

  //@Override public final WritableArray invoke(final ReadableMap pParams) {
  //  final String iid = pParams.getString("iid");
  //  final String func = pParams.getString("func");
  //  final ReadableArray args = pParams.getArray("args");

  //  final ArrayList<String> result = new ArrayList<>();

  //  final String[] args2 = new String[args.size()];

  //  for (int i = 0; i < args.size(); i += 1)
  //    args2[i] = args.getString(i);

  //  if (nativeInvoke(iid, func, args2, result) != 0)
  //    throw new RuntimeException("Invocation of " + func + " failed.");

  //  final WritableArray writableArray = new WritableNativeArray();

  //  for (int i = 0; i < result.size(); i += 1)
  //    writableArray.pushString(result.get(i));

  //  return writableArray;
  //}

}
