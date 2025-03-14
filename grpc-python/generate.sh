#!/bin/bash

python -m grpc_tools.protoc -Iprotos --python_out=src --pyi_out=src --grpc_python_out=src protos/mygrpc/v1/greeter.proto