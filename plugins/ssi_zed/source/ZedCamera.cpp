#include "pch.h"
#include "ZedCamera.h"

namespace ssi {

    ZedCamera::ZedCamera(const ssi_char_t* file)
        : _provider_rgb(0),
        _provider_imu(0),
        _provider_depth(0),
        _provider_skeleton(0), // ★追加
        _file(0) {

        if (file) { _file = ssi_strcpy(file); }
        memset(&_vparams, 0, sizeof(ssi_video_params_t));
    }

    ZedCamera::~ZedCamera() {
        disconnect();
        if (_file) { delete[] _file; _file = 0; }
    }

    bool ZedCamera::setProvider(const ssi_char_t* name, IProvider* provider) {
        if (!provider) return false;

        if (strcmp(name, _video_channel.getName()) == 0) {
            Lock lock(_mutex);
            _provider_rgb = provider;
            _vparams.widthInPixels = 1280;
            _vparams.heightInPixels = 720;
            _vparams.depthInBitsPerChannel = SSI_VIDEO_DEPTH_8U;
            _vparams.numOfChannels = 4;
            _vparams.framesPerSecond = 30.0;
            _provider_rgb->setMetaData(sizeof(ssi_video_params_t), &_vparams);
            ssi_stream_init(_video_channel.stream, 0, 1, 1280 * 720 * 4, SSI_IMAGE, 30.0);
            _provider_rgb->init(&_video_channel);
            return true;
        }
        else if (strcmp(name, _imu_channel.getName()) == 0) {
            Lock lock(_mutex);
            _provider_imu = provider;
            _provider_imu->init(&_imu_channel);
            return true;
        }
        else if (strcmp(name, _depth_channel.getName()) == 0) {
            Lock lock(_mutex);
            _provider_depth = provider;
            _vparams.widthInPixels = 1280;
            _vparams.heightInPixels = 720;
            _vparams.depthInBitsPerChannel = SSI_VIDEO_DEPTH_8U;
            _vparams.numOfChannels = 4;
            _vparams.framesPerSecond = 30.0;
            _provider_depth->setMetaData(sizeof(ssi_video_params_t), &_vparams);
            ssi_stream_init(_depth_channel.stream, 0, 1, 1280 * 720 * 4, SSI_IMAGE, 30.0);
            _provider_depth->init(&_depth_channel);
            return true;
        }
        else if (strcmp(name, _skeleton_channel.getName()) == 0) {
            Lock lock(_mutex);
            _provider_skeleton = provider;
            _provider_skeleton->init(&_skeleton_channel);
            return true;
        }

        return false;
    }

    bool ZedCamera::connect() {
        Lock lock(_mutex);
        if (!_provider_rgb && !_provider_imu && !_provider_depth && !_provider_skeleton) {
            ssi_wrn("no provider set");
            return false;
        }

        sl::InitParameters params;
        params.camera_resolution = sl::RESOLUTION::HD720;
        params.camera_fps = 30;
        // 深度計算は必須
        params.depth_mode = sl::DEPTH_MODE::NEURAL;
        params.coordinate_units = sl::UNIT::METER;
        params.coordinate_system = sl::COORDINATE_SYSTEM::RIGHT_HANDED_Y_UP;

        sl::ERROR_CODE err = _zed.open(params);
        if (err != sl::ERROR_CODE::SUCCESS) {
            ssi_err("ZED open failed");
            return false;
        }

        // ★公式サンプルに基づく骨格トラッキングAIの起動処理
        if (_provider_skeleton) {
            // 1. ポジショナルトラッキングの有効化 (必須)
            sl::PositionalTrackingParameters tracking_params;
            _zed.enablePositionalTracking(tracking_params);

            // 2. 骨格トラッキング(Body Tracking)の有効化
            sl::BodyTrackingParameters body_tracker_params;
            body_tracker_params.enable_tracking = true;
            body_tracker_params.enable_body_fitting = true; // 滑らかな動きにする
            body_tracker_params.body_format = sl::BODY_FORMAT::BODY_34;
            body_tracker_params.detection_model = sl::BODY_TRACKING_MODEL::HUMAN_BODY_FAST;

            sl::ERROR_CODE bt_err = _zed.enableBodyTracking(body_tracker_params);
            if (bt_err != sl::ERROR_CODE::SUCCESS) {
                ssi_err("Failed to enable Body Tracking");
                return false;
            }
        }

        _body_tracker_parameters_rt.detection_confidence_threshold = 40;

        ssi_msg(SSI_LOG_LEVEL_BASIC, "ZED connected (All Modules Ready)");
        return true;
    }

