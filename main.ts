
import * as NS from 'node-sdl2';

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