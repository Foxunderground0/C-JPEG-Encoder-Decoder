#include <iostream>
#include <cstdlib>
#include <array>
#include <tuple>
#include <vector>
#include <queue>
#include <unordered_map>
#include "jpeg_data.h"
#include <bitset>
#include <algorithm>

#define print(x) cout << "[+] " << x << endl;

using namespace std;


vector<vector<uint8_t>> quantization_tables;
vector<unordered_map<uint16_t, tuple<uint8_t, uint8_t>>> huffman_tables;
int8_t bit_index = 7;
uint64_t byte_index = 0;

struct components_info{
	uint8_t component_id = 0;
	uint8_t	component_height_subsampling = 0;
	uint8_t component_width_subsampling = 0;
	uint8_t component_destination = 0;
	
};

struct img_header_info{
	uint16_t height = 0;
	uint16_t width = 0;
	uint8_t number_of_components = 0;
	vector<*components_info> components_info_vector; //component number, width subsampling, height subsampling, destination
};


uint8_t get_next_bit_from_stream(std::array<uint8_t, img_data_len>& arr) {
	uint8_t bit = 0;
	bit = arr.at(byte_index);
	bit = bit >> bit_index;
	bit = 0x01 & bit;
	bit_index--;
	if (bit_index < 0) {
		byte_index++;
		bit_index = 7;
	}
	return bit;
}

bool check_if_valid_img(std::array<uint8_t, img_data_len>& arr) {
	if (((arr.at(0) << 8) | arr.at(1)) == ((0xff<<8) | SOI )) {
		print("JPEG header found")
			if (((arr.at(arr.size() - 2) << 8) | arr.at(arr.size() - 1)) == ((0xff<<8) | EOI )) {
				print("JPEG end found")
					return 0;
			}
	}
	return 1;
}

uint8_t count_instances(std::array<uint8_t, img_data_len>& arr, uint16_t data) {
	uint8_t instance_counter = 0;
	for (unsigned int i = 0; i < arr.size(); i++) {
		if (arr.at(i) == (data >> 8) && arr.at(i + 1) == (data & 0xff)) {
			instance_counter++;
		}
	}
	return instance_counter;
}

