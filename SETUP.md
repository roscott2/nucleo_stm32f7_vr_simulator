# Prerequisites for STM32 Development

Before building this project, ensure you have the following tools installed in your WSL Ubuntu environment:

## Install ARM GCC Toolchain
```bash
sudo apt update
sudo apt install gcc-arm-none-eabi
```

## Install OpenOCD for Debugging
```bash
sudo apt install openocd
```

## Install Build Tools
```bash
sudo apt install build-essential
sudo apt install git
```

## Verify Installation
```bash
arm-none-eabi-gcc --version
openocd --version
make --version
```

## Build the Project
```bash
make clean
make all
```

## Flash to Board
```bash
make flash
```

## Check Binary Size
```bash
make size
```

Note: You will need to generate the STM32 HAL drivers and startup files using STM32CubeMX 6.1.0 before the first build.
