
# OneGUI

OneGUI aims to be a file format for creating consistent, flexible, and portable GUIs. Use the same tool to create your GUIs for any platform and any language with consistent results across each supported platform.

## OneGUI for programmers

OneGUI will be decoupled from any specific platform or language and will try to stay as simple as possible. A OneGUI component will have its own internal state. The internal state will control the presentation of the GUI such as information to display on the GUI or instantiating nested components. The state can be controlled by assigning JSON like data to keys. 

## Possible API
Interacting with an OneGUI component may look something like this.

```
// this function will be called after a GUI is created but before it is displayed
// and allows you to setup GUI state and respond to events
function appCallback(appGui) {
    // programmatically set inputs on the internal state
    appGui.set('value1', 'value1-override');

    // listen for events
    appGui.set('onEvent', function(eventData) {
        appGui.set('value1', 'value1-newvalue');
    });
}

// gui components will have a default internal state but
// that state can be overriden with initialState
initialState = {param0: 'value0', param1: 'value1'};

// load the gui template
oneGui.loadTemplate('./app.onegui');

// - instantiate the template.
// - templates are stored by `namespace:name` when loaded 
//   and referenced by that name when instantiated
oneGui.instantiate('example:app', appCallback, initialState);

```

## Built on Web Assembly

GUI layout algorithms are complicated and there is no such thing as a one size fits all solution. This conficts with OneGUIs goal of being cross platform. The more complicated OneGUI is to implement, the fewer platforms it will reach. It also conflicts with the goal to be flexible. If the layout algorithms aren't flexible enought, it will be very limiting to the user. To solve this problem, OneGUI will be built on WebAssembly. This way the layout algorithms can be bundled with the GUI. The implementation just needs to call the WebAssembly layout function. This will allow a huge amount of flexibilty while keeping things simple. (Well, as simple as implmenenting WebAssembly is but that work has already been done on many platforms). It will also keep implementations consistent as the specifics of the layout logic will be contained in the GUI itself.

Having a web assembly core will also allow for GUI logic to be written in WebAssembly. This could be used to create simple form validators, date pickers, and other logic that should be tighly coupled to the GUI while exposing a simpler interface to the code using the GUI. With this, modular and reusable components can be built without needing to know what platform it is targeting and without the need to reimplement the logic on each platform.