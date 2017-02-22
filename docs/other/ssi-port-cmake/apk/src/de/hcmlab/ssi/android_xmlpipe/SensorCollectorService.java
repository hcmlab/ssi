package de.hcmlab.ssi.android_xmlpipe;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.util.Log;
import android.widget.Toast;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.res.Resources;  
import android.content.res.*;

public class SensorCollectorService extends Service{
	private NotificationManager mNM;
	//private int NOTIFICATION_ID = R.string.local_service_started;
	private static boolean overwriteFiles=false;
	private static final String TAG = "SensorCollectorService";

	//used for getting the handler from other class for sending messages
	public static Handler 		mSensorCollectorServiceHandler 			= null;
	//used for keep track on Android running status
	public static Boolean 		mIsServiceRunning 			= false;
	
	
	 PowerManager.WakeLock wl;	//prevent CPU going to sleep
	 
	 
	 MyThread myThread;

	
    /**
     * Class for clients to access.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with
     * IPC.
     */
    public class LocalBinder extends Binder {
    	SensorCollectorService getService() {
            return SensorCollectorService.this;
        }
    }
	
	
    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder mBinder = new LocalBinder();
	
	
	@Override
	public IBinder onBind(Intent arg0) {
		return mBinder;
	}

	@Override
	public void onCreate() {
		
		PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
		wl = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "SensorCollectorService");
		wl.acquire();
		
		Toast.makeText(this, "SensorCollectorService Created overwrite: "+ Boolean.toString(overwriteFiles) , Toast.LENGTH_LONG).show();
		Log.d(TAG, "onCreate");
		
		mNM = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
		AssetManager am =getApplicationContext().getAssets();
		if(!mIsServiceRunning)
			startSSI(getCacheDir().getAbsolutePath(), am, overwriteFiles);

