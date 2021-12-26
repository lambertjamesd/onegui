
class Frame {
    constructor(
        public readonly id: number,
        public readonly parentID: number, // 0 indicates no parent
        public readonly testID: string,
        public readonly typeHint: string,
    ) {

    }
}

enum ElementType {
    String,
    Number,
    Boolean,
}

class InterfaceElement {
    constructor(
        public readonly name: string,
        public readonly elementType: ElementType,
        public readonly defaultValue: string | number | boolean,
    ) {

    }
}

class LayoutFunction {
    constructor(
        public readonly dependsOn: string[],
        public readonly modifies: string[],
        public readonly layoutFunction: () => void,
    ) {
        
    }
}

class GUITemplate {
    constructor(
        public readonly frames: Frame[],
        public readonly layoutFunctions: LayoutFunction[], 
        public readonly templateInterface: InterfaceElement[],
        public readonly hiddenVariables: InterfaceElement[],
    ) {

    }
}