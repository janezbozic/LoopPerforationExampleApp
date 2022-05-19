package com.example.ndkbinderclient;

import android.graphics.Bitmap;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.example.IMyService;

import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.imgproc.Imgproc;
import static org.opencv.core.CvType.CV_32F;
import static java.lang.Math.abs;

public class PerforationHelper {

    private IMyService service;
    private boolean firstRun;
    private double perfectResult;
    private Bitmap perfectPic;
    private double perfectTime;

    public PerforationHelper(){
        OpenCVLoader.initDebug();
    }

    public void connectService(IBinder iBinder){
        this.service = IMyService.Stub.asInterface(iBinder);
    }

    public void disconnectService(){
        this.service = null;
    }

    public void startCalibrationMode() throws RemoteException {
        service.startCalibrationMode();
        firstRun = true;
    }

    public boolean midTestResult(Bitmap perforatedPic, double time) throws RemoteException {
        if (firstRun){
            perfectPic = perforatedPic;
            perfectTime = time;
            firstRun = false;
            service.midTestResult(1, time);
            return false;
        }
        double result = compareBitmaps(perfectPic, perforatedPic);
        return service.midTestResult(result, time);
    }

    public boolean midTestResult(double result, double time) throws RemoteException {
        if (firstRun){
            perfectResult = result;
            perfectTime = time;
            firstRun = false;
        }
        double retResult = 1-abs(result-perfectResult) / perfectResult;
        return service.midTestResult(retResult, time);
    }

    public void endCalibrationMode() throws RemoteException {
        service.endCalibrationMode();
    }

    private double compareBitmaps(Bitmap image1, Bitmap image2) {
        Mat originalImage = new Mat();
        Mat changedImage = new Mat();
        Utils.bitmapToMat(image1, originalImage);
        Utils.bitmapToMat(image2, changedImage);
        Scalar similarity = calculateSSIM(originalImage, changedImage);
        Log.d("SSIM_Debug", similarity.toString());
        double sim = Math.sqrt(
                Math.pow(similarity.val[0], 2) +
                        Math.pow(similarity.val[0], 2) +
                        Math.pow(similarity.val[0], 2) +
                        Math.pow(similarity.val[0], 2)
        );

        return sim;
    }

    private Scalar calculateSSIM(Mat i1, Mat i2) {
        Scalar C1 = new Scalar(6.5025, 6.5025, 6.5025, 6.5025);
        Scalar C2 = new Scalar(58.5225, 58.5225, 58.5225, 58.5225);

        int d = CV_32F;

        Mat I1 = new Mat();
        Mat I2 = new Mat();
        i1.convertTo(I1, d);
        i2.convertTo(I2, d);


        Mat I2_2 = I2.mul(I2);
        Mat I1_2 = I1.mul(I1);
        Mat I1_I2 = I1.mul(I2);

        Mat mu1 = new Mat();
        Mat mu2 = new Mat();

        Imgproc.GaussianBlur(I1, mu1, new Size(11, 11), 1.5);
        Imgproc.GaussianBlur(I2, mu2, new Size(11, 11), 1.5);

        Mat mu1_2   =   mu1.mul(mu1);
        Mat mu2_2   =   mu2.mul(mu2);
        Mat mu1_mu2 =   mu1.mul(mu2);

        Mat sigma1_2 = new Mat();
        Mat sigma2_2 = new Mat();
        Mat sigma12 = new Mat();


        Imgproc.GaussianBlur(I1_2, sigma1_2, new Size(11, 11), 1.5);
        Core.subtract(sigma1_2, mu1_2, sigma1_2);
        Imgproc.GaussianBlur(I2_2, sigma2_2, new Size(11, 11), 1.5);
        Core.subtract(sigma2_2, mu2_2, sigma2_2);
        Imgproc.GaussianBlur(I1_I2, sigma12, new Size(11, 11), 1.5);
        Core.subtract(sigma12, mu1_mu2, sigma12);

        Mat t1 = new Mat();
        Mat t2 = new Mat();
        Mat t3 = new Mat();
        Scalar two = new Scalar(2.0, 2.0, 2.0, 2.0);

        Core.multiply(mu1_mu2, two, t1);
        Core.add(t1, C1, t1);
        Core.multiply(sigma12, two, t2);
        Core.add(t2, C2, t2);
        t3 = t1.mul(t2);

        Core.add(mu1_2, mu2_2, t1);
        Core.add(t1, C1, t1);
        Core.add(sigma1_2, sigma2_2, t2);
        Core.add(t2, C2, t2);
        t1 = t1.mul(t2);

        Mat ssim_map = new Mat();
        Core.divide(t3, t1, ssim_map);

        Scalar mssim = Core.mean(ssim_map);

        return mssim;
    }

}
