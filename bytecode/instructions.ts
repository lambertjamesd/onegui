
export enum Instructions {
    NOP = 0x00,

    ADD = 0x01,
    SUB = 0x02,
    MUL = 0x03,
    DIV = 0x04,
    MOD = 0x05,
    NEGATE = 0x06,

    AND = 0x20,
    OR = 0x21,
    NOT = 0x22,
    EQ = 0x23,
    NEQ = 0x24,
    GT = 0x25,
    GTE = 0x26,

    BAND = 0x40,
    BOR = 0x41,
    BNOT = 0x42,
    BXOR = 0x43,
    BSHIFTR = 0x44,
    BSHIFTL = 0x45,

    CALL = 0x80,
    JUMP = 0x81,
    RETURN = 0x82,
    BIND_CLOSURE = 0x83,
    
    COPY = 0x90,
    INSERT = 0x91,
    REMOVE = 0x92,
    POP = 0x93,
    LOAD_NIL = 0x94,
    LOAD_TRUE = 0x95,
    LOAD_FALSE = 0x96,
    LOAD_INTEGER = 0x97,
    LOAD_CONSTANT = 0x98,
    NEW_TABLE = 0x99,
    NEW_ARRAY = 0x9A,

    LOAD_GLOBAL = 0xA0,
    STORE_GLOBAL = 0xA1,
    LOAD_GLOBAL_CONSTANT = 0xA0,
    STORE_GLOBAL_CONSTANT = 0xA1,
    LOAD_TABLE = 0xA2,
    STORE_TABLE = 0xA3,
    LOAD_TABLE_CONSTANT = 0xA2,
    STORE_TABLE_CONSTANT = 0xA3,
    LOAD_FROM_CLOSURE = 0xA4,
    STORE_INTO_CLOSURE = 0xA5,
}