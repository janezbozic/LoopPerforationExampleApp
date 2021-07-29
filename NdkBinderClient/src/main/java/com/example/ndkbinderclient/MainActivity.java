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

        String [] options = {"Black-Scholes", "Monte-Carlo", "Picture Brightness 640x427", "Picture Brightness 1920x1280", "Edge Detection"};

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
            imageView2.setImageBitmap(null);
            new Thread(new Runnable()
            {
                @Override
                public void run()
                {
                    runOnUiThread(new SetTextRunnable("Waiting to talk to IMyService..."));

                    // Not connected to service yet?
                    while(!mIsServiceConnected)
                    {
                        mServiceConnectionWaitLock.block(); // waits for service connection
                    }

                    runOnUiThread(new SetTextRunnable("Running test..."));

                    switch (test){
                        case 0:
                            runOnUiThread(new SetTextRunnable(talkToService(1)));
                            runOnUiThread(new SetButtonRunnable(true));
                        break;
                        case 1:
                            runOnUiThread(new SetTextRunnable(talkToService(2)));
                            runOnUiThread(new SetButtonRunnable(true));
                        break;
                        case 2:
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadlow);
                            adjustBrightness();
                        break;
                        case 3:
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                            adjustBrightness();
                        break;
                        case 4:
                            original = BitmapFactory.decodeResource(getResources(), R.drawable.breadhigh);
                            Bitmap bitmapBase = original.copy(Bitmap.Config.ARGB_8888, true);
                            Bitmap bitmapToChange2 = original.copy(Bitmap.Config.ARGB_8888, true);
                            double normTime = edgeDetection(bitmapBase, bitmapToChange2, false);
                            runOnUiThread(new SetImageView2(bitmapToChange2));
                            Bitmap bitmapToChange = original.copy(Bitmap.Config.ARGB_8888, true);
                            double perfTime = edgeDetection(bitmapBase, bitmapToChange, true);
                            runOnUiThread(new SetImageView(bitmapToChange));
                            runOnUiThread(new SetTextRunnable("Normal time: " + normTime + "s\nPerforated time: " + perfTime + "s"));
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

        Intent intent = new Intent();
        intent.setClassName("com.example.ndkbinderclient",
                "com.example.ndkbinderclient.MyService");

        Log.d(Constants.LOG_TAG, "[App] [java] bindService");

        bindService(intent, this, BIND_AUTO_CREATE);
    }

    @Override
    protected void onPause()
    {
        unbindService(this);

        mIsServiceConnected = false;

        onServiceDisconnected();

        Log.d(Constants.LOG_TAG, "[App] [java] unbindService");

        super.onPause();
    }

    @Override
    public void onServiceConnected(ComponentName componentName, IBinder iBinder)
    {
        Log.d(Constants.LOG_TAG, "[App] [java] onServiceConnected");

        onServiceConnected(iBinder);

        mIsServiceConnected = true;

        mServiceConnectionWaitLock.open(); // breaks service connection waits
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName)
    {
        mIsServiceConnected = false;

        onServiceDisconnected();

        Log.d(Constants.LOG_TAG, "[App] [java] onServiceDisconnected");
    }

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

    private void adjustBrightness() {
        Bitmap bitmapNorm = original.copy(Bitmap.Config.ARGB_8888, true);
        final Handler handlerNorm = new Handler(getMainLooper());
        handlerNorm.postDelayed(new Runnable() {
            public void run() {
                if (shadowNorm == 0)
                    normTime += brightness(bitmapNorm, (float) 0.1, false, true);
                else
                    normTime += brightness(bitmapNorm, (float) 0.1, false, false);
                imageView2.setImageBitmap(bitmapNorm);
                shadowNorm = (float) (shadowNorm + 0.1);
                if (shadowNorm < 10)
                    handlerNorm.postDelayed(this, 1);
                else {
                    runOnUiThread(new SetTextRunnable("Normal: " + normTime + "s"));
                    runOnUiThread(new SetButtonRunnable(true));
                }
            }
        }, 100);
        shadowNorm = 0;
        normTime = 0;

        Bitmap bitmap = original.copy(Bitmap.Config.ARGB_8888, true);
        final Handler handler = new Handler(getMainLooper());
        handler.postDelayed(new Runnable() {
            public void run() {
                if (shadowPerf == 0)
                    perfTime += brightness(bitmap, (float) 0.1, true, true);
                else
                    perfTime += brightness(bitmap, (float) 0.1, true, false);
                imageView.setImageBitmap(bitmap);
                shadowPerf = (float) (shadowPerf + 0.1);
                if (shadowPerf < 10)
                    handler.postDelayed(this, 1);
                else
                    runOnUiThread(new SetTextRunnable(mTV.getText() + "\nPerforated: " + perfTime + "s"));
            }
        }, 100);
        perfTime = 0;
        shadowPerf = 0;
    }

    /**
     * A native methods that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native void onServiceConnected(IBinder binder);
    public native void onServiceDisconnected();
    public native String talkToService(int numOfRuns);
    public native double brightness(Bitmap bmp, float brightness, boolean perf, boolean first);
    public native double edgeDetection(Bitmap bitmapBase, Bitmap bitmapToChange, boolean perf);
}
