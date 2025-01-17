#include <future>

#include <doctest/doctest.h>

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"
#include "open62541pp/open62541.h"
#include "open62541pp/services/detail/ClientService.h"
#include "open62541pp/services/detail/ResponseHandling.h"
#include "open62541pp/types/Composed.h"  // ReadResponse

#include "helper/Runner.h"

using namespace opcua;

TEST_CASE("AsyncServiceAdapter") {
    SUBCASE("createCallbackAndContext") {
        using Response = int;
        using Adapter = services::detail::AsyncServiceAdapter<Response>;

        StatusCode status;
        Response result;
        bool throwInTransform = false;
        bool throwInCompletionHandler = false;
        detail::ExceptionCatcher catcher;

        auto callbackAndContext = Adapter::createCallbackAndContext(
            catcher,
            [&](int value) {
                if (throwInTransform) {
                    throw std::runtime_error("Transform");
                }
                return value;
            },
            [&](StatusCode code, int value) {
                status = code;
                result = value;
                if (throwInCompletionHandler) {
                    throw std::runtime_error("CompletionHandler");
                }
            }
        );

        auto invokeCallback = [&](int* response) {
            callbackAndContext.callback(
                nullptr,  // client
                callbackAndContext.context.release(),
                0,  // requestId
                response
            );
        };

        Response response = 5;
        SUBCASE("Success") {
            invokeCallback(&response);
            CHECK(status == UA_STATUSCODE_GOOD);
            CHECK(result == 5);
            CHECK_FALSE(catcher.hasException());
        }
        SUBCASE("Response nullptr") {
            invokeCallback(nullptr);
            CHECK(status == UA_STATUSCODE_BADUNEXPECTEDERROR);
            CHECK(result == 0);
            CHECK_FALSE(catcher.hasException());
        }
        SUBCASE("Exception in transform function") {
            throwInTransform = true;
            invokeCallback(&response);
            CHECK(status == UA_STATUSCODE_BADINTERNALERROR);
            CHECK(result == 0);
            CHECK_FALSE(catcher.hasException());
        }
        SUBCASE("Exception in completion handler") {
            throwInCompletionHandler = true;
            invokeCallback(&response);
            CHECK(status == UA_STATUSCODE_GOOD);
            CHECK(result == response);
            CHECK(catcher.hasException());
            CHECK_THROWS_AS_MESSAGE(catcher.rethrow(), std::runtime_error, "CompletionHandler");
        }
    }
}

TEST_CASE("sendRequest") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect("opc.tcp://localhost:4840");

    auto sendReadRequest = [&](auto&& transform, auto&& token) {
        UA_ReadValueId item{};
        item.nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
        item.attributeId = UA_ATTRIBUTEID_BROWSENAME;
        UA_ReadRequest request{};
        request.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
        request.nodesToReadSize = 1;
        request.nodesToRead = &item;

        return services::detail::sendRequest<UA_ReadRequest, UA_ReadResponse>(
            client,
            request,
            std::forward<decltype(transform)>(transform),
            std::forward<decltype(token)>(token)
        );
    };

    auto checkReadResponse = [](const ReadResponse& response) {
        CHECK(response.getResponseHeader().getServiceResult().isGood());
        CHECK_EQ(
            response.getResults()[0].getValue().getScalar<QualifiedName>(),
            QualifiedName(0, "Objects")
        );
    };

    SUBCASE("Async") {
        SUBCASE("Success") {
            sendReadRequest(
                services::detail::WrapResponse<ReadResponse>{},
                [&](StatusCode code, ReadResponse& response) {
                    CHECK(code.isGood());
                    checkReadResponse(response);
                }
            );
            CHECK_NOTHROW(client.runIterate());
        }

        SUBCASE("Exception in transform function") {
            sendReadRequest(
                [](UA_ReadResponse&) { throw std::runtime_error("Error"); },
                [](StatusCode code) { CHECK(code == UA_STATUSCODE_BADINTERNALERROR); }
            );
            CHECK_NOTHROW(client.runIterate());
        }

        SUBCASE("Exception in user callback") {
            sendReadRequest(services::detail::WrapResponse<ReadResponse>{}, [](auto&&...) {
                throw std::runtime_error("Error");
            });
            CHECK_THROWS_AS_MESSAGE(client.runIterate(), std::runtime_error, "Error");
        }
    }

    SUBCASE("Sync") {
        SUBCASE("Success") {
            const auto response = sendReadRequest(
                services::detail::WrapResponse<ReadResponse>{}, services::detail::SyncOperation{}
            );
            checkReadResponse(response);
        }

        SUBCASE("Exception in transform function") {
            CHECK_THROWS_AS_MESSAGE(
                sendReadRequest(
                    [](UA_ReadResponse&) { throw std::runtime_error("Error"); },
                    services::detail::SyncOperation{}
                ),
                std::runtime_error,
                "Error"
            );
        }
    }
}
