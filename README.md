### P4C Extension

#### Build
Suppose you have installed all the [dependencies](https://github.com/p4lang/p4c/#ubuntu-dependencies) for p4c. 

1. Clone the repo of p4c and be sure to add the ```--recursive``` flag and checkout to the commit that we are working at

```
git clone --recursive https://github.com/p4lang/p4c.git
cd p4c
git checkout cc0741d722c2fc99513716e86170d70d000503d6
```

2. Clone the repo under `p4c/backends`

```
cd p4c/backends
git clone git@github.com:multitenancy-project/our-FPGA-backend.git FPGA-backend
cd FPGA-backend
```

3. add the following to the line 276 of `p4c/CMakeLists.txt`
```
add_subdirectory (backends/FPGA-backends)
```

4. build
```
cd p4c
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=DEBUG -DENABLE_DOCS=OFF
make -j10
```

#### Test
0. set the path to your compiled `p4c-fpga`
```
export P4C_FPGA=/path/to/p4c/build/p4c-fpga
```

1. generate stateful memory conf and system conf
```
$P4C_FPGA --outputfile stateconf.txt --conffile allocate.txt --statefulconf 1
$P4C_FPGA ./sys.p4 --vid 15 --outputfile confsys.txt --conffile allocate.txt --onlysys 1
```

2. generate conf for user program
```
# generate conf for the program
$P4C_FPGA ./calc.p4 --vid 1 --outputfile conf1.txt --conffile allocate.txt
```

#### Miscs
1. lkupconf --> vid, [start, end)
2. statefulmem --> vid, offset, range
