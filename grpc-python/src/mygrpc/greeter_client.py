import grpc
import mygrpc.v1.greeter_pb2 as greeter_pb2
import mygrpc.v1.greeter_pb2_grpc as greeter_pb2_grpc

def run():
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = greeter_pb2_grpc.GreeterStub(channel)
        response: greeter_pb2.HelloReponse = stub.SayHello(greeter_pb2.HelloRequest(name='you'))
        print("Greeter client received: " + response.message)
        response: greeter_pb2.HelloReponse = stub.SayHelloAgain(greeter_pb2.HelloRequest(name='you'))
        print("Greeter client received: " + response.message)