tuple<uint8_t, uint16_t, uint8_t> find_quantization_table_position_info(std::array<unsigned char, img_data_len>& arr, unsigned int instance = 0) {
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

tuple<uint64_t, uint16_t, uint8_t, uint8_t> find_huffman_table_position_info(std::array<unsigned char, img_data_len>& arr, unsigned int instance = 0) {
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

img_header_info* find_start_of_frame_info(std::array<unsigned char, img_data_len>& arr) {
	for (unsigned int i = 0; i < arr.size(); i++) { // Parse file
		if (((arr.at(i) << 8) | arr.at(i + 1)) == ((0xff<<8) | SOF0 ) && arr.at(i + 4) == (0x08)) { // Check if header is found and the image is of 8 bit per channel color depth
			img_header_info* pointer = new img_header_info;
			pointer->height = arr.at(i + 5) << 8 | arr.at(i + 6);
			pointer->width = arr.at(i + 7) << 8 | arr.at(i + 8);
			pointer->number_of_components = arr.at(i + 9);
			i = i+10; // Set the index to the start of components
			for(uint8_t j = 0; j< number_of_components; j++){
				components_info* component_pointer = new components_info;
				component_pointer->component_id = arr.at(i); // TODO
				component_pointer->component_height_subsampling = arr.at(i);
				component_pointer->component_width_subsampling = arr.at(i);
				component_pointer->component_destination = arr.at(i);
				
			}
			//, arr.at(i + 7) << 8 | arr.at(i + 8);
			return pointer;
		}
	}
	return nullptr;
}

uint64_t get_start_of_byte_stream(std::array<unsigned char, img_data_len>& arr) {
	for (unsigned int i = 0; i < arr.size(); i++) {
		if (arr.at(i) == (0xffda >> 8) && arr.at(i + 1) == (0xffda & 0xff)) {
			return  i + 2 + (arr.at(i + 2) << 8 | arr.at(i + 3)); // index immedeately after tag, Height, Width
		}
	}
	return -1;
}

uint8_t get_symbol_from_huffman_map(unordered_map<uint16_t, tuple<uint8_t, uint8_t>>& map, uint16_t& key) {
	if (map.find(key) == map.end())
		return 0xff;

	return get<0>(map[key]);
}

int main(void) {

	print("Entered");
	print("Size: " << img_data.size() << " bytes");
	print("Parsing");

	if (check_if_valid_img(img_data)) {
		print("Not a valid JPEG immage");
		return 1;
	}
	
	//Get Frame Height, Width, and info on huffman tables;
	auto frame_info = find_start_of_frame_info(img_data);

	if(frame_info == nullptr){
		print("Error making frame_info struct");
		return 1;
	}

	print("Frame Height: " << static_cast<int>(frame_info->height));
	print("Frame Width: " << static_cast<int>(frame_info->width));

	/*
	//Get Quantization Tables
	uint8_t num_of_quantization_tables = count_instances(img_data, 0xffdb);
	print("Number of Quantization tables found: " << static_cast<int>(num_of_quantization_tables));

	if (num_of_quantization_tables <= 0) {
		print("No Quantization tables found");
		return 1;
	} else {
		for (uint8_t i = 0; i < num_of_quantization_tables; i++) {
			print("Quantization table number: " << static_cast<int>(i));
			auto data = find_quantization_table_position_info(img_data, i);
			print("Index: " << static_cast<int>(get<0>(data)));
			print("Length: " << get<1>(data));
			print("Destination: " << static_cast<int>(get<2>(data)));

			vector<uint8_t> quantization_table_values;
			for (uint16_t i = get<0>(data) + 3; i < get<0>(data) + get<1>(data); i++) { // Loop over the quantization tables contents
				quantization_table_values.push_back(img_data.at(i));
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
	}

	//Get Huffman Tables
	uint8_t num_of_huffman_tables = count_instances(img_data, 0xffc4);
	print("Number of Huffman tables found: " << static_cast<int>(num_of_huffman_tables));

	if (num_of_quantization_tables <= 0) {
		print("No Huffman tables found");
		return 1;
	} else {
		for (uint8_t i = 0; i < num_of_huffman_tables; i++) {
			print("Huffman table number: " << static_cast<int>(i));
			auto data = find_huffman_table_position_info(img_data, i);
			print("Index: " << static_cast<int>(get<0>(data)));
			print("Length: " << get<1>(data));
			print("Class: " << static_cast<int>(get<2>(data)));
			print("Destination: " << static_cast<int>(get<3>(data)));
			print("Huffman table contents below:");
			uint16_t elements_count = 0;

			vector<uint8_t> length;
			vector<uint8_t> elements;
			unordered_map<uint16_t, tuple<uint8_t, uint8_t>> huffman_codes_map; // Code, Data, Code Length

			for (uint16_t i = get<0>(data) + 3; i < get<0>(data) + get<1>(data); i++) { // Loop over the Huffman tables contents
				if ((i - (get<0>(data) + 3)) < 16) {
					cout << static_cast<int>(img_data.at(i)) << " ";
					elements_count += static_cast<int>(img_data.at(i));
					length.push_back(img_data.at(i));
				} else {
					cout << static_cast<int>(img_data.at(i)) << " ";
					elements.push_back(img_data.at(i));
				}
			}

			cout << endl;
			print("Number of elements: " << static_cast<int>(elements_count));
			cout << endl;

			//Get the codes using canonical huffman encoding and store them in a hashmap
			uint8_t elements_index = 0;
			uint16_t code = 0x00;
			uint16_t elements_count_copy = elements_count;
			for (uint8_t i = 0; i < 16; i++) { //loop over the 16 elements in vector length
				while (length[i] > 0) {
					huffman_codes_map[code] = make_tuple(elements[elements_index], i + 1);
					print(static_cast<int>(i + 1) << " " << static_cast<int>(elements[elements_index]));
					code = code + 1;
					length[i]--;
					elements_index++;
					elements_count--;
				}
				code = code << 1;
			}

			if (elements_count != 0) {
				print("Error while combining length and elements");
				return 1;
			}

			if (elements_count_copy != huffman_codes_map.size()) {
				print("Map size dosent match the number of elements");
				return 1;
			}

			//Ouptput the generated codes
			for (const auto& pair : huffman_codes_map) {
				std::cout << "Key: " << bitset<16>(pair.first) << ", Value: " << static_cast<int>(get<0>(pair.second)) << ", Length: " << static_cast<int>(get<1>(pair.second)) << std::endl;
			}
			huffman_tables.push_back(huffman_codes_map);
		}
		cout << endl;
	}

	//Get Frame Height and Width
	auto frame_info = find_start_of_frame_info(img_data);
	print("Frame Height: " << static_cast<int>(get<1>(frame_info)));
	print("Frame Width: " << static_cast<int>(get<2>(frame_info)));


	uint64_t start_of_byte_stream_address = get_start_of_byte_stream(img_data);
	print(start_of_byte_stream_address);
	byte_index = start_of_byte_stream_address;

	uint16_t stream_temporary_container = 0;
	uint8_t length = 0;
	for (uint8_t i = 0; i < 8; i++) {
		stream_temporary_container = (stream_temporary_container << 1) | (get_next_bit_from_stream(img_data));
		length = get_symbol_from_huffman_map(huffman_tables[0], stream_temporary_container);
		print(bitset<16>(stream_temporary_container) << " " << (int)length);
		if (length != 0xff) {
			break;
		}
		if (i == 8) {
			print("Error reading the length of the DC coeff")
				return 1;
		}
	}

	stream_temporary_container = 0;
	for (uint8_t i = 0; i < length; i++) {
		stream_temporary_container = (stream_temporary_container << 1) | (get_next_bit_from_stream(img_data));
	}

	int8_t coefficent = (stream_temporary_container);

	if (coefficent < (1 << (length - 1))) {
		coefficent -= (1 << length) - 1;
	}

	print("DC coeff: " << (int)coefficent);


	for (uint i = 1; i < 64; ++i) {
		length = 0;
		for (uint8_t i = 0; i < 8; i++) {
			stream_temporary_container = (stream_temporary_container << 1) | (get_next_bit_from_stream(img_data));
			length = get_symbol_from_huffman_map(huffman_tables[1], stream_temporary_container);
			print(bitset<16>(stream_temporary_container) << " " << (int)length);
			if (length != 0xff) {
				break;
			}
			if (i == 8) {
				print("Error reading the length of the AC coeff")
					return 1;
			}
		}

		// symbol 0x00 means fill remainder of component with 0
		if (length == 0x00) {
			print("All Zeros Ahead");
			return true;
		}

		// otherwise, read next component coefficient
		uint8_t numZeroes = length >> 4;
		uint8_t coeffLength = length & 0x0F;
		coefficent = 0;

		if (i + numZeroes >= 64) {
			std::cout << "Error - Zero run-length exceeded block component\n";
			return false;
		}
		i += numZeroes;

		if (coeffLength > 10) {
			std::cout << "Error - AC coefficient length greater than 10\n";
			return false;
		}

		for (uint8_t i = 0; i < 8; i++) {
			stream_temporary_container = (stream_temporary_container << 1) | (get_next_bit_from_stream(img_data));
			length = get_symbol_from_huffman_map(huffman_tables[1], stream_temporary_container);
			print(bitset<16>(stream_temporary_container) << " " << (int)length);
			if (length != 0xff) {
				break;
			}
			if (i == 8) {
				print("Error reading the length of the AC coeff")
					return 1;
			}
		}

		if (length < (1 << (coeffLength - 1))) {
			length -= (1 << coeffLength) - 1;
		}
		print((uint)zigZagMap[i] << " " << length);
	}
	*/
	return 0;
}

