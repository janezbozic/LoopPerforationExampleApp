package com.example.ndkbinderclient;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.example.Constants;

public class MainActivity extends AppCompatActivity implements ServiceConnection
{
    // Used to load the 'native-lib' library on application startup.
    static
    {
        System.loadLibrary("native-lib");
    }

    private volatile boolean mIsServiceConnected = false;
    private final ConditionVariable mServiceConnectionWaitLock = new ConditionVariable();
    private TextView mTV = null;
    private Button runButton;

    private ImageView imageView;
    private ImageView imageView2;
    private Bitmap original;

    Spinner spinner;

    int test = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTV = findViewById(R.id.sample_text);
        runButton = findViewById(R.id.runButton);

        spinner = findViewById(R.id.spinner);

        String [] options = {"Picture Brightness 640x427", "Picture Brightness 1920x1280"};

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(MainActivity.this,
                android.R.layout.simple_spinner_item, options);

        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                test = position;
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                test = 0;
            }
        });

        runButton.setOnClickListener((v)->{

            runButton.setEnabled(false);
            imageView.setImageBitmap(null);
            new Thread(new Runnable()
            {
                //Running test n on separate thread
                @Override
                public void run()
                {
                    runOnUiThread(new SetTextRunnable("Waiting to talk to Perforation service..."));

                    // Waiting for perforation service connection
                    while(!mIsServiceConnected)
                    {
                        mServiceConnectionWaitLock.block(); // waits for service connection
                    }

                    runOnUiThread(new SetTextRunnable("Running test..."));

                    switch (test){
                        //Running picture brightness test with lower resolution picture
                        case 0:
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadlow);
                            adjustBrightness();
                        break;
                        //Running picture brightness test with higher resolution picture
                        case 1:
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                            adjustBrightness();
                        break;
                    }
                }
            }).start();
        });

        imageView = (ImageView) findViewById(R.id.imageView);
        imageView2 = (ImageView) findViewById(R.id.imageView2);

    }

    @Override
    protected void onResume()
    {
        super.onResume();

        //When activity is back in use, we bind back to our service
        Intent intent = new Intent();
        intent.setClassName("com.example.ndkbinderclient",
                "com.example.ndkbinderclient.MyService");

        Log.d(Constants.LOG_TAG, "[App] [java] bindService");

        //Call to native function for binding
        bindService(intent, this, BIND_AUTO_CREATE);
    }

    @Override
    protected void onPause()
    {
        //Disconnect from service, when activity is paused
        unbindService(this);

        mIsServiceConnected = false;

        //Call to native function to disconnect from service
        onServiceDisconnected();

        Log.d(Constants.LOG_TAG, "[App] [java] unbindService");

        super.onPause();
    }

    @Override
    public void onServiceConnected(ComponentName componentName, IBinder iBinder)
    {
        Log.d(Constants.LOG_TAG, "[App] [java] onServiceConnected");

        //Call to native function for connecting to the service
        onServiceConnected(iBinder);

        mIsServiceConnected = true;

        mServiceConnectionWaitLock.open(); // breaks service connection waits
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName)
    {
        mIsServiceConnected = false;

        //Call to native function for disconnecting from the service
        onServiceDisconnected();

        Log.d(Constants.LOG_TAG, "[App] [java] onServiceDisconnected");
    }

    //Set text on UI Thread
    private class SetTextRunnable implements Runnable
    {
        final String mText;

        SetTextRunnable(String text)
        {
            mText = text;
        }

        @Override
        public void run()
        {
            mTV.setText(mText);
        }
    }

    double perfTime = 0;
    double normTime = 0;
    Bitmap bitmap, normBitmap;
    int countPerf = 0;
    int countNorm = 0;

    float brightness [] = {6, (float)2.5, (float)1.5};

    //Main method for picture brightness test
    private void adjustBrightness() {

        bitmap = original.copy(Bitmap.Config.ARGB_8888, true);
        normBitmap = original.copy(Bitmap.Config.ARGB_8888, true);

        //Unperforated images with brightness factors specific for every run.
        final Handler handlerNorm = new Handler(getMainLooper());
        handlerNorm.postDelayed(new Runnable() {
            public void run() {
                normTime += brightness(normBitmap, brightness[countNorm], false);
                imageView2.setImageBitmap(normBitmap);
                if (countNorm < 2) {
                    handlerNorm.postDelayed(this, 3000);
                    normBitmap = original.copy(Bitmap.Config.ARGB_8888, true);
                    countNorm++;
                }
                else {
                    normBitmap = original.copy(Bitmap.Config.ARGB_8888, true);
                    runButton.setEnabled(true);
                    mTV.setText(mTV.getText() + "\nNormal time: " + normTime+"s");
                    normTime = 0;
                    countNorm = 0;
                }
            }
        }, 1000);

        //Perforated images with brightness 100% and changing perforation factor from the service
        //  Perforated function means, that main loop has #pragma clang loop perforate (enable)
        final Handler handler = new Handler(getMainLooper());
        handler.postDelayed(new Runnable() {
            public void run() {
                perfTime += brightness(bitmap, 10, true);
                imageView.setImageBitmap(bitmap);
                if (countPerf < 2) {
                    handlerNorm.postDelayed(this, 3000);
                    bitmap = original.copy(Bitmap.Config.ARGB_8888, true);
                    countPerf++;
                }
                else {
                    bitmap = original.copy(Bitmap.Config.ARGB_8888, true);
                    runButton.setEnabled(true);
                    mTV.setText(mTV.getText() + "\nPerforated time: " + perfTime+"s");
                    mTV.setText(mTV.getText() + "\nEnd of picture brightness demo");
                    perfTime = 0;
                    countPerf = 0;
                }
            }
        }, 1000);
    }

    /**
     * A native methods that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    //For service connection
    public native void onServiceConnected(IBinder binder);
    //For service disconnection
    public native void onServiceDisconnected();
    //For picture brightness test
    public native double brightness(Bitmap bmp, float brightness, boolean perf);
}
