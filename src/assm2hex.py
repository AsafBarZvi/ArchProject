



hex_out = []
assm_lines = [line.rstrip() for line in open('assm.txt').readlines()]
for assm in assm_lines:
    hex_line = []
    assm_parts = assm.split()
    op_code = assm_parts[0]
    if   (op_code.upper() == "LD"):
           hex_line.append("00")
    elif (op_code.upper() == "ST"):
           hex_line.append("01")
    elif (op_code.upper() == "ADD"):
           hex_line.append("02")
    elif (op_code.upper() == "SUB"):
           hex_line.append("03")
    elif (op_code.upper() == "MULT"):
           hex_line.append("04")
    elif (op_code.upper() == "DIV"):
           hex_line.append("05")
    elif (op_code.upper() == "HALT"):
           hex_line.append("06")
    else:
        print "unknown code {}".format(assm)
        assert(0)

    hex_line.append(hex(int(assm_parts[1]))[2:])  #Destination
    hex_line.append(hex(int(assm_parts[2]))[2:])  #SRC0
    hex_line.append(hex(int(assm_parts[3]))[2:])  #SRC1
    hex_line.append(hex(int(assm_parts[4]))[2:].rjust(3, '0'))  #IMM
    hex_out.append("".join(hex_line))

with open('memin.txt','w') as f:
    f.write("\n".join(hex_out))




