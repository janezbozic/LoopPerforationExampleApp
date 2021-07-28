package com.example;

import android.os.Parcel;
import android.os.Parcelable;

public class LoopPerfFactor implements Parcelable
{
    public final int perfFactor;
    public final int factorLife;

    public LoopPerfFactor(int perfFactorIn, int factorLifeIn)
    {
        perfFactor = perfFactorIn;
        factorLife = factorLifeIn;
    }

    protected LoopPerfFactor(Parcel in)
    {
        perfFactor = in.readInt();
        factorLife = in.readInt();
    }

    public static final Creator<LoopPerfFactor> CREATOR = new Creator<LoopPerfFactor>()
    {
        @Override
        public LoopPerfFactor createFromParcel(Parcel in)
        {
            return new LoopPerfFactor(in);
        }

        @Override
        public LoopPerfFactor[] newArray(int size)
        {
            return new LoopPerfFactor[size];
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
        parcel.writeInt(perfFactor);
        parcel.writeInt(factorLife);
    }

    public int getPerfFactor (){
        return perfFactor;
    }

    public int getLifeTime(){
        return factorLife;
    }
}
