#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "OneOCR.h"

namespace ImageUtil {

std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(size);
    file.read(reinterpret_cast<char*>(buf.data()), size);
    if (!file) buf.clear();
    return buf;
}

bool extract_bmp(const std::vector<uint8_t>& bmp, int& w, int& h, int& ch, std::vector<uint8_t>& pix) {
    if (bmp.size() < 54) return false;
    uint16_t bpp = *reinterpret_cast<const uint16_t*>(&bmp[28]);
    if (bpp != 24 && bpp != 32) return false;
    w = *reinterpret_cast<const int*>(&bmp[18]);
    h = *reinterpret_cast<const int*>(&bmp[22]);
    ch = (bpp == 24) ? 3 : 4;
    int row_bytes = ((w * ch + 3) / 4) * 4;
    pix.resize(w * h * ch);
    uint32_t offset = *reinterpret_cast<const uint32_t*>(&bmp[10]);
    for (int y = 0; y < h; ++y) {
        const uint8_t* src = &bmp[offset + (h - 1 - y) * row_bytes];
        uint8_t* dst = &pix[y * w * ch];
        std::copy(src, src + w * ch, dst);
    }
    return true;
}

void rgb_to_bgra(const std::vector<uint8_t>& rgb, std::vector<uint8_t>& bgra, int w, int h) {
    bgra.resize(w * h * 4);
    for (int i = 0; i < w * h; ++i) {
        bgra[i * 4 + 0] = rgb[i * 3 + 2];
        bgra[i * 4 + 1] = rgb[i * 3 + 1];
        bgra[i * 4 + 2] = rgb[i * 3 + 0];
        bgra[i * 4 + 3] = 255;
    }
}

bool bmp_to_bgra(const std::string& path, std::vector<uint8_t>& bgra, int& w, int& h) {
    std::vector<uint8_t> bmp = read_file(path);
    if (bmp.empty()) return false;
    int ch = 0;
    std::vector<uint8_t> pix;
    if (!extract_bmp(bmp, w, h, ch, pix)) return false;
    if (ch == 3) rgb_to_bgra(pix, bgra, w, h);
    else bgra = std::move(pix);
    return true;
}

OneOCR::Img to_img(const std::string& path, std::vector<uint8_t>& data) {
    int w = 0, h = 0;
    if (!bmp_to_bgra(path, data, w, h)) return {};
    OneOCR::Img img{};
    img.t = 3;
    img.col = w;
    img.row = h;
    img._unk = 0;
    img.step = w * 4;
    img.data_ptr = reinterpret_cast<uintptr_t>(data.data());
    return img;
}

} // namespace ImageUtil

int main() {
    std::vector<uint8_t> data;
    auto img = ImageUtil::to_img("777.bmp", data);
    if (!data.empty()) {
        OneOCR ocr;
        std::vector<OneOCR::Line> lines;
        ocr.Run(img, lines);
        for (const auto& line : lines) std::cout << line.text << std::endl;
    }
    std::cout << "Hello World!\n";
}