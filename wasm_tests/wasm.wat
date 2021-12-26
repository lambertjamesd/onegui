(module
    (memory $0 1)
    (data (i32.const 0) "Hello World")

    (func $HelloWorld (param $lhs i32) (param $rhs i32) (result i32)
        local.get $lhs
        local.get $rhs
        i32.add
    )

    ;; allocated block structure
    ;; size of block | indicate block is allocated
    ;; bytes * size
    ;; size of block | indicate block is allocated

    ;; free block structure
    ;; size of block | indicate block is free
    ;; pointer to next block
    ;; pointer to prev block
    ;; bytes * (size - 8)
    ;; size of block | indicate block is free

    ;; $address - the address of the memory block to initialize
    ;; $size - the size in bytes of the usable space of the memory block
    ;; $isFree - 0 is the block is used, 1 if the block is free
    (func $init_block (param $address i32) (param $size i32) (param $isFree i32) (local $header i32)
        ;; header = $size | isFree
        (local.set $header (i32.or (local.get $size) (local.get $isFree)))
        (i32.store (local.get $address) (local.get $header))
        (i32.store (i32.add (local.get $address) (i32.const 4)) (i32.const 0))
        (i32.store (i32.add (local.get $address) (i32.const 8)) (i32.const 0))
        (i32.store (i32.add (i32.add (local.get $address) (local.get $address)) (i32.const 4)) (local.get $isFree))
    )

    (func $remove_block (param $address i32) (local $other_block i32)
    )

    (func $malloc (param $amount i32) (result i32)
        (i32.store (i32.const 4) (i32.const 42))
        (i32.load (i32.const 0))
    )

    (export "HelloWorld" (func $HelloWorld))
    (export "malloc" (func $malloc))
    (export "LayoutMemory" (memory $0))
)