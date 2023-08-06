import numpy as np
import jpegio as jio
import matplotlib.pyplot as plt
from scipy.fftpack import idct


def inverse_dct_2d(block):
    # Perform 2D inverse DCT on an 8x8 block
    return np.round(idct(idct(block.T, norm='ortho').T, norm='ortho'))


def reconstruct_image(coefficients, mcu_width):
    # Calculate the image width and height in pixels
    image_width = mcu_width * len(coefficients[0])
    image_height = mcu_width * len(coefficients)

    # Create an empty image to store the pixel values
    image = np.zeros((image_height, image_width))

    # Iterate over each MCU and apply inverse DCT to reconstruct the image
    for mcu_index, mcu in enumerate(coefficients):
        mcu_row = mcu_index // (image_width // mcu_width)
        mcu_col = mcu_index % (image_width // mcu_width)

        mcu_pixels = inverse_dct_2d(
            np.array(mcu).reshape((mcu_width, mcu_width)))
        image[mcu_row*mcu_width:(mcu_row+1)*mcu_width, mcu_col *
              mcu_width:(mcu_col+1)*mcu_width] = mcu_pixels

    return image


def main():
    # Replace with the path to your image
    mcu_width = 8

    # Assuming you have already extracted the quantized coefficients
    # For this example, I'll use the coefficients you provided
    coefficients = [
        [276, 42, 117, 9, 39, 12, 20, 0, 272, 11, 183, -70, 41, 5, -5, 2, 9, 242, 9, -7, -64, -23, -5, -9, -34, -229, 26, 25, -73, 19, 2, -13, -71,
            106, -80, -62, 25, 17, 15, -2, -16, -57, -89, 38, 4, -22, 18, -11, 39, -26, -71, 71, 18, -1, -11, -11, -80, -105, -21, 29, 30, -1, -16, 27],
        [-1, 29, -17, 8, -2, -7, -3, 4, 25, -11, -40, -10, 6, 1, -2, -3, 40, -19, -13, 19, -15, -1, 6, 11, -15, -3, -12, -13, 7, 5, -
            2, -5, -8, -5, 2, 9, -4, 2, 0, -3, -9, -20, -5, 10, 4, 4, 0, -3, 7, -2, 0, 4, -7, 2, 0, -4, 13, 5, -12, -7, 1, -1, 0, -2],
        [258, 68, 24, 53, 66, 6, 5, 7, 258, -146, 71, -64, 62, 22, -10, 7, 4, -218, -48, 36, 55, 18, 19, 1, -84, 192, 30, -58, 67, -4, -21, 4,
            95, -25, -58, -50, -1, 22, 15, 15, 79, 68, -50, 8, 16, -24, -2, 5, 0, -17, 60, -21, -30, 3, 2, 15, -4, 19, 18, 30, -11, -11, 13, -6],
        [68, -32, -10, 26, -8, -5, -3, -11, 45, 58, -2, -27, -17, 0, 8, -9, -53, 65, -26, 0, 9, -2, -3, -5, -52, -34, -27, -19, 3, 2,
            6, 6, -4, -13, 20, -11, 1, 2, 1, 2, -6, 3, 12, 19, 10, -3, 1, 2, -3, -4, -1, 1, 11, -5, -2, 4, 11, 6, -3, 0, 0, 1, 3, -5]
    ]

    # Reconstruct the image
    reconstructed_image = reconstruct_image(coefficients, mcu_width)

    # Display the resulting image
    plt.imshow(reconstructed_image, cmap='gray')
    plt.axis('off')
    plt.show()


if __name__ == "__main__":
    main()
