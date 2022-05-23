package com.example.ndkbinderclient;

import com.example.LoopPerfFactor;

import java.util.HashMap;

public class PerfIterationInfo {

    private int id;
    private double time, result;
    private HashMap<Integer, LoopPerfFactor> rates;

    public PerfIterationInfo(int id, double time, double result, HashMap<Integer, LoopPerfFactor> rates){
        this.id = id;
        this.time = time;
        this.result = result;
        this.rates = rates;
    }

    public int getId() {
        return id;
    }

    public double getTime() {
        return time;
    }

    public void setTime(double time) {
        this.time = time;
    }

    public double getResult() {
        return result;
    }

    public void setResult(double result) {
        this.result = result;
    }

    public HashMap<Integer, LoopPerfFactor> getRates() {
        return rates;
    }

    public void setRates(HashMap<Integer, LoopPerfFactor> rates) {
        this.rates = rates;
    }
}
