#ifndef __JNI__
#ifndef __CLIB__
#include <cstdio>
#include "PredString.h"
#include "version.h"
#include "OcrLite.h"
#include "OcrUtils.h"

void PredString::printHelp(FILE *out, char *argv0) {
    fprintf(out, " ------- Usage -------\n");
    fprintf(out, "%s %s", argv0, usageMsg);
    fprintf(out, " ------- Required Parameters -------\n");
    fprintf(out, "%s", requiredMsg);
    fprintf(out, " ------- Optional Parameters -------\n");
    fprintf(out, "%s", optionalMsg);
    fprintf(out, " ------- Other Parameters -------\n");
    fprintf(out, "%s", otherMsg);
    fprintf(out, " ------- Examples -------\n");
    fprintf(out, example1Msg, argv0);
    fprintf(out, example2Msg, argv0);
}
/*
 * ./${EXE_PATH}/OcrLiteOnnx --models models \
             --det dbnet.onnx \
             --cls angle_net.onnx \
             --rec crnn_lite_lstm.onnx \
             --keys keys.txt \
             --image $TARGET_IMG \
             --numThread $NUM_THREADS \
             --padding 50 \
             --maxSideLen 1024 \
             --boxScoreThresh 0.6 \
             --boxThresh 0.3 \
             --unClipRatio 2.0 \
             --doAngle 1 \
 * */
std::vector<std::string> PredString::getPredString(const cv::Mat& image) {

    std::string modelsDir, modelDetPath, modelClsPath, modelRecPath, keysPath;
    std::string imgPath, imgDir, imgName;
    std::vector<std::string> v;
    int numThread = 4;
    int padding = 50;
    int maxSideLen = 1024;
    float boxScoreThresh = 0.6f;
    float boxThresh = 0.3f;
    float unClipRatio = 2.0f;
    bool doAngle = true;
    int flagDoAngle = 1;
    bool mostAngle = true;
    int flagMostAngle = 1;
    modelsDir = "/home/ge/code/Wang/CPPOCR/OCR_Server/models";

    if (modelDetPath.empty()) {
        modelDetPath = modelsDir + "/" + "dbnet.onnx";
    }
    if (modelClsPath.empty()) {
        modelClsPath = modelsDir + "/" + "angle_net.onnx";
    }
    if (modelRecPath.empty()) {
        modelRecPath = modelsDir + "/" + "crnn_lite_lstm.onnx";
    }
    if (keysPath.empty()) {
        keysPath = modelsDir + "/" + "keys.txt";
    }



    bool hasModelDetFile = isFileExists(modelDetPath);
    if (!hasModelDetFile) {
        fprintf(stderr, "Model dbnet file not found: %s\n", modelDetPath.c_str());
        return v;
    }
    bool hasModelClsFile = isFileExists(modelClsPath);
    if (!hasModelClsFile) {
        fprintf(stderr, "Model angle file not found: %s\n", modelClsPath.c_str());
        return v;
    }
    bool hasModelRecFile = isFileExists(modelRecPath);
    if (!hasModelRecFile) {
        fprintf(stderr, "Model crnn file not found: %s\n", modelRecPath.c_str());
        return v;
    }
    bool hasKeysFile = isFileExists(keysPath);
    if (!hasKeysFile) {
        fprintf(stderr, "keys file not found: %s\n", keysPath.c_str());
        return v;
    }
    OcrLite ocrLite;
    ocrLite.setNumThread(numThread);
    ocrLite.initLogger(
            true,//isOutputConsole
            false,//isOutputPartImg
            false);//isOutputResultImg

    ocrLite.initModels(modelDetPath, modelClsPath, modelRecPath, keysPath);

    std::vector<TextLine> result = ocrLite.detect(image, padding, maxSideLen,
                                      boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle);
//    ocrLite.Logger("%s\n", result.strRes.c_str());
    v = ocrLite.getResult();
    int size = v.size();
//    Debug("PredString:size:%d",size);
//    for(int i=0;i<size;i++){
//        Debug("%s\n",v[i].c_str());
//    }

    return v;
}

#endif
#endif
