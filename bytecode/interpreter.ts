
import {DataType, deepCopy, FunctionPointer, TruthyDataType} from './types';
import {Instructions} from './instructions';
import { BAD_CLOSURE_LOOKUP, EXPECTED_ARRAY, EXPECTED_FUNCTION, EXPECTED_NUMBER, EXPECTED_OBJECT, EXPECTED_OBJECT_OR_ARRAY, EXPECTED_STRING, INVALID_CONSTANT_INDEX, INVALID_INSTRUCTION, NULL_POINTER_EXCEPTION, PROGRAM_COUNTER_OVERRUN, STACK_UNDERFLOW } from './errors';


interface FunctionFrame {
    expectedReturnValueCount: number;
    paramCount: number;
    dataStack: DataType[];
    fn: FunctionPointer;
    pc: number;
}

interface ProgramState {
    frames: FunctionFrame[];
    globals: Map<string, DataType>;
}

type InstructionExecutor = (programState: ProgramState) => string | null;

const instructionExecutors = new Array<InstructionExecutor>(256);

for (let i = 0; i < instructionExecutors.length; ++i) {
    instructionExecutors[i] = () => INVALID_INSTRUCTION;
}

function isTruthy(value: DataType): value is TruthyDataType {
    if (typeof value === 'boolean') {
        return value;
    }

    if (value === null) {
        return false;
    }

    return true;
}

function getTopFunctionFrame(programState: ProgramState) {
    return programState.frames[programState.frames.length - 1];
}

function next(programState: ProgramState): number | null {
    const frame = getTopFunctionFrame(programState);

    if (frame.pc < frame.fn.fn.bytecode.length) {
        const result = frame.fn.fn.bytecode[frame.pc];
        frame.pc++;
        return result;
    } else {
        return null;
    }
}

function nextVarLengthInt(programState: ProgramState): number | null {
    let current = next(programState);

    if (current === null) {
        return null;
    }

    let result = current & 0x7F;
    let bitShift = 0;

    while (current & 0x80) {
        const nextByte = next(programState);

        if (nextByte === null) {
            return null;
        }

        bitShift += 7;
        result = result | ((nextByte & 0x7F) << bitShift);

        current = nextByte;

    }

    if (result >> (bitShift + 6)) {
        result = -result + 1
    }

    return result;
}

// NOP instruction

instructionExecutors[Instructions.NOP] = () => null;

// Arithmetic

instructionExecutors[Instructions.ADD] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)

    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a + b)

    return null;
};

instructionExecutors[Instructions.SUB] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)

    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a - b)

    return null;
};

instructionExecutors[Instructions.MUL] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)

    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a * b)

    return null;
};

instructionExecutors[Instructions.DIV] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)

    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a / b)

    return null;
};

instructionExecutors[Instructions.MOD] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)

    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a % b)

    return null;
};

instructionExecutors[Instructions.NEGATE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)

    if (frame.dataStack.length < 1) return STACK_UNDERFLOW;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(-a)

    return null;
};

// Boolean

instructionExecutors[Instructions.AND] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)

    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b === 'undefined') return STACK_UNDERFLOW;

    const a = frame.dataStack.pop();
    if (typeof a === 'undefined') return STACK_UNDERFLOW;

    if (!isTruthy(a)) {
        frame.dataStack.push(a);
    } else {
        frame.dataStack.push(b);
    }

    return null;
};

instructionExecutors[Instructions.OR] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b === 'undefined') return STACK_UNDERFLOW;

    const a = frame.dataStack.pop();
    if (typeof a === 'undefined') return STACK_UNDERFLOW;

    if (isTruthy(a)) {
        frame.dataStack.push(a);
    } else {
        frame.dataStack.push(b);
    }

    return null;
};

instructionExecutors[Instructions.NOT] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)

    if (frame.dataStack.length < 1) return STACK_UNDERFLOW;

    const a = frame.dataStack.pop();
    if (typeof a === 'undefined') return STACK_UNDERFLOW;

    frame.dataStack.push(!isTruthy(a))

    return null;
};


instructionExecutors[Instructions.EQ] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)

    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b === 'undefined') return STACK_UNDERFLOW;

    const a = frame.dataStack.pop();
    if (typeof a === 'undefined') return STACK_UNDERFLOW;

    frame.dataStack.push(a === b)

    return null;
};


instructionExecutors[Instructions.NEQ] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b === 'undefined') return STACK_UNDERFLOW;

    const a = frame.dataStack.pop();
    if (typeof a === 'undefined') return STACK_UNDERFLOW;

    frame.dataStack.push(a !== b)

    return null;
};


