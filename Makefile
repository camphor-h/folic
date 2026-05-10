CC = cc
CFLAGS = -std=c99 -Wall -Wextra
LDFLAGS = -lm -lncursesw -lmenuw -lregex

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
DOC_DIR = doc
TEXT_DIR = text
SYNTAX_DIR = syntax
INSTALL_PREFIX = /usr/local
BIN_DIR = $(INSTALL_PREFIX)/bin
DOC_INSTALL_DIR = $(INSTALL_PREFIX)/share/folic/doc
SYNTAX_INSTALL_DIR = $(INSTALL_PREFIX)/share/folic/syntax

SRCS = buffer.c console.c folic.c history.c keyboard.c line.c message.c syntax.c textfile.c window.c sundries.c mouse/mouse.c clipboard/clipboard.c \
       $(TEXT_DIR)/string.c $(TEXT_DIR)/utf8char.c $(TEXT_DIR)/vector.c

OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)

TARGET = $(BUILD_DIR)/folic

all: release

debug: CFLAGS += -ggdb
debug: $(TARGET) doc-to-build

release: CFLAGS += -O2  
release: $(TARGET) doc-to-build

$(BUILD_DIR) $(OBJ_DIR) $(OBJ_DIR)/$(TEXT_DIR) $(OBJ_DIR)/mouse $(OBJ_DIR)/clipboard:
	@mkdir -p $@

$(TARGET): $(BUILD_DIR) $(OBJ_DIR) $(OBJ_DIR)/$(TEXT_DIR) $(OBJ_DIR)/mouse $(OBJ_DIR)/clipboard $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DDOC_PATH=\"./doc\" -DSYNTAX_PATH=\"./syntax\" -c $< -o $@

doc-to-build: $(TARGET)
	@echo "Copying documentation to build directory..."
	@cp -r $(DOC_DIR) $(BUILD_DIR)/
	@echo "Copying syntax files to build directory..."
	@cp -r $(SYNTAX_DIR) $(BUILD_DIR)/

install:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(DOC_INSTALL_DIR)
	@mkdir -p $(SYNTAX_INSTALL_DIR)
	$(CC) $(CFLAGS) -DDOC_PATH=\"$(DOC_INSTALL_DIR)\" -DSYNTAX_PATH=\"$(SYNTAX_INSTALL_DIR)\" $(SRCS) -o $(BIN_DIR)/folic $(LDFLAGS)
	@cp -r $(DOC_DIR)/* $(DOC_INSTALL_DIR)/
	@cp -r $(SYNTAX_DIR)/* $(SYNTAX_INSTALL_DIR)/
	@echo "Installation completed. Binary: $(BIN_DIR)/folic, Documentation: $(DOC_INSTALL_DIR), Syntax: $(SYNTAX_INSTALL_DIR)"
	chmod -R a+rX $(DOC_INSTALL_DIR)
	chmod -R a-w $(DOC_INSTALL_DIR)
	chmod -R 755 $(SYNTAX_INSTALL_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all debug release doc-to-build install clean
