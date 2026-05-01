import os

def count_lines(file_path):
    with open(file_path, 'r') as file:
        return sum(1 for line in file)
    
def traverse_directory(directory):
    total_lines = 0

    for root, dirs, files in os.walk(directory):
        if 'MarlinTT' in root or 'fonts' in root or 'graphics' in root or 'images' in root or 'tinyusb' in root or 'fatfs' in root:
            continue
        
        for file in files:
            if 'cobs' in file or 'system_stm32h7xx_dualcore_bootcm7_cm4gated' in file:
                continue
            if file.endswith('.cpp') or file.endswith('.h') or file.endswith('.c'):
                file_path = os.path.join(root, file)
                lines = count_lines(file_path)
                print(f"{file_path}: {lines} lines")
                total_lines += lines

    print(f"Total lines of code: {total_lines}")
    return total_lines

if __name__ == "__main__":
    directory = "."
    traverse_directory(directory)