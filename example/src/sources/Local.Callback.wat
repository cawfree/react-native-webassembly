(module
  (type $callback_type (func (param i32) (result i32)))
  (import "runtime" "callback" (func $callback (type $callback_type)))

  (func $callBackFunction (param $param i32) (result i32)
    (call $callback (local.get $param))
  )

  (export "callBackFunction" (func $callBackFunction))
)
