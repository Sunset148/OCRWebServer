//#define _GNU_SOURCE
#include "Buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>

Buffer::Buffer(int size):m_capacity(size)
{
    m_data = (char*)malloc(size);
    bzero(m_data, size);
}

Buffer::~Buffer()
{
    if (m_data != nullptr)
    {
        free(m_data);
    }
}

void Buffer::extendRoom(int size)
{
    // 1. 内存够用 - 不需要扩容
    if (writeableSize() >= size)
    {
        return;
    }
    // 2. 内存需要合并才够用 - 不需要扩容
    // 剩余的可写的内存 + 已读的内存 > size
    else if (m_readPos + writeableSize() >= size)
    {
        // 得到未读的内存大小
        int readable = readableSize();
        // 移动内存
        memcpy(m_data, m_data + m_readPos, readable);
        // 更新位置
        m_readPos = 0;
        m_writePos = readable;
    }
    // 3. 内存不够用 - 扩容
    else
    {
        void* temp = realloc(m_data, m_capacity + size);
        if (temp == NULL)
        {
            return; // 失败了
        }
        memset((char*)temp + m_capacity, 0, size);
        // 更新数据
        m_data = static_cast<char*>(temp);
        m_capacity += size;
    }
}

int Buffer::appendString(const char* data, int size)
{
    if (data == nullptr || size <= 0)
    {
        return -1;
    }
    // 扩容
    extendRoom(size);
//    Debug("OldData:%s",m_data);
//    Debug("m_writePos:%d | data:%s",m_writePos,data);
    // 数据拷贝
    memcpy(m_data + m_writePos, data, size);
    m_writePos += size;
//    Debug("appendChar:%s",m_data);
    return 0;
}

int Buffer::appendString(const char* data)
{
    int size = strlen(data);
    int ret = appendString(data, size);
    return ret;
}

int Buffer::appendString(const string data)
{
    int ret = appendString(data.data());
//    Debug("appendString:%s",m_data);
    return ret;
}

std::string Buffer::base64_decode(const std::string &encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

int Buffer::socketRead(int fd)
{
    // 初始化数组元素
    int writeable = writeableSize();
    char* tmpbuf = (char*)malloc(40960);
    bool readingImage = false;
    int result = -1;
    int totalResult = 0;
    char* imagePtr = nullptr;
    vector<char> img_data;
    std::string base64_image_data;

    do {
        writeable = writeableSize();
        result = read(fd, tmpbuf, 40960);
        totalResult += result;
//        Debug("result: %d\ntmpbuf: %.*s", result, result, tmpbuf);
        if (result == -1)
        {
            free(tmpbuf);
            return -1;
        }

        imagePtr = findStr(tmpbuf, "{\"image\":");
        if (readingImage || (result > 8 && imagePtr != nullptr))
        {
            int pos = 0;
            if (!readingImage && imagePtr != tmpbuf)
            {
                appendString(tmpbuf, imagePtr - tmpbuf);
                pos = imagePtr - tmpbuf + 10;
            }
            else if (!readingImage && imagePtr == tmpbuf)
            {
                pos = 10;
            }
            const char* imageEndPtr = nullptr;
            imageEndPtr = findStr(tmpbuf, "\"}");
            int endPos = imageEndPtr ? (imageEndPtr - tmpbuf) : result;
//            Debug("pos: %d, endPos: %d", pos, endPos);
            base64_image_data.append(tmpbuf+pos, endPos - pos);

            if (imageEndPtr != nullptr)
            {
                Debug("Image data end found");
                readingImage = false;
                // 解码 Base64
                std::string image_data = base64_decode(base64_image_data);
                // 将解码后的数据转换为 cv::Mat
                std::vector<uchar> data(image_data.begin(), image_data.end());
                m_image = cv::imdecode(data, cv::IMREAD_COLOR);
                if (!m_image.empty()) {
                    cv::imwrite(imagePath, m_image);  // 保存图像
                    Debug("Image received and saved.");
                    return totalResult;
                } else {
                    Debug("Failed to decode image.");
                }
            }
            readingImage = true;
        }
        else
        {
            appendString(tmpbuf, result);
//            Debug("Not Image || m_writePos: %d", m_writePos);
        }
    } while (result > 0);

    free(tmpbuf);

    return totalResult;
}


char* Buffer::findCRLF()
{
    char* ptr = (char*)memmem(m_data + m_readPos, readableSize(), "\r\n", 2);
    return ptr;
}

char* Buffer::findStr(const char* str,const char* substr)
{
    char* ptr = (char*)memmem(str, strlen(str), substr, strlen(substr));
    return ptr;
}

int Buffer::sendData(int socket)
{
    // 判断有无数据
    int readable = readableSize();
//    Debug("readable:%d  m_readPos:%d ",readable,m_readPos);
    string str(m_data + m_readPos, readable);
    Debug("sendData:%s",str.c_str());
    if (readable > 0)
    {
        int count = send(socket, m_data + m_readPos, readable, MSG_NOSIGNAL);
        if (count > 0)
        {
            m_readPos += count;
            usleep(1);
        }
        return count;
    }
    return 0;
}