instructionExecutors[Instructions.GT] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a > b)

    return null;
};


instructionExecutors[Instructions.GTE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a >= b)

    return null;
};

// Bitwise instructions

instructionExecutors[Instructions.BAND] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a & b)

    return null;
};

instructionExecutors[Instructions.BOR] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a | b)

    return null;
};

instructionExecutors[Instructions.BNOT] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 1) return STACK_UNDERFLOW;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(~a)

    return null;
};

instructionExecutors[Instructions.BXOR] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a ^ b)

    return null;
};

instructionExecutors[Instructions.BSHIFTR] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;
    
    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a >> b)

    return null;
};

instructionExecutors[Instructions.BSHIFTL] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState)
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;

    const b = frame.dataStack.pop();
    if (typeof b !== 'number') return EXPECTED_NUMBER;

    const a = frame.dataStack.pop();
    if (typeof a !== 'number') return EXPECTED_NUMBER;

    frame.dataStack.push(a << b)

    return null;
};

// Function operations

instructionExecutors[Instructions.CALL] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);
    const parameterCount = next(programState);

    if (parameterCount === null) {
        return PROGRAM_COUNTER_OVERRUN;
    }

    const expectedReturnCount = next(programState);

    if (expectedReturnCount === null) {
        return PROGRAM_COUNTER_OVERRUN;
    }
    
    if (frame.dataStack.length < 1 + parameterCount) return STACK_UNDERFLOW;
    
    const fn = frame.dataStack.pop();
    if (!(fn instanceof FunctionPointer) && fn !== null) return EXPECTED_FUNCTION;
    
    const params = frame.dataStack.splice(frame.dataStack.length - parameterCount, parameterCount);

    if (fn === null) return NULL_POINTER_EXCEPTION;

    while (params.length < fn.fn.paramCount) {
        params.push(null);
    }

    if (fn.fn.varArgs) {
        params.push(params.splice(fn.fn.paramCount, params.length - fn.fn.paramCount));
    }

    programState.frames.push({
        expectedReturnValueCount: expectedReturnCount,
        paramCount: parameterCount,
        dataStack: params,
        fn: fn,
        pc: fn.fn.offset,
    });

    return null;
};

instructionExecutors[Instructions.JUMP] = (programState: ProgramState) => {
    const offset = nextVarLengthInt(programState);

    if (offset === null) return PROGRAM_COUNTER_OVERRUN;

    const frame = getTopFunctionFrame(programState);

    frame.pc + offset;

    return null;
}

instructionExecutors[Instructions.RETURN] = (programState: ProgramState) => {
    const returnCount = next(programState);

    if (returnCount === null) return PROGRAM_COUNTER_OVERRUN;

    const frame = programState.frames.pop();

    if (frame === undefined) return STACK_UNDERFLOW;

    if (returnCount < frame.dataStack.length) return STACK_UNDERFLOW;
    
    const result = frame.dataStack.splice(frame.dataStack.length - returnCount, Math.min(returnCount, frame.expectedReturnValueCount));

    while (result.length < frame.expectedReturnValueCount) {
        result.push(null);
    }

    const prevFrame = getTopFunctionFrame(programState);
    
    prevFrame.dataStack.push(...result);

    return null;
}

instructionExecutors[Instructions.BIND_CLOSURE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);
    
    const table = frame.dataStack.pop();

    if (!(table instanceof Array)) return EXPECTED_ARRAY;

    const fn = frame.dataStack.pop();
    if (!(fn instanceof FunctionPointer)) return EXPECTED_FUNCTION;

    frame.dataStack.push(new FunctionPointer(
        {
            parent: frame.fn.closure,
            data: table,
        }, 
        fn.fn,
    ));

    return null;
};

// Stack operations

instructionExecutors[Instructions.COPY] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const stackPositionOffset = next(programState);

    if (stackPositionOffset === null) return PROGRAM_COUNTER_OVERRUN;

    if (stackPositionOffset >= frame.dataStack.length) return STACK_UNDERFLOW;

    frame.dataStack.push(frame.dataStack[frame.dataStack.length - (1 + stackPositionOffset)]);

    return null;
}

instructionExecutors[Instructions.INSERT] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    let stackPositionOffset = next(programState);

    if (stackPositionOffset === null) return PROGRAM_COUNTER_OVERRUN;

    stackPositionOffset += 1;

    if (stackPositionOffset >= frame.dataStack.length) return STACK_UNDERFLOW;

    const value = frame.dataStack.pop()!;

    frame.dataStack.splice(frame.dataStack.length - stackPositionOffset, 0, value);

    return null;
}

