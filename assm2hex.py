import struct
import sys
import argparse
import ipdb

parser = argparse.ArgumentParser("assm to hex converter")

parser.add_argument("assm",
                    help="input assm file , one line for Header, cmd per line")

args   = parser.parse_args()




def float_to_hex(f):
    return hex(struct.unpack('<I', struct.pack('<f', f))[0])[2:]


hex_out = []
assm_lines = [line.rstrip() for line in open(args.assm).readlines()][1:]
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
    elif (op_code.upper() == "MUL"):
        hex_line.append("04")
    elif (op_code.upper() == "DIV"):
        hex_line.append("05")
    elif (op_code.upper() == "HALT"):
        hex_line.append("06")
    elif (op_code.upper() == "NOPE"):
        hex_line.append("09")
    elif (op_code.upper() == "VALUE"):
        hex_out.append(float_to_hex(float(assm_parts[1])))
        continue
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




