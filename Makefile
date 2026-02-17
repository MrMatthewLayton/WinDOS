# BCL (Base Class Library) for DOS - Makefile
# For use with DJGPP cross-compiler

# Compiler settings
CXX = ~/djgpp/bin/i586-pc-msdosdjgpp-g++
AR = ~/djgpp/bin/i586-pc-msdosdjgpp-ar
CXXFLAGS = -std=gnu++17 -O2 -Wall -Wextra -I./src
LDFLAGS =

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin

# Source files
PLATFORM_SRCS = \
    $(SRC_DIR)/Platform/DOS/Video.cpp \
    $(SRC_DIR)/Platform/DOS/Keyboard.cpp \
    $(SRC_DIR)/Platform/DOS/System.cpp \
    $(SRC_DIR)/Platform/DOS/Graphics.cpp \
    $(SRC_DIR)/Platform/DOS/Mouse.cpp

SYSTEM_SRCS = \
    $(SRC_DIR)/System/Exception.cpp \
    $(SRC_DIR)/System/String.cpp \
    $(SRC_DIR)/System/Types.cpp \
    $(SRC_DIR)/System/Memory.cpp \
    $(SRC_DIR)/System/Console.cpp \
    $(SRC_DIR)/System/IO/IO.cpp \
    $(SRC_DIR)/System/Drawing/Drawing.cpp \
    $(SRC_DIR)/System/Devices/Devices.cpp \
    $(SRC_DIR)/System/Windows/Forms/Forms.cpp \
    $(SRC_DIR)/ThirdParty/stb_truetype_impl.cpp

BCL_SRCS = $(PLATFORM_SRCS) $(SYSTEM_SRCS)

# Object files
PLATFORM_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(PLATFORM_SRCS))
SYSTEM_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SYSTEM_SRCS))
BCL_OBJS = $(PLATFORM_OBJS) $(SYSTEM_OBJS)

# Library
BCL_LIB = $(LIB_DIR)/libbcl.a

# Test executables
TEST_TYPES = $(BIN_DIR)/test_types.exe
TEST_STRING = $(BIN_DIR)/test_string.exe
TEST_ARRAY = $(BIN_DIR)/test_array.exe
TEST_EXCEPTION = $(BIN_DIR)/test_exception.exe
TEST_CONSOLE = $(BIN_DIR)/test_console.exe
RUN_ALL_TESTS = $(BIN_DIR)/run_all_tests.exe
TEST_GRAPHICS = $(BIN_DIR)/test_gfx.exe
TEST_DEVICES = $(BIN_DIR)/test_devices.exe
TEST_FORMS = $(BIN_DIR)/test_forms.exe
TEST_DRAWING_EXT = $(BIN_DIR)/test_drawing_ext.exe
TEST_COMPREHENSIVE = $(BIN_DIR)/test.exe
TEST_ICON = $(BIN_DIR)/test_icon.exe
TEST_LAYOUT = $(BIN_DIR)/test_layout.exe

# Demo executables
DEMO_GFX = $(BIN_DIR)/gfxdemo.exe
DEMO_FORMS = $(BIN_DIR)/forms.exe
DEMO_ICONS = $(BIN_DIR)/icons.exe
DEMO_HATCH = $(BIN_DIR)/hatch.exe

TESTS = $(TEST_TYPES) $(TEST_STRING) $(TEST_ARRAY) $(TEST_EXCEPTION) $(TEST_CONSOLE) $(RUN_ALL_TESTS) $(TEST_GRAPHICS) $(TEST_DEVICES) $(TEST_FORMS) $(TEST_DRAWING_EXT) $(TEST_COMPREHENSIVE) $(TEST_ICON) $(TEST_LAYOUT)

# Default target
all: directories $(BCL_LIB)

# Create directories
directories:
	@mkdir -p $(OBJ_DIR)/Platform/DOS
	@mkdir -p $(OBJ_DIR)/System
	@mkdir -p $(OBJ_DIR)/System/IO
	@mkdir -p $(OBJ_DIR)/System/Drawing
	@mkdir -p $(OBJ_DIR)/System/Devices
	@mkdir -p $(OBJ_DIR)/System/Windows/Forms
	@mkdir -p $(OBJ_DIR)/ThirdParty
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(BIN_DIR)

# Build the BCL library
$(BCL_LIB): $(BCL_OBJS)
	$(AR) rcs $@ $^
	@echo "Built BCL library: $@"

