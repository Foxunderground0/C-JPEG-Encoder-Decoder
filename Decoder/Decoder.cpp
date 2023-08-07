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

using namespace std;

#define print(x) cout << "[+] " << x << endl;

vector<vector<uint8_t>> quantization_tables;
//vector<int> huffman_tables;
int8_t bit_index = 7;
uint64_t byte_index = 0;


struct mcu_container {
	int8_t values[64] = { 0 };
};

struct components_info_container {
	uint8_t component_id = 0;
	uint8_t component_width_subsampling = 0;
	uint8_t	component_height_subsampling = 0;
	uint8_t component_destination = 0;
};

struct scan_components_info_and_order_container {
	uint8_t mcu_order = 0;
	uint8_t dc_table_id = 0;
	uint8_t	ac_table_id = 0;
};

struct huffman_hashmap_container {
	uint8_t huffman_class = 0;
	uint8_t huffman_destination = 0;
	unordered_map<uint16_t, tuple<uint8_t, uint8_t>> huffman_hashmap;
};

struct quantization_table_container {
	uint8_t quantization_table_destination = 0;
	uint8_t values[64] = { 0 };
};

struct img_info {
	uint16_t height = 0;
	uint16_t width = 0;
	uint16_t number_of_mcus_per_channel = 0;
	uint8_t number_of_components = 0;
	vector<struct quantization_table_container*> quantization_table_vector;
	vector<struct components_info_container*> components_info_vector; //component number, width subsampling, height subsampling, destination
	vector<vector<struct mcu_container*>> mcu_vector;
	vector<struct huffman_hashmap_container*> huffman_vector;

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
	if (((arr.at(0) << 8) | arr.at(1)) == ((0xff << 8) | SOI)) {
		print("JPEG header found")
			if (((arr.at(arr.size() - 2) << 8) | arr.at(arr.size() - 1)) == ((0xff << 8) | EOI)) {
				print("JPEG end found")
					return 0;
			}
	}
	return 1;
}

uint8_t count_instances(std::array<uint8_t, img_data_len>& arr, uint16_t data) {
	uint8_t instance_counter = 0;
	for (uint64_t i = 0; i < arr.size(); i++) {
		if (arr.at(i) == (data >> 8) && arr.at(i + 1) == (data & 0xff)) {
			instance_counter++;
		}
	}
	return instance_counter;
}

tuple<uint8_t, uint8_t> get_symbol_from_huffman_map(unordered_map<uint16_t, tuple<uint8_t, uint8_t>>& map, uint16_t key, uint8_t length) {
	auto iter = map.find(key);
	if (iter == map.end()) {
		return make_tuple(0xff, 0xff);
	}

	auto& value = iter->second;
	if (get<1>(value) == length) {
		return value;
	}

	return make_tuple(0xff, 0xff);
}

img_info* get_frame_info(std::array<unsigned char, img_data_len>& arr) {
	for (uint64_t i = 0; i < arr.size(); i++) { // Parse file
		if (((arr.at(i) << 8) | arr.at(i + 1)) == ((0xff << 8) | SOF0) && arr.at(i + 4) == (0x08)) { // Check if header is found and the image is of 8 bit per channel color depth
			img_info* img_info_pointer = new img_info;
			img_info_pointer->height = arr.at(i + 5) << 8 | arr.at(i + 6);
			img_info_pointer->width = arr.at(i + 7) << 8 | arr.at(i + 8);
			img_info_pointer->number_of_components = arr.at(i + 9);
			i = i + 10; // Set the index to the start of components

			// Fill in component info
			for (uint8_t j = 1; j <= img_info_pointer->number_of_components; j++) {
				components_info_container* component_pointer = new components_info_container;
				component_pointer->component_id = arr.at(i);
				component_pointer->component_width_subsampling = arr.at(i + 1) >> 4;
				component_pointer->component_height_subsampling = arr.at(i + 1) & 0x0f;
				component_pointer->component_destination = arr.at(i + 2);
				img_info_pointer->components_info_vector.push_back(component_pointer);
				i += 3;
			}

			// Create MCU array
			if (((img_info_pointer->height * img_info_pointer->width) / 64) > 0xFFFFFFFFFFFFFFFF) {
				print("Too many mcus to keep track of in a uint64");
				return nullptr;
			}
			img_info_pointer->number_of_mcus_per_channel = ((img_info_pointer->height * img_info_pointer->width) / 64);

			// Create MCU array for each component
			for (uint8_t k = 0; k < img_info_pointer->number_of_components; k++) {
				vector<struct mcu_container*> mcu_per_component;
				for (uint64_t j = 0; j < img_info_pointer->number_of_mcus_per_channel; j++) {
					mcu_container* mcu_pointer = new mcu_container;
					mcu_per_component.push_back(mcu_pointer);
				}
				img_info_pointer->mcu_vector.push_back(mcu_per_component);
			}
			return img_info_pointer;
		}
	}
	return nullptr;
}

