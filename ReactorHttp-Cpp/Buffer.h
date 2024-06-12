#pragma once
#include <string>
#include "opencv2/core.hpp"
#include <vector>
#include <fstream>
#include "Log.h"
#include <stdarg.h> //windows&linux
#include <onnxruntime/core/session/onnxruntime_cxx_api.h>
#include "OcrStruct.h"
#include "DbNet.h"
#include "AngleNet.h"
#include "CrnnNet.h"
#include <algorithm>

using namespace std;

class Buffer
{
public:
    Buffer(int size);
    ~Buffer();

    // 扩容
    void extendRoom(int size);
    // 得到剩余的可写的内存容量
    inline int writeableSize()
    {
        return m_capacity - m_writePos;
    }
    // 得到剩余的可读的内存容量
    inline int readableSize()
    {
        return m_writePos - m_readPos;
    }
    // 写内存 1. 直接写 2. 接收套接字数据
    int appendString(const char* data, int size);
    int appendString(const char* data);
    int appendString(const string data);
    int socketRead(int fd);
    // 根据\r\n取出一行, 找到其在数据块中的位置, 返回该位置
    char* findCRLF();

    // 根据substr取出之前内容，找到其在数据块的位置，放回该位置
    char* findStr(const char* str,const char* substr);

    // 发送数据
    int sendData(int socket);    // 指向内存的指针
    // 得到读数据的起始位置
    inline char* data()
    {
        return m_data + m_readPos;
    }
    inline int readPosIncrease(int count)
    {
        m_readPos += count;
        return m_readPos;
    }
    static inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }
    std::string base64_decode(const std::string &encoded_string);
    inline cv::Mat getImage(){
        return m_image;
    }
private:
    char* m_data;
    int m_capacity;
    int m_readPos = 0;
    int m_writePos = 0;
    cv::Mat m_image;  // 用于存储图像数据
    const string imagePath = "/home/ge/code/Wang/CPPOCR/OCR_Server/images/image.jpg";
    const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
};