# Compile Platform source files
$(OBJ_DIR)/Platform/DOS/%.o: $(SRC_DIR)/Platform/DOS/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile System source files (root level)
$(OBJ_DIR)/System/%.o: $(SRC_DIR)/System/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile System/IO source files
$(OBJ_DIR)/System/IO/%.o: $(SRC_DIR)/System/IO/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile System/Drawing source files
$(OBJ_DIR)/System/Drawing/%.o: $(SRC_DIR)/System/Drawing/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile System/Devices source files
$(OBJ_DIR)/System/Devices/%.o: $(SRC_DIR)/System/Devices/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile System/Windows/Forms source files
$(OBJ_DIR)/System/Windows/Forms/%.o: $(SRC_DIR)/System/Windows/Forms/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile ThirdParty source files
$(OBJ_DIR)/ThirdParty/%.o: $(SRC_DIR)/ThirdParty/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build all tests
tests: directories $(BCL_LIB) $(TESTS)
	@echo "All tests built successfully!"

# Individual test executables
$(TEST_TYPES): $(TEST_DIR)/test_types.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_STRING): $(TEST_DIR)/test_string.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_ARRAY): $(TEST_DIR)/test_array.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_EXCEPTION): $(TEST_DIR)/test_exception.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_CONSOLE): $(TEST_DIR)/test_console.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(RUN_ALL_TESTS): $(TEST_DIR)/run_all_tests.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_GRAPHICS): $(TEST_DIR)/test_graphics.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_DEVICES): $(TEST_DIR)/test_devices.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_FORMS): $(TEST_DIR)/test_forms.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_DRAWING_EXT): $(TEST_DIR)/test_drawing_extended.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_COMPREHENSIVE): $(TEST_DIR)/test.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_ICON): $(TEST_DIR)/test_icon.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

$(TEST_LAYOUT): $(TEST_DIR)/test_layout.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

# Comprehensive test target (builds and names as test.exe)
test: directories $(BCL_LIB) $(TEST_COMPREHENSIVE)
	@echo "Comprehensive test suite built: $(TEST_COMPREHENSIVE)"

# Graphics demo
demo: directories $(BCL_LIB) $(DEMO_GFX)
	@echo "Graphics demo built!"

$(DEMO_GFX): $(TEST_DIR)/gfx_demo.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

# Forms demo
forms_demo: directories $(BCL_LIB) $(DEMO_FORMS)
	@echo "Forms demo built!"

$(DEMO_FORMS): $(TEST_DIR)/forms_demo.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

# Icon demo
icon_demo: directories $(BCL_LIB) $(DEMO_ICONS)
	@echo "Icon demo built!"

$(DEMO_ICONS): $(TEST_DIR)/icon_demo.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

# Hatch pattern demo
hatch_demo: directories $(BCL_LIB) $(DEMO_HATCH)
	@echo "Hatch demo built!"

$(DEMO_HATCH): $(TEST_DIR)/hatch_demo.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

# VBE debug test (standalone, doesn't need BCL)
vbe_debug: directories $(BIN_DIR)/vbetest.exe
	@echo "VBE debug test built!"

$(BIN_DIR)/vbetest.exe: $(TEST_DIR)/vbetest.cpp
	$(CXX) $(CXXFLAGS) $< -o $@
	@echo "Built: $@"

# VBE BCL test (uses BCL library)
vbe_bcl: directories $(BCL_LIB) $(BIN_DIR)/vbebcl.exe
	@echo "VBE BCL test built!"

$(BIN_DIR)/vbebcl.exe: $(TEST_DIR)/vbebcl.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

# VBE Forms test (minimal forms with VBE)
vbe_form: directories $(BCL_LIB) $(BIN_DIR)/vbeform.exe
	@echo "VBE Forms test built!"

$(BIN_DIR)/vbeform.exe: $(TEST_DIR)/vbeform.cpp $(BCL_LIB)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lbcl -o $@
	@echo "Built: $@"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	@echo "Build directory cleaned"

# Clean and rebuild
rebuild: clean all

# Show help
help:
	@echo "BCL Makefile targets:"
	@echo "  all        - Build BCL library (default)"
	@echo "  test       - Build comprehensive test suite (test.exe)"
	@echo "  tests      - Build all test executables"
	@echo "  demo       - Build graphics demo"
	@echo "  forms_demo - Build forms demo"
	@echo "  clean      - Remove build artifacts"
	@echo "  rebuild    - Clean and rebuild"
	@echo ""
	@echo "Test executables:"
	@echo "  $(TEST_COMPREHENSIVE) - Comprehensive test suite (runs ALL tests)"
	@echo "  $(TEST_TYPES)"
	@echo "  $(TEST_STRING)"
	@echo "  $(TEST_ARRAY)"
	@echo "  $(TEST_EXCEPTION)"
	@echo "  $(TEST_CONSOLE)"
	@echo "  $(TEST_GRAPHICS)"
	@echo "  $(TEST_DEVICES)"
	@echo "  $(TEST_FORMS)"
	@echo "  $(TEST_DRAWING_EXT)"
	@echo ""
	@echo "Output:"
	@echo "  Library: $(BCL_LIB)"
	@echo "  Tests:   $(BIN_DIR)/*.exe"

.PHONY: all directories test tests demo forms_demo icon_demo vbe_debug clean rebuild help
