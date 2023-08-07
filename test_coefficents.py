import numpy as np
import matplotlib.pyplot as plt
from scipy.fftpack import idct
from scipy.signal import convolve2d


def zigzag_decode(coefficients):
    # Zigzag pattern for reordering coefficients
    zigzag_pattern = np.array([
        [0,  1,  2,  3, 4, 5, 6, 7],
        [8,  9,  10, 11, 12, 13, 14, 15],
        [16,  17, 18, 19, 20, 21, 22, 23],
        [24, 25, 26, 27, 28, 29, 30, 31],
        [32, 33, 34, 35, 36, 37, 38, 39],
        [40, 41, 42, 43, 44, 45, 46, 47],
        [48, 49, 50, 51, 52, 53, 54, 55],
        [56, 57, 58, 59, 60, 61, 62, 63]
    ])

    # Create an empty 8x8 block
    block = np.zeros((8, 8), dtype=int)

    # Fill the block with coefficients in zigzag pattern
    for index, pos in enumerate(np.ndindex(8, 8)):
        block[pos] = coefficients[zigzag_pattern[pos]]

    return block


def inverse_dct_2d(coefficients):
    return idct(idct(coefficients, norm='ortho').T, norm='ortho').T


def display_image(image):
    im = plt.imshow(image, cmap='gray')
    plt.colorbar(im)
    plt.axis('off')
    plt.show()


'''
                        0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0
'''

if __name__ == "__main__":
    # Replace this with your 8x8 array of DCT coefficients (in zigzag pattern)
    dct_coefficients = [-3, 18, -90, -32, 0, 24, 24, -18, 74, 0, -11, 0, 57, 0, 60, 0, -36, -44, -84, 64, 80, -88, -12, 78, -45, 0, 90, 0, -20, 0, -
                        27, 0, -125, 45, 0, -60, 76, 48, 0, -77, -52, 0, -49, 0, -91, 0, 126, 0, 13, -16, 48, 34, -48, -20, -36, 54, -76, 0, -128, 0, -45, 0, -112, 0]

    block = zigzag_decode(dct_coefficients)
    # Decode and inverse DCT transform
    reconstructed_image = inverse_dct_2d(block)

    # Display the image
    display_image(reconstructed_image)
