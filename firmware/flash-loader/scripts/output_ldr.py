Import("env", "projenv")

import shutil

def after_build(source, target, env):
    try:
        print("Copying loader file...")
        shutil.copy(".pio/build/disco_h747xi/firmware.elf", "MX25L51245G_STM32H747-TT.stldr")
    except shutil.SameFileError:
        print("Source and destination represent the same file.")
    except IsADirectoryError:
        print("Destination is a directory but the copy function expects a file name.")
    except PermissionError:
        print("Permission denied.")
    except FileNotFoundError:
        print("Source file not found or destination directory does not exist.")
    except Exception as e:
        print(f"An error occurred: {e}")

env.AddPostAction("buildprog", after_build)