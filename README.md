CondenseNetwork
===============
Combine short segments in Network.dat files describing networks of microvessels  
Reads Network.dat and writes NetworkCondensed.dat.  
Only nodes with two segments attached can be removed.  
These parameters must be varied within the program (beginning of main.cpp).   
ltarget: combine segments until this target is reached.  
lmin: combine with another segment if a segment is shorter than this.  
TWS, April 2019
