package com.example.ndkbinderclient;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.ConditionVariable;
import android.os.Environment;
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
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.example.AllPerforations;
import com.example.Constants;
import com.example.IMyService;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.DecimalFormat;
import java.util.HashMap;
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

    HashMap<Integer, AllPerforations> perforations = new HashMap<>();

    Spinner spinner;
    Spinner spinnerResults;

    int testAcc;
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
        spinnerResults = findViewById(R.id.spinnerResults);
        firstBrightnessRun = true;

        String [] options = {"Black-Scholes Perf", "Monte-Carlo Perf", "Picture Brightness Normal", "Picture Brightness Perf", "Edge Detection", "Blur Filter Perf", "Black-Scholes Norm", "Monte-Carlo Norm", "Blur Filter Norm"};

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(MainActivity.this,
                android.R.layout.simple_spinner_item, options);

        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                test = position;
                switch (test) {
                    case 0:
                    case 1:
                    case 5:
                        setTest(test);
                        break;
                    case 3:
                        setTest(2);
                        break;
                    default:
                        setTest(-1);
                        break;
                }
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
                        perfTime = 0;
                        normTime = 0;

                        boolean first = true;

                        while (!calibrated) {
                            switch (test) {
                                case 0:
                                    ResultInfo tempRes1 = talkToService(1, true);
                                    runOnUiThread(new SetTextRunnable(tempRes1.getResult() + ""));
                                    calibrated = perforationHelper.midTestResult(tempRes1.getResult(), tempRes1.getTime());
                                    TimeUnit.SECONDS.sleep(1);
                                    break;

                                case 1:
                                    ResultInfo tempRes2 = talkToService(2, true);
                                    runOnUiThread(new SetTextRunnable(tempRes2.getResult() + ""));
                                    calibrated = perforationHelper.midTestResult(tempRes2.getResult(), tempRes2.getTime());
                                    TimeUnit.SECONDS.sleep(1);
                                    break;

                                case 2:
                                    original = BitmapFactory.decodeResource(getResources(), R.drawable.breadlow);
                                    Bitmap bitmap1 = original.copy(Bitmap.Config.ARGB_8888, true);
                                    if (first) {
                                        for (int i = 0; i<10; i++) {
                                            perfTime += brightness(bitmap1, (float) 0.1, true, firstBrightnessRun);
                                            firstBrightnessRun = false;
                                        }
                                        first = false;
                                    }
                                    else {
                                        for (int i = 0; i<10; i++) {
                                            perfTime += brightness(bitmap1, (float) 0.1, true, firstBrightnessRun);
                                        }
                                    }
                                    runOnUiThread(new SetImageView(bitmap1));
                                    calibrated = perforationHelper.midTestResult(bitmap1, perfTime);
                                    runOnUiThread(new SetTextRunnable(perfTime + "s"));
                                    TimeUnit.SECONDS.sleep(1);
                                    break;

                                case 3:
                                    original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                                    Bitmap bitmap2 = original.copy(Bitmap.Config.ARGB_8888, true);
                                    if (first) {
                                        perfTime = brightness(bitmap2, (float) 1, true, firstBrightnessRun) * 10;
                                        first = false;
                                        firstBrightnessRun = false;
                                    }
                                    else {
                                        for (int i = 0; i<10; i++)
                                            perfTime += brightness(bitmap2, (float) 0.1, true, firstBrightnessRun);
                                    }
                                    runOnUiThread(new SetImageView(bitmap2));
                                    calibrated = perforationHelper.midTestResult(bitmap2, perfTime);
                                    runOnUiThread(new SetTextRunnable(perfTime + "s"));
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
                        switch (test) {
                            case 0:
                            case 1:
                            case 5:
                                perforations.put(test, perforationHelper.endCalibrationMode());
                                writeResults(test);
                                break;
                            case 2:
                                perforations.put(2, perforationHelper.endCalibrationMode());
                                writeResults(2);
                                break;
                        }
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
                            ResultInfo res = talkToService(1, true);
                            runOnUiThread(new SetTextRunnable(res.getResult() + " " + res.getTime() + "s"));
                            runOnUiThread(new SetButtonRunnable(true));
                        break;
                        //Run Monte-Carlo
                        case 1:
                            ResultInfo res1 = talkToService(2, true);
                            runOnUiThread(new SetTextRunnable(res1.getResult() + " " + res1.getTime() + "s"));
                            runOnUiThread(new SetButtonRunnable(true));
                        break;
                        //Running picture brightness test with lower resolution picture
                        case 2:
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadlow);
                            adjustBrightnessNorm();
                        break;
                        //Running picture brightness test with higher resolution picture
                        case 3:
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadlow);
                            adjustBrightnessPerf();
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
                            double perfTimeBlur = 0;
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadlow);
                            for (int i = 0; i<100; i++) {
                                //Setting working copy
                                Bitmap bitmapBlurPerf = original.copy(Bitmap.Config.ARGB_8888, true);
                                perfTimeBlur += blur(bitmapBlurPerf, 120, true);
                                //Setting perforated image
                                runOnUiThread(new SetImageView2(bitmapBlurPerf));
                            }
                            //Setting time result
                            runOnUiThread(new SetTextRunnable("Perforated time: " + perfTimeBlur + "s"));
                            runOnUiThread(new SetButtonRunnable(true));
                        break;
                        case 6:
                            ResultInfo res2 = talkToService(1, false);
                            runOnUiThread(new SetTextRunnable(res2.getResult() + " " + res2.getTime() + "s"));
                            runOnUiThread(new SetButtonRunnable(true));
                            break;
                        //Run Monte-Carlo
                        case 7:
                            ResultInfo res3 = talkToService(2, false);
                            runOnUiThread(new SetTextRunnable(res3.getResult() + " " + res3.getTime() + "s"));
                            runOnUiThread(new SetButtonRunnable(true));
                            break;
                        case 8:
                            double normTimeBlur = 0;
                            //Setting base bitmap
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadlow);
                            for (int i = 0; i<100; i++) {
                                //Setting working copy
                                Bitmap bitmapBlur = original.copy(Bitmap.Config.ARGB_8888, true);
                                normTimeBlur += blur(bitmapBlur, 120, false);
                                //Setting image on UI thread
                                runOnUiThread(new SetImageView(bitmapBlur));
                            }
                            runOnUiThread(new SetTextRunnable("Normal time: " + normTimeBlur + "s"));
                            runOnUiThread(new SetButtonRunnable(true));
                            break;
                    }
                }
            }).start();
        });

        imageView = (ImageView) findViewById(R.id.imageView);
        imageView2 = (ImageView) findViewById(R.id.imageView2);

    }

    private void writeResults(int test) {
        try
        {
            File root = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), "Results");
            if (!root.exists()) {
                root.mkdirs();
            }
            File gpxfile = new File(root, "Test.csv");
            FileWriter writer = new FileWriter(gpxfile);
            writer.append("Accuracy;Speedup\n");
            AllPerforations allPerforations = perforations.get(test);
            for (int i = 0; i<allPerforations.result.length; i++){
                String s = allPerforations.result[i] + ";" + allPerforations.speedup[i] + "\n";
                writer.append(s);
            }
            writer.flush();
            writer.close();
            //Toast.makeText(this, "Done", Toast.LENGTH_SHORT).show();
        }
        catch(IOException e)
        {
            e.printStackTrace();

        }
    }

    @Override
    protected void onResume()
    {
        super.onResume();

        //When activity is back in use, we bind back to our service,
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
    private void adjustBrightnessNorm() {
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
                if (shadowNorm < 12)
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
    }

    private void adjustBrightnessPerf() {

        Bitmap bitmap = original.copy(Bitmap.Config.ARGB_8888, true);
        final Handler handler = new Handler(getMainLooper());
        handler.postDelayed(new Runnable() {
            public void run() {
                perfTime += brightness(bitmap, (float) 0.1, true, firstBrightnessRun);
                imageView.setImageBitmap(bitmap);
                shadowPerf = (float) (shadowPerf + 0.1);
                if (shadowPerf < 12)
                    handler.postDelayed(this, 1);
                else {
                    runOnUiThread(new SetTextRunnable("\nPerforated: " + perfTime + "s"));
                    runOnUiThread(new SetButtonRunnable(true));
                }
            }
        }, 100);
        perfTime = 0;
        shadowPerf = 0;
    }

    void setTest(int testId){

        AllPerforations allPerforations = perforations.get(testId);

        if (allPerforations != null) {
            String[] options = new String[allPerforations.result.length];

            DecimalFormat df = new DecimalFormat("0.00");

            for (int i = 0; i < allPerforations.result.length; i++) {
                options[i] = df.format(allPerforations.result[i]) + " / " + df.format(allPerforations.speedup[i]);
            }

            ArrayAdapter<String> adapter = new ArrayAdapter<String>(MainActivity.this,
                    android.R.layout.simple_spinner_item, options);

            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinnerResults.setAdapter(adapter);
            spinnerResults.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                    testAcc = position;
                    try {
                        perforationHelper.setTest(testId, testAcc);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {
                    testAcc = 0;
                    try {
                        perforationHelper.setTest(testId, testAcc);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
            });
        }
        else{
            try {
                perforationHelper.setTestCalibration(test);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
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
    public native ResultInfo talkToService(int testId, boolean perf);
    //For picture brightness test
    public native double brightness(Bitmap bmp, float brightness, boolean perf, boolean first);
    //For edge detection test
    public native double edgeDetection(Bitmap bitmapBase, Bitmap bitmapToChange, boolean perf);
    //For picture blur test
    public native double blur(Bitmap bmp, int radious, boolean perf);
}
