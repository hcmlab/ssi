#pragma once

#ifndef SSI_ZED_CAMERA_H
#define SSI_ZED_CAMERA_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "thread/Lock.h"
#include "base/IOptions.h"
#include "SSI_Cons.h"

#include <sl/Camera.hpp>

namespace ssi {

    class ZedCamera : public ISensor, public Thread {

    public:
        // 1. RGBチャンネル
        class VideoChannel : public IChannel {
            friend class ZedCamera;
        public:
            VideoChannel() { ssi_stream_init(stream, 0, 0, 0, SSI_IMAGE, 0); }
            ~VideoChannel() { ssi_stream_destroy(stream); }
            const ssi_char_t* getName() { return "zed_rgb"; }
            const ssi_char_t* getInfo() { return "ZED RGB image stream."; }
            ssi_stream_t getStream() { return stream; }
        protected:
            ssi_stream_t stream;
        };

        // 2. IMUチャンネル
        class ImuChannel : public IChannel {
            friend class ZedCamera;
        public:
            ImuChannel() { ssi_stream_init(stream, 0, 6, 4, SSI_FLOAT, 30.0); }
            ~ImuChannel() { ssi_stream_destroy(stream); }
            const ssi_char_t* getName() { return "zed_imu"; }
            const ssi_char_t* getInfo() { return "ZED IMU stream."; }
            ssi_stream_t getStream() { return stream; }
        protected:
            ssi_stream_t stream;
        };

        // 3. 深度(Depth)チャンネル
        class DepthChannel : public IChannel {
            friend class ZedCamera;
        public:
            DepthChannel() { ssi_stream_init(stream, 0, 0, 0, SSI_IMAGE, 0); }
            ~DepthChannel() { ssi_stream_destroy(stream); }
            const ssi_char_t* getName() { return "zed_depth"; }
            const ssi_char_t* getInfo() { return "ZED Depth visual stream."; }
            ssi_stream_t getStream() { return stream; }
        protected:
            ssi_stream_t stream;
        };

        // 4. 骨格(Skeleton)チャンネル
        class SkeletonChannel : public IChannel {
            friend class ZedCamera;
        public:
            SkeletonChannel() {
                // 関節34個 × XYZ(3次元) = 102次元、1次元あたり4バイト(Float)、30FPS
                ssi_stream_init(stream, 0, 102, 4, SSI_FLOAT, 30.0);
            }
            ~SkeletonChannel() { ssi_stream_destroy(stream); }
            const ssi_char_t* getName() { return "zed_skeleton"; }
            const ssi_char_t* getInfo() { return "ZED Skeleton stream (34 keypoints * 3D)."; }
            ssi_stream_t getStream() { return stream; }
        protected:
            ssi_stream_t stream;
        };

    public:
        static const ssi_char_t* GetCreateName() { return "ZedCamera"; }
        static IObject* Create(const ssi_char_t* file) { return new ZedCamera(file); }

        ZedCamera(const ssi_char_t* file = 0);
        virtual ~ZedCamera();

        IOptions* getOptions() { return 0; }
        const ssi_char_t* getName() { return GetCreateName(); }
        const ssi_char_t* getInfo() { return "ZED camera sensor plugin (RGB, IMU, Depth, Skeleton)."; }

        // 出口の数を「4」に変更
        ssi_size_t getChannelSize() { return 4; }

        IChannel* getChannel(ssi_size_t index) {
            if (index == 0) return &_video_channel;
            if (index == 1) return &_imu_channel;
            if (index == 2) return &_depth_channel;
            if (index == 3) return &_skeleton_channel;
            return 0;
        }

        virtual const void* getMetaData(ssi_size_t& size) {
            size = sizeof(_vparams);
            return &_vparams;
        }

        bool setProvider(const ssi_char_t* name, IProvider* provider);
        bool connect();
        bool start() { return Thread::start(); }
        void run();
        bool stop() { return Thread::stop(); }
        bool disconnect();

    protected:
        VideoChannel _video_channel;
        ImuChannel _imu_channel;
        DepthChannel _depth_channel;
        SkeletonChannel _skeleton_channel; // ★骨格用

        ssi_video_params_t _vparams;
        Mutex _mutex;

        IProvider* _provider_rgb;
        IProvider* _provider_imu;
        IProvider* _provider_depth;
        IProvider* _provider_skeleton; // ★骨格用

        sl::Camera _zed;
        sl::Mat _rgb;
        sl::Mat _depth_image;
        sl::BodyTrackingRuntimeParameters _body_tracker_parameters_rt;
        ssi_char_t* _file;
    };
}
#endif