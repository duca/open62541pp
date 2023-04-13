//
// Created by elopes1 on 11/04/23.
//

#include "open62541pp/services/SubscriptionManager.h"

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>

#include <sstream>

namespace {
auto constexpr LAST_CHECK_TIMEOUT = 100;
}

namespace opcua {
namespace services {

inline std::shared_ptr<Client> lockOrThrowClient(std::weak_ptr<Client> client) {
    auto sharedClient = client.lock();

    if (!sharedClient) {
        throw std::runtime_error("Client has been destroyed");
    }
    return sharedClient;
}

inline std::shared_ptr<Subscription> lockOrThrowSubscription(
    std::weak_ptr<Subscription> subscription
) {
    auto sharedSubscription = subscription.lock();

    if (!sharedSubscription) {
        throw std::runtime_error("Subscription has been destroyed");
    }
    return sharedSubscription;
}

inline std::shared_ptr<MonitoredItem> lockOrThrowMonitoredItems(std::weak_ptr<MonitoredItem> item) {
    auto sharedItem = item.lock();

    if (!sharedItem) {
        throw std::runtime_error("MonitoredItem has been destroyed");
    }
    return sharedItem;
}

static void monitoredValueChangedCallback(
    UA_Client* client,
    UA_UInt32 subId,
    void* subContext,
    UA_UInt32 monId,
    void* monContext,
    UA_DataValue* value
) {
    (void)client;
    (void)subId;
    (void)subContext;

    auto monitoredItem = static_cast<MonitoredItem*>(monContext);

    if (monitoredItem) {
        // saving the values just in case the callback is not available
        monitoredItem->lastReading = DataValue(*value);
        monitoredItem->monitorId = monId;

        // run the callback
        if (monitoredItem->dataChangedCallback) {
            std::invoke(monitoredItem->dataChangedCallback.value(), monId, DataValue(*value));
        }
    }
}

std::shared_ptr<Subscription> createSubscription(
    std::weak_ptr<Client> client, std::chrono::milliseconds period
) {
    auto sharedClient = lockOrThrowClient(client);

    auto subscription = std::make_shared<Subscription>();

    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    request.requestedPublishingInterval = period.count();

    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(
        sharedClient->handle(), request, subscription.get(), NULL, NULL
    );

    subscription->subscriptionId = response.subscriptionId;
    subscription->revisedPublishingInterval = static_cast<double>(response.revisedPublishingInterval
    );
    subscription->revisedLifetimeCount = static_cast<double>(response.revisedLifetimeCount);
    subscription->revisedPublishingInterval = static_cast<double>(response.revisedPublishingInterval
    );

    return subscription;
}

void removeSubscription(std::weak_ptr<Client> client, std::shared_ptr<Subscription> subscription) {
    auto sharedClient = lockOrThrowClient(client);

    UA_Client_run_iterate(sharedClient->handle(), LAST_CHECK_TIMEOUT);
    /* Delete the subscription */
    if (UA_Client_Subscriptions_deleteSingle(
            sharedClient->handle(), subscription->subscriptionId
        ) != UA_STATUSCODE_GOOD) {
        throw std::runtime_error("Failed to delete subscription");
    }
}

std::shared_ptr<MonitoredItem> monitorAttributeFromNode(
    std::weak_ptr<Client> client,
    std::weak_ptr<Subscription> subscription,
    const NodeId& nodeId,
    AttributeId attributeId,
    MonitoringMode mode,
    std::optional<AttributeChangedCallback> callback
) {
    auto sharedClient = lockOrThrowClient(client);

    auto sharedSubscription = lockOrThrowSubscription(subscription);

    auto monitoredItem = std::make_shared<MonitoredItem>();

    if (callback) {
        monitoredItem->attributeChangedCallback = callback;
    }

    UA_MonitoredItemCreateRequest item;
    UA_MonitoredItemCreateRequest_init(&item);
    item.itemToMonitor.nodeId = *(nodeId.handle());
    item.itemToMonitor.attributeId = static_cast<UA_UInt32>(attributeId);
    item.monitoringMode = static_cast<UA_MonitoringMode>(mode);

    // hint
    // https://github.com/qt/qtopcua/blob/5.15.2/src/plugins/opcua/open62541/qopen62541subscription.cpp
    throw std::runtime_error("Not implemented yet");
}

std::shared_ptr<MonitoredItem> monitorValueChangesFromNode(
    std::weak_ptr<Client> client,
    std::weak_ptr<Subscription> subscription,
    const NodeId& nodeId,
    std::optional<DataChangedCallback> callback
) {
    auto sharedClient = lockOrThrowClient(client);

    auto sharedSubscription = lockOrThrowSubscription(subscription);

    auto monitoredItem = std::make_shared<MonitoredItem>();

    UA_MonitoredItemCreateRequest monRequest = UA_MonitoredItemCreateRequest_default(
        *(nodeId.handle())
    );

    UA_MonitoredItemCreateResult monResponse = UA_Client_MonitoredItems_createDataChange(
        sharedClient->handle(),
        sharedSubscription->subscriptionId,
        UA_TIMESTAMPSTORETURN_BOTH,
        monRequest,
        monitoredItem.get(),
        monitoredValueChangedCallback,
        NULL
    );
    if (monResponse.statusCode != UA_STATUSCODE_GOOD) {
        std::stringstream ss;
        ss << "Failed to create monitored item for node: "
           << nodeId.getIdentifierAs<opcua::String>().get();
        throw std::runtime_error(ss.str());
    }
    if (callback) {
        monitoredItem->dataChangedCallback = callback;
    }

    return monitoredItem;
}

void iterateSubscription(std::shared_ptr<Client> client, uint32_t timeout) {
    /* The first publish request should return the initial value of the variable */
    UA_Client_run_iterate(client->handle(), timeout);
}

}  // namespace services
}  // namespace opcua