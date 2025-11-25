# LCD Prototyping Project

This PlatformIO project is used to prototype LCD functionality on the STM32F469 Discovery Kit.

## How to use:

1. Install platformio on VSCode
2. Install python
3. Install STM Cube Programmer
4. Open the project in VSCode
5. In the root directory (with platformio.ini), add a script called `upload.bat` that will call STM Cube Programmer on the command line. 

## Upload.bat contents:

```
path/to/STM32_Programmer_CLI.exe -c port=SWD -el path/to/MT25QL128A_STM32F469I-DK.stldr" -d .pio/build/default/firmware.elf -rst
```

The path to STM32 Cube Programmer is usually: 

```
"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
```

and the path to the loader file is usually: 

```
"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\ExternalLoader\MT25QL128A_STM32F469I-DK.stldr"
```

The second path is to an external loader file, which allows STM Cube Programmer to program external flash memories connected to the QSPI.