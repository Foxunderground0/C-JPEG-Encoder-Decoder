#include <iostream>
#include <cstdlib>
#include <array>
#include <tuple>
#include <vector>
#include "jpeg_data.h"

#define print(x) cout << "[+] " << x << endl;

using namespace std;

vector < vector<uint8_t> > quantization_tables;
vector< vector<uint8_t> > huffman_tables;

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

uint8_t count_instances(std::array<uint8_t, IMG_data_len>& arr, uint16_t data) {
	uint8_t instance_counter = 0;
	for (unsigned int i = 0; i < arr.size(); i++) {
		if (arr.at(i) == (data >> 8) && arr.at(i + 1) == (data & 0xff)) {
			instance_counter++;
		}
	}
	return instance_counter;
}

tuple<uint8_t, uint16_t, uint8_t> find_quantization_table_position_info(std::array<unsigned char, IMG_data_len>& arr, unsigned int instance = 0) {
	unsigned int instance_counter = 0;
	for (unsigned int i = 0; i < arr.size(); i++) {
		if (arr.at(i) == (0xffdb >> 8) && arr.at(i + 1) == (0xffdb & 0xff)) {
			if (instance_counter >= instance) {
				return { i + 2, arr.at(i + 2) << 8 | arr.at(i + 3), arr.at(i + 4) }; // index immedeately after tag, length, destination (luma or chroma)
			}
			instance_counter++;
		}
	}
	return { -1, -1 , -1 };
}

tuple<uint64_t, uint16_t, uint8_t, uint8_t> find_huffman_table_position_info(std::array<unsigned char, IMG_data_len>& arr, unsigned int instance = 0) {
	unsigned int instance_counter = 0;
	for (unsigned int i = 0; i < arr.size(); i++) {
		if (arr.at(i) == (0xffc4 >> 8) && arr.at(i + 1) == (0xffc4 & 0xff)) {
			if (instance_counter >= instance) {
				return { i + 2, arr.at(i + 2) << 8 | arr.at(i + 3), arr.at(i + 4) >> 4, arr.at(i + 4) & 0x0f }; // index immedeately after tag, length, class, destination (luma or chroma)
			}
			instance_counter++;
		}
	}
	return { -1, -1 , -1, -1 };
}

int main(void) {

	print("Entered");
	print("Size: " << IMG_data.size() << " bytes");
	print("Parsing");

	if (check_if_valid_img(IMG_data)) {
		print("Not a valid JPEG immage");
		return 1;
	}

	//Get Quantization Tables
	uint8_t num_of_quantization_tables = count_instances(IMG_data, 0xffdb);
	print("Number of Quantization tables found: " << static_cast<int>(num_of_quantization_tables));

	if (num_of_quantization_tables <= 0) {
		print("No Quantization tables found");
		return 1;
	}


	for (uint8_t i = 0; i < num_of_quantization_tables; i++) {
		print("Quantization table number: " << static_cast<int>(i));
		auto data = find_quantization_table_position_info(IMG_data, i);
		print("Index: " << static_cast<int>(get<0>(data)));
		print("Length: " << get<1>(data));
		print("Destination: " << static_cast<int>(get<2>(data)));

		vector<uint8_t> quantization_table_values;
		for (uint16_t i = get<0>(data) + 3; i < get<0>(data) + get<1>(data); i++) { // Loop over the quantization tables contents
			quantization_table_values.push_back(IMG_data.at(i));
		}
		quantization_tables.push_back(quantization_table_values);
	}
	cout << endl;

	for (uint16_t i = 0; i < quantization_tables.size(); ++i) {
		print("Quantization table " << static_cast<int>(i) << " contents below:");
		for (uint16_t j = 0; j < quantization_tables[i].size(); ++j) {
			if (j % 8 == 0) {
				cout << endl;
			}
			std::cout << static_cast<int>(quantization_tables[i][j]) << " ";
		}
		cout << endl;
		cout << endl;
	}

	//Get Huffman Tables
	uint8_t num_of_huffman_tables = count_instances(IMG_data, 0xffc4);
	print("Number of Huffman tables found: " << static_cast<int>(num_of_huffman_tables));

	if (num_of_quantization_tables <= 0) {
		print("No Huffman tables found");
		return 1;
	}

	for (uint8_t i = 0; i < num_of_huffman_tables; i++) {
		print("Huffman table number: " << static_cast<int>(i));
		auto data = find_huffman_table_position_info(IMG_data, i);
		print("Index: " << static_cast<int>(get<0>(data)));
		print("Length: " << get<1>(data));
		print("Class: " << static_cast<int>(get<2>(data)));
		print("Destination: " << static_cast<int>(get<3>(data)));
		print("Huffman table contents below:");
		uint16_t elements_count = 0;
		bool mode = 0;
		for (uint16_t i = get<0>(data) + 3; i < get<0>(data) + get<1>(data); i++) { // Loop over the Huffman tables contents
			if (mode == 0) {
				if ((i - (get<0>(data) + 3)) < 16) {
					cout << static_cast<int>(IMG_data.at(i)) << " ";
					elements_count += static_cast<int>(IMG_data.at(i));
				} else {
					mode = 1;
					cout << endl;
					cout << static_cast<int>(IMG_data.at(i)) << " ";
				}
			} else {
				cout << static_cast<int>(IMG_data.at(i)) << " ";
			}

		}
		cout << endl;
		print("Number of elements: " << static_cast<int>(elements_count));
		cout << endl;


	}

	return 0;
}

