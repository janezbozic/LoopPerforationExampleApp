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
    private Bitmap original;
    private boolean firstBrightnessRun;

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
        firstBrightnessRun = true;

        String [] options = {"Black-Scholes", "Monte-Carlo", "Picture Brightness 640x427", "Picture Brightness 1920x1280"};

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
                        //Run Black-Scholes
                        case 0:
                            runOnUiThread(new SetTextRunnable(talkToService(1)));
                            runOnUiThread(new SetButtonRunnable(true));
                        break;
                        //Run Monte-Carlo
                        case 1:
                            runOnUiThread(new SetTextRunnable(talkToService(2)));
                            runOnUiThread(new SetButtonRunnable(true));
                        break;
                        //Running picture brightness test with lower resolution picture
                        case 2:
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadlow);
                            adjustBrightness();
                        break;
                        //Running picture brightness test with higher resolution picture
                        case 3:
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                            adjustBrightness();
                        break;
                    }
                }
            }).start();
        });

        imageView = (ImageView) findViewById(R.id.imageView);

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

    //Set button enabled/disabled on UI Thread
    private class SetButtonRunnable implements Runnable
    {
        final boolean buttonEnabled;

        SetButtonRunnable(boolean b)
        {
            buttonEnabled = b;
        }

        @Override
        public void run()
        {
            runButton.setEnabled(buttonEnabled);
        }
    }

    //Setting image on UI Thread
    private class SetImageView implements Runnable
    {
        final Bitmap image;

        SetImageView(Bitmap b)
        {
            image = b;
        }

        @Override
        public void run()
        {
            imageView.setImageBitmap(image);
        }
    }

    float shadowPerf = (float) 0.0;
    double perfTime = 0;
    Bitmap bitmap;
    int countPerf = 0;
    double dtime = 0;

    //Main method for picture brightness test
    private void adjustBrightness() {

        bitmap = original.copy(Bitmap.Config.ARGB_8888, true);

        //We do the exact thing for perforated function
        //  Perforated function means, that main loop has #pragma clang loop perforate (enable)
        final Handler handler = new Handler(getMainLooper());
        handler.postDelayed(new Runnable() {
            public void run() {
                dtime += brightness(bitmap, (float) 0.1, true, firstBrightnessRun);
                firstBrightnessRun = false;
                imageView.setImageBitmap(bitmap);
                shadowPerf = (float) (shadowPerf + 0.1);
                if (shadowPerf < 10)
                    handler.postDelayed(this, 1);
                else {
                    shadowPerf = 0;
                    perfTime = 0;
                    if (countPerf < 5) {
                        handler.postDelayed(this, 12000);
                        perfTime+=dtime;
                        mTV.setText(mTV.getText() + "\nRun: " + (countPerf+1) + "   time: " + dtime+"s");
                        dtime = 0;
                        bitmap = original.copy(Bitmap.Config.ARGB_8888, true);
                        countPerf++;
                    }
                    else {
                        bitmap = original.copy(Bitmap.Config.ARGB_8888, true);
                        runButton.setEnabled(true);
                        mTV.setText(mTV.getText() + "\nRun: " + (countPerf+1) + "   time: " + dtime+"s");
                        mTV.setText(mTV.getText() + "\nEnd of picture brightness demo");
                        countPerf = 0;
                    }
                }
            }
        }, 100);
        perfTime = 0;
        shadowPerf = 0;
    }

    /**
     * A native methods that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    //For service connection
    public native void onServiceConnected(IBinder binder);
    //For service disconnection
    public native void onServiceDisconnected();
    //For tests Black-Scholes and Monte-Carlo
    public native String talkToService(int numOfRuns);
    //For picture brightness test
    public native double brightness(Bitmap bmp, float brightness, boolean perf, boolean first);
    //For edge detection test
    public native double edgeDetection(Bitmap bitmapBase, Bitmap bitmapToChange, boolean perf);
    //For picture blur test
    public native double blur(Bitmap bmp, int radious, boolean perf);
}
