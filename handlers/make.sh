g++ -std=c++0x -I/usr/local/include -pthread -g -c -o files.pb.o files.pb.cc
g++ -std=c++0x -I/usr/local/include -pthread  -g -c -o curl_test.o curl_test.c
g++ files.pb.o curl_test.o -g -L/usr/local/lib -lgrpc++ -lgrpc -lcurl -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl -o curl_test


gcc -g -I /root/chencq/code/libfcgi-master/include/ -lfcgi  test_fastcgi.c -o test_fastcgi
