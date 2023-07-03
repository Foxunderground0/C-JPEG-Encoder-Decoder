#include <iostream>
#include <cstdlib>
#include <array>

#include "jpeg_data.h"

#define print(x) cout << "[+] " << x << endl;

using namespace std;

bool check_if_valid_img(std::array<unsigned char, IMG_data_len>& arr) {
	if (arr.at(0) == 0xFF && arr.at(1) == 0xD8) {
		print("JPEG header found")
			if (arr.at(arr.size() - 2) == 0xFF && arr.at(arr.size() - 1) == 0xD9) {
				print("JPEG end found")
					return 0;
			}
		return 2;
	}
	return 1;
}

int main(void) {

	print("Entered");
	print("Size: " << IMG_data.size() << " bytes");
	print("Parsing");

	check_if_valid_img(IMG_data);
	return 0;
}

