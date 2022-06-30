# SegaController #

Read Sega Genesis (Mega Drive) controllers with a STM32F103C8T6 "blue pill".

## Installation ##

Download [GenesisToUSBDual.zip](https://github.com/PathfinderCast/GenesisToUSBDual/releases/latest) which has the source code, arduino hardware drivers, Windows drivers, and firmware programing software.

Download and install the [Arduino IDE](https://www.arduino.cc/en/software). Installing from windows app store has limitations that will not work in this scenario.

## Setup Arduino Environment ##
1. Open the Arduino IDE
2. Open "File > Preferences"
3. Add `https://raw.githubusercontent.com/stm32duino/BoardManagerFiles/main/package_stmicroelectronics_index.json` to Additional Boards Manager URLs
4. Open "Tools > Board > Board Manager"
5. Install STM32 MCU based boards
6. Open "Tools > Board > Board Manager"
7. Install Arduino SAM boards (Cortex-M3)

## Driver Installation ##
1. Unzip the [GenesisToUSBDual.zip](https://github.com/PathfinderCast/GenesisToUSBDual/releases/latest)
2. From the `Install` folder copy `Arduino_STM32` to your Arduino hardware folder, typically at `C:\Program Files (x86)\Arduino\hardware` or `<user>\Documents\arduino\hardware`
3. From `Arduino_STM32\drivers\win` folder run the `install_drivers.bat` file. This will take a while to run and accept all of the prompts.

## Install STLink Drivers the Bootloader (optional) ##
If Windows does not detect the ST-Link usb device, you will need to [download](https://www.st.com/en/development-tools/stm32cubeprog.html) and install the drivers. Run the `SetupSTM32CubeProgrammer_win64.exe` after registering and downloading.

## Usage ##

The controller pins are as follows, looking face on to the (female) DB9 plug:

    5 4 3 2 1
     9 8 7 6

![STM32 Wiring Diagram](connections.svg?raw=true "STM32 Wiring Diagram")


1. Connect controller pin 5 to +5VCC
2. Connect controller pin 8 to GND
3. Connect pins 1,2,3,4,6,9  to a digital I/O
4. Connect pin 7 to the digital I/O pin 8 on your Arduino



#### Attribution ####

Modified and Published under MIT license
Copyright (c) 2017 Jon Thysell
For details on how SegaController reads the controller's state, see [How To Read Sega Controllers](https://github.com/jonthysell/SegaController/wiki/How-To-Read-Sega-Controllers).
