#pragma once

#include <android/binder_status.h>

namespace aidl {
    namespace com {
        namespace example {

            class AllPerforations
            {
            public:
                double *result;
                double *speedup;

            public:
                AllPerforations() : result(nullptr), speedup(nullptr)
                {}

                AllPerforations(double *iDouble, double *i2Double)
                {
                    result = iDouble;
                    speedup = i2Double;
                }

            public:
                binder_status_t readFromParcel(const AParcel* pParcel)
                {
                    int x, y;
                    AParcel_readInt32(pParcel, &x);
                    AParcel_readInt32(pParcel, &y);
                    AParcel_readDoubleArray(pParcel, result, nullptr); //Probably not OK, but not needed
                    AParcel_readDoubleArray(pParcel, speedup, nullptr); //Probably not OK, but not needed

                    return STATUS_OK;
                }

                binder_status_t writeToParcel(AParcel* pParcel) const
                {
                    int x = sizeof(result)/sizeof(result[0]);
                    int y = sizeof(speedup)/sizeof(speedup[0]);
                    AParcel_writeInt32(pParcel, x);
                    AParcel_writeInt32(pParcel, y);
                    AParcel_writeDoubleArray(pParcel, result, x);
                    AParcel_writeDoubleArray(pParcel, speedup, y);

                    return STATUS_OK;
                }

                void getResult(double *outResult){
                    outResult = result;
                }

                void getSpeedup(double *outspeedup){
                    outspeedup = speedup;
                }
            };

        } // namespace example
    } // namespace com
} // namespace aidl
