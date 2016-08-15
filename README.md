##PSI##
###Cuckoo Hashing###

__Install:__ sudo make install

__Clean:__ sudo make clean

__Remove:__ sudo make remove

__Dependencies:__ libglib2.0-dev, _psi-utils_, libssl-dev

__Usage:__ 

####Server####
_psi-naive-hashing_  --server -s _"path to own dataset"_ -d _"Path to result a"_ -b _"path to result b"_  -r _read buffer size_ -e _element size_ -t _threads_ -p _port_ -i _client ip_

####Client####
_psi-naive-hashing_  --client -s _"path to own dataset"_ -r _read buffer size_ -e _element size_ -t _threads_ -p _port_ -i _server ip_
