######################################################################
# makefile
#
# Builds the example
######################################################################

##########
# Source files
##########

EXAMPLE_SKETCH=4_10_WiFi_AP_Server

EXECUTABLES += example

PARTITION_CSV_FILE=RTKEverywhere

ifeq ($(OS),Windows_NT)
# Windows NT utilities
CLEAR=cls
COPY=copy
DELETE=rmdir /s
DIR_LISTING=dir
TERMINAL_APP=
TERMINAL_PORT="COM3"
TERMINAL_PARAMS=

# Windows NT generic paths
USER_DIRECTORY_PATH=C:\Users\$(USERNAME)
ARDUINO_LIBRARY_PATH=$(USER_DIRECTORY_PATH)\Documents\Arduino\libraries
HOME_BOARD_PATH=$(USER_DIRECTORY_PATH)\AppData\Local\Arduino15\packages\esp32
PATCH_SRC_PATH=Patch\

# Windows NT patch source paths
PARTITION_SRC_PATH=..\$(PARTITION_CSV_FILE).csv
PATCH_SRC_PATH=Patch\

# Windows NT patch destination paths
BLE_PATCH_DST_PATH=$(ARDUINO_LIBRARY_PATH)\ESP32_BleSerial\src\
MBED_LIB_DEST_PATH=$(HOME_BOARD_PATH)\tools\esp32-arduino-libs\${{ env.ESP_IDF }}\esp32/lib\
PARTITION_DST_PATH=$(HOME_BOARD_PATH)\hardware\esp32\$(ESP_CORE_VERSION)\tools\partitions\$(PARTITION_CSV_FILE).csv

else
# Linux utilities
CLEAR=clear
COPY=cp
DELETE=rm -Rf
DIR_LISTING=ls
TERMINAL_APP=minicom
#TERMINAL_PORT="/dev/ttyACM1"
TERMINAL_PORT="/dev/ttyUSB0"
TERMINAL_PARAMS=-b 115200 -8 < /dev/tty

# Linux generic paths
USER_DIRECTORY_PATH=~
ARDUINO_LIBRARY_PATH=$(USER_DIRECTORY_PATH)/Arduino/libraries
ESP_IDF_PATH=$(HOME_BOARD_PATH)/tools/esp32-arduino-libs
HOME_BOARD_PATH=$(USER_DIRECTORY_PATH)/.arduino15/packages/esp32

# Linux patch source paths
PARTITION_SRC_PATH=../$(PARTITION_CSV_FILE).csv
PATCH_SRC_PATH=Patch/

# Linux patch destination paths
BLE_PATCH_DST_PATH=$(ARDUINO_LIBRARY_PATH)/ESP32_BleSerial/src/
MBED_LIB_DEST_PATH=$(ESP_IDF_PATH)/$(ESP_IDF_VERSION)/esp32/lib/
PARTITION_DST_PATH=$(HOME_BOARD_PATH)/hardware/esp32/$(ESP_CORE_VERSION)/tools/partitions/$(PARTITION_CSV_FILE).csv

endif

##########
# Buid all the sources - must be first
##########

.PHONY: all

all: $(EXECUTABLES)

##########
# Add ESP32 board support
##########

.PHONY: arduino-config

arduino-config:
	arduino-cli config init --overwrite --additional-urls "https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json,https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json"

ESP_CORE_VERSION=3.0.1
#ESP_CORE_VERSION=3.0.5
#ESP_CORE_VERSION=3.0.7

.PHONY: core-update

core-update:
	arduino-cli core update-index
	arduino-cli core install esp32:esp32@$(ESP_CORE_VERSION)

.PHONY: partition-update

partition-update:
	$(COPY) $(PARTITION_SRC_PATH)   $(PARTITION_DST_PATH)

##########
# Build an example
##########

.PHONY: example

example:	build/esp32.esp32.esp32/$(EXAMPLE_SKETCH).ino.bin

DEBUG_LEVEL=none
#DEBUG_LEVEL=debug
PSRAM=disabled
#PSRAM=enabled

build/esp32.esp32.esp32/$(EXAMPLE_SKETCH).ino.bin:	$(EXAMPLE_SKETCH).ino   *.ino   *.h   makefile
	$(CLEAR)
	arduino-cli compile --fqbn "esp32:esp32:esp32":DebugLevel=$(DEBUG_LEVEL),PSRAM=$(PSRAM) $< \
		--warnings default \
		--build-property build.partitions=$(PARTITION_CSV_FILE) \
		--build-property upload.maximum_size=6291456 \
		--build-property "compiler.cpp.extra_flags=-MMD -c \"-DPOINTPERFECT_TOKEN=$(POINTPERFECT_TOKEN)\" \"-DFIRMWARE_VERSION_MAJOR=$(FIRMWARE_VERSION_MAJOR)\" \"-DFIRMWARE_VERSION_MINOR=$(FIRMWARE_VERSION_MINOR)\" \"-DENABLE_DEVELOPER=$(ENABLE_DEVELOPER)\"" \
		--export-binaries

##########
# Upload the example
##########

ESPTOOL_PATH=~/Arduino/hardware/espressif/esp32/tools/esptool
#UPLOAD_SPEED=460800
UPLOAD_SPEED=921600
BOOT_LOADER_PATH=~/SparkFun/SparkFun_RTK_Firmware_Uploader/RTK_Firmware_Uploader/resource

.PHONY: upload

upload:	build/esp32.esp32.esp32/$(EXAMPLE_SKETCH).ino.bin
	python3 $(ESPTOOL_PATH)/esptool.py \
        --chip   esp32 \
        --port   $(TERMINAL_PORT) \
        --baud   $(UPLOAD_SPEED) \
        --before   default_reset \
        --after   hard_reset \
        write_flash \
        --flash_mode dio \
        --flash_freq 80m \
        --flash_size detect \
        --compress \
         0x1000   $(BOOT_LOADER_PATH)/RTK_Surveyor.ino.bootloader.bin \
         0x8000   $(BOOT_LOADER_PATH)/RTK_Surveyor_Partitions_16MB.bin \
         0xe000   $(BOOT_LOADER_PATH)/boot_app0.bin \
        0x10000   $<
	$(TERMINAL_APP) -D $(TERMINAL_PORT) $(TERMINAL_PARAMS)

##########
# Terminal
##########

.PHONY: terminal

terminal:
	$(TERMINAL_APP) -D $(TERMINAL_PORT) $(TERMINAL_PARAMS)

##########
# Clean up the example
##########

clean:
	$(DELETE) build