	        // Display a notification about us starting.  We put an icon in the status bar.
	        //showNotification();
	}

	@Override
	public void onStart(Intent intent, int startId) {
		Toast.makeText(this, "My Service Started", Toast.LENGTH_LONG).show();
		Log.d(TAG, "onStart");	
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {

	 myThread = new MyThread();
	 myThread.start();

		try 
		{
			Thread.sleep(250);
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
		}
				
		

		mIsServiceRunning = true; // set service running status = true

		Toast.makeText(this, "Congrats! My Service Started", Toast.LENGTH_LONG).show();
		// We need to return if we want to handle this service explicitly. 

		//JNI
		//startSensorLoop();

		
		
		return START_STICKY;
	}

	@Override
	public void onDestroy() {
		//JNI

		stopSSI();
		myThread.quit();
		
		
		Toast.makeText(this, "SensorCollectorService Stopped", Toast.LENGTH_LONG).show();
		Log.d(TAG, "onDestroy");
		
		//mNM.cancel(NOTIFICATION_ID);

		mIsServiceRunning = false; // make it false, as the service is already destroyed.
		
		wl.release();	//allow the cpu to go to sleep
	}

	//Your inner thread class is here to getting response from Activity and processing them
	class MyThread extends Thread
	{
		private static final String INNER_TAG = "MyThread";
		
		//BLE ------------------------------------------
	    private BluetoothAdapter bluetoothAdapter;
	    private BleDevicesScanner scanner;
	    private static final long SCAN_PERIOD = 1000;	//500
	    //BLE ------------------------------------------
	    
	    
	    public void quit() {
	    	//BLE
	    	if (scanner != null) scanner.stop();
        	//Battery
        	unregisterReceiver(this.mBatInfoReceiver);
        	unregisterReceiver(this.mSensorTagReceiver);
	    }
	    
	    
	    //Battery
	    private BroadcastReceiver mBatInfoReceiver = new BroadcastReceiver(){
	        @Override
	        public void onReceive(Context arg0, Intent intent) {
        
		        int level = intent.getIntExtra("level", 0);
		        int plugged = intent.getIntExtra("plugged", 0);
		        Log.e("battery", String.valueOf(level) + "%");
		        Log.e("plugged", String.valueOf(plugged));
		        
		        addEvent("BatteryChanged", "Level;Plugged", String.valueOf(level)+";"+String.valueOf(plugged));
	         }
	    };
	    
	    
	    //SensorTag
	    private BroadcastReceiver mSensorTagReceiver = new BroadcastReceiver(){
	        @Override
	        public void onReceive(Context arg0, Intent intent) {
		        int keypressed = intent.getIntExtra("key state", 0);
		        
		        Log.e("ti.android.ble.sensortag", String.valueOf(keypressed));
		        
		        addEvent("SensorTag", "Key state", String.valueOf(keypressed));
	         }
	    };
	    
	    
    	public void run() 
    	{  
    		//Battery    		
    		registerReceiver(this.mBatInfoReceiver, 
    	            new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
    		
    		//SensorTag
    		registerReceiver(this.mSensorTagReceiver, 
    	            new IntentFilter("ti.android.ble.sensortag"));
    		
    		
    		
    		//BLE -----------------------------------------------------------------------------------------------------------------
    		
    		boolean bleError = false;
    		
    		final int bleStatus = BleUtils.getBleStatus(getBaseContext());
            switch (bleStatus) {
            	case BleUtils.STATUS_BLUETOOTH_DISABLED:
            		bleError = true;
            		return;
                case BleUtils.STATUS_BLE_NOT_AVAILABLE:
                	bleError = true;
                    //ErrorDialog.newInstance(R.string.dialog_error_no_ble).show(getFragmentManager(), ErrorDialog.TAG);
                    return;
                case BleUtils.STATUS_BLUETOOTH_NOT_AVAILABLE:
                	bleError = true;
                    //ErrorDialog.newInstance(R.string.dialog_error_no_bluetooth).show(getFragmentManager(), ErrorDialog.TAG);
                    return;
                default:
                    bluetoothAdapter = BleUtils.getBluetoothAdapter(getBaseContext());
            }
    		
            if (!bleError) {
	    		scanner = new BleDevicesScanner(bluetoothAdapter, new BluetoothAdapter.LeScanCallback() {
	                @Override
	                public void onLeScan(final BluetoothDevice device, final int rssi, byte[] scanRecord) {
	                    //leDeviceListAdapter.addDevice(device, rssi);
	                    //leDeviceListAdapter.notifyDataSetChanged();
	                	
	                	addEvent("BleDevicesScanner", device.getAddress(), String.valueOf(rssi));
	                	Log.i("BackgroundThread",device.getAddress()+"; RSSI: "+String.valueOf(rssi ));
	                	
	                }
	            });
	            scanner.setScanPeriod(SCAN_PERIOD);
	            
	            scanner.start();
            }
            
            //BLE -----------------------------------------------------------------------------------------------------------------
    		
    		
    		this.setName(INNER_TAG);

    		// Prepare the looper before creating the handler.
			Looper.prepare();
			mSensorCollectorServiceHandler = new Handler()
			{
				//here we will receive messages from activity(using sendMessage() from activity)
    			public void handleMessage(Message msg)
    			{
    				Log.i("BackgroundThread","handleMessage(Message msg)" );
    				/*
    				switch(msg.what)
    				{
    				case 0: // we sent message with what value =0 from the activity. here it is
    						//Reply to the activity from here using same process handle.sendMessage()
    						//So first get the Activity handler then send the message
    						
    					if(null != SensorCollectorActivity.mUiHandler)
    					{
    						//first build the message and send.
    						//put a integer value here and get it from the Activity handler
    						//For Example: lets use 0 (msg.what = 0;) 
    						//for receiving service running status in the activity
    						Message msgToActivity = new Message();
    						msgToActivity.what = 0; 
    						if(true ==mIsServiceRunning)
    							msgToActivity.obj  = "Request Received. Service is Running"; // you can put extra message here
    						else
    							msgToActivity.obj  = "Request Received. Service is not Running"; // you can put extra message here

    						SensorCollectorActivity.mUiHandler.sendMessage(msgToActivity);
    					}
    					
    				break;
    				
    				
    				case 1:
    					if(null != SensorCollectorActivity.mUiHandler)
    					{
    						addEvent("SensorCollector", (String)msg.obj, "pressed");
    					}
    					break;

					default:
						break;
    				}*/
				}
    		};
    		Looper.loop();
    	}
    	

	}
	public static boolean getOverwriteFiles()
	{
		return overwriteFiles;
	}
	public static void setOverwriteFiles(boolean overFiles)
	{
		overwriteFiles=overFiles;
	}
	
	/*
	private void showNotification() {
        CharSequence text = getText(R.string.local_service_started);
      
        Intent intent = new Intent(this, SensorCollectorActivity.class);      
        intent.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
        
        // The PendingIntent to launch our activity if the user selects this notification
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0, intent, 0);        
        
        
        //set the notification
		Notification.Builder builder = new Notification.Builder(this);
		Notification notific = builder
		 .setSmallIcon(android.R.drawable.ic_dialog_alert)
		 .setContentTitle(getString(R.string.app_name))
		 .setContentText(text)
		 .setContentIntent(contentIntent)
		 .setOngoing(true)
		 .build();
		
		//notify
		//mNM.notify(NOTIFICATION_ID, notific);
		
		//allows us to connect to our service again after closing the activity (and removing from history)
		//prevents the system to kill the service if resources are needed
		startForeground(NOTIFICATION_ID, notific);
    }*/
	
	
	
	//JNI
	


    
    public native void addEvent(String in_eventSender, String in_eventName, String in_eventText);
    public native void startSSI(String path, AssetManager am, boolean extractFiles);
    public native void stopSSI();
    static{
		try
		{
        System.loadLibrary("ssiframe");
        System.loadLibrary("ssievent");
        System.loadLibrary("ssiioput");
        System.loadLibrary("ssiandroidsensors");
        System.loadLibrary("ssiandroidjavasensors");
        System.loadLibrary("ssiaudio");
        System.loadLibrary("ssimodel");
        System.loadLibrary("ssisignal");
        System.loadLibrary("android_xmlpipe");

		}
		catch(Exception e)
		{
			System.out.println(e.getMessage());
			
			//we need to extract libs manually :/
		}
		
	
}

}
