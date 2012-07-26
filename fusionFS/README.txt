Author: dongfang.zhao@hawk.iit.edu

What is FusionFS
	- In short, FusionFS is a completely distributed file system, that said, nothing (at all) is centralized
		including meta data management. 

What is working
	- File create/open/read/write/close/remove
	- Direcotry create/read/remove
	
What is NOT working (will not be added unless requested)
	- File rename/link/slink
	- Directory rename
	- Other POSIX interfaces 

How to install fusionfs:
	1) Make sure all dependent libraries are specified, e.g. echo $LD_LIBRARY_PATH 
		:/usr/local/lib:/home/dongfang/fusionFS/src/ffsnet/:/home/dongfang/fusionFS/src/udt4/src
	2) Install FUSE 2.8 or later for your Linux distribution
	3) Install Google Protocol Buffer 
	4) Go to ./src/zht/ and run Makefile
	5) Go to ./src/udt/ and run Makefile
	6) Go to ./src/ffsnet/, run Makefile
	7) Go to ./src, run Makefile
	8) You can use ./compileAll if you are sure each individual Makefile works fine
	9) ./cleanALL to `make clean` everything

How to use fusionfs:
	1) ./clearRootDir to cleanup your scratch data
	2) ./start_service to start the backup services
	3) ./start to run fusionfs
	4) ./stop to stop fusionfs
	5) ./stop_service to stop all services

How to test fusionfs with IOZone:
	1) It's trivial to run IOZone on the local node
	2) To test multiple nodes:
		2.1) Create a file on node A, say `touch testfile1`
		2.2) On node B, run: `iozone -awpe -s 1m -r 4k -i 0 -i 1 -i 2 -f testfile1`
		2.3) Now check back in node A and verify testfile1 has 1MB by `ls testfile1 -lh`

How to test fusionfs with IOR:
	1) Install openmpi
	2) Install IOR
	3) For single node test, just run `fusion_mount/IOR`
	4) For multiple node test:
		4.1) Create a file of list of hosts, YOUR_HOSTFILE e.g.
			hec-01
			hec-02
			...
		4.2) On any node, run `fusion_mouint/mpiexec -hostfile YOUR_HOSTFILE IOR [COMMAND OPTIONS]`

Update history:
	07/26/2012: add metadata benchmark; testing IOR on 10 nodes
	07/24/2012: for ZHT values, update PATH_MAX to ZHT_MAX_BUFF; found a ZHT bug for long value (>=1K); tested IOR on Fedora and HEC; create test script and pass on 1 node 
	07/22/2012: fixed a bug in ffsnet.c::_getaddr(); tested IOzone on two nodes
	07/21/2012: update ZHT for new _lookup() signature and return code, restructure code and update Makefiles
	07/20/2012: major changes, tested on two nodes Fedora and Fusion
	07/17/2012: remote file removal supported and tested on two nodes
	07/16/2012: all one-node test cases have passed over two nodes - Fedora and Fusion.
	07/15/2012: started testing on single node (test_plan.txt added): directory passed; files on root directory passed 
	07/09/2012: clean up the warnings
	07/05/2012: hsearch replaced by ZHT
  	06/27/2012: read with UDT
	06/01/2012: read/write with LFTP
	05/22/2012: read/write with SCP

TODO:
	*[Important] Deploy FusionFS on 1K node
	*[Important] Add lock/unlock to ZHT to synchronize concurrent accesses	
	*[Important] Add data replicas
	*[Nice to Have] Instead of moving primary copy, support push-back of updates
	*[Nice to have] Support more POSIX interfaces, e.g. rename(), link(), slink() etc
	
Note:
    *If you make your desktop run ffsnetd service, please make sure no firewall is blocking this service from outside request.
		- In Fedora, you can turn off firewall by `sudo service iptables stop`
		- Note that there's no firewall inside IBM Bluegene