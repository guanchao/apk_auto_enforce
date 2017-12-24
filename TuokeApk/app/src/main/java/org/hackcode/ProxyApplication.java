package org.hackcode;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Map;

import android.app.Application;
import android.app.Instrumentation;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.util.Log;
import android.widget.Toast;

public class ProxyApplication extends Application{

	static{
		System.loadLibrary("hackcodejiagu");
	}

	public String bytesToHexString(byte[] src){   
	    StringBuilder stringBuilder = new StringBuilder("");   
	    if (src == null || src.length <= 0) {   
	        return null;   
	    }   
	    for (int i = 0; i < src.length; i++) {   
	        int v = src[i] & 0xFF;   
	        String hv = Integer.toHexString(v);   
	        if (hv.length() < 2) {   
	            stringBuilder.append(0);   
	        }   
	        stringBuilder.append(hv);   
	    }   
	    return stringBuilder.toString();   
	}
	
	@Override
	public void onCreate() {
		try {
			// 如果源应用配置有Appliction对象，则替换为源应用Applicaiton，以便不影响源程序逻辑。  
			String appClassName = "com.gh.base.AppController";
			/**
			 * 调用静态方法android.app.ActivityThread.currentActivityThread
			 * 获取当前activity所在的线程对象
			 */
			Object currentActivityThread = ReflectUtils.invokeStaticMethod(  
			        "android.app.ActivityThread", "currentActivityThread",  
			        new Class[] {}, new Object[] {}); 
			/**
			 * 获取currentActivityThread中的mBoundApplication属性对象，该对象是一个
			 *  AppBindData类对象，该类是ActivityThread的一个内部类
			 */
			Object mBoundApplication = ReflectUtils.getFieldObject(  
			        "android.app.ActivityThread", currentActivityThread,  
			        "mBoundApplication"); 
			/**
			 * 获取mBoundApplication中的info属性，info 是 LoadedApk类对象
			 */
			Object loadedApkInfo = ReflectUtils.getFieldObject(  
			        "android.app.ActivityThread$AppBindData",  
			        mBoundApplication, "info");  
			 if(null == loadedApkInfo){
			    	Log.v("demo", "[JiaguApk]=>onCreate()=>loadedApkInfo is null!!!");
			  }else{
				  Log.v("demo", "[JiaguApk]=>onCreate()=>loadedApkInfo:" + loadedApkInfo);
			  }
			
			/**
			 * loadedApkInfo对象的mApplication属性置为null
			 */
			ReflectUtils.setFieldObject("android.app.LoadedApk", "mApplication",  
			        loadedApkInfo, null);  
			
			/**
			 * 获取currentActivityThread对象中的mInitialApplication属性
			 * 这货是个正牌的 Application
			 */
			Object oldApplication = ReflectUtils.getFieldObject(  
			        "android.app.ActivityThread", currentActivityThread,  
			        "mInitialApplication");  
			
			/**
			 * 获取currentActivityThread对象中的mAllApplications属性
			 * 这货是 装Application的列表
			 */
			@SuppressWarnings("unchecked")
			ArrayList<Application> mAllApplications = (ArrayList<Application>) ReflectUtils  
			        .getFieldObject("android.app.ActivityThread",  
			                currentActivityThread, "mAllApplications"); 
			
			//列表对象终于可以直接调用了 remove调了之前获取的application 抹去记录的样子
			mAllApplications.remove(oldApplication); 
			
			/**
			 * 获取前面得到LoadedApk对象中的mApplicationInfo属性，是个ApplicationInfo对象
			 */
			ApplicationInfo appinfo_In_LoadedApk = (ApplicationInfo) ReflectUtils  
			        .getFieldObject("android.app.LoadedApk", loadedApkInfo,  
			                "mApplicationInfo"); 
			
			/**
			 * 获取前面得到AppBindData对象中的appInfo属性，也是个ApplicationInfo对象
			 */
			ApplicationInfo appinfo_In_AppBindData = (ApplicationInfo) ReflectUtils  
			        .getFieldObject("android.app.ActivityThread$AppBindData",  
			                mBoundApplication, "appInfo"); 
			
			//把这两个对象的className属性设置为从meta-data中获取的被加密apk的application路径
			appinfo_In_LoadedApk.className = appClassName;  
			appinfo_In_AppBindData.className = appClassName; 
			/**
			 * 调用LoadedApk中的makeApplication 方法 造一个application
			 * 前面改过路径了 
			 */
			Application app = (Application) ReflectUtils.invokeMethod(  
			        "android.app.LoadedApk", "makeApplication", loadedApkInfo,  
			        new Class[] { boolean.class, Instrumentation.class },  
			        new Object[] { false, null });  
			ReflectUtils.setFieldObject("android.app.ActivityThread",  
			        "mInitialApplication", currentActivityThread, app);  
			if(null == app){
				Log.v("demo", "[JiaguApk]=>onCreate()=>app is null!!!");
			  }else{
			      app.onCreate();
			      Log.v("demo", "[JiaguApk]=>onCreate() success!");
			  }
		} catch (Exception e) {
			Log.v("demo", "[JiaguApk]=>onCreate() " + Log.getStackTraceString(e));
		}
		Toast.makeText(this, "Enforced by 01hackcode", Toast.LENGTH_LONG).show();
	}
	
	

	@Override
	public void attachBaseContext(Context base) {
		super.attachBaseContext(base);
		
		Log.v("demo", "[JiaguApk]=>attachBaseContext() start...");
		
		try {
			
			Object currentActivityThread = ReflectUtils.invokeStaticMethod("android.app.ActivityThread", "currentActivityThread", new Class[] {}, new Object[] {});
			String packageName = getPackageName();
			Map mPackages = (Map) ReflectUtils.getFieldObject("android.app.ActivityThread", currentActivityThread, "mPackages");
			WeakReference wr = (WeakReference) mPackages.get(packageName);
			
			start((Application)this, "/data/data/" + packageName + "/lib", base.getClassLoader().getParent(), wr.get());

		} catch (Exception e) {
			Log.v("demo", "[JiaguApk]=>attachBaseContext() " + Log.getStackTraceString(e));
		}
		
		
		Log.v("demo", "[JiaguApk]=>attachBaseContext() end...");
		
	}
	
    private native void start(Application wrapper, String libraryPath, ClassLoader parent, Object loadedApk);

}
