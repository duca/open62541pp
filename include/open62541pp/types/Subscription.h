#pragma once

#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/MonitoredItem.h"
#include "open62541pp/types/NodeId.h"

#include <functional>
#include <inttypes.h>
#include <memory>
#include <unordered_map>

namespace opcua {

struct Subscription {
    uint32_t subscriptionId;
    std::unordered_map<NodeId, std::unordered_map<NodeId, std::shared_ptr<MonitoredItem>>>
        m_nodeHandleToItemMapping;  // Handle -> Attribute -> MonitoredItem
    std::unordered_map<NodeId, std::shared_ptr<MonitoredItem>>
        m_itemIdToItemMapping;  // ItemId -> Item for fast lookup on data change
};

}  // namespace opcua
