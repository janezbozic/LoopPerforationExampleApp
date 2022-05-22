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
import android.os.RemoteException;
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
import com.example.IMyService;

import java.util.concurrent.TimeUnit;

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
    private Button calibrateButton;

    private ImageView imageView;
    private ImageView imageView2;
    private Bitmap original;
    private boolean firstBrightnessRun;

    PerforationHelper perforationHelper;


    Spinner spinner;

    int test = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        perforationHelper = new PerforationHelper();

        mTV = findViewById(R.id.sample_text);
        runButton = findViewById(R.id.runButton);
        calibrateButton = findViewById(R.id.calibrateButton);

        spinner = findViewById(R.id.spinner);
        firstBrightnessRun = true;

        String [] options = {"Black-Scholes", "Monte-Carlo", "Picture Brightness 640x427", "Picture Brightness 1920x1280", "Edge Detection", "Blur Filter"};

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

        calibrateButton.setOnClickListener(v -> {
            runButton.setEnabled(false);
            calibrateButton.setEnabled(false);
            imageView.setImageBitmap(null);
            imageView2.setImageBitmap(null);

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

                    try {
                        perforationHelper.startCalibrationMode();
                        boolean calibrated = false;

                        runOnUiThread(new SetTextRunnable("Calibrating loops..."));

                        boolean first = true;

                        while (!calibrated) {
                            switch (test) {
                                case 0:
                                    ResultInfo tempRes1 = talkToService(1);
                                    runOnUiThread(new SetTextRunnable(tempRes1.getResult() + ""));
                                    calibrated = perforationHelper.midTestResult(tempRes1.getResult(), tempRes1.getTime());
                                    TimeUnit.SECONDS.sleep(1);
                                    break;

                                case 1:
                                    ResultInfo tempRes2 = talkToService(2);
                                    runOnUiThread(new SetTextRunnable(tempRes2.getResult() + ""));
                                    calibrated = perforationHelper.midTestResult(tempRes2.getResult(), tempRes2.getTime());
                                    TimeUnit.SECONDS.sleep(1);
                                    break;

                                case 2:
                                    original = BitmapFactory.decodeResource(getResources(), R.drawable.breadlow);
                                    Bitmap bitmap1 = original.copy(Bitmap.Config.ARGB_8888, true);
                                    perfTime = 0;
                                    if (first) {
                                        perfTime = brightness(bitmap1, (float) 1, true, firstBrightnessRun) * 10;
                                        first = false;
                                    }
                                    else {
                                        for (int i = 0; i<10; i++)
                                            perfTime += brightness(bitmap1, (float) 0.1, true, firstBrightnessRun);
                                    }
                                    runOnUiThread(new SetImageView(bitmap1));
                                    firstBrightnessRun = false;
                                    calibrated = perforationHelper.midTestResult(bitmap1, perfTime);
                                    runOnUiThread(new SetTextRunnable(perfTime + "s"));
                                    perfTime = 0;
                                    TimeUnit.SECONDS.sleep(1);
                                    break;

                                case 3:
                                    original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                                    Bitmap bitmap2 = adjustBrightness();
                                    perfTime = 0;
                                    if (first) {
                                        perfTime = brightness(bitmap2, (float) 1, true, firstBrightnessRun) * 10;
                                        first = false;
                                    }
                                    else {
                                        for (int i = 0; i<10; i++)
                                            perfTime += brightness(bitmap2, (float) 0.1, true, firstBrightnessRun);
                                    }
                                    runOnUiThread(new SetImageView(bitmap2));
                                    firstBrightnessRun = false;
                                    calibrated = perforationHelper.midTestResult(bitmap2, perfTime);
                                    runOnUiThread(new SetTextRunnable(perfTime + "s"));
                                    perfTime = 0;
                                    TimeUnit.SECONDS.sleep(1);
                                    break;

                                case 4:
                                    original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                                    //Setting working copy and reference
                                    Bitmap bitmapBase = original.copy(Bitmap.Config.ARGB_8888, true);
                                    Bitmap bitmapToChange = original.copy(Bitmap.Config.ARGB_8888, true);
                                    double perfTime = edgeDetection(bitmapBase, bitmapToChange, true);
                                    //Setting perforated image
                                    runOnUiThread(new SetImageView(bitmapToChange));
                                    //Setting time result
                                    runOnUiThread(new SetTextRunnable("Perforated time: " + perfTime + "s"));
                                    calibrated = perforationHelper.midTestResult(bitmapToChange, perfTime);
                                    TimeUnit.SECONDS.sleep(1);
                                    break;

                                case 5:
                                    //Setting base bitmap
                                    original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                                    //Setting working copy
                                    Bitmap bitmapBlurPerf = original.copy(Bitmap.Config.ARGB_8888, true);
                                    double perfTimeBlur = blur(bitmapBlurPerf, 120, true);
                                    //Setting perforated image
                                    runOnUiThread(new SetImageView2(bitmapBlurPerf));
                                    //Setting time result
                                    runOnUiThread(new SetTextRunnable("Perforated time: " + perfTimeBlur + "s"));
                                    calibrated = perforationHelper.midTestResult(bitmapBlurPerf, perfTimeBlur);
                                    TimeUnit.SECONDS.sleep(1);
                                    break;

                            }

                        }
                        perforationHelper.endCalibrationMode();
                    } catch (RemoteException | InterruptedException e) {
                        e.printStackTrace();
                    }

                    runOnUiThread(new SetButtonRunnable(true));
                    runOnUiThread(new SetTextRunnable("Calibrated for test."));
                }
            }).start();

        });

        runButton.setOnClickListener((v)->{

            runButton.setEnabled(false);
            imageView.setImageBitmap(null);
            imageView2.setImageBitmap(null);
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
                            runOnUiThread(new SetTextRunnable(talkToService(1).getResult() + ""));
                            runOnUiThread(new SetButtonRunnable(true));
                        break;
                        //Run Monte-Carlo
                        case 1:
                            runOnUiThread(new SetTextRunnable(talkToService(2).getResult() + ""));
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
                        //Running edge detection test
                        case 4:
                            //Setting base bitmap
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                            //Setting working copy and reference
                            Bitmap bitmapBase = original.copy(Bitmap.Config.ARGB_8888, true);
                            Bitmap bitmapToChange2 = original.copy(Bitmap.Config.ARGB_8888, true);
                            //Running non-perforated
                            double normTime = edgeDetection(bitmapBase, bitmapToChange2, false);
                            //Setting image on UI thread
                            runOnUiThread(new SetImageView2(bitmapToChange2));
                            //Setting new working copy
                            Bitmap bitmapToChange = original.copy(Bitmap.Config.ARGB_8888, true);
                            //Running perforated test
                            double perfTime = edgeDetection(bitmapBase, bitmapToChange, true);
                            //Setting perforated image
                            runOnUiThread(new SetImageView(bitmapToChange));
                            //Setting time result
                            runOnUiThread(new SetTextRunnable("Normal time: " + normTime + "s\nPerforated time: " + perfTime + "s"));
                            runOnUiThread(new SetButtonRunnable(true));
                        break;
                        case 5:
                            //Setting base bitmap
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                            //Setting working copy
                            Bitmap bitmapBlur = original.copy(Bitmap.Config.ARGB_8888, true);
                            double normTimeBlur = blur(bitmapBlur, 120, false);
                            //Setting image on UI thread
                            runOnUiThread(new SetImageView(bitmapBlur));
                            //Setting working copy
                            Bitmap bitmapBlurPerf = original.copy(Bitmap.Config.ARGB_8888, true);
                            double perfTimeBlur = blur(bitmapBlurPerf, 120, true);
                            //Setting perforated image
                            runOnUiThread(new SetImageView2(bitmapBlurPerf));
                            //Setting time result
                            runOnUiThread(new SetTextRunnable("Normal time: " + normTimeBlur + "s\nPerforated time: " + perfTimeBlur + "s"));
                            runOnUiThread(new SetButtonRunnable(true));
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

        perforationHelper.disconnectService();

        Log.d(Constants.LOG_TAG, "[App] [java] unbindService");

        super.onPause();
    }

    @Override
    public void onServiceConnected(ComponentName componentName, IBinder iBinder)
    {
        Log.d(Constants.LOG_TAG, "[App] [java] onServiceConnected");

        //Call to native function for connecting to the service
        onServiceConnected(iBinder);

        perforationHelper.connectService(iBinder);

        mIsServiceConnected = true;

        mServiceConnectionWaitLock.open(); // breaks service connection waits
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName)
    {
        mIsServiceConnected = false;

        //Call to native function for disconnecting from the service
        onServiceDisconnected();
        perforationHelper.disconnectService();

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
            calibrateButton.setEnabled(buttonEnabled);
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

    //Setting image on UI Thread
    private class SetImageView2 implements Runnable
    {
        final Bitmap image;

        SetImageView2(Bitmap b)
        {
            image = b;
        }

        @Override
        public void run()
        {
            imageView2.setImageBitmap(image);
        }
    }

    float shadowNorm = (float) 0.0;
    double normTime = 0;

    float shadowPerf = (float) 0.0;
    double perfTime = 0;

    //Main method for picture brightness test
    private Bitmap adjustBrightness() {
        Bitmap bitmapNorm = original.copy(Bitmap.Config.ARGB_8888, true);
        final Handler handlerNorm = new Handler(getMainLooper());
        //We create handler and post picture for every increment in brightness, so the users see progress
        //  The delay is not added to run time for the test
        handlerNorm.postDelayed(new Runnable() {
            public void run() {
                normTime += brightness(bitmapNorm, (float) 0.1, false, firstBrightnessRun);
                firstBrightnessRun = false;
                //Every time we set image, so the users see the difference of every increment
                imageView2.setImageBitmap(bitmapNorm);
                shadowNorm = (float) (shadowNorm + 0.1);
                if (shadowNorm < 10)
                    handlerNorm.postDelayed(this, 1);
                else {
                    //At the end we output test time results
                    runOnUiThread(new SetTextRunnable("Normal: " + normTime + "s"));
                    runOnUiThread(new SetButtonRunnable(true));
                }
            }
        }, 100);
        shadowNorm = 0;
        normTime = 0;

        //We do the exact thing for perforated function
        //  Perforated function means, that main loop has #pragma clang loop perforate (enable)
        Bitmap bitmap = original.copy(Bitmap.Config.ARGB_8888, true);
        final Handler handler = new Handler(getMainLooper());
        handler.postDelayed(new Runnable() {
            public void run() {
                perfTime += brightness(bitmap, (float) 0.1, true, firstBrightnessRun);
                imageView.setImageBitmap(bitmap);
                shadowPerf = (float) (shadowPerf + 0.1);
                if (shadowPerf < 10)
                    handler.postDelayed(this, 1);
                else {
                    runOnUiThread(new SetTextRunnable(mTV.getText() + "\nPerforated: " + perfTime + "s"));
                }
            }
        }, 100);
        perfTime = 0;
        shadowPerf = 0;

        return bitmap;
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
    public native ResultInfo talkToService(int testId);
    //For picture brightness test
    public native double brightness(Bitmap bmp, float brightness, boolean perf, boolean first);
    //For edge detection test
    public native double edgeDetection(Bitmap bitmapBase, Bitmap bitmapToChange, boolean perf);
    //For picture blur test
    public native double blur(Bitmap bmp, int radious, boolean perf);
}
