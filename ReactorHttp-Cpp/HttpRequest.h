#pragma once
#include "Buffer.h"
#include <stdbool.h>
#include "HttpResponse.h"
#include <map>
#include <vector>
#include "PredString.h"
#include "opencv2/core.hpp"

using namespace std;

// 当前的解析状态
enum class PrecessState:char
{
    ParseReqLine,
    ParseReqHeaders,
    ParseReqBody,
    ParseReqDone
};
// 定义http请求结构体
class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();
    // 重置
    void reset();
    // 添加请求头
    void addHeader(const string key, const string value);
    // 根据key得到请求头的value
    string getHeader(const string key);
    // 解析请求行
    bool parseRequestLine(Buffer* readBuf);
    // 解析请求头
    bool parseRequestHeader(Buffer* readBuf);
    // 解析请求体
    bool parseRequestBody(Buffer* readBuf);
    // 解析http请求协议
    bool parseHttpRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket);
    // 处理http请求协议
    bool processHttpRequest(HttpResponse* response);
    // 解码字符串
    string decodeMsg(string from);
    const string getFileType(const string name);
    static void sendDir(string dirName, Buffer* sendBuf, int cfd);
    static void sendFile(string dirName, Buffer* sendBuf, int cfd);
    static void sendJson(string JsonData, Buffer* sendBuf,int cfd);
    inline void setMethod(string method)
    {
        m_method = method;
    }
    inline void seturl(string url)
    {
        m_url = url;
    }
    inline void setVersion(string version)
    {
        m_version = version;
    }
    // 获取处理状态
    inline PrecessState getState()
    {
        return m_curState;
    }
    inline void setState(PrecessState state)
    {
        m_curState = state;
    }


private:
    char* splitRequestLine(const char* start, const char* end,
        const char* sub, function<void(string)> callback);
    int hexToDec(char c);

private:
    std::string m_method;
    std::string m_url;
    std::string m_version;
    map<string, string> m_reqHeaders;
    PrecessState m_curState;
    cv::Mat m_image;
    vector<string> m_requestContent={"idRecognition","digitalRecognition","bankIdentification"};
};


class IDRecognition{
public:
    IDRecognition(std::vector<std::string>& reconResult){
        m_nameMap["m_name"] = "姓名";
        m_nameMap["m_gender"] = "性别";
        m_nameMap["m_nationality"] = "民族";
        m_nameMap["m_birth"] = "出生";
        m_nameMap["m_address"] = "住址";
        m_nameMap["m_id"] = "公民身份号码";
        m_nameMap["m_authority"] = "签发机关";
        m_nameMap["m_issueDate"] = "签发日期";
        m_nameMap["m_expirationDate"] = "失效日期";


        int size = reconResult.size();
        size_t position = reconResult[0].find("中华");
        //国徽一面/正面
        if(position != std::string::npos){
            parseFront(reconResult);
        }else{
            parseBack(reconResult);
        }
    }
    ~IDRecognition(){};

    void parseFront(std::vector<std::string>& reconResult){
        //一个汉字移动3位
        //签发机关
        m_authority = reconResult[2].substr(12);

        m_issueDate = reconResult[3].substr(12,10);
        m_expirationDate = reconResult[3].substr(23);
//        Debug("m_authority:%s",m_authority.c_str());
//        Debug("m_issueDate:%s",m_issueDate.c_str());
//        Debug("m_expirationDate:%s",m_expirationDate.c_str());
        toFrontJson();
//        Debug("Json:%s",json.c_str());
    }

    void parseBack(std::vector<std::string>& reconResult) {
        m_name = reconResult[0].substr(6);
        m_gender = reconResult[1].substr(6,3);
        m_nationality =reconResult[1].substr(15);
        m_birth = reconResult[2].substr(6);
        m_address = reconResult[3].substr(6);
        for(int i=4;i<reconResult.size();i++){
            if(reconResult[i].find("身份") == std::string::npos)
                m_address += reconResult[i];
            else
                m_id = reconResult[i].substr(18);
        }
        toBackJson();
    }

    string toJson(){
        return json;
    }

    void toBackJson(){
        json = "{ \"words_result\": {";
        json += "\"" + m_nameMap["m_name"] + "\": \"" + m_name + "\",";
        json += "\"" + m_nameMap["m_gender"] + "\": \"" + m_gender + "\",";
        json += "\"" + m_nameMap["m_nationality"] + "\": \"" + m_nationality + "\",";
        json += "\"" + m_nameMap["m_birth"] + "\": \"" + m_birth + "\",";
        json += "\"" + m_nameMap["m_address"] + "\": \"" + m_address + "\",";
        json += "\"" + m_nameMap["m_id"] + "\": \"" + m_id + "\"";
        json += " } }";
    }

    void toFrontJson(){
        json = "{ \"words_result\": {";
        json += "\"" + m_nameMap["m_authority"] + "\": \"" + m_authority + "\",";
        json += "\"" + m_nameMap["m_issueDate"] + "\": \"" + m_issueDate + "\",";
        json += "\"" + m_nameMap["m_expirationDate"] + "\": \"" + m_expirationDate + "\"";
        json += " } }";
    }
private:
    string m_name;
    string m_gender;
    string m_nationality;
    string m_birth;
    string m_address;
    string m_id;
    string m_authority;
    string m_issueDate;
    string m_expirationDate;
    string json;
    map<string,string> m_nameMap;
    bool frontFlag = false;
    bool backFlag = false;
};

class BankIdentification{
public:
    BankIdentification(std::vector<std::string>& reconResult){
        int size = reconResult.size();
        for(int i=0;i<size;i++){
            int len = reconResult[i].size();
            if(len<8)
                continue;
            bool flag = true;
            Debug("i=%d len=%d",i,len);
            for(int j=0;j<len;j++) {
                if (('0' > reconResult[i][j] || reconResult[i][j] > '9')&& reconResult[i][j] !=' ') {
                    flag = false;
                    break;
                }
//                if (check(reconResult[i][j])) {
//
//                    char c[3];
//                    strncpy(&c[0], &reconResult[i][j], 3);
//                    Debug("中文:%s",c);
//                    flag = false;
//                    break;
//                }
            }
            Debug("flag=%d reconResult=%s",flag,reconResult[i].c_str());
            if(flag){
                m_bankId += reconResult[i];
                Debug("m_bankId:%s",m_bankId);
                break;
            }
        }
    }
    ~BankIdentification(){

    }
    string toJson(){
        m_json = "{ \"result\": {";
        m_json += "\"" + string("bank_card_number") + "\": \"" + m_bankId + "\"";
        m_json += " } }";
        return m_json;
    }
private:
    bool check(unsigned char c){
        //通过utf-8字节码进行判断
        return c >= 0xE0;
    }
    string m_bankId;
    string m_json;
};

class DigitalRecognition{
public:
    DigitalRecognition(std::vector<std::string>& reconResult){
        m_recong = reconResult[0];
    }
    ~DigitalRecognition(){

    }
    string toJson(){
        m_json = "{ \"words_result\": {";
        m_json += "\"" + string("words") + "\": \"" + m_recong + "\"";
        m_json += " } }";
        return m_json;
    }

private:
    string m_recong;
    string m_json;
};