#pragma once

#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/AttributeId.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/MonitoringMode.h"

#include <functional>
#include <optional>

namespace opcua {

using DataChangedCallback = std::function<void(uint32_t, DataValue)>;
using AttributeChangedCallback = std::function<void(uint32_t, AttributeId, DataValue)>;

struct MonitoredItem {
    MonitoredItem() = default;
    ~MonitoredItem() = default;
    MonitoredItem(MonitoredItem&&) = default;
    MonitoredItem& operator=(MonitoredItem&&) = default;
    MonitoredItem(const MonitoredItem&) = default;
    MonitoredItem(MonitoredItem&) = default;
    MonitoredItem& operator=(const MonitoredItem&) = default;


    uint32_t monitorId;

    bool isMonitoringAttribute{false};
    bool isMonitoringData{false};

    AttributeId monitoredAttributeId;
    std::optional<DataChangedCallback> dataChangedCallback;
    std::optional<AttributeChangedCallback> attributeChangedCallback;
    MonitoringMode monitoringMode;

    std::optional<DataValue> lastReading;
};

}  // namespace opcua