const fs = require('fs');

async function testMain() {
    const mainModule = await WebAssembly.instantiate(
        fs.readFileSync('main.wasm'),
    );

    const callbackTable = mainModule.instance.exports.__indirect_function_table;
    
    const callback42Index = callbackTable.length;
    callbackTable.grow(1);
    callbackTable.set(callback42Index, mainModule.instance.exports.callback42);    

    console.log(mainModule.instance.exports.callback42());
    console.log(mainModule.instance.exports.runCallback(callback42Index));

    const subModule = await WebAssembly.instantiate(
        fs.readFileSync('module.wasm'),
    );

    const callback24Index = callbackTable.length;
    callbackTable.grow(1);
    callbackTable.set(callback24Index, subModule.instance.exports.callback24);

    console.log(mainModule.instance.exports.runCallback(callback24Index));

    const jsIndex = callbackTable.length;
    callbackTable.grow(1);
    callbackTable.set(jsIndex, () => 12);

    console.log(mainModule.instance.exports.runCallback(jsIndex));

    return 'success';
}

testMain().then(console.log, console.error);