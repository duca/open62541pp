//
// Created by elopes1 on 11/04/23.
//

#include "open62541pp/services/SubscriptionManager.h"

namespace opcua {
namespace services {

std::shared_ptr<Subscription> createSubscription(std::weak_ptr<Client> client, std::chrono::milliseconds period
) {
    return {};
}
void removeSubscription(std::weak_ptr<Client> client, std::shared_ptr<Subscription> subscription) {}

std::shared_ptr<MonitoredItem> monitorAttributeFromNode(
    std::weak_ptr<Subscription> subscription, const NodeId& nodeId, AttributeId attributeId
){
    return {};
}

std::shared_ptr<MonitoredItem> monitorValueChangesFromNode(
    std::weak_ptr<Subscription> subscription, const NodeId& nodeId
){
    return {};
}

}  // namespace services
}  // namespace opcua