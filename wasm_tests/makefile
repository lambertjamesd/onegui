
main.wasm: main.c common.h
	emcc -s ALLOW_TABLE_GROWTH=1 -s EXPORTED_FUNCTIONS=_runCallback,_callback42 -Os --no-entry -o main.wasm main.c

module.wasm: module.c common.h
	emcc -s ALLOW_TABLE_GROWTH=1 -s EXPORTED_FUNCTIONS=_callback24 -Os --no-entry -o module.wasm module.c