#include "open62541pp/ApplicationDescription.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/Helper.h"

#include "open62541_impl.h"

namespace opcua {

const std::string& ApplicationDescription::getUri() const {
    return detail::toString(handle()->applicationUri);
}

void ApplicationDescription::setUri(const std::string& uri) {
    TypeConverter<std::string>::toNative(uri, handle()->applicationUri);
}

const std::string& ApplicationDescription::getProductUri() const {
    return detail::toString(handle()->productUri);
}

void ApplicationDescription::setProductUri(const std::string& productUri) {
    TypeConverter<std::string>::toNative(productUri, handle()->productUri);
}

const LocalizedText& ApplicationDescription::getName() const {
    LocalizedText ret;
    TypeConverter<LocalizedText>::fromNative(handle()->applicationName, ret);
    return ret;
}

void ApplicationDescription::setName(const LocalizedText& name) {
    TypeConverter<std::string>::toNative(name.getText(), handle()->applicationName.text);
    TypeConverter<std::string>::toNative(name.getLocale(), handle()->applicationName.locale);
}

ApplicationType ApplicationDescription::getType() const {
    return static_cast<ApplicationType>(handle()->applicationType);
}

void ApplicationDescription::setType(ApplicationType type) {
    handle()->applicationType = static_cast<UA_ApplicationType>(type);
}

const std::string& ApplicationDescription::getGatewayServerUri() const {
    return detail::toString(handle()->gatewayServerUri);
}

void ApplicationDescription::setGatewayServerUri(const std::string& gatewayServerUri) {
    TypeConverter<std::string>::toNative(gatewayServerUri, handle()->gatewayServerUri);
}

const std::string& ApplicationDescription::getDiscoveryProfileUri() const {
    return detail::toString(handle()->discoveryProfileUri);
}

void ApplicationDescription::setDiscoveryProfileUri(const std::string& discoveryProfileUri) {
    TypeConverter<std::string>::toNative(discoveryProfileUri, handle()->discoveryProfileUri);
}

const std::vector<std::string>& ApplicationDescription::getDiscoveryUrls() const {
    return detail::fromNativeArray<std::string>(handle()->discoveryUrls, handle()->discoveryUrlsSize);
}

void ApplicationDescription::setDiscoveryUrls(const std::vector<std::string>& discoveryUrls) {
    handle()->discoveryUrlsSize = discoveryUrls.size();
    handle()->discoveryUrls = detail::allocNativeArray<UA_String, opcua::Type::String>(discoveryUrls.size());
    for (size_t i = 0; i < discoveryUrls.size(); ++i) {
        TypeConverter<std::string>::toNative(discoveryUrls[i], handle()->discoveryUrls[i]);
    }
}
}  // namespace opcua