# T Sim
```
cd src
python assm2hex.py
mkdir build
cd build
cmake ..
make 
ln -s ../src/memin.txt memin.txt
./sim
```


output is :
```
In Resv -> op:LD,d:6,s0:0,s1:0,i:34
In Resv -> op:LD,d:2,s0:0,s1:0,i:45
In Resv -> op:MULT,d:0,s0:2,s1:4,i:0
In Resv -> op:SUB,d:8,s0:6,s1:2,i:0
In Resv -> op:DIV,d:10,s0:0,s1:6,i:0
In Resv -> op:ADD,d:6,s0:8,s1:2,i:0
0x00600022 0 ld0 3 3 6 7
0x0020002d 1 ld1 3 4 7 8
0x04024000 2 mult0 4 7 11 12
0x03862000 3 add0 4 7 9 10
0x05a06000 4 div0 5 11 17 -1
0x02682000 5 add1 5 9 11 12
0x06000000 17  17 -1 -1 -1
```
