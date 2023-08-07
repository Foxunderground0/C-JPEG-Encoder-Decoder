from PIL import Image
from PIL.JpegImagePlugin import JpegImageFile


def extract_huffman_tables(jpeg_file_path):
    try:
        with Image.open(jpeg_file_path) as img:
            if isinstance(img, JpegImageFile):
                huffman_tables = img.app["huffmantables"]
                print("Huffman Tables:")
                for i, table in enumerate(huffman_tables):
                    print(f"\nTable {i + 1}:")
                    for symbol, code in table.items():
                        print(f"Symbol: {symbol}, Huffman Code: {code}")
            else:
                print("Not a valid JPEG image.")
    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    jpeg_file_path = "Test Immages\8x8.jpg"
    print_huffman_tables(jpeg_file_path)
