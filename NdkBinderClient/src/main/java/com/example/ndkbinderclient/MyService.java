package com.example.ndkbinderclient;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

import com.example.Constants;
import com.example.IMyService;
import com.example.LoopPerfFactor;

import java.util.Random;

public class MyService extends Service
{
    private IBinder mBinder;

    @Override
    public void onCreate()
    {
        super.onCreate();

        mBinder = new MyServiceBinder();
    }

    @Override
    public IBinder onBind(Intent intent)
    {
        Log.d(Constants.LOG_TAG, "[MyService] [java] A client binds the service");

        return mBinder;
    }

    private class MyServiceBinder extends IMyService.Stub
    {
        @Override
        public void basicTypes(int anInt, long aLong, boolean aBoolean, float aFloat,
                               double aDouble, String aString) throws RemoteException
        {
            StringBuilder str = new StringBuilder();
            str.append("[MyService] [java] basicTypes : ")
                .append("int=").append(anInt)
                .append(", long=").append(aLong)
                .append(", boolean=").append(aBoolean)
                .append(", float=").append(aFloat)
                .append(", double=").append(aDouble)
                .append(", string=").append(aString);
            Log.d(Constants.LOG_TAG, str.toString());
        }

        int perforationFactor = 0;

        //Method for calculating perforation factor
        @Override
        public LoopPerfFactor getPerforationFactor(int loopId) throws RemoteException {
            if (loopId == 105440720) //Specific for the build
                perforationFactor++;
            LoopPerfFactor t = new LoopPerfFactor(perforationFactor, 12000000);
            Handler handler = new Handler(Looper.getMainLooper());
            handler.post(new Runnable() {

                @Override
                public void run() {
                    Toast.makeText(getBaseContext(),"Perforation factor is: " + perforationFactor,Toast.LENGTH_SHORT).show();
                }
            });
            return t;
        }
    }
}
