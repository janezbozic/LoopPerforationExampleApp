package com.example.ndkbinderclient;

import com.example.LoopPerfFactor;

import java.util.HashMap;

public class PerfIterationInfo {

    private int id;
    private double speedup, accuracy;
    private HashMap<Integer, LoopPerfFactor> factors;

    public PerfIterationInfo(int id, double speedup, double accuracy, HashMap<Integer, LoopPerfFactor> factors){
        this.id = id;
        this.speedup = speedup;
        this.accuracy = accuracy;
        this.factors = factors;
    }

    public int getId() {
        return id;
    }

    public double getSpeedup() {
        return speedup;
    }

    public void setSpeedup(double speedup) {
        this.speedup = speedup;
    }

    public double getAccuracy() {
        return accuracy;
    }

    public void setAccuracy(double accuracy) {
        this.accuracy = accuracy;
    }

    public HashMap<Integer, LoopPerfFactor> getFactors() {
        return factors;
    }

    public void setFactors(HashMap<Integer, LoopPerfFactor> factors) {
        this.factors = factors;
    }
}
