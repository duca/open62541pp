#pragma once

#include "open62541pp/Client.h"
#include "open62541pp/types/AttributeId.h"
#include "open62541pp/types/MonitoredItem.h"
#include "open62541pp/types/MonitoringMode.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Subscription.h"

#include <chrono>
#include <memory>
#include <optional>

namespace opcua {
namespace services {

/**
 * @brief Create a subscription to a client
 * @param client opcua client
 * @param period period of the update
 * @return shared_ptr to Subscription
 * @todo add the possibility of customize the request
 * @throws std::runtime_error if the client is unavailable
 */
std::shared_ptr<Subscription> createSubscription(
    std::weak_ptr<Client> client, std::chrono::milliseconds period
);

/**
 * @brief Remove the subscription from the client
 * @param client opcua client
 * @param subscription subscription to remove
 * @throws std::runtime_error if the client is unavailable
 */
void removeSubscription(std::weak_ptr<Client> client, std::weak_ptr<Subscription> subscription);

/**
 * @brief register a callback for a given attribute of a given node
 * @param client opcua client
 * @param subscription subscription to update
 * @param attributeId attribute id
 * @param mode monitoring mode
 * @param callback from the user
 * @throws std::runtime_error if the client or the subscription is unavailable
 * @todo add the possibility of customize the request
 */
std::shared_ptr<MonitoredItem> monitorAttributeFromNode(
    std::weak_ptr<Client> client,
    std::weak_ptr<Subscription> subscription,
    const NodeId& nodeId,
    AttributeId attributeId,
    MonitoringMode mode,
    std::optional<AttributeChangedCallback> callback
);

/**
 * @register registers a callback of data changes from a NodeId
 * @param client opcua client
 * @param subscription
 * @param nodeId
 * @param callback user callback
 * @return shared_ptr to MonitoredItem
 * @throws std::runtime_error if the client or the subscription is unavailable
 * @todo add the possibility of customize the request
 */
std::shared_ptr<MonitoredItem> monitorValueChangesFromNode(
    std::weak_ptr<Client> client,
    std::weak_ptr<Subscription> subscription,
    const NodeId& nodeId,
    std::optional<DataChangedCallback> callback
);

/**
 * @brief run the iterate method from the UA_client
 * @param client
 * @param timeout
 */

void iterateSubscription(std::shared_ptr<Client> client, uint32_t timeout = 1000);

}  // namespace services
}  // namespace opcua
