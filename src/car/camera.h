#ifndef CAMERA_H_
#define CAMERA_H_

#include "Arduino.h"
#include "esp_camera.h"
#include "ESPAsyncWebServer.h"

typedef struct
{
    camera_fb_t *fb;
    size_t index;
} camera_frame_t;

class AsyncJpegStreamResponse: public AsyncAbstractResponse {
    private:
        camera_frame_t _frame;
        size_t _index;
        size_t _jpg_buf_len;
        uint8_t * _jpg_buf;
        uint64_t lastAsyncRequest;
    public:
        AsyncJpegStreamResponse();
        ~AsyncJpegStreamResponse();

        bool _sourceValid() const;
        virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;
        size_t _content(uint8_t *buffer, size_t maxLen, size_t index);
};

#endif