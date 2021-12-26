import * as fs from 'fs';

export async function HelloWorld() {
    const data = fs.readFileSync('wasm_tests/wasm.wasm');
    const wasmModule = await WebAssembly.instantiate(data);
    const add = wasmModule.instance.exports.HelloWorld as  (a: number, b: number) => number;
    const malloc = wasmModule.instance.exports.malloc as  (size: number) => number;
    const result = add(1, 2);
    console.log(result);
    console.log(malloc(100));
    console.log(new Int32Array((wasmModule.instance.exports.LayoutMemory as {buffer: ArrayBuffer}).buffer)[1])
}