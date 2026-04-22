#include "camera_block.h"

#include "application.h"

#ifdef __linux__
#include <sys/mman.h>
#endif
#include <iostream>

#include "imgui.h"

CameraBlock::CameraBlock() : Block() {
    // Initialize io
    m_inputs.push_back({"Camera Index", int{0}, true});
    m_outputs.push_back({"Frame", cv::Mat(), true});

    // Get camera
#ifndef __linux__
    m_camera = std::make_unique<cv::VideoCapture>(m_currentCameraIndex);
#else
    std::vector<std::shared_ptr<libcamera::Camera>> cameras = Application::get()->getCameraManager()->cameras();
    if (m_currentCameraIndex >= 0 && m_currentCameraIndex < cameras.size()) {
        m_camera = cameras[m_currentCameraIndex];
        if(!m_camera->acquire()) {
            m_config = m_camera->generateConfiguration( { libcamera::StreamRole::VideoRecording } );
            m_config->at(0).pixelFormat = libcamera::formats::RGB888;
            m_config->at(0).size = libcamera::Size(1280, 720);
            m_camera->configure(m_config.get());
            m_fbAlloc = std::make_unique<libcamera::FrameBufferAllocator>(m_camera);
            for (libcamera::StreamConfiguration &cfg : *m_config) {
                m_fbAlloc->allocate(cfg.stream());

                // mmap wizardry
                for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : m_fbAlloc->buffers(cfg.stream())) {
                    // "Single plane" buffers appear as multi-plane here, but we can spot them because then
                    // planes all share the same fd. We accumulate them so as to mmap the buffer only once.
                    size_t buffer_size = 0;
                    for (unsigned i = 0; i < buffer->planes().size(); i++) {
                    const libcamera::FrameBuffer::Plane &plane = buffer->planes()[i];
                    buffer_size += plane.length;
                    if (i == buffer->planes().size() - 1 || plane.fd.get() != buffer->planes()[i + 1].fd.get()) {
                        void *memory = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);
                        mapped_buffers[buffer.get()].push_back(libcamera::Span<uint8_t>(static_cast<uint8_t *>(memory),
                                                        buffer_size));
                        buffer_size = 0;
                    }
                    }
                }
            }
            m_camera->start();
        } else {
            m_camera.reset();
        }
    }
#endif

    m_cameraThread = std::thread([this]() { this->cameraThreadFunc(); });
}

CameraBlock::CameraBlock(std::string id) : Block(id) {
    // Initialize io
    m_inputs.push_back({"Camera Index", int{0}, true});
    m_outputs.push_back({"Frame", cv::Mat(), true});

    // Get camera
#ifndef __linux__
    m_camera = std::make_unique<cv::VideoCapture>(m_currentCameraIndex);
#else
    std::vector<std::shared_ptr<libcamera::Camera>> cameras = Application::get()->getCameraManager()->cameras();
    if (m_currentCameraIndex >= 0 && m_currentCameraIndex < cameras.size()) {
        m_camera = cameras[m_currentCameraIndex];
        if(!m_camera->acquire()) {
            m_config = m_camera->generateConfiguration( { libcamera::StreamRole::VideoRecording } );
            m_config->at(0).pixelFormat = libcamera::formats::RGB888;
            m_config->at(0).size = libcamera::Size(1280, 720);
            m_camera->configure(m_config.get());
            m_fbAlloc = std::make_unique<libcamera::FrameBufferAllocator>(m_camera);
            for (libcamera::StreamConfiguration &cfg : *m_config) {
                m_fbAlloc->allocate(cfg.stream());

                // mmap wizardry
                for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : m_fbAlloc->buffers(cfg.stream())) {
                    // "Single plane" buffers appear as multi-plane here, but we can spot them because then
                    // planes all share the same fd. We accumulate them so as to mmap the buffer only once.
                    size_t buffer_size = 0;
                    for (unsigned i = 0; i < buffer->planes().size(); i++) {
                    const libcamera::FrameBuffer::Plane &plane = buffer->planes()[i];
                    buffer_size += plane.length;
                    if (i == buffer->planes().size() - 1 || plane.fd.get() != buffer->planes()[i + 1].fd.get()) {
                        void *memory = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);
                        mapped_buffers[buffer.get()].push_back(libcamera::Span<uint8_t>(static_cast<uint8_t *>(memory),
                                                        buffer_size));
                        buffer_size = 0;
                    }
                    }
                }
            }
            m_camera->start();
        } else {
            m_camera.reset();
        }
    }
#endif

    m_cameraThread = std::thread([this]() { this->cameraThreadFunc(); });
}