    void ZedCamera::run() {
        sl::ERROR_CODE err = _zed.grab();

        if (err != sl::ERROR_CODE::SUCCESS) {
            sleep_ms(1);
            return;
        }

        // 1. RGB
        if (_provider_rgb) {
            _zed.retrieveImage(_rgb, sl::VIEW::LEFT, sl::MEM::CPU);
            unsigned char* src = _rgb.getPtr<sl::uchar1>();
            Lock lock(_mutex);
            _provider_rgb->provide((ssi_byte_t*)src, 1);
        }

        // 2. Depth
        if (_provider_depth) {
            _zed.retrieveImage(_depth_image, sl::VIEW::DEPTH, sl::MEM::CPU);
            unsigned char* src_depth = _depth_image.getPtr<sl::uchar1>();
            Lock lock(_mutex);
            _provider_depth->provide((ssi_byte_t*)src_depth, 1);
        }

        // 3. IMU
        if (_provider_imu) {
            sl::SensorsData sensors_data;
            if (_zed.getSensorsData(sensors_data, sl::TIME_REFERENCE::IMAGE) == sl::ERROR_CODE::SUCCESS) {
                float imu_buf[6];
                imu_buf[0] = sensors_data.imu.linear_acceleration.x;
                imu_buf[1] = sensors_data.imu.linear_acceleration.y;
                imu_buf[2] = sensors_data.imu.linear_acceleration.z;
                imu_buf[3] = sensors_data.imu.angular_velocity.x;
                imu_buf[4] = sensors_data.imu.angular_velocity.y;
                imu_buf[5] = sensors_data.imu.angular_velocity.z;

                Lock lock(_mutex);
                _provider_imu->provide((ssi_byte_t*)imu_buf, 1);
            }
        }

        // 4. Skeleton (骨格)
        if (_provider_skeleton) {
            sl::Bodies bodies;
            // 102個(34関節×3次元)のゼロ配列を用意
            float skel_buf[102] = { 0.0f };

            if (_zed.retrieveBodies(bodies, _body_tracker_parameters_rt) == sl::ERROR_CODE::SUCCESS) {
                if (bodies.body_list.size() > 0) {
                    auto& body = bodies.body_list[0];
                    for (int i = 0; i < 34; i++) {
                        skel_buf[i * 3 + 0] = body.keypoint[i].x;
                        skel_buf[i * 3 + 1] = body.keypoint[i].y;
                        skel_buf[i * 3 + 2] = body.keypoint[i].z;
                    }
                }
            }

            // 人がいなくても0の波形を送り続け、SSIの同期を止めない
            Lock lock(_mutex);
            _provider_skeleton->provide((ssi_byte_t*)skel_buf, 1);
        }
    }

    bool ZedCamera::disconnect() {
        stop();
        if (_provider_skeleton) {
            _zed.disableBodyTracking();
            _zed.disablePositionalTracking();
        }
        if (_zed.isOpened()) {
            _zed.close();
        }
        Lock lock(_mutex);
        _provider_rgb = 0;
        _provider_imu = 0;
        _provider_depth = 0;
        _provider_skeleton = 0;
        return true;
    }
}