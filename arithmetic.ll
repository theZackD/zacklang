; ModuleID = 'arithmetic_test'
source_filename = "arithmetic_test"

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 10, ptr %x, align 4
  %y = alloca i32, align 4
  store i32 20, ptr %y, align 4
  %z = alloca i32, align 4
  %x1 = load i32, ptr %x, align 4
  %y2 = load i32, ptr %y, align 4
  %multmp = mul i32 %y2, 2
  %addtmp = add i32 %x1, %multmp
  store i32 %addtmp, ptr %z, align 4
  ret i32 0
}

