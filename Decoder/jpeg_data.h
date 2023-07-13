#include <array>
#include <cstdlib>
#include <cstdint>

#ifndef IMG_DATA_HEADER
#define IMG_DATA_HEADER

const int img_data_len = 395; // 614236;

const uint8_t zigZagMap[] = {
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

// Start of Frame markers, non-differential, Huffman coding
const uint8_t SOF0 = 0xC0; // Baseline DCT
const uint8_t SOF1 = 0xC1; // Extended sequential DCT
const uint8_t SOF2 = 0xC2; // Progressive DCT
const uint8_t SOF3 = 0xC3; // Lossless (sequential)

// Start of Frame markers, differential, Huffman coding
const uint8_t SOF5 = 0xC5; // Differential sequential DCT
const uint8_t SOF6 = 0xC6; // Differential progressive DCT
const uint8_t SOF7 = 0xC7; // Differential lossless (sequential)

// Start of Frame markers, non-differential, arithmetic coding
const uint8_t SOF9 = 0xC9; // Extended sequential DCT
const uint8_t SOF10 = 0xCA; // Progressive DCT
const uint8_t SOF11 = 0xCB; // Lossless (sequential)

// Start of Frame markers, differential, arithmetic coding
const uint8_t SOF13 = 0xCD; // Differential sequential DCT
const uint8_t SOF14 = 0xCE; // Differential progressive DCT
const uint8_t SOF15 = 0xCF; // Differential lossless (sequential)

// Define Huffman Table(s)
const uint8_t DHT = 0xC4;

// JPEG extensions
const uint8_t JPG = 0xC8;

// Define Arithmetic Coding Conditioning(s)
const uint8_t DAC = 0xCC;

// Restart interval Markers
const uint8_t RST0 = 0xD0;
const uint8_t RST1 = 0xD1;
const uint8_t RST2 = 0xD2;
const uint8_t RST3 = 0xD3;
const uint8_t RST4 = 0xD4;
const uint8_t RST5 = 0xD5;
const uint8_t RST6 = 0xD6;
const uint8_t RST7 = 0xD7;

// Other Markers
const uint8_t SOI = 0xD8; // Start of Image
const uint8_t EOI = 0xD9; // End of Image
const uint8_t SOS = 0xDA; // Start of Scan
const uint8_t DQT = 0xDB; // Define Quantization Table(s)
const uint8_t DNL = 0xDC; // Define Number of Lines
const uint8_t DRI = 0xDD; // Define Restart Interval
const uint8_t DHP = 0xDE; // Define Hierarchical Progression
const uint8_t EXP = 0xDF; // Expand Reference Component(s)

// APPN Markers
const uint8_t APP0 = 0xE0;
const uint8_t APP1 = 0xE1;
const uint8_t APP2 = 0xE2;
const uint8_t APP3 = 0xE3;
const uint8_t APP4 = 0xE4;
const uint8_t APP5 = 0xE5;
const uint8_t APP6 = 0xE6;
const uint8_t APP7 = 0xE7;
const uint8_t APP8 = 0xE8;
const uint8_t APP9 = 0xE9;
const uint8_t APP10 = 0xEA;
const uint8_t APP11 = 0xEB;
const uint8_t APP12 = 0xEC;
const uint8_t APP13 = 0xED;
const uint8_t APP14 = 0xEE;
const uint8_t APP15 = 0xEF;

// Misc Markers
const uint8_t JPG0 = 0xF0;
const uint8_t JPG1 = 0xF1;
const uint8_t JPG2 = 0xF2;
const uint8_t JPG3 = 0xF3;
const uint8_t JPG4 = 0xF4;
const uint8_t JPG5 = 0xF5;
const uint8_t JPG6 = 0xF6;
const uint8_t JPG7 = 0xF7;
const uint8_t JPG8 = 0xF8;
const uint8_t JPG9 = 0xF9;
const uint8_t JPG10 = 0xFA;
const uint8_t JPG11 = 0xFB;
const uint8_t JPG12 = 0xFC;
const uint8_t JPG13 = 0xFD;
const uint8_t COM = 0xFE;
const uint8_t TEM = 0x01;

//extern const unsigned int IMG_data_len;
extern std::array <uint8_t, img_data_len> img_data;

#endif