package com.example;

import android.os.Parcel;
import android.os.Parcelable;

public class AllPerforations implements Parcelable
{
    public final double [] result;
    public final double [] speedup;

    public AllPerforations(double [] resultIn, double [] speedupIn)
    {
        result = resultIn;
        speedup = speedupIn;
    }

    protected AllPerforations(Parcel in)
    {
        result = new double[in.readInt()];
        speedup = new double[in.readInt()];
        in.readDoubleArray(result);
        in.readDoubleArray(speedup);
    }

    public static final Creator<AllPerforations> CREATOR = new Creator<AllPerforations>()
    {
        @Override
        public AllPerforations createFromParcel(Parcel in)
        {
            return new AllPerforations(in);
        }

        @Override
        public AllPerforations[] newArray(int size)
        {
            return new AllPerforations[size];
        }
    };

    @Override
    public int describeContents()
    {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel parcel, int i)
    {
        parcel.writeInt(result.length);
        parcel.writeInt(speedup.length);
        parcel.writeDoubleArray(result);
        parcel.writeDoubleArray(speedup);
    }

    public double[] getResult() {
        return result;
    }

    public double[] getSpeedup() {
        return speedup;
    }
}