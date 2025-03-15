#include <memory>
#include <string>

#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"

#include "mygrpc/v1/greeter.grpc.pb.h"

#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using mygrpc::v1::Greeter;
using mygrpc::v1::HelloResponse;
using mygrpc::v1::HelloRequest;

class GreeterServiceImpl final : public Greeter::Service {
    Status SayHello(ServerContext *context, const HelloRequest *request, HelloResponse *reply) override {
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        return Status::OK;
    }
    Status SayHelloAgain(ServerContext *context, const HelloRequest *request, HelloResponse *reply) override {
        std::string prefix("Hello again ");
        reply->set_message(prefix + request->name());
        return Status::OK;
    }
};

void run_server(uint16_t port) {
    std::string server_address = fmt::format("0.0.0.0:{}", port);
    GreeterServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    SPDLOG_INFO("Server listening on {}", server_address);
    server->Wait();
}

int main() {
    run_server(50051);
    return 0;
}