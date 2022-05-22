package com.example.ndkbinderclient;

import static java.lang.Math.abs;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.example.Constants;
import com.example.IMyService;
import com.example.LoopPerfFactor;

import java.util.HashMap;
import java.util.Random;

public class MyService extends Service
{
    private IBinder mBinder;
    private boolean calibrationMode;

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

    private static class MyServiceBinder extends IMyService.Stub
    {

        boolean calibrationMode = false;
        double perfectTime = -1.0;
        double perfectResult = -1.0;
        int iter;
        double prevResult;
        int prevIndex;
        int prevValue;
        HashMap<Integer, LoopPerfFactor> perforationRates = new HashMap<>();
        HashMap<Integer, LoopPerfFactor> currentRates;
        int CUTOFF = 100;
        double simAnealFactor;

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

        //Method for calculating perforation factor
        @Override
        public LoopPerfFactor getPerforationFactor(int loopId) throws RemoteException {
            if (calibrationMode && iter == 0) {
                currentRates.putIfAbsent(loopId, new LoopPerfFactor(Integer.MAX_VALUE, 1000000));
                return new LoopPerfFactor(1, 1000000);
            }

            if (calibrationMode)
                return currentRates.get(loopId);
            else if (perforationRates.containsKey(loopId))
                return perforationRates.get(loopId);
            else
                return new LoopPerfFactor(1, 1000000);
        }

        @Override
        public void startCalibrationMode() throws RemoteException {
            if (!calibrationMode) {
                calibrationMode = true;
                currentRates = new HashMap<>();
                iter = 0;
                simAnealFactor = 0.5;
            }
            prevResult = -1;
            prevValue = -1;
            prevIndex = -1;
        }

        @Override
        public boolean midTestResult(double result, double time) throws RemoteException {
            if (iter == CUTOFF)
                return true;

            if (iter == 0){
                perfectResult = result;
                perfectTime = time;
                iter++;
                return false;
            }

            if (result < 0.9 || result > 1.1){
                int maxElementIdx = currentRates.entrySet().stream().max((entry1, entry2) -> entry1.getValue().perfFactor >= entry2.getValue().getPerfFactor() ? 1 : -1)
                        .get().getKey();
                if (Math.random() > 0.6){
                    maxElementIdx = (Integer) currentRates.keySet().toArray()[new Random().nextInt(currentRates.keySet().toArray().length)];
                }
                LoopPerfFactor maxElement = currentRates.get(maxElementIdx);
                int tempInt = (int)(maxElement.perfFactor / 2);
                if (tempInt == 0)
                    tempInt = 1;
                double curRand = Math.random();
                if (!(result >= prevResult && curRand > simAnealFactor || result < prevResult && curRand <= simAnealFactor)){
                    LoopPerfFactor temp = new LoopPerfFactor(prevValue, 1000000);
                    currentRates.replace(prevIndex, temp);
                }
                prevIndex = maxElementIdx;
                prevValue = maxElement.perfFactor;
                prevResult = result;
                LoopPerfFactor temp = new LoopPerfFactor(tempInt, maxElement.factorLife);
                currentRates.replace(maxElementIdx, temp);
                simAnealFactor -= 0.5/CUTOFF;
                iter++;
                return false;
            }

            iter++;
            return true;
        }

        @Override
        public void endCalibrationMode() throws RemoteException {
            perforationRates.putAll(currentRates);
            calibrationMode = false;
            perfectTime = -1.0;
            perfectResult = -1.0;
            currentRates = null;
            iter = -1;
            simAnealFactor = -1;
            prevResult = -1;
            prevIndex = -1;
            prevValue = -1;
        }

    }
}
