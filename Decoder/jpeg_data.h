#include <array>
#include <cstdlib>

#ifndef IMG_DATA
#define IMG_DATA

const int IMG_data_len = 395;

//extern const unsigned int IMG_data_len;
extern std::array <unsigned char, IMG_data_len> IMG_data;

#endif