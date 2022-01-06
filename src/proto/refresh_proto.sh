mkdir -p ${PWD}/build

protoc -I=. \
       --cpp_out=build \
       --python_out=build \
       --grpc_out=build \
       --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
       *.proto


python3 -m grpc_tools.protoc -I./ --grpc_python_out=build *.proto
