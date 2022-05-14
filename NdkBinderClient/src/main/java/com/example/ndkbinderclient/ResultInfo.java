package com.example.ndkbinderclient;

public class ResultInfo {

    private double time;
    private double result;

    public ResultInfo (double time, double result){
        this.time = time;
        this.result = result;
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
}
