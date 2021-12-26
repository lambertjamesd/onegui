
import * as NS from 'node-sdl2';
import { HelloWorld } from './wasm_tests/loader';

let win = new NS.window;

win.on('close', () => {
    NS.App.quit();
});

win.on('change', () => {
    draw()
});

const draw = () => {

};

setTimeout(() => {
    
}, 1000);

HelloWorld();