instructionExecutors[Instructions.REMOVE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    let stackPositionOffset = next(programState);

    if (stackPositionOffset === null) return PROGRAM_COUNTER_OVERRUN;

    stackPositionOffset += 2;

    if (stackPositionOffset > frame.dataStack.length) return STACK_UNDERFLOW;

    frame.dataStack.splice(frame.dataStack.length - stackPositionOffset, 1);

    return null;
}

instructionExecutors[Instructions.POP] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const popCount = next(programState);

    if (popCount === null) return PROGRAM_COUNTER_OVERRUN;

    if (frame.dataStack.length < popCount) return STACK_UNDERFLOW;

    for (let i = 0; i <= popCount; ++i) {
        frame.dataStack.pop();
    }

    return null;
}

instructionExecutors[Instructions.LOAD_NIL] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const nilCount = next(programState);

    if (nilCount === null) return PROGRAM_COUNTER_OVERRUN;

    for (let i = 0; i <= nilCount; ++i) {
        frame.dataStack.push(null);
    }

    return null;
}

instructionExecutors[Instructions.LOAD_TRUE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    frame.dataStack.push(true);

    return null;
}

instructionExecutors[Instructions.LOAD_FALSE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    frame.dataStack.push(false);

    return null;
}

instructionExecutors[Instructions.LOAD_INTEGER] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const intValue = nextVarLengthInt(programState);

    if (intValue === null) return PROGRAM_COUNTER_OVERRUN;

    frame.dataStack.push(intValue);

    return null;
}

instructionExecutors[Instructions.LOAD_CONSTANT] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const constantIndex = nextVarLengthInt(programState);

    if (constantIndex === null) return PROGRAM_COUNTER_OVERRUN;

    const constant = frame.fn.fn.constantTable[constantIndex];

    if (constant === undefined) return INVALID_CONSTANT_INDEX;

    frame.dataStack.push(deepCopy(constant));
    
    return null;
}

instructionExecutors[Instructions.NEW_TABLE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    frame.dataStack.push(new Map());
    
    return null;
}

instructionExecutors[Instructions.NEW_ARRAY] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    frame.dataStack.push([]);
    
    return null;
}

// Storing and loading

instructionExecutors[Instructions.LOAD_GLOBAL] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const key = frame.dataStack.pop();

    if (key === undefined) return STACK_UNDERFLOW;

    if (typeof key !== 'string') return EXPECTED_STRING;

    frame.dataStack.push(programState.globals.get(key) ?? null);
    
    return null;
}

instructionExecutors[Instructions.STORE_GLOBAL] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const value = frame.dataStack.pop();

    if (value === undefined) return STACK_UNDERFLOW;

    const key = frame.dataStack.pop();

    if (key === undefined) return STACK_UNDERFLOW;

    if (typeof key !== 'string') return EXPECTED_STRING;

    programState.globals.set(key, value);
    
    return null;
}

instructionExecutors[Instructions.LOAD_GLOBAL_CONSTANT] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const constantIndex = nextVarLengthInt(programState);

    if (constantIndex === null) return PROGRAM_COUNTER_OVERRUN;

    const key = frame.fn.fn.constantTable[constantIndex];

    if (key === undefined) return INVALID_CONSTANT_INDEX;

    if (typeof key !== 'string') return EXPECTED_STRING;

    frame.dataStack.push(programState.globals.get(key) ?? null);
    
    return null;
}

instructionExecutors[Instructions.STORE_GLOBAL_CONSTANT] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const value = frame.dataStack.pop();

    if (value === undefined) return STACK_UNDERFLOW;

    const constantIndex = nextVarLengthInt(programState);

    if (constantIndex === null) return PROGRAM_COUNTER_OVERRUN;

    const key = frame.fn.fn.constantTable[constantIndex];

    if (key === undefined) return INVALID_CONSTANT_INDEX;

    if (typeof key !== 'string') return EXPECTED_STRING;

    programState.globals.set(key, value);
    
    return null;
}

instructionExecutors[Instructions.LOAD_TABLE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;

    const key = frame.dataStack.pop()!;
    const top = frame.dataStack[frame.dataStack.length - 1];

    if (top === null) return NULL_POINTER_EXCEPTION;
    
    if (top instanceof Map) {
        if (typeof key !== 'string') return EXPECTED_STRING;

        frame.dataStack.push(top.get(key) ?? null);
    } else if (top instanceof Array) {
        if (typeof key !== 'number') return EXPECTED_NUMBER;

        frame.dataStack.push(top[key] ?? null);
    } else {
        return EXPECTED_OBJECT_OR_ARRAY;
    }

    return null;
}

