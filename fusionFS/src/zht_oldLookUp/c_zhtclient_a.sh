#force overwritten
gcc -g -c meta.pb.cc novoht.cxx net_util.cpp zht_util.cpp cpp_zhtclient.cpp c_zhtclient.cpp


#lib_name="czhtclient"
#obj_main="c_zhtclient_main"
#ar -rcv lib$lib_name.a d3_tcp.o d3_udp.o meta.pb.o net_util.o novoht.o zht_util.o c_zhtclient.o client_general.o
#gcc -g $obj_main.c -o $obj_main -L. -l$lib_name -lstdc++ -lprotobuf
#./$obj_main
#rm -rf $obj_main *.o *.a


ar -rcv libczhtclient.a meta.pb.o novoht.o net_util.o zht_util.o cpp_zhtclient.o c_zhtclient.o
gcc -g c_zhtclient_main.c -o c_zhtclient_main -L. -lczhtclient -lstdc++ -lprotobuf -lpthread

./c_zhtclient_main neighbor zht.cfg TCP

#rm -rf c_zhtclient_main *.o *.a
cp libczhtclient.a c-zhtclient-test
rm -rf *.o



