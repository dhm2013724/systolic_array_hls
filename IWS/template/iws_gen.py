# import numpy as np
import sys,os

TILE_M = 13
SA_R = 7
SA_C = 11

head_define_str = ""
head_define_str += "#define TILE_M " + str(TILE_M) + "\n"
head_define_str += "#define SA_R " + str(SA_R) + "\n"
head_define_str += "#define SA_C " + str(SA_C) + "\n"
# print(head_define_str)

fill_b = ""
for i in range(SA_R):
    for j in range(SA_C):
        fill_b += "PE_array["+str(i)+"]["+str(j)+"].fillB<"+str(i)+","+str(j)+">"
        if(i == 0):
            fill_b += "(Bin_s["+str(j)
        else:
            fill_b += "(B_inter["+str(i)+"]["+str(j)
        fill_b += "], B_inter["+str(i+1)+"]["+str(j)+"]);\n"
            
    fill_b += "\n"
# print(fill_b)  

sa_iws = ""
for i in range(SA_R):
    for j in range(SA_C):
        sa_iws += "PE_array["+str(i)+"]["+str(j)+"].compute<"+str(i)+","+str(j)+">"
        if (j == 0):
            sa_iws += "(     Ain_s["+str(i)+"],        M_MIN_s["+str(i)+"]"
        else:
            sa_iws += "(A_inter["+str(i)+"]["+str(j)+"], M_MIN_inter["+str(i)+"]["+str(j)+"]"
        sa_iws += ", C_out["+str(i)+"]["+str(j)+"], A_inter["+str(i)+"]["+str(j+1)+"], M_MIN_inter["+str(i)+"]["+str(j+1)+"],"
        if (i<SA_R-1):
            sa_iws += " C_out["+str(i+1)+"]["+str(j)+"]);\n"
        else:
            sa_iws += " Cout_s["+str(j)+"]);\n"
        
    sa_iws += "\n"
# print(sa_iws)

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

find_str0 = "#FILL_B#".encode()
replace_str0 = fill_b.encode()

find_str1 = "#Compute_SA_Loop#".encode()
replace_str1 = sa_iws.encode()

for line in f_head:
    if find_str0 in line:
        line = line.replace(find_str0,replace_str0)
    
    if find_str1 in line:
        line = line.replace(find_str1,replace_str1)    

    f_new.write(line)
    
f_head.close()
f_new.close()