instructionExecutors[Instructions.STORE_TABLE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);
    
    if (frame.dataStack.length < 3) return STACK_UNDERFLOW;

    const value = frame.dataStack.pop()!;
    const key = frame.dataStack.pop()!;
    const top = frame.dataStack[frame.dataStack.length - 1];
    
    if (top === null) return NULL_POINTER_EXCEPTION;
    
    if (top instanceof Map) {
        if (typeof key !== 'string') return EXPECTED_STRING;
        top.set(key, value);
    } else if (top instanceof Array) {
        if (typeof key !== 'number') return EXPECTED_NUMBER;
        for (let i = top.length; i < key; ++i) {
            top.push(null);
        }
        top[key] = value;
    } else {
        return EXPECTED_OBJECT_OR_ARRAY;
    }
    
    return null;
}

instructionExecutors[Instructions.LOAD_TABLE_CONSTANT] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    if (frame.dataStack.length < 1) return STACK_UNDERFLOW;

    const constantIndex = nextVarLengthInt(programState);

    if (constantIndex === null) return PROGRAM_COUNTER_OVERRUN;

    const key = frame.fn.fn.constantTable[constantIndex];

    if (key === undefined) return INVALID_CONSTANT_INDEX;

    const top = frame.dataStack[frame.dataStack.length - 1];

    if (top === null) return NULL_POINTER_EXCEPTION;

    if (top instanceof Map) {
        if (typeof key !== 'string') return EXPECTED_STRING;

        frame.dataStack.push(top.get(key) ?? null);
    } else if (top instanceof Array) {
        if (typeof key !== 'number') return EXPECTED_NUMBER;

        frame.dataStack.push(top[key] ?? null);
    } else {
        return EXPECTED_OBJECT_OR_ARRAY;
    }
    
    return null;
}

instructionExecutors[Instructions.STORE_TABLE_CONSTANT] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);
    
    if (frame.dataStack.length < 2) return STACK_UNDERFLOW;

    const value = frame.dataStack.pop()!;

    const constantIndex = nextVarLengthInt(programState);

    if (constantIndex === null) return PROGRAM_COUNTER_OVERRUN;

    const key = frame.fn.fn.constantTable[constantIndex];

    if (key === undefined) return INVALID_CONSTANT_INDEX;

    const top = frame.dataStack[frame.dataStack.length - 1];

    if (top === null) return NULL_POINTER_EXCEPTION;

    if (top instanceof Map) {
        if (typeof key !== 'string') return EXPECTED_STRING;
        top.set(key, value);
    } else if (top instanceof Array) {
        if (typeof key !== 'number') return EXPECTED_NUMBER;
        for (let i = top.length; i < key; ++i) {
            top.push(null);
        }
        top[key] = value;
    } else {
        return EXPECTED_OBJECT_OR_ARRAY;
    }
    
    return null;
}

instructionExecutors[Instructions.LOAD_FROM_CLOSURE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    let variableIndex = nextVarLengthInt(programState);

    if (variableIndex === null) return PROGRAM_COUNTER_OVERRUN;

    let closure = frame.fn.closure;

    while (closure && variableIndex >= closure.data.length) {
        variableIndex -= closure.data.length;
        closure = closure.parent;
    }

    if (closure === null) return BAD_CLOSURE_LOOKUP;

    frame.dataStack.push(closure.data[variableIndex]);

    return null;
}

instructionExecutors[Instructions.STORE_INTO_CLOSURE] = (programState: ProgramState) => {
    const frame = getTopFunctionFrame(programState);

    const value = frame.dataStack.pop();

    if (value === undefined) return STACK_UNDERFLOW;

    let variableIndex = nextVarLengthInt(programState);

    if (variableIndex === null) return PROGRAM_COUNTER_OVERRUN;

    let closure = frame.fn.closure;

    while (closure && variableIndex >= closure.data.length) {
        variableIndex -= closure.data.length;
        closure = closure.parent;
    }

    if (closure === null) return BAD_CLOSURE_LOOKUP;

    closure.data[variableIndex] = value;

    return null;
}

function runProgramState(programState: ProgramState, maxCycles: number): string | null {
    while (programState.frames.length && maxCycles > 0) {
        const nextInstruction = next(programState);
        if (nextInstruction === null) return PROGRAM_COUNTER_OVERRUN;
        const result = instructionExecutors[nextInstruction](programState);
        if (result === null) return result;
        maxCycles -= 1;
    }

    return null;
}