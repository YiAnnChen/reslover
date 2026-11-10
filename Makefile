BUILD_DIR = build
TARGET := STM32F446RECx
RTT_ADDR := 0x20000000
BUILD_TYPE := Debug

all:
	mkdir -p $(BUILD_DIR)
	- ./run_clang-format.sh 2> /dev/null &
	cd $(BUILD_DIR) && cmake .. -GNinja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=$(CURDIR)/cmake/gcc-arm-none-eabi.cmake
	cd $(BUILD_DIR) && ninja
	@-mv $(BUILD_DIR)/compile_commands.json .

clean:
	rm -rf $(BUILD_DIR)

flash:
	@-pyocd flash -t $(TARGET) $(BUILD_DIR)/easyresolver.elf

monitor:
	@-pyocd rtt -t $(TARGET) -a $(RTT_ADDR)

debug:
	@-pyocd gdbserver -t $(TARGET) --persist

.PHONY: all clean flash monitor
