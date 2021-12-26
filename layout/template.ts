
// left, top, right, bottom
export type Rect = [number, number, number, number];

export enum LayoutDataType {
    i8,
    i16,
    i32,
    i64,
    f32,
    f64,
    Array,
    Structure,
    SubLayout,
};

export interface LayoutNumberType {
    type: LayoutDataType.i8 | LayoutDataType.i16 | LayoutDataType.i32 | LayoutDataType.i64 | LayoutDataType.f32 | LayoutDataType.f64;
}

export interface LayoutArrayType {
    type: LayoutDataType.Array,
    subType: LayoutType;
}

export interface LayoutStructureType {
    type: LayoutDataType.Structure,
    subTypes: {name?: string, type: LayoutType}[];
}

export interface LayoutSubLayoutType {
    type: LayoutDataType.SubLayout,
    view: LayoutView;
}

export type LayoutType = LayoutNumberType | LayoutArrayType | LayoutStructureType | LayoutSubLayoutType;

export type LayoutSerializable = number | LayoutView | LayoutArray | LayoutStructure;

export interface LayoutArray {
    [index: number]: LayoutSerializable;
    length: number;
}

export interface LayoutStructure {
    [index: string]: LayoutSerializable;
}

export interface LayoutView {}

export interface LayoutTemplate {
    dataLayout: LayoutStructureType;
    defaultValues: ArrayBuffer;
}

export function Instantiate(template: LayoutTemplate, initialData: LayoutSerializable): LayoutView {
    return {};
}

export function DestroyLayout(view: LayoutView) {

}

export function SetValue(view: LayoutView, dataPath: number[], inputValue: LayoutSerializable) {

}

export function Relayout(view: LayoutView) {

}

export function GetPosition(view: LayoutView, dataPath: number[]): Rect {
    return [0, 0, 0, 0];
}