; ModuleID = 'var_decl_test'
source_filename = "var_decl_test"

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 42, ptr %x, align 4
  ret i32 0
}

