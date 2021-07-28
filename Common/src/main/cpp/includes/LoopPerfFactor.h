#pragma once

#include <android/binder_status.h>

namespace aidl {
namespace com {
namespace example {

class LoopPerfFactor
{
public:
    int perfFactor;
    int factorLife;

public:
    LoopPerfFactor() : perfFactor(-1), factorLife(-1)
    {}

    LoopPerfFactor(int iInt, int i2Int)
    {
        perfFactor = iInt;
        factorLife = i2Int;
    }

public:
    binder_status_t readFromParcel(const AParcel* pParcel)
    {
        AParcel_readInt32(pParcel, &perfFactor);
        AParcel_readInt32(pParcel, &factorLife);

        return STATUS_OK;
    }

    binder_status_t writeToParcel(AParcel* pParcel) const
    {

        AParcel_writeInt32(pParcel, perfFactor);

        AParcel_writeInt64(pParcel, factorLife);

        return STATUS_OK;
    }

    void getPerfFactor(int* outFactor){
        *outFactor = perfFactor;
    }

    void getLifeTime(int *outTime){
        *outTime = factorLife;
    }
};

} // namespace example
} // namespace com
} // namespace aidl
