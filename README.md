##PSI##
###Cuckoo Hashing###

__Install:__ sudo make install

__Clean:__ sudo make clean

__Remove:__ sudo make remove

__Dependencies:__ libglib2.0-dev, _lpsi-util_, libssl-dev

__Usage:__ 

####Server####
_psi-naive-hashing_  --server -s _"path to own dataset"_ -d _"Path to result a"_ -b _"path to result b"_  -r _read buffer size_ -e _element size_ -t _threads_ -p _port_ -i _client ip_

_psi-intersection_ -e _element size_ -a _path to a_ -b _path to b_ -s _path to buckets_ -n _bucket number_ -q _queue buffer size_ -r _read buffer size_ -t _thread number_ -p _path result_ -l _path lookup_

Server mode NH only hashes its own elements without looking for intersection between its own and opposite side's hashes. 
psi-intersection finds intersection and looks up the initial elements using -l argument.

####Client####
_psi-naive-hashing_  --client -s _"path to own dataset"_ -r _read buffer size_ -e _element size_ -t _threads_ -p _port_ -i _server ip_
