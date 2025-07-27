# level-1 
# base usage hex2raw 

# cat exploit.txt | ./hex2raw | ./ctarget -q 
# ./hex2raw < exploit.txt > exploit-raw.txt
# ./ctarget -q < exploit-raw.txt


# 4.2 level-2
# ./hex2raw < exploit-4.2-level-2.txt > exploit-4.2-raw.txt
# ./ctarget -q -i exploit-4.2-raw.txt 

48 89 c7 # encodes the instruction movq %rax, %rdi

