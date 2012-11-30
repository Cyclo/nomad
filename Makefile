
OUTPUT_DIR="bin"

all:
	mkdir -p $(OUTPUT_DIR)
	cc main.c ndutils.c nomad.c -o $(OUTPUT_DIR)/a.out

clean:
	rm "$OUTPUT_DIR/a.out"