CC  = gcc
LF  = -lm
CF  = -g

SRC_DIR = src
INC_DIR = -Ithirdparty/
OUT_DIR = build


SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OUT_DIR)/%.o, $(SRCS))
EXE  = ssr

all: $(OUT_DIR)/$(EXE)

$(OUT_DIR)/$(EXE): $(OBJS) $(OUT_DIR)
	$(CC) $(OBJS) -o $@ $(LF)

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c $(OUT_DIR)
	$(CC) $(CF) -c $< -o $@ $(INC_DIR)

$(OUT_DIR): 
	mkdir -p $@

clean:
	$(RM) -r $(OUT_DIR)