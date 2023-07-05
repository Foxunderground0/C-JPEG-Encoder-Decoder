#include <iostream>
#include <cstdlib>
#include <array>
#include <tuple>

#include "jpeg_data.h"

#define print(x) cout << "[+] " << x << endl;

using namespace std;

bool check_if_valid_img(std::array<uint8_t, IMG_data_len>& arr) {
	if (arr.at(0) == 0xff && arr.at(1) == 0xd8) {
		print("JPEG header found")
			if (arr.at(arr.size() - 2) == 0xff && arr.at(arr.size() - 1) == 0xd9) {
				print("JPEG end found")
					return 0;
			}
		return 2;
	}
	return 1;
}

tuple<unsigned int, uint16_t, unsigned char> find_quantization_table_position_info(std::array<unsigned char, IMG_data_len>& arr, uint16_t byte, unsigned int instance = 0) {
	unsigned int instance_counter = 0;
	for (unsigned int i = 0; i < arr.size(); i++) {
		if (arr.at(i) == (byte >> 8) && arr.at(i + 1) == (byte & 0xff)) {
			if (instance_counter >= instance) {
				return { i, arr.at(i + 2) << 8 | arr.at(i + 3), arr.at(i + 4) }; // index, length, destination
			}
			instance_counter++;
		}
	}
	return { -1, -1 , -1 };
}

int main(void) {

	print("Entered");
	print("Size: " << IMG_data.size() << " bytes");
	print("Parsing");

	if (!check_if_valid_img(IMG_data)) {
		auto data = find_quantization_table_position_info(IMG_data, 0xffc4, 1);
		print(get<0>(data));
		print(get<1>(data));
		print(get<2>(data));
	} else {
		print("Not a valid JPEG immage")
	}
	return 0;
}

