#start the service for file transfer
./src/ffsnet/ffsnetd 2>&1 1>/dev/null &  

#start the service for ZHT server
./src/zht/bin/server_zht 50000 ./src/zht/neighbor ./src/zht/zht.cfg TCP 2>&1 1>/dev/null &