CameraBlock::~CameraBlock() {
    if (m_cameraThread.joinable()) {
        m_cameraThread.join();
    }
}

void CameraBlock::onUpdate() {
    Block::onUpdate();

    // Input and output frame
    m_frameMutex.lock();
    if (m_newFrame) {
        m_outputs[0].data = m_internalFrame;
        m_outputs[0].newData = true;
        m_newFrame = false;
    }
    m_frameMutex.unlock();
}

void CameraBlock::cameraThreadFunc() {
    while (true) {
        if (std::get<int>(m_inputs[0].data) != m_currentCameraIndex) {
            m_currentCameraIndex = std::get<int>(m_inputs[0].data);
#ifndef __linux__
            m_camera->open(m_currentCameraIndex);
#else
            if (m_camera) {
                m_camera->stop();
                m_camera->release();
            }
            m_camera.reset();

            std::vector<std::shared_ptr<libcamera::Camera>> cameras = Application::get()->getCameraManager()->cameras();
            if (m_currentCameraIndex >= 0 && m_currentCameraIndex < cameras.size()) {
                m_camera = cameras[m_currentCameraIndex];
                if(!m_camera->acquire()) {
                    m_config = m_camera->generateConfiguration( { libcamera::StreamRole::VideoRecording } );
                    m_config->at(0).pixelFormat = libcamera::formats::RGB888;
                    m_config->at(0).size = libcamera::Size(1280, 720);
                    m_camera->configure(m_config.get());
                    m_fbAlloc = std::make_unique<libcamera::FrameBufferAllocator>(m_camera);
                    for (libcamera::StreamConfiguration &cfg : *m_config) {
                        m_fbAlloc->allocate(cfg.stream());

                        // mmap wizardry
                        for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : m_fbAlloc->buffers(cfg.stream())) {
                            // "Single plane" buffers appear as multi-plane here, but we can spot them because then
                            // planes all share the same fd. We accumulate them so as to mmap the buffer only once.
                            size_t buffer_size = 0;
                            for (unsigned i = 0; i < buffer->planes().size(); i++) {
                            const libcamera::FrameBuffer::Plane &plane = buffer->planes()[i];
                            buffer_size += plane.length;
                            if (i == buffer->planes().size() - 1 || plane.fd.get() != buffer->planes()[i + 1].fd.get()) {
                                void *memory = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);
                                mapped_buffers[buffer.get()].push_back(libcamera::Span<uint8_t>(static_cast<uint8_t *>(memory),
                                                                buffer_size));
                                buffer_size = 0;
                            }
                            }
                        }
                    }
                    m_camera->start();
                } else {
                    m_camera.reset();
                }
            } 
#endif
        }

#ifndef __linux__
        if (!m_camera->isOpened()) {
#else
        if (!m_camera) {
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

#ifndef __linux__
        if (!m_camera->isOpened()) {
            continue;
        }

        cv::Mat frame;
        m_camera->read(frame);

        if (frame.empty()) {
            continue;
        }

#else
        if (!m_camera) {
            continue;
        }

        std::unique_ptr<libcamera::Request> req = m_camera->createRequest();
        req->addBuffer(m_config->at(0).stream(), m_fbAlloc->buffers(m_config->at(0).stream())[0].get());
        m_camera->queueRequest(req.get());

        while(req->status() == libcamera::Request::RequestPending) {}

        auto bufferPair = req->buffers().begin();
        const libcamera::Stream *stream = bufferPair->first;
		libcamera::FrameBuffer *buffer = bufferPair->second;
		libcamera::StreamConfiguration const &cfg = stream->configuration();
        
        cv::Mat frame;

        const libcamera::Request::BufferMap &buffers = req->buffers();
        for (auto bufferPair : buffers) {
            libcamera::FrameBuffer *buffer = bufferPair.second;
            libcamera::StreamConfiguration &streamConfig = m_config->at(0);
            unsigned int vw = streamConfig.size.width;
            unsigned int vh = streamConfig.size.height;
            unsigned int vstr = streamConfig.stride;
            auto mem = Mmap(buffer);
            cv::Mat thisFrame(vh,vw,CV_8UC3);
            uint ls = vw*3;
            uint8_t *ptr = mem[0].data();
            for (unsigned int i = 0; i < vh; i++, ptr += vstr) {
                memcpy(thisFrame.ptr(i),ptr,ls);
            }
            frame = thisFrame;
        }
#endif

        m_frameMutex.lock();
        m_internalFrame = frame;
        m_newFrame = true;
        m_frameMutex.unlock();
    }
}
