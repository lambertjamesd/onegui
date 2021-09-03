
# Bytecode

OneGUI will have basic scripting functionality implemented with a simple bytecode likely modeled after Lua

## Opcodes

| Operator Name | Description | Opcode |
|---------------|-------------|--------|
| ADD           | Add two values on the stack and push the result | 0000 0000 |
| SUB           | Subtract two values on the stack and push the result | 0000 0001 |
| MUL           | Multipy two values on the stack and push the result | 0000 0010 |
| DIV           | Divide two values on the stack and push the result | 0000 0011 |
| MOD           | Calculate the remainder of two values on the stack and push the result | 0000 0100 |
| NEGATE        | Calculate the negation of a value from the stack and push the result | 0000 0101 |
| AND        | | 0010 0000 |
| OR         | | 0010 0001 |
| NOT        | | 0010 0010 |
| BAND        | | 0100 0000 |
| BOR         | | 0100 0001 |
| BNOT        | | 0100 0010 |
| BXOR        | | 0100 0011 |
| BSHIFTR        | | 0100 0100 |
| BSHIFTL        | | 0100 0101 |
| CALL            | Pop a function off the stack, then the n given operands for the function create a new function frame and call the function with the given operands  | 1000 0000 |
| JUMP | Unconditionally jump to an instruction relative to the current PC | 1000 0001 |
| RETURN | Return n given values off the stack to the calling function. Returned values are pushed on to the callers stack |


COPY
POP
LOAD_NIL
LOAD_i0
LOAD_i1
LOAD_f0
LOAD_f1
LOAD_STRING
LOAD_CONSTANT

LOAD_GLOBAL
STORE_GLOBAL
LOAD_TABLE
STORE_TABLE

