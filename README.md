### Get Started

Suppose you have installed all the [dependencies](https://github.com/p4lang/p4c/#ubuntu-dependencies) for p4c. 

1. Clone the repo of p4c and be sure to add the ```--recursive``` flag

```
git clone --recursive https://github.com/p4lang/p4c.git
```

2. Clone the repo of isolator

```
git clone git@github.com:anirudhSK/isolator.git
```

3. link the isolator backend in p4c repo

```
cd ./p4c/backends
ln -s /absolute/path/to/isolator/src ./t-merge
```

4. add the following to the line 273 of ./p4c/CMakeLists.txt

```
add_subdirectory (backends/t-merge)
```

5. build

```
cd ./p4c
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=DEBUG
make -j10
```


### Test

Simple test for merging programs.

Go to ```./p4c/build``` folder, 

```
./p4merge /path/to/isolator/test_files/empty.p4 /path/to/isolator/test_files/tna_multicast.p4
```

The ```empty.p4``` is the system-provided program and ```tna_multicast.p4``` is the user-defined program. And the output file is named as ```merged.p4```.

### Miscs

The user-defined programs should follow the programming template, i.e., the header is defined in ```header_t```, and the metadata is defined in ```ingress_metadata_t``` and ```egress_metadata_t``` for ingress and egress pipelines, respectively.

And the control pipelines are defined in ```SwitchIngressParser```, ```SwitchIngress```, ```SwitchIngressDeparser```, ```SwitchEgressParser```, ```SwitchEgress``` and ```SwitchEgressDeparser```.

The common headers are ```ethernet_h, vlan_tag_h, ipv4_h, tcp_h, udp_h```, and we can increase this set gradually. All the user-defined headers are in the payload part.
