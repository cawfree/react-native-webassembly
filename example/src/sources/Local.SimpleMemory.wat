(module
  ;; Define a memory block with one page (64KB) of memory
  (memory $my_memory 1)

  ;; Export the memory block so that it can be accessed from JavaScript
  (export "memory" (memory $my_memory))

  ;; Define a function that writes a byte to the first byte of memory
  (func (export "write_byte_to_memory") (param $value i32)
    (i32.store8 (i32.const 0) (local.get $value))
  )

  ;; Define a function that reads a byte from the first byte of memory
  (func (export "read_byte_from_memory") (result i32)
    (i32.load8_u (i32.const 0))
  )
)
