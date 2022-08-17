package com.example.ndkbinderclient;

import static java.lang.Math.exp;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.example.AllPerforations;
import com.example.Constants;
import com.example.IMyService;
import com.example.LoopPerfFactor;

import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
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

    private static class MyServiceBinder extends IMyService.Stub
    {

        private static final int N_T1 = 50;
        private static final int N_T2 = 30;
        private int counter_N_T2;
        int N_ACCEPTED;
        int N_B;
        LinkedList<PerfIterationInfo> acceptedPerforations;
        LinkedList<PerfIterationInfo> archivedPerforations;
        double phi;
        int CUTOFF = 300;

        static Random random = new Random();

        static boolean calibrationMode = false;
        int iter;
        double [] TEMPERATURE;

        HashMap<Integer, LoopPerfFactor> currentRates;
        HashMap<Integer, LinkedList<PerfIterationInfo>> allPerforations = new HashMap<>();

        int testId, testAcc;

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

        // Method for calculating perforation factor
        @Override
        public LoopPerfFactor getPerforationFactor(int loopId, int upperValue) throws RemoteException {
            if (calibrationMode && iter == 0) {
                currentRates.putIfAbsent(loopId, new LoopPerfFactor(Integer.MAX_VALUE, 1000000));
                return new LoopPerfFactor(1, 1000000);
            }

            if (calibrationMode)
                return currentRates.get(loopId);
            else {
                if (allPerforations.containsKey(testId) &&
                        allPerforations.get(testId).get(testAcc).getFactors().containsKey(loopId))
                    return allPerforations.get(testId).get(testAcc).getFactors().get(loopId);
                else
                    return new LoopPerfFactor(1, 1000000);
            }
        }

        @Override
        public void startCalibrationMode() throws RemoteException {
            if (!calibrationMode) {
                calibrationMode = true;
                currentRates = new HashMap<>();
                iter = 0;

                acceptedPerforations = new LinkedList<>();
                archivedPerforations = new LinkedList<>();
                N_ACCEPTED = 0;
                N_B= -1;
                counter_N_T2 = 0;
                phi = 1.0;

                TEMPERATURE = new double [2];
                TEMPERATURE[0] = Double.MAX_VALUE;
                TEMPERATURE[1] = Double.MAX_VALUE;
            }
        }

        @Override
        public void setTest(int testId, int testAcc) throws RemoteException {
            this.testId = testId;
            this.testAcc = testAcc;
        }

        @Override
        public void setTestCalibration(int testId) throws RemoteException {
            this.testId = testId;
        }

        public double getStandardDeviationAccuracy(LinkedList<PerfIterationInfo> perfInfo){

            double mean = perfInfo.stream().mapToDouble(PerfIterationInfo::getAccuracy).sum() / perfInfo.size();
            double SD = Math.sqrt(perfInfo.stream().mapToDouble(n -> Math.pow(n.getAccuracy() - mean, 2)).sum() / perfInfo.size());

            return SD;

        }

        public double getStandardDeviationSpeedup(LinkedList<PerfIterationInfo> perfInfo){

            double mean = perfInfo.stream().mapToDouble(PerfIterationInfo::getSpeedup).sum() / perfInfo.size();
            double SD = Math.sqrt(perfInfo.stream().mapToDouble(n -> Math.pow(n.getSpeedup() - mean, 2)).sum() / perfInfo.size());

            return SD;

        }

        @Override
        public boolean midTestResult(double accuracy, double speedup) throws RemoteException {

            if (iter == CUTOFF)
                return true;

            PerfIterationInfo newSolution = new PerfIterationInfo(iter, speedup, accuracy, currentRates);

            if (iter ==  0){
                HashMap<Integer, LoopPerfFactor> nMap = new HashMap<>();
                for (Integer key : currentRates.keySet()) {
                    nMap.put(key, new LoopPerfFactor(1, 1000000));
                }
                PerfIterationInfo x = new PerfIterationInfo(iter, speedup, accuracy, nMap);
                archivedPerforations.add(x);
                acceptedPerforations.add(x);
                N_ACCEPTED++;

                counter_N_T2++;
                iter++;

                return false;
            }

            // ARCHIVE

            if (domintesArchivedSolution(newSolution)){
                archivedPerforations.add(newSolution);
                acceptedPerforations.add(newSolution);
                N_ACCEPTED++;
            }
            // END ARCHIVE
            // ACCEPTENCE
            else {
                double p = 1;
                PerfIterationInfo prevSolution = acceptedPerforations.get(acceptedPerforations.size() - 1);
                for (double v : TEMPERATURE) {
                    p *= exp(-(accuracy - prevSolution.getAccuracy()) / v);
                }
                if (p >= random.nextDouble()) {
                    acceptedPerforations.add(newSolution);  // ARCHIVE THE SOLUTION
                    N_ACCEPTED++;
                }
                else
                    currentRates = prevSolution.getFactors();
            }
            // END ACCEPTENCE

            // SETTING THE TEMPERATURE

            boolean retToBase = false;

            if (iter == N_T1){

                TEMPERATURE[0] = getStandardDeviationAccuracy(acceptedPerforations); // ACCURACY TEMPERATURE
                TEMPERATURE[1] = getStandardDeviationSpeedup(acceptedPerforations);  // SPEEDUP TEMPERATURE

                N_B = 2 * N_T2;
                counter_N_T2 = 0;
                retToBase = true;

            }
            else if (iter > N_T1 && (counter_N_T2 == N_T2 || N_ACCEPTED > (N_T2) * 0.4)){

                N_ACCEPTED = 0;
                counter_N_T2 = 0;

                double alphaAcc = Math.max(0.5, Math.exp(- (0.7 * TEMPERATURE[0] / getStandardDeviationAccuracy(acceptedPerforations))));    // ACCURACY TEMPERATURE
                double alphaSpe = Math.max(0.5, Math.exp(- (0.7 * TEMPERATURE[1] / getStandardDeviationSpeedup(acceptedPerforations))));     // SPEEDUP TEMPERATURE

                TEMPERATURE[0] *= alphaAcc;
                TEMPERATURE[1] *= alphaSpe;

            }

            // END TEMPERATURE

            if (retToBase || iter > N_T1 && iter % N_B == 0){ // maybe we have to have a separate counter
                // perform return to base

                double [] degreesOfIso = getDegreesOfIsolation();
                int numOfCandidates = (int) (phi * degreesOfIso.length);

                Arrays.sort(degreesOfIso);

                int startIndex = degreesOfIso.length - numOfCandidates;
                int baseIndex = random.nextInt(degreesOfIso.length-(startIndex-1)) + startIndex;

                if (baseIndex >= degreesOfIso.length)
                    baseIndex = degreesOfIso.length - 1;


                currentRates = archivedPerforations.get(baseIndex).getFactors();

                phi *= 0.9;
                N_B *= 0.9;
                if (N_B < 10)
                    N_B = 10;
            }

            currentRates = executeSimStep(currentRates);

            counter_N_T2++;
            iter++;

            return false;
        }

        private HashMap<Integer, LoopPerfFactor> executeSimStep(HashMap<Integer, LoopPerfFactor> currentRates) {

            double temperature = TEMPERATURE[0] / 2;
            temperature += TEMPERATURE[1] / 2;

            int nChange = (int) (currentRates.size() * (((double) temperature) / Integer.MAX_VALUE));

            if (nChange < 0)
                nChange = 0;
            if (nChange > currentRates.size())
                nChange = currentRates.size();

            for (int i = 0; i<nChange; i++){
                int randomIdx = random.nextInt(currentRates.size());
                if (randomIdx >= currentRates.size())
                    randomIdx = currentRates.size() - 1;
                int sub = random.nextInt((int) (Integer.MAX_VALUE * (((double) temperature) / Integer.MAX_VALUE)));
                LoopPerfFactor t = currentRates.get(currentRates.keySet().toArray()[randomIdx]);
                int subbedValue = t.perfFactor - sub;
                if (subbedValue <= 0)
                    subbedValue = 1;
                LoopPerfFactor newFactor = new LoopPerfFactor(subbedValue, t.factorLife);
                currentRates.replace(randomIdx, newFactor);
            }

            return currentRates;

        }

        private boolean domintesArchivedSolution(PerfIterationInfo newSolution) {

            int accuracy = (int) (newSolution.getAccuracy() * 100);
            int speedup = (int) (newSolution.getSpeedup() * 100);

            boolean notSeen = true;

            for (int i = 0; i<archivedPerforations.size(); i++) {
                PerfIterationInfo curArch = archivedPerforations.get(i);
                int curAccuracy = (int) (curArch.getAccuracy() * 100);
                int curSpeedup = (int) (curArch.getSpeedup() * 100);
                if (accuracy == curAccuracy){
                    notSeen = false;
                    if (speedup > curSpeedup) {
                        archivedPerforations.remove(curArch);
                        return true;
                    }
                }
                /* if (speedup == curSpeedup){
                    notSeen = false;
                    if (accuracy > curAccuracy) {
                        archivedPerforations.remove(curArch);
                        return true;
                    }
                }*/
            }

            return notSeen;

        }

        private double [] getDegreesOfIsolation(){

            double [] degrees = new double [archivedPerforations.size()];

            double min_accuracy = archivedPerforations.stream().min((entry1, entry2) ->
                            entry1.getAccuracy() <= entry2.getAccuracy() ? 1 : -1)
                    .get().getAccuracy();

            double max_accuracy = archivedPerforations.stream().max((entry1, entry2) ->
                            entry1.getAccuracy() >= entry2.getAccuracy() ? 1 : -1)
                    .get().getAccuracy();

            double min_speedup = archivedPerforations.stream().min((entry1, entry2) ->
                            entry1.getSpeedup() <= entry2.getSpeedup() ? 1 : -1)
                    .get().getSpeedup();

            double max_speedup = archivedPerforations.stream().max((entry1, entry2) ->
                            entry1.getSpeedup() >= entry2.getSpeedup() ? 1 : -1)
                    .get().getSpeedup();

            for (int i = 0; i<archivedPerforations.size(); i++){
                double degree = 0;
                double accuracy_i = archivedPerforations.get(i).getAccuracy();
                double speedup_i = archivedPerforations.get(i).getAccuracy();
                for (int j = 0; j<archivedPerforations.size(); j++){
                    if (i != j){
                        double accuracy_j = archivedPerforations.get(j).getAccuracy();
                        double speedup_j = archivedPerforations.get(j).getAccuracy();
                        degree += Math.pow((accuracy_i - accuracy_j) / (max_accuracy - min_accuracy), 2);
                        degree += Math.pow((speedup_i - speedup_j) / (max_speedup - min_speedup), 2);
                    }
                }
                degrees[i] = degree;
            }

            return degrees;

        }

        @Override
        public AllPerforations endCalibrationMode() throws RemoteException {

            allPerforations.put(testId, archivedPerforations);

            int len = archivedPerforations.size();
            double [] results = new double[len];
            double [] speedups = new double[len];

            for (int i = 0; i < len; i++) {
                PerfIterationInfo temp = archivedPerforations.get(i);
                results[i] = temp.getAccuracy() * 100;
                speedups[i] = temp.getSpeedup();
            }

            archivedPerforations = null;
            acceptedPerforations = null;
            calibrationMode = false;
            currentRates = null;
            iter = -1;
            
            return new AllPerforations(results, speedups);
        }

    }
}
