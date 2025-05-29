#include "OneOCR.h"

typedef __int64(__cdecl* CreateOcrPipeline_t)(const char*, const char*, __int64, __int64*);
typedef __int64(__cdecl* CreateOcrInitOptions_t)(__int64*);
typedef __int64(__cdecl* CreateOcrProcessOptions_t)(__int64*);
typedef __int64(__cdecl* GetOcrLineCount_t)(__int64, __int64*);
typedef __int64(__cdecl* GetOcrLine_t)(__int64, __int64, __int64*);
typedef __int64(__cdecl* GetOcrLineContent_t)(__int64, __int64*);
typedef __int64(__cdecl* GetOcrLineBoundingBox_t)(__int64, __int64*);
typedef __int64(__cdecl* GetOcrLineWordCount_t)(__int64, __int64*);
typedef __int64(__cdecl* GetOcrWord_t)(__int64, __int64, __int64*);
typedef __int64(__cdecl* GetOcrWordContent_t)(__int64, __int64*);
typedef __int64(__cdecl* GetOcrWordBoundingBox_t)(__int64, __int64*);
typedef __int64(__cdecl* OcrProcessOptionsSetMaxRecognitionLineCount_t)(__int64, __int64);
typedef __int64(__cdecl* OcrInitOptionsSetUseModelDelayLoad_t)(__int64, char);
typedef __int64(__cdecl* RunOcrPipeline_t)(__int64, OneOCR::Img*, __int64, __int64*);
typedef __int64(__cdecl* ReleaseOcrPipeline_t)(__int64);
typedef __int64(__cdecl* ReleaseOcrInitOptions_t)(__int64);
typedef __int64(__cdecl* ReleaseOcrProcessOptions_t)(__int64);
typedef void(__cdecl* ReleaseOcrResult_t)(__int64);

HINSTANCE hDLL = LoadLibraryA("oneocr.dll");
#define GetProc(Name) Name##_t Name = (Name##_t)GetProcAddress(hDLL, #Name);

GetProc(CreateOcrPipeline);
GetProc(CreateOcrInitOptions);
GetProc(CreateOcrProcessOptions);
GetProc(GetOcrLine);
GetProc(GetOcrLineContent);
GetProc(GetOcrLineCount);
GetProc(GetOcrLineBoundingBox);
GetProc(GetOcrLineWordCount);
GetProc(GetOcrWord);
GetProc(GetOcrWordContent);
GetProc(GetOcrWordBoundingBox);
GetProc(OcrInitOptionsSetUseModelDelayLoad);
GetProc(OcrProcessOptionsSetMaxRecognitionLineCount);
GetProc(RunOcrPipeline);
GetProc(ReleaseOcrResult);
GetProc(ReleaseOcrPipeline);
GetProc(ReleaseOcrInitOptions);
GetProc(ReleaseOcrProcessOptions);

std::string static ocr_utf8_to_string(const std::string& utf8_str) {
    if (utf8_str.empty()) {
        return "";
    }

    // Step 1: UTF-8 ¡ú WideChar (UTF-16)
    int wstr_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), (int)utf8_str.size(), NULL, 0);
    if (wstr_len <= 0) {
        return "";
    }

    std::vector<wchar_t> wbuffer(wstr_len);
    if (!MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), (int)utf8_str.size(), wbuffer.data(), wstr_len)) {
        return "";
    }

    // Step 2: WideChar (UTF-16) ¡ú ANSI
    int ansi_len = WideCharToMultiByte(CP_ACP, 0, wbuffer.data(), wstr_len, NULL, 0, 0, 0);
    if (ansi_len <= 0) {
        return "";
    }

    std::vector<char> abuffer(ansi_len);
    if (!WideCharToMultiByte(CP_ACP, 0, wbuffer.data(), wstr_len, abuffer.data(), ansi_len, 0, 0)) {
        return "";
    }

    return std::string(abuffer.data(), abuffer.size());
}

std::string get_executable_directory() {
    char path[MAX_PATH]{};
    if (!GetModuleFileNameA(nullptr, path, MAX_PATH)) return "";
    if (auto* slash = strrchr(path, '\\')) *(slash + 1) = '\0';
    return path;
}

OneOCR::OneOCR()
{
    auto key = std::string("kj)TGtrK>f]b[Piow.gU+nC@s\"\"\"\"\"\"4");
    auto model = get_executable_directory() + "oneocr.onemodel";

    if (hDLL)
    {
        if (SUCCEEDED(CreateOcrInitOptions(&m_ctx))) {
            if (SUCCEEDED(OcrInitOptionsSetUseModelDelayLoad(m_ctx, 0))) {
                if (SUCCEEDED(CreateOcrPipeline(model.c_str(), key.c_str(), m_ctx, &m_pipeline))) {
                    if (SUCCEEDED(OcrInitOptionsSetUseModelDelayLoad(m_ctx, 0))) {
                        if (SUCCEEDED(CreateOcrProcessOptions(&m_opt))) {
                            if (SUCCEEDED(OcrProcessOptionsSetMaxRecognitionLineCount(m_opt, 1000))) {
                                initialized = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

OneOCR::~OneOCR()
{
    if (m_opt) {
        ReleaseOcrProcessOptions(m_opt);
    }

    if (m_pipeline) {
        ReleaseOcrPipeline(m_pipeline);
    }

    if (m_ctx) {
        ReleaseOcrInitOptions(m_ctx);
    }
}

void OneOCR::Run(Img& img, std::vector<Line>& lines)
{
    if (initialized)
    {
        __int64 instance = 0;
        __int64 line_count = 0;
        if (SUCCEEDED(RunOcrPipeline(m_pipeline, &img, m_opt, &instance))) {
            if (SUCCEEDED(GetOcrLineCount(instance, &line_count))) {
                for (__int64 i = 0; i < line_count; i++) {

                    __int64 line = 0;
                    __int64 line_content = 0;
                    __int64 line_bounding_box = 0;
                    if (SUCCEEDED(GetOcrLine(instance, i, &line))) {
                        if (SUCCEEDED(GetOcrLineContent(line, &line_content))) {
                            GetOcrLineBoundingBox(line, &line_bounding_box);
                        }
                    }

                    if (line_bounding_box) {
                        Line line_data;
                        line_data.text = ocr_utf8_to_string(std::string((char*)line_content));
                        line_data.bbox = *(BoundingBox*)line_bounding_box;

                        __int64 word;
                        __int64 word_count;
                        __int64 word_content = 0;
                        __int64 word_bounding_box = 0;
                        if (SUCCEEDED(GetOcrLineWordCount(line, &word_count))) {
                            for (__int64 j = 0; j < word_count; j++) {
                                if (SUCCEEDED(GetOcrWord(line, j, &word))) {
                                    if (SUCCEEDED(GetOcrWordContent(word, &word_content))) {
                                        if (SUCCEEDED(GetOcrLineBoundingBox(line, &word_bounding_box))) {
                                            Word word_data;
                                            word_data.text = ocr_utf8_to_string(std::string((char*)word_content));
                                            word_data.bbox = *(BoundingBox*)word_bounding_box;
                                            line_data.words.push_back(word_data);
                                        }
                                    }
                                }
                            }
                        }
                        lines.push_back(line_data);
                    }
                }
            }
            ReleaseOcrResult(instance);
        }
    }
}

