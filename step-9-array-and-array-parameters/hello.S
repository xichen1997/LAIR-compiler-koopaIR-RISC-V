  .data
g_buf:
  .zero 800

  .text
  .globl merge_sort
merge_sort:
  addi sp, sp, -432
  sw ra, 416(sp)
entry_0:
  sw a0, 0(sp)
  sw a1, 4(sp)
  lw t0, 0(sp)
  sw t0, 8(sp)
  lw t0, 8(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 12(sp)
  lw t0, 4(sp)
  sw t0, 16(sp)
  lw t0, 12(sp)
  lw t1, 16(sp)
  slt t0, t0, t1
  seqz t0, t0
  sw t0, 20(sp)
  lw t0, 20(sp)
  bnez t0, then_0
  j else_0

then_0:
  lw ra, 416(sp)
  addi sp, sp, 432
  ret

else_0:
  j end_0
end_0:
  lw t0, 0(sp)
  sw t0, 28(sp)
  lw t0, 4(sp)
  sw t0, 32(sp)
  lw t0, 28(sp)
  lw t1, 32(sp)
  add t0, t0, t1
  sw t0, 36(sp)
  lw t0, 36(sp)
  li t1, 2
  div t0, t0, t1
  sw t0, 40(sp)
  lw t0, 40(sp)
  sw t0, 24(sp)
  lw t0, 0(sp)
  sw t0, 44(sp)
  lw t0, 24(sp)
  sw t0, 48(sp)
  lw a0, 44(sp)
  lw a1, 48(sp)
  call merge_sort
  lw t0, 24(sp)
  sw t0, 52(sp)
  lw t0, 4(sp)
  sw t0, 56(sp)
  lw a0, 52(sp)
  lw a1, 56(sp)
  call merge_sort
  lw t0, 0(sp)
  sw t0, 64(sp)
  lw t0, 64(sp)
  sw t0, 60(sp)
  lw t0, 24(sp)
  sw t0, 72(sp)
  lw t0, 72(sp)
  sw t0, 68(sp)
  lw t0, 0(sp)
  sw t0, 80(sp)
  lw t0, 80(sp)
  sw t0, 76(sp)
  j while_entry_1
while_entry_1:
  li t0, 0
  sw t0, 84(sp)
  lw t0, 60(sp)
  sw t0, 88(sp)
  lw t0, 24(sp)
  sw t0, 92(sp)
  lw t0, 88(sp)
  lw t1, 92(sp)
  slt t0, t0, t1
  sw t0, 96(sp)
  lw t0, 96(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 100(sp)
  lw t0, 100(sp)
  bnez t0, logic_true_0
  j logic_false_0

logic_true_0:
  lw t0, 68(sp)
  sw t0, 104(sp)
  lw t0, 4(sp)
  sw t0, 108(sp)
  lw t0, 104(sp)
  lw t1, 108(sp)
  slt t0, t0, t1
  sw t0, 112(sp)
  lw t0, 112(sp)
  li t1, 0
  xor t0, t0, t1
  snez t0, t0
  sw t0, 116(sp)
  lw t0, 116(sp)
  sw t0, 84(sp)
  j logic_end_0
logic_false_0:
  li t0, 0
  sw t0, 84(sp)
  j logic_end_0
logic_end_0:
  lw t0, 84(sp)
  sw t0, 120(sp)
  lw t0, 120(sp)
  bnez t0, while_stmt_1
  j while_next_1

while_stmt_1:
  la t0, g_buf
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 124(sp)
  lw t0, 60(sp)
  sw t0, 128(sp)
  lw t0, 124(sp)
  lw t1, 128(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 132(sp)
  lw t0, 132(sp)
  lw t0, 0(t0)
  sw t0, 136(sp)
  la t0, g_buf
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 140(sp)
  lw t0, 68(sp)
  sw t0, 144(sp)
  lw t0, 140(sp)
  lw t1, 144(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 148(sp)
  lw t0, 148(sp)
  lw t0, 0(t0)
  sw t0, 152(sp)
  lw t0, 136(sp)
  lw t1, 152(sp)
  slt t0, t0, t1
  sw t0, 156(sp)
  lw t0, 156(sp)
  bnez t0, then_1
  j else_1

while_next_1:
  j while_entry_2
then_1:
  la t0, g_buf
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 160(sp)
  lw t0, 60(sp)
  sw t0, 164(sp)
  lw t0, 160(sp)
  lw t1, 164(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 168(sp)
  lw t0, 168(sp)
  lw t0, 0(t0)
  sw t0, 172(sp)
  la t0, g_buf
  li t1, 1
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 176(sp)
  lw t0, 76(sp)
  sw t0, 180(sp)
  lw t0, 176(sp)
  lw t1, 180(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 184(sp)
  lw t0, 172(sp)
  lw t1, 184(sp)
  sw t0, 0(t1)
  lw t0, 60(sp)
  sw t0, 188(sp)
  lw t0, 188(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 192(sp)
  lw t0, 192(sp)
  sw t0, 60(sp)
  j end_1
else_1:
  la t0, g_buf
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 196(sp)
  lw t0, 68(sp)
  sw t0, 200(sp)
  lw t0, 196(sp)
  lw t1, 200(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 204(sp)
  lw t0, 204(sp)
  lw t0, 0(t0)
  sw t0, 208(sp)
  la t0, g_buf
  li t1, 1
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 212(sp)
  lw t0, 76(sp)
  sw t0, 216(sp)
  lw t0, 212(sp)
  lw t1, 216(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 220(sp)
  lw t0, 208(sp)
  lw t1, 220(sp)
  sw t0, 0(t1)
  lw t0, 68(sp)
  sw t0, 224(sp)
  lw t0, 224(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 228(sp)
  lw t0, 228(sp)
  sw t0, 68(sp)
  j end_1
while_entry_2:
  lw t0, 60(sp)
  sw t0, 232(sp)
  lw t0, 24(sp)
  sw t0, 236(sp)
  lw t0, 232(sp)
  lw t1, 236(sp)
  slt t0, t0, t1
  sw t0, 240(sp)
  lw t0, 240(sp)
  bnez t0, while_stmt_2
  j while_next_2

end_1:
  lw t0, 76(sp)
  sw t0, 244(sp)
  lw t0, 244(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 248(sp)
  lw t0, 248(sp)
  sw t0, 76(sp)
  j while_entry_1
while_stmt_2:
  la t0, g_buf
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 252(sp)
  lw t0, 60(sp)
  sw t0, 256(sp)
  lw t0, 252(sp)
  lw t1, 256(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 260(sp)
  lw t0, 260(sp)
  lw t0, 0(t0)
  sw t0, 264(sp)
  la t0, g_buf
  li t1, 1
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 268(sp)
  lw t0, 76(sp)
  sw t0, 272(sp)
  lw t0, 268(sp)
  lw t1, 272(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 276(sp)
  lw t0, 264(sp)
  lw t1, 276(sp)
  sw t0, 0(t1)
  lw t0, 60(sp)
  sw t0, 280(sp)
  lw t0, 280(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 284(sp)
  lw t0, 284(sp)
  sw t0, 60(sp)
  lw t0, 76(sp)
  sw t0, 288(sp)
  lw t0, 288(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 292(sp)
  lw t0, 292(sp)
  sw t0, 76(sp)
  j while_entry_2
while_next_2:
  j while_entry_3
while_entry_3:
  lw t0, 68(sp)
  sw t0, 296(sp)
  lw t0, 4(sp)
  sw t0, 300(sp)
  lw t0, 296(sp)
  lw t1, 300(sp)
  slt t0, t0, t1
  sw t0, 304(sp)
  lw t0, 304(sp)
  bnez t0, while_stmt_3
  j while_next_3

while_stmt_3:
  la t0, g_buf
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 308(sp)
  lw t0, 68(sp)
  sw t0, 312(sp)
  lw t0, 308(sp)
  lw t1, 312(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 316(sp)
  lw t0, 316(sp)
  lw t0, 0(t0)
  sw t0, 320(sp)
  la t0, g_buf
  li t1, 1
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 324(sp)
  lw t0, 76(sp)
  sw t0, 328(sp)
  lw t0, 324(sp)
  lw t1, 328(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 332(sp)
  lw t0, 320(sp)
  lw t1, 332(sp)
  sw t0, 0(t1)
  lw t0, 68(sp)
  sw t0, 336(sp)
  lw t0, 336(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 340(sp)
  lw t0, 340(sp)
  sw t0, 68(sp)
  lw t0, 76(sp)
  sw t0, 344(sp)
  lw t0, 344(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 348(sp)
  lw t0, 348(sp)
  sw t0, 76(sp)
  j while_entry_3
while_next_3:
  j while_entry_4
while_entry_4:
  lw t0, 0(sp)
  sw t0, 352(sp)
  lw t0, 4(sp)
  sw t0, 356(sp)
  lw t0, 352(sp)
  lw t1, 356(sp)
  slt t0, t0, t1
  sw t0, 360(sp)
  lw t0, 360(sp)
  bnez t0, while_stmt_4
  j while_next_4

while_stmt_4:
  la t0, g_buf
  li t1, 1
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 364(sp)
  lw t0, 0(sp)
  sw t0, 368(sp)
  lw t0, 364(sp)
  lw t1, 368(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 372(sp)
  lw t0, 372(sp)
  lw t0, 0(t0)
  sw t0, 376(sp)
  la t0, g_buf
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 380(sp)
  lw t0, 0(sp)
  sw t0, 384(sp)
  lw t0, 380(sp)
  lw t1, 384(sp)
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 388(sp)
  lw t0, 376(sp)
  lw t1, 388(sp)
  sw t0, 0(t1)
  lw t0, 0(sp)
  sw t0, 392(sp)
  lw t0, 392(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 396(sp)
  lw t0, 396(sp)
  sw t0, 0(sp)
  j while_entry_4
while_next_4:
  lw ra, 416(sp)
  addi sp, sp, 432
  ret

  .globl main
main:
  addi sp, sp, -48
  sw ra, 32(sp)
entry_1:
  la t0, g_buf
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 4(sp)
  lw t0, 4(sp)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 8(sp)
  lw a0, 8(sp)
  call getarray
  sw a0, 12(sp)
  lw t0, 12(sp)
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 16(sp)
  li a0, 0
  lw a1, 16(sp)
  call merge_sort
  lw t0, 0(sp)
  sw t0, 20(sp)
  la t0, g_buf
  li t1, 0
  li t2, 400
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 24(sp)
  lw t0, 24(sp)
  li t1, 0
  li t2, 4
  mul t1, t1, t2
  add t0, t0, t1
  sw t0, 28(sp)
  lw a0, 20(sp)
  lw a1, 28(sp)
  call putarray
  li a0, 0
  lw ra, 32(sp)
  addi sp, sp, 48
  ret

