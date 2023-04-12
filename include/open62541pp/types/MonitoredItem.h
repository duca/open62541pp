#pragma once

#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/AttributeId.h"
#include "open62541pp/types/NodeId.h"

#include <functional>
#include <optional>

namespace opcua {

using DataChangedCallback = std::function<void(uint32_t, DataValue)>;
using AttributeChangedCallback = std::function<void(uint32_t, AttributeId, DataValue)>;

struct MonitoredItem {
    NodeId monitorId;

    bool isMonitoringAttribute{false};
    bool isMonitoringData{false};

    AttributeId monitoredAttributeId;
    DataChangedCallback dataChangedCallback;
    AttributeChangedCallback attributeChangedCallback;

    std::optional<DataValue> lastReading;
};

}  // namespace opcua