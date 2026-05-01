import os

file_exclusions = [
    'cobs',
    'system_stm32h7xx_dualcore_bootcm7_cm4gated',
]

dir_exclusions = [
    'MarlinTT',
    'fonts',
    'graphics',
    'images',
    'tinyusb',
    'fatfs'
]

def count_lines(file_path):
    with open(file_path, 'rb') as file:
        return sum(1 for line in file)
    
def traverse_directory(directory, exclude_libs=True):
    total_lines = 0

    for root, dirs, files in os.walk(directory):
        if exclude_libs and any(exclusion in root for exclusion in dir_exclusions):
            continue
        
        for file in files:
            if exclude_libs and any(exclusion in file for exclusion in file_exclusions):
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
    traverse_directory(directory, True)