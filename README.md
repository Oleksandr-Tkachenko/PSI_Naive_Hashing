# Private Set Intersection (PSI)
###Cuckoo Hashing###
---
__Install:__ 
```
sudo make install
```

__Clean:__ 
```
sudo make clean
```

__Remove:__ 
```
sudo make remove
```

###Dependencies:
---
 * libglib2.0-dev
 * lpsi-util
 * libssl-dev

###Usage:
---
####Server####
```
psi-naive-hashing  --server -s path to own dataset -d Path to result a -b path to result b  -r read buffer size -e element size -t threads -p port -i client ip
```
```
psi-intersection -e element size -a path to a -b path to b -s path to buckets -n number of buckets -q queue buffer size -r read buffer size -t number of threads -p path result -l path lookup
```
Server mode NH only hashes its own elements without looking for intersection between its own and opposite side's hashes. 
psi-intersection finds intersection and looks up the initial elements using -l argument.

####Client####
```
psi-naive-hashing  --client -s path to own dataset -r read buffer size -e element size -t number of threads -p port -i server ip
```
