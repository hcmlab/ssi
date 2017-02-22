package de.hcmlab.ssi.android_xmlpipe;

import java.io.ByteArrayOutputStream;  
import java.io.IOException;  
import java.io.InputStream;  

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.content.res.Resources;  
import android.content.res.*;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.CheckBox;
import android.widget.Button;
import android.view.Menu;
import android.view.View;
import android.widget.Toast;



public class DummyActivity extends Activity  {

	private long buttonCounter = 0;
	
	public static Handler mUiHandler = null;
		@Override
		protected void onCreate(Bundle savedInstanceState) {
			super.onCreate(savedInstanceState);
			setContentView(R.layout.activity_dummy);
			if(SensorCollectorService.mIsServiceRunning)
						((Button)findViewById(R.id.start_service)).setEnabled(false);
			mUiHandler = new Handler() // Receive messages from service class 
	        {
				public void handleMessage(Message msg)
	        	{
	          		switch(msg.what)
	        		{
		          		case 0:
		          			// add the status which came from service and show on GUI
		          			Toast.makeText(DummyActivity.this, msg.obj.toString(), Toast.LENGTH_LONG).show();
		          			break;

		        		default:
	        			break;
	        		}
	        	}
	        };
		}

		//start the service
		public void onClickStartServie(View V)
		{
			//start the service from here //SensorCollectorService is your service class name
			getApplicationContext().startService(new Intent(this, SensorCollectorService.class));
			((Button)findViewById(R.id.start_service)).setEnabled(false);
		}
		//Stop the started service
		public void onClickStopService(View V)
		{
			//Stop the running service from here//SensorCollectorService is your service class name
			//Service will only stop if it is already running.
			getApplicationContext().stopService(new Intent(this, SensorCollectorService.class));
			((Button)findViewById(R.id.start_service)).setEnabled(true);
			buttonCounter = 0;
		}
		//send message to service
		public void onClickOverwrite(View V)
		{
			boolean b= false;
			if(((CheckBox) findViewById(R.id.chkOverwrite)).isChecked()) b=true;
			SensorCollectorService.setOverwriteFiles(b);
			String tMsg="FALSe";
			if(SensorCollectorService.getOverwriteFiles())tMsg="truE";
			Toast.makeText(DummyActivity.this, tMsg, Toast.LENGTH_LONG).show();
		}

		@Override
		public boolean onCreateOptionsMenu(Menu menu) {
			// Inflate the menu; this adds items to the action bar if it is present.
			getMenuInflater().inflate(R.menu.activity_dummy, menu);
			return true;
		}

}
