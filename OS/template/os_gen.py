import sys,os

TILE_M = 7
TILE_N = 10
TILE_K = 12

head_define_str = ""
head_define_str += "#define TILE_M " + str(TILE_M) + "\n"
head_define_str += "#define TILE_N " + str(TILE_N) + "\n"
head_define_str += "#define TILE_K " + str(TILE_K) + "\n"
print(head_define_str)

sa_os = ""
for i in range(TILE_M):
    for j in range(TILE_N):
        sa_os += "compute_pe<"+ str(i) +", " + str(j) + ">"
        if (j == 0):
            sa_os += "(     Ain_s[" + str(i)
        else:
            sa_os += "(A_inter[" + str(i) + "][" + str(j)
            
        if(i == 0):
            sa_os += "], Bin_s[" + str(j) + "],"
        else:
            sa_os += "], B_inter["+str(i)+"]["+str(j)+"],"
            
        sa_os += " C_out["+str(i)+"]["+str(j)+"], A_inter["+str(i)+"]["+str(j+1)+"], B_inter["+str(i+1)+"]["+str(j)
        
        if (j<TILE_N-1):
            sa_os += "], C_out["+str(i)+"]["+str(j+1)+"],"
        else:
            sa_os += "],   Cout_s["+str(i)+"],"
            
        if (j == 0):
            sa_os += "        cws_s["+str(i)+"],"
        else:
            sa_os += " cws_inter["+str(i)+"]["+str(j)+"],"
            
        sa_os += " cws_inter["+str(i)+"]["+str(j+1)+"]);\n"

print(sa_os)

f_head = open("systolic_array.h","rb")
f_new = open("./gen/systolic_array.h","wb")

find_str = "#DEFINE_HEADER#".encode()
replace_str = head_define_str.encode()

for line in f_head:
    if find_str in line:
        line = line.replace(find_str,replace_str)
    f_new.write(line)
    
f_head.close()
f_new.close()

f_head = open("systolic_array.cpp","rb")
f_new = open("./gen/systolic_array.cpp","wb")

find_str = "#SA_GEN#".encode()
replace_str = sa_os.encode()

for line in f_head:
    if find_str in line:
        line = line.replace(find_str,replace_str)
    f_new.write(line)
    
f_head.close()
f_new.close()