img_info* get_huffman_tables(std::array<unsigned char, img_data_len>& arr, img_info* img_info_pointer) {
	uint8_t num_of_huffman_tables = count_instances(img_data, 0xff00 | DHT);

	if (num_of_huffman_tables == 0) {
		print("No Huffman tables found");
		return nullptr;
	} else {
		//print("Number of Huffman tables found: " << static_cast<int>(num_of_huffman_tables));
		uint8_t instance_counter = 0;
		for (uint64_t i = 0; i < arr.size() - 1; i++) {
			if (((arr.at(i) << 8) | arr.at(i + 1)) == (0xff << 8 | DHT)) { // Loop over each huffman table
				//Get the raw table values
				huffman_hashmap_container* huffman_hashmap_container_pointer = new huffman_hashmap_container;
				huffman_hashmap_container_pointer->huffman_class = arr.at(i + 4) >> 4; // Class
				huffman_hashmap_container_pointer->huffman_destination = arr.at(i + 4) & 0x0f; // Destination
				uint16_t length = arr.at(i + 2) << 8 | arr.at(i + 3); // Length of huffman table
				uint16_t elements_count = 0;
				vector<uint8_t> code_length;
				vector<uint8_t> elements;

				print("Table " << (int)instance_counter + 1 << ":");
				for (uint16_t j = i + 5; j < 2 + i + length; j++) { // Loop over the Huffman tables contents
					if ((j - (i + 4)) < 16) {
						elements_count += static_cast<int>(img_data.at(j));
						code_length.push_back(img_data.at(j));
					} else {
						elements.push_back(img_data.at(j));
					}
				}
				//cout << endl << (int)elements_count << endl;
				instance_counter++;


				//Get the codes using canonical huffman encoding and store them in a hashmap
				uint8_t elements_index = 1;
				uint16_t code = 0x00;
				for (uint8_t j = 0; j < 16; j++) { //loop over the 16 elements in vector length
					while (code_length[j] > 0) {
						print(bitset<16>(code));
						huffman_hashmap_container_pointer->huffman_hashmap[code] = make_tuple(elements[elements_index], j + 1);
						code = code + 1;
						code_length[j]--;
						elements_index++;
						elements_count--;
					}
					code = code << 1;
				}

				if (elements_count != 0) {
					print("Error while combining length and elements");
					return nullptr;
				}

				//Add the new huffman_hashmap_container_pointer to the main img pointer
				img_info_pointer->huffman_vector.push_back(huffman_hashmap_container_pointer);
			}
		}
		return img_info_pointer;
	}
	return nullptr;
}

img_info* get_quantization_tables(std::array<unsigned char, img_data_len>& arr, img_info* img_info_pointer) {
	uint8_t num_of_quantization_tables = count_instances(img_data, 0xff00 | DQT);

	if (num_of_quantization_tables == 0) {
		print("No Quantization tables found");
		return nullptr;
	} else {
		print("Number of Quantization found: " << static_cast<int>(num_of_quantization_tables));
		for (uint64_t i = 0; i < arr.size() - 1; i++) { // Go through the immage searching for scans
			if (((arr.at(i) << 8) | arr.at(i + 1)) == (0xff << 8 | DQT)) { // If valid scan found
				quantization_table_container* quantization_table_container_pointer = new quantization_table_container;
				//Get Quantization Tables
				quantization_table_container_pointer->quantization_table_destination = arr.at(i + 4);
				i += 5;

				// Loop over the quantization tables contents
				for (uint8_t j = 0; j < 64; j++) {
					quantization_table_container_pointer->values[j] = arr.at(i + j);
				}
				img_info_pointer->quantization_table_vector.push_back(quantization_table_container_pointer);
			}
		}
		return img_info_pointer;
	}
	return nullptr;
}

