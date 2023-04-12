#pragma once

#include "open62541pp/Client.h"
#include "open62541pp/types/AttributeId.h"
#include "open62541pp/types/MonitoredItem.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Subscription.h"

#include <chrono>
#include <memory>

namespace opcua {
namespace services {

    /**
 * @brief Create a subscription to a client
 * @param client opcua client
 * @param period period of the update
 * @return shared_ptr to Subscription
 */
std::shared_ptr<Subscription> createSubscription(std::weak_ptr<Client> client, std::chrono::milliseconds period
);

/**
 * @brief Remove the subscription from the client
 * @param client opcua client
 * @param subscription subscription to remove
 */
void removeSubscription(std::weak_ptr<Client> client, std::weak_ptr<Subscription> subscription);

/**
 * @brief register a callback for a given attribute of a given node
 * @param client opcua client
 * @param subscription subscription to update
 * @param attributeId attribute id
 * @param callback from the user
 */
std::shared_ptr<MonitoredItem> monitorAttributeFromNode(
    std::weak_ptr<Subscription> subscription, const NodeId& nodeId, AttributeId attributeId,
    AttributeChangedCallback callback
);

/**
 * @register registers a callback of data changes from a NodeId
 * @param subscription
 * @param nodeId
 * @param callback
 * @return
 */
std::shared_ptr<MonitoredItem> monitorValueChangesFromNode(
    std::weak_ptr<Subscription> subscription, const NodeId& nodeId,
    DataChangedCallback callback
);

}  // namespace services
}  // namespace opcua
