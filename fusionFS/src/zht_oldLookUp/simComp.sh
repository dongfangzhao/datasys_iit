sourceFile=$1
fileName=$(echo $sourceFile | awk -F '.' '{print $1}')

g++ -g -Xlinker -zmuldefs -I/usr/local/include/google/protobuf/ $sourceFile -L/usr/local/lib -lstdc++ -lrt -lpthread -lm -lc -lprotobuf -lprotoc meta.pb.cc novoht.cxx net_util.cpp zht_util.cpp cpp_zhtclient.cpp -o $fileName