img_info* decode_start_of_scan(std::array<unsigned char, img_data_len>& arr, img_info* img_info_pointer) {
	uint8_t num_of_scans = count_instances(img_data, 0xff00 | SOS);

	if (num_of_scans == 0) {
		print("No Scan data found");
		return nullptr;
	} else {
		print("Number of Scans found: " << static_cast<int>(num_of_scans));
		for (uint64_t i = 0; i < arr.size() - 1; i++) { // Go through the immage searching for scans
			if (((arr.at(i) << 8) | arr.at(i + 1)) == (0xff << 8 | SOS)) { // If valid scan found
				uint16_t length_of_scan_header = (arr.at(i + 2) << 8) | arr.at(i + 3); // Length of scan header
				const uint8_t number_of_componens_in_scan = arr.at(i + 4);
				uint64_t start_of_scan_bit_stream = i + length_of_scan_header + 2;
				if (number_of_componens_in_scan != img_info_pointer->number_of_components) {
					print("Components mismatch");
					return nullptr;
				}
				i += 5; // Place i to the starting position of scan's component info
				length_of_scan_header -= 6;

				vector<struct scan_components_info_and_order_container*> scan_components_info_and_order;

				for (int j = 0; j < number_of_componens_in_scan; j++) {
					scan_components_info_and_order_container* scan_components_info_and_order_container_pointer = new scan_components_info_and_order_container;
					scan_components_info_and_order_container_pointer->mcu_order = arr.at(i);
					scan_components_info_and_order_container_pointer->dc_table_id = arr.at(i + 1) >> 4;
					scan_components_info_and_order_container_pointer->ac_table_id = arr.at(i + 1) & 0x0f;
					i += 2;
					length_of_scan_header -= 2;
					scan_components_info_and_order.push_back(scan_components_info_and_order_container_pointer);
				}

				if (length_of_scan_header != 0) {
					print("Error in storing component info for scans");
					return nullptr;
				}

				if (!(arr.at(i) == 0x00 && arr.at(i + 1) == 0x3f && arr.at(i + 2) == 0x00)) {
					print("Undefined specteral select and sucessive approximation values");
					return nullptr;
				}

				byte_index = start_of_scan_bit_stream;


				//Loop for every 8x8 segment across all channels
				for (uint64_t mcu_index = 0; mcu_index < img_info_pointer->number_of_mcus_per_channel; mcu_index++) {
					uint8_t current_component = 0; // Well need to increment this in the loop to get all the components one after the other
					//print((int)component_destination_for_huffman);

					// For each component
					for (current_component = 0; current_component < number_of_componens_in_scan; current_component++) {
						//print(" ------------------------> " << (int)current_component);
						// Get the destination id of the huffman table for the channel
						uint8_t component_destination_for_huffman = img_info_pointer->components_info_vector[scan_components_info_and_order[current_component]->mcu_order - 1]->component_destination;

						unordered_map<uint16_t, tuple<uint8_t, uint8_t>>* DC;
						unordered_map<uint16_t, tuple<uint8_t, uint8_t>>* AC;
						quantization_table_container* quant;

						// Get appropriate huffman tables
						for (uint8_t j = 0; j < img_info_pointer->huffman_vector.size(); j++) {
							if (img_info_pointer->huffman_vector[j]->huffman_destination == component_destination_for_huffman && img_info_pointer->huffman_vector[j]->huffman_class == 0) {
								print("DC Matched: " << (int)j);
								DC = &img_info_pointer->huffman_vector.at(j)->huffman_hashmap;
							}

							if (img_info_pointer->huffman_vector[j]->huffman_destination == component_destination_for_huffman && img_info_pointer->huffman_vector[j]->huffman_class == 1) {
								print("AC Matched: " << (int)j);
								AC = &img_info_pointer->huffman_vector.at(j)->huffman_hashmap;
							}
						}

						// Get appropriate quantization table
						for (uint8_t j = 0; j < img_info_pointer->quantization_table_vector.size(); j++) {
							if (img_info_pointer->quantization_table_vector[j]->quantization_table_destination == component_destination_for_huffman) {
								print("Quantization table Matched: " << (int)j);
								quant = img_info_pointer->quantization_table_vector.at(j);
							}

						}


						// Get DC length
						uint16_t temporay_stream_containter = 0;
						uint8_t temporay_stream_length = 0;
						uint8_t length_dc = 0;

						for (uint8_t j = 0; j < 8; j++) {
							temporay_stream_containter = (temporay_stream_containter << 1) | (get_next_bit_from_stream(img_data));
							temporay_stream_length += 1;
							length_dc = get<0>(get_symbol_from_huffman_map(*DC, temporay_stream_containter, temporay_stream_length));
							//print(bitset<16>(temporay_stream_containter) << " " << (int)length_dc);
							if (length_dc != 0xff) {
								break;
							}
							if (j == 8) {
								print("Error reading the length of the DC coeff")
									return nullptr;
							}
						}

						if (length_dc > 11) {
							print("Error - DC coefficient length greater than 11");
							return nullptr;
						}

						//Get DC value
						temporay_stream_containter = 0;
						temporay_stream_length = 0;
						int16_t value_dc = 0;

						for (uint8_t j = 0; j < 8; j++) {
							temporay_stream_containter = (temporay_stream_containter << 1) | (get_next_bit_from_stream(img_data));
							temporay_stream_length += 1;
							value_dc = get<0>(get_symbol_from_huffman_map(*DC, temporay_stream_containter, temporay_stream_length));
							//print(bitset<16>(temporay_stream_containter) << " " << (int)value_dc);
							if (value_dc != 0xff) {
								break;
							}
							if (j == 8) {
								print("Error reading the value of the DC coeff")
									return nullptr;
							}
						}

						if (length_dc != 0 && value_dc < (1 << (length_dc - 1))) {
							value_dc -= (1 << length_dc) - 1;
						}

						img_info_pointer->mcu_vector[current_component][mcu_index]->values[zigZagMap[0]] = value_dc;

						for (uint8_t mcu_coefficent_index = 1; mcu_coefficent_index < 64; ++mcu_coefficent_index) {
							//print("HERE     : " << (int)mcu_coefficent_index);
							// Get DC length
							temporay_stream_containter = 0;
							temporay_stream_length = 0;
							uint8_t symbol_ac = 0;

							for (uint8_t j = 0; j < 8; j++) {
								temporay_stream_containter = (temporay_stream_containter << 1) | (get_next_bit_from_stream(img_data));
								temporay_stream_length += 1;
								symbol_ac = get<0>(get_symbol_from_huffman_map(*AC, temporay_stream_containter, temporay_stream_length));
								//print(bitset<16>(temporay_stream_containter) << " " << (int)symbol_ac);
								if (symbol_ac != 0xff) {
									break;
								}
								if (j == 8) {
									print("Error reading the length of the AC coeff")
										return nullptr;
								}
							}

							if (symbol_ac == 0x00) {
								print("All remaining values are 0"); // Debug
								break;
							}

							uint8_t numZeroes = symbol_ac >> 4;
							uint8_t coeffLength = symbol_ac & 0x0F;

							if (mcu_coefficent_index + numZeroes >= 64) {
								print("Zero run-length exceeded block componnt by: " << (int)numZeroes);
								break; // Temporary fix, THIS basically IGNORES THE LAST AC VALUE
								return nullptr;
							}

							mcu_coefficent_index += numZeroes;

							if (coeffLength > 10) {
								print("AC coefficient length greater than 10");
								return nullptr;
							}

							int16_t coeff = 0;

							for (size_t j = 0; j < coeffLength; j++) {
								coeff = (coeff << 1) | (get_next_bit_from_stream(img_data));
							}

							if (coeff < (1 << (coeffLength - 1))) {
								coeff -= (1 << coeffLength) - 1;
							}

							//print("coeff: " << (int)coeff);
							img_info_pointer->mcu_vector[current_component][mcu_index]->values[zigZagMap[mcu_coefficent_index]] = coeff;
						}

						// Dequantize the coeffs

						for (uint8_t j = 0; j < 64; j++) {
							cout << (int)img_info_pointer->mcu_vector[current_component][mcu_index]->values[zigZagMap[j]] << ", ";
							img_info_pointer->mcu_vector[current_component][mcu_index]->values[j] = img_info_pointer->mcu_vector[current_component][mcu_index]->values[j] * quant->values[j];
						}
						cout << endl << endl << endl;
					}
				}
			}
		}
		return img_info_pointer;
	}
	return nullptr;
}

