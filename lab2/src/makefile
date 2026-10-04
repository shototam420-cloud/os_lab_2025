CC = gcc
AR = ar

CFLAGS = -Wall -Wextra -Wpedantic -std=c11

BUILD = build
SWAP_DIR = swap
REVERT_DIR = revert_string

SWAP_APP = $(BUILD)/swap_app
STATIC_APP = $(BUILD)/revert_static
SHARED_APP = $(BUILD)/revert_shared

REVERT_OBJECT = $(BUILD)/revert_string.o
STATIC_LIBRARY = $(BUILD)/librevert_string.a
SHARED_LIBRARY = $(BUILD)/librevert_string.so

.PHONY: all demo clean

all: $(SWAP_APP) $(STATIC_APP) $(SHARED_APP)

$(BUILD):
	mkdir -p $(BUILD)

$(SWAP_APP): $(SWAP_DIR)/main.c $(SWAP_DIR)/swap.c $(SWAP_DIR)/swap.h | $(BUILD)
	$(CC) $(CFLAGS) \
	  $(SWAP_DIR)/main.c \
	  $(SWAP_DIR)/swap.c \
	  -I$(SWAP_DIR) \
	  -o $(SWAP_APP)

$(REVERT_OBJECT): $(REVERT_DIR)/revert_string.c $(REVERT_DIR)/revert_string.h | $(BUILD)
	$(CC) $(CFLAGS) -fPIC \
	  -I$(REVERT_DIR) \
	  -c $(REVERT_DIR)/revert_string.c \
	  -o $(REVERT_OBJECT)

$(STATIC_LIBRARY): $(REVERT_OBJECT)
	$(AR) rcs $(STATIC_LIBRARY) $(REVERT_OBJECT)

$(SHARED_LIBRARY): $(REVERT_OBJECT)
	$(CC) -shared \
	  $(REVERT_OBJECT) \
	  -o $(SHARED_LIBRARY)

$(STATIC_APP): $(REVERT_DIR)/main.c $(REVERT_DIR)/revert_string.h $(STATIC_LIBRARY)
	$(CC) $(CFLAGS) \
	  $(REVERT_DIR)/main.c \
	  -I$(REVERT_DIR) \
	  -L$(BUILD) \
	  -Wl,-Bstatic -lrevert_string \
	  -Wl,-Bdynamic \
	  -o $(STATIC_APP)

$(SHARED_APP): $(REVERT_DIR)/main.c $(REVERT_DIR)/revert_string.h $(SHARED_LIBRARY)
	$(CC) $(CFLAGS) \
	  $(REVERT_DIR)/main.c \
	  -I$(REVERT_DIR) \
	  -L$(BUILD) \
	  -lrevert_string \
	  -Wl,-rpath,'$$ORIGIN' \
	  -o $(SHARED_APP)

demo: all
	@echo
	@echo "===== SWAP ====="
	./$(SWAP_APP)

	@echo
	@echo "===== STATIC LIBRARY ====="
	./$(STATIC_APP) "String with spaces"

	@echo
	@echo "===== DYNAMIC LIBRARY ====="
	./$(SHARED_APP) "String with spaces"

	@echo
	@echo "===== STATIC DEPENDENCIES ====="
	ldd $(STATIC_APP)

	@echo
	@echo "===== DYNAMIC DEPENDENCIES ====="
	ldd $(SHARED_APP)

clean:
	rm -rf $(BUILD)