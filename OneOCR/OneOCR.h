#pragma once
#include <Windows.h>
#include <string>
#include <vector>

class OneOCR
{
public:

    struct Img
    {
        __int32 t;        //写死为3 (BGRA)
        __int32 col;      //宽度
        __int32 row;      //高度
        __int32 _unk;     //写死为0
        __int64 step;     //col * 4
        __int64 data_ptr; //像素数据指针
    };

    struct BoundingBox
    {
        float x1;
        float y1;
        float x2;
        float y2;
        float x3;
        float y3;
        float x4;
        float y4;
    };

    struct Word
    {
        std::string text;
        BoundingBox bbox;
    };

    struct Line
    {
        std::string text;
        BoundingBox bbox;
        std::vector<Word> words;
    };
public:
    OneOCR();
	~OneOCR();
    void Run(Img& img, std::vector<Line>& lines);
private:
    bool initialized = false;
    __int64 m_ctx = 0;
    __int64 m_opt = 0;
    __int64 m_pipeline = 0;
};

