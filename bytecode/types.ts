
export type DataType = string | number | boolean | null | DataTypeArray | DataTypeObject | FunctionPointer;

export type TruthyDataType = string | number | true | DataTypeArray | DataTypeObject | FunctionPointer;

export interface DataTypeArray {
    [key: number]: DataType;
    length: number;
}

export interface DataTypeObject extends Map<string, DataType> {}

export interface FunctionClosure {
    parent: FunctionClosure | null;
    data: DataType[];
}

export interface FunctionData {
    readonly bytecode: Uint8Array;
    readonly offset: number;
    readonly paramCount: number;
    readonly varArgs: boolean;
    readonly constantTable: DataType[];
}

export class FunctionPointer {
    constructor(
        public readonly closure: FunctionClosure | null,
        public readonly fn: FunctionData,
    ) {

    }
}

function deepCopyWithCycles(data: DataType, done: Map<DataType, DataType>): DataType {
    const alreadyDone = done.get(data);

    if (alreadyDone) {
        return alreadyDone;
    }

    if (data instanceof Map) {
        const result = new Map();

        done.set(data, result);

        const entries = result.entries();
        let curr = entries.next();

        while (curr) {
            result.set(curr.value[0], deepCopyWithCycles(curr.value[1], done));
            curr = entries.next();
        }

        return result;
    } else if (data instanceof Array) {
        const result = new Array(data.length);

        done.set(data, result);

        for (let i = 0; i < data.length; ++i) {
            result[i] = deepCopyWithCycles(data[i], done);
        }

        return result;
    } else {
        return data;
    }
}

export function deepCopy(data: DataType): DataType {
    return deepCopyWithCycles(data, new Map());
}