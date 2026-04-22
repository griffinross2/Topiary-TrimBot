#pragma once

#include <block.h>
#include <thread>

#ifdef __linux__
#include "libcamera/libcamera.h"
#endif

class CameraBlock : public Block {
public:
    CameraBlock();
    CameraBlock(std::string id);
    ~CameraBlock();
    void onUpdate() override;

private:
    void cameraThreadFunc();

    int m_currentCameraIndex = 0;
#ifndef __linux__
    std::unique_ptr<cv::VideoCapture> m_camera;
#else
    std::shared_ptr<libcamera::Camera> m_camera;
    std::map<libcamera::FrameBuffer *, std::vector<libcamera::Span<uint8_t>>> mapped_buffers;
    std::unique_ptr<libcamera::CameraConfiguration> m_config;
    std::unique_ptr<libcamera::FrameBufferAllocator> m_fbAlloc;

    std::vector<libcamera::Span<uint8_t>> Mmap(libcamera::FrameBuffer *buffer) const
    {
	auto item = mapped_buffers.find(buffer);
	if (item == mapped_buffers.end())
	    return {};
	return item->second;
    }
#endif
    std::thread m_cameraThread;
    cv::Mat m_internalFrame;
    bool m_newFrame = false;
    std::mutex m_frameMutex;
};
