import grpc
from concurrent import futures
from mygrpc.v1.greeter_pb2_grpc import GreeterServicer, add_GreeterServicer_to_server
from mygrpc.v1.greeter_pb2 import HelloRequest, HelloReponse

class Greeter(GreeterServicer):

    def SayHello(self, request: HelloRequest, context) -> HelloReponse:
        return HelloReponse(message=f"Hello, {request.name}")
    
    def SayHelloAgain(self, request: HelloRequest, context) -> HelloReponse:
        return HelloReponse(message=f"Hello again, {request.name}")

def serve():
    port = "50051"
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    add_GreeterServicer_to_server(Greeter(), server)
    server.add_insecure_port("[::]:" + port)
    server.start()
    print("Server started, listening on " + port)
    server.wait_for_termination()
