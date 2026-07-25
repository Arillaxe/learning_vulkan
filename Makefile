# Build for the Vulkan project.
# Usage:
#   mingw32-make            build (incremental, add -jN for parallel)
#   mingw32-make clean      remove build artifacts
#
# Object files live in a single flat build/ directory (source path separators
# are replaced with dots) so no nested directories need to be created. This
# keeps the recipes portable across cmd.exe and sh on Windows.

CXX      := g++
CXXSTD   := -std=c++20
DEFINES  := -D DEBUG
DEPFLAGS := -MMD -MP -g -O0
INCLUDES := -Isrc -I"$(VULKAN_SDK)/Include" -I"$(GLFW_PATH)/include" -I./include
LDFLAGS  := -L"$(GLFW_PATH)/lib-mingw-w64" -L"$(VULKAN_SDK)/Lib"
LDLIBS   := -lglfw3 -lvulkan-1 -lgdi32

TARGET    := main.exe
BUILD_DIR := build

# Precompiled header. Force-included into every TU; GCC transparently uses the
# generated .gch as long as the compile flags match.
PCH_HDR := src/pch.hpp
PCH_GCH := $(PCH_HDR).gch

# Recursively collect every .cpp under src/.
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
SRCS := $(call rwildcard,src,*.cpp)

# src/renderer/vk/vk_context.cpp -> build/renderer.vk.vk_context.o
obj_of = $(BUILD_DIR)/$(subst /,.,$(patsubst src/%.cpp,%,$1)).o
OBJS := $(foreach s,$(SRCS),$(call obj_of,$s))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# Build the precompiled header once. Objects depend on it and force-include it,
# so any change to pch.hpp (or a header it pulls in) rebuilds the .gch and, in
# turn, every object.
$(PCH_GCH): $(PCH_HDR)
	$(CXX) $(CXXSTD) $(DEFINES) $(DEPFLAGS) $(INCLUDES) -x c++-header $< -o $@

# Generate one compile rule per source so the flattened object name maps back
# to its (nested) source file.
define COMPILE_RULE
$(call obj_of,$1): $1 $(PCH_GCH) | $(BUILD_DIR)
	$(CXX) $(CXXSTD) $(DEFINES) $(DEPFLAGS) $(INCLUDES) -include $(PCH_HDR) -c $$< -o $$@
endef
$(foreach s,$(SRCS),$(eval $(call COMPILE_RULE,$s)))

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

# mingw32-make may run recipes through either a POSIX shell (e.g. w64devkit's
# sh) or cmd.exe depending on what is on PATH, so pick the delete syntax based
# on the shell actually in use rather than on the OS.
ifneq (,$(findstring sh,$(SHELL)))
clean:
	rm -rf "$(BUILD_DIR)" "$(TARGET)" "$(PCH_GCH)" "$(PCH_HDR).d"
else
clean:
	if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	if exist "$(TARGET)" del /q "$(TARGET)"
	if exist "$(subst /,\,$(PCH_GCH))" del /q "$(subst /,\,$(PCH_GCH))"
	if exist "$(subst /,\,$(PCH_HDR)).d" del /q "$(subst /,\,$(PCH_HDR)).d"
endif

-include $(DEPS)
-include $(PCH_HDR).d