int main(void) {

	print("Entered");
	print("Size: " << img_data.size() << " bytes");
	print("Parsing");

	if (check_if_valid_img(img_data)) {
		print("Not a valid JPEG immage");
		return 1;
	}

	//Get Frame Height, Width, and set up other relevent info on huffman tables;
	auto img_info = get_frame_info(img_data);

	if (img_info == nullptr) {
		print("Error making frame_info struct");
		return 1;
	}

	//Debuging
	{
		print("Frame Height: " << static_cast<int>(img_info->height));
		print("Frame Width: " << static_cast<int>(img_info->width));
		print("Number of components: " << static_cast<int>(img_info->components_info_vector.size()));

		for (uint8_t i = 0; i < (uint8_t)img_info->components_info_vector.size(); i++) {
			print("Component info (id, width subsample, height subsample, destination): " << static_cast<int>(img_info->components_info_vector[i]->component_id) << " "
				<< static_cast<int>(img_info->components_info_vector[i]->component_width_subsampling) << " "
				<< static_cast<int>(img_info->components_info_vector[i]->component_height_subsampling) << " "
				<< static_cast<int>(img_info->components_info_vector[i]->component_destination));
		}

		print("Number of MCUs: " << static_cast<int>(img_info->mcu_vector.size()));
	}

	//Get Huffman Tables
	img_info = get_huffman_tables(img_data, img_info);

	if (img_info == nullptr) {
		print("Error adding huffman tables");
		return 1;
	}

	//Debuging
	{
		print("Number of Huffman tables: " << static_cast<int>(img_info->huffman_vector.size()));
		for (uint8_t i = 0; i < (uint8_t)img_info->huffman_vector.size(); i++) {
			print("Table: " << static_cast<int>(i));
			print("Huffman class: " << static_cast<int>(img_info->huffman_vector[i]->huffman_class));
			print("Huffman destination: " << static_cast<int>(img_info->huffman_vector[i]->huffman_destination));

			//Output the generated codes
			for (const auto& pair : img_info->huffman_vector[i]->huffman_hashmap) {
				std::cout << "Key: " << bitset<16>(pair.first) << ", Value: " << static_cast<int>(get<0>(pair.second)) << ", Length: " << static_cast<int>(get<1>(pair.second)) << std::endl;
			}
		}
	}

	//Get quantization tables
	img_info = get_quantization_tables(img_data, img_info);

	if (img_info == nullptr) {
		print("Error getting quantization tables");
		return 1;
	}

	//Debuging
	{
		for (auto container : img_info->quantization_table_vector) {

			print("Quantization table " << (int)container->quantization_table_destination << " contents below:");
			for (uint16_t j = 0; j < 64; ++j) {
				if (j % 8 == 0) {
					cout << endl;
				}
				std::cout << static_cast<int>(container->values[j]) << " ";
			}
			cout << endl << endl;
		}
	}


	//Decode Start of scans
	img_info = decode_start_of_scan(img_data, img_info);

	if (img_info == nullptr) {
		print("Error decoding MCUs");
		return 1;
	}

	//Debuging
	{
		for (auto component : img_info->mcu_vector) {
			int j = 0;
			for (auto mcus_containers : component) {
				//print(endl << "MCU " << j);
				cout << "[";
				j++;
				for (auto a : mcus_containers->values) {
					cout << (signed short)a << " ,";
				}
				cout << "],";

			}
		}
	}

	return 0;
}

