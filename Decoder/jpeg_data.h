#include <array>
#include <cstdlib>
#include <cstdint>

#ifndef IMG_DATA
#define IMG_DATA

const int IMG_data_len = 395;

//extern const unsigned int IMG_data_len;
extern std::array <uint8_t, IMG_data_len> IMG_data;

#endif