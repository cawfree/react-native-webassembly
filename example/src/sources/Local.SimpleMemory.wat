(module
  (memory $my_memory (export "memory") 20)

  (func (export "toggle_memory")
    (i32.store8 (i32.const 0) (i32.xor (i32.load8_u (i32.const 0)) (i32.const 1)))
  )

  (func (export "get_memory")
    (i32.load8_u (i32.const 0))
    drop
  )
)
