import jpegio as jio


def extract_dct_coefficients(image_path):
    # Read the JPEG image using jpegio
    jpeg = jio.read(image_path)

    # Organize DCT coefficients per MCU
    mcu_coefficients = []
    for component in jpeg.coef_arrays:
        num_blocks_v, num_blocks_h = component.shape
        for i in range(num_blocks_v):
            for j in range(num_blocks_h):
                mcu_coefficients.append(component[i, j])

    return mcu_coefficients


def main():
    # Replace with the path to your image
    image_path = r"Test Immages\8x8.jpg"

    # Extract DCT coefficients
    dct_coefficients = extract_dct_coefficients(image_path)

    # Display the DCT coefficients
    for i, mcu in enumerate(dct_coefficients):
        # print(f"MCU {i}:")
        print(f'{mcu},')


if __name__ == "__main__":
    main()
