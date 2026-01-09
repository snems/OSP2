FORMAT_COMMAND=./tools/format.sh
CPPCHECK_COMMAND=./tools/cpp_check.sh

LIB_ROOT_DIR=./build/library
PK3_ROOT_DIR=./build/qvm
TESTS_ROOT_DIR=./build/tests
MOD_ROOT_DIR=./osp
DOC_ROOT_DIR=$(MOD_ROOT_DIR)/Docs

LIB_BUILD_DIR=$(LIB_ROOT_DIR)/build
PK3_BUILD_DIR=$(PK3_ROOT_DIR)/build
TESTS_BUILD_DIR=$(TESTS_ROOT_DIR)/build
LIB_BUILD_CLIENT_DIR=$(LIB_BUILD_DIR)/release-linux-x86_64/osp2

TESTS_FILE=osp2_tests
PK3_CLIENT_FILE=zz-osp-pak8.pk3
LIB_CLIENT_FILE=cgamex86_64.so
RELEASE_FILE=osp2_linux.tar.gz

MAKE_RELEASE_ARCHIVE=tar czf $(RELEASE_FILE) $(MOD_ROOT_DIR)


# check platform
COMPILE_PLATFORM=$(shell uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')

ifndef PLATFORM
PLATFORM=$(COMPILE_PLATFORM)
endif

ifeq ($(PLATFORM),mingw32)
  WINDOWS=1
endif
ifeq ($(PLATFORM),mingw64)
  WINDOWS=1
endif
ifeq ($(PLATFORM),windows)
  WINDOWS=1
endif

# change varibles for windows
ifdef WINDOWS
	LIB_CLIENT_FILE=cgamex86_64.dll
	RELEASE_FILE=osp2_windows.zip
	LIB_BUILD_CLIENT_DIR=$(LIB_BUILD_DIR)/release-$(PLATFORM)-x86_64/osp2
	MAKE_RELEASE_ARCHIVE=$(PK3_ROOT_DIR)/tools/7za.exe u $(RELEASE_FILE) $(MOD_ROOT_DIR)
endif

.PHONY: all clean client server osp2 format cppcheck pk3 lib dirs docs

all: $(RELEASE_FILE)

clean:
	@rm -rf $(LIB_BUILD_DIR)
	@rm -rf $(PK3_ROOT_DIR)/vm
	@rm -rf $(PK3_ROOT_DIR)/$(PK3_CLIENT_FILE)
	@rm -rf $(MOD_ROOT_DIR)
	@rm -f $(RELEASE_FILE)
	@rm -rf $(TESTS_BUILD_DIR)

dirs:
	@mkdir -p $(MOD_ROOT_DIR) 
	@mkdir -p $(DOC_ROOT_DIR)
	@mkdir -p $(DOC_ROOT_DIR)/RUS

format:
	$(FORMAT_COMMAND)

cppcheck:
	$(CPPCHECK_COMMAND)

pk3:
	@make -C $(PK3_ROOT_DIR)

lib:
	@make -C $(LIB_ROOT_DIR)

docs: dirs
	@rm -f $(DOC_ROOT_DIR)/RUS/osp2-q3-ClientReadme.txt
	@install -v docs/osp2-q3-ClientReadme.md $(DOC_ROOT_DIR)/RUS/osp2-q3-ClientReadme.txt
	@rm -f $(DOC_ROOT_DIR)/RUS/osp2-q3-SuperHUD.txt
	@install -v docs/osp2-q3-SuperHUD.md $(DOC_ROOT_DIR)/RUS/osp2-q3-SuperHUD.txt

osp2: pk3 lib dirs docs
	@rm -f $(MOD_ROOT_DIR)/$(LIB_CLIENT_FILE)
	@install -v $(LIB_BUILD_CLIENT_DIR)/$(LIB_CLIENT_FILE) $(MOD_ROOT_DIR)
	@rm -f $(MOD_ROOT_DIR)/$(PK3_CLIENT_FILE)
	@install -v $(PK3_ROOT_DIR)/$(PK3_CLIENT_FILE) $(MOD_ROOT_DIR)

$(RELEASE_FILE): osp2
	$(MAKE_RELEASE_ARCHIVE)

$(TESTS_BUILD_DIR):
	@cmake -S $(TESTS_ROOT_DIR) -B $(TESTS_BUILD_DIR)

$(TESTS_FILE): $(TESTS_BUILD_DIR)
	@cmake --build $(TESTS_BUILD_DIR)

tests: $(TESTS_FILE)
	$(TESTS_BUILD_DIR)/$(TESTS_FILE)

