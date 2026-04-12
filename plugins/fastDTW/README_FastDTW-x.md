FastDTW-x
=========

C++ porting of Stan Salvador's FastDTW

Usage:
iOS,use CocoaPods,add line in podfile:
pod 'FastDTW-x'
Other platform,Import all source files in Classes folder.

Sample code:

    #include "DTW.h"
    #include "FastDTW.h"
    #include "EuclideanDistance.h"
    #include <iostream>

    using namespace fastdtw;

    extern double *sample1;
    extern double *sample2;
    extern int sampleLength;
    
    void testDTW()
    {
        TimeSeries<double,1> tsI;
        for (int i = 0; i<sampleLength; ++i) {
            tsI.addLast(i, TimeSeriesPoint<double,1>(sample1+i));
        }
    
        TimeSeries<double,1> tsJ;
        for (int i = 0;i<sampleLength; ++i)
        {
            tsJ.addLast(i, TimeSeriesPoint<double,1>(sample2+i));
        }
    
        TimeWarpInfo<double> info =  STRI::getWarpInfoBetween(tsI,tsJ,EuclideanDistance());
        printf("Warp Distance by DTW:%lf\n",info.getDistance());
        info.getPath()->print(std::cout);
    }

    void testFastDTW()
    {
        TimeSeries<double,1> tsI;
        for (int i = 0; i<sampleLength; ++i) {
            tsI.addLast(i, TimeSeriesPoint<double,1>(sample1+i));
        }
    
        TimeSeries<double,1> tsJ;
        for (int i = 0;i<sampleLength; ++i)
        {
            tsJ.addLast(i, TimeSeriesPoint<double,1>(sample2+i));
        }
    
        TimeWarpInfo<double> info =  FAST::getWarpInfoBetween(tsI,tsJ,EuclideanDistance());
        printf("Warp Distance by DTW:%lf\n",info.getDistance());
        info.getPath()->print(std::cout);
    }

