#pragma once

#include <array>
#include <chrono>
#include <iterator>  // distance
#include <string>
#include <type_traits>
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/detail/traits.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/DateTime.h"

namespace opcua {

template <TypeIndex... typeIndexes>
struct TypeIndexList {
    using TypeIndexes = std::integer_sequence<TypeIndex, typeIndexes...>;

    static constexpr size_t size() {
        return sizeof...(typeIndexes);
    }

    static constexpr bool contains(TypeIndex typeIndex) {
        return ((typeIndex == typeIndexes) || ...);
    }

    static constexpr bool contains(Type type) {
        return contains(static_cast<TypeIndex>(type));
    }

    static constexpr auto toArray() {
        return std::array<TypeIndex, sizeof...(typeIndexes)>{typeIndexes...};
    }
};

template <typename T, typename Enable = void>
struct TypeConverter {
    static_assert(detail::AlwaysFalse<T>::value, "Missing specialization of TypeConverter");

    using ValueType = T;
    using NativeType = std::nullptr_t;
    using ValidTypes = TypeIndexList<>;

    static void fromNative(const NativeType& src, ValueType& dst);
    static void toNative(const ValueType& src, NativeType& dst);
};

/* ------------------------------------------- Helper ------------------------------------------- */

namespace detail {

template <typename T, Type type>
constexpr bool isValidTypeCombination() {
    return TypeConverter<T>::ValidTypes::contains(type);
}

template <typename T>
constexpr bool isValidTypeCombination(Type type) {
    return TypeConverter<T>::ValidTypes::contains(type);
}

template <typename T, Type type>
constexpr void assertTypeCombination() {
    static_assert(
        isValidTypeCombination<T, type>(),
        "Invalid template type / type enum (opcua::Type) combination"
    );
}

template <typename T>
constexpr Type guessType() {
    using ValueType = typename std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(
        TypeConverter<ValueType>::ValidTypes::size() == 1,
        "Ambiguous template type, please specify type enum (opcua::Type) manually"
    );
    constexpr auto typeIndexGuess = TypeConverter<ValueType>::ValidTypes::toArray().at(0);
    static_assert(typeIndexGuess < builtinTypesCount, "T doesn't seem to be a builtin type");
    return static_cast<Type>(typeIndexGuess);
}

template <typename It>
constexpr Type guessTypeFromIterator() {
    using ValueType = typename std::iterator_traits<It>::value_type;
    return guessType<ValueType>();
}

/* ------------------------------------- Converter functions ------------------------------------ */

/// Convert and copy from native type.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
[[nodiscard]] T fromNative(NativeType* value) {
    T result{};
    TypeConverter<T>::fromNative(*value, result);
    return result;
}

/// Convert and copy from native type.
/// @warning Type erased version, use with caution.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
[[nodiscard]] T fromNative(void* value, [[maybe_unused]] Type type) {
    assert(isValidTypeCombination<T>(type));  // NOLINT
    return fromNative<T>(static_cast<NativeType*>(value));
}

/// Create and convert vector from native array.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
[[nodiscard]] std::vector<T> fromNativeArray(NativeType* array, size_t size) {
    if constexpr (isBuiltinType<T>() && std::is_fundamental_v<T>) {
        return std::vector<T>(array, array + size);  // NOLINT
    } else {
        std::vector<T> result(size);
        for (size_t i = 0; i < size; ++i) {
            TypeConverter<T>::fromNative(array[i], result[i]);  // NOLINT
        }
        return result;
    }
}

/// Create and convert vector from native array.
/// @warning Type erased version, use with caution.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
[[nodiscard]] std::vector<T> fromNativeArray(void* array, size_t size, [[maybe_unused]] Type type) {
    assert(isValidTypeCombination<T>(type));  // NOLINT
    return fromNativeArray<T>(static_cast<NativeType*>(array), size);
}

/// Allocate native type.
template <typename TNative, Type type>
[[nodiscard]] TNative* allocNative() {
    assertTypeCombination<TNative, type>();
    auto* result = static_cast<TNative*>(UA_new(getUaDataType<type>()));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

/// Allocate and copy to native type.
template <typename T, Type type>
[[nodiscard]] auto* toNativeAlloc(const T& value) {
    assertTypeCombination<T, type>();
    using NativeType = typename TypeConverter<T>::NativeType;
    auto* result = allocNative<NativeType, type>();
    TypeConverter<T>::toNative(value, *result);
    return result;
}

/// Allocate native array
template <typename TNative, Type type>
[[nodiscard]] auto* allocNativeArray(size_t size) {
    assertTypeCombination<TNative, type>();
    auto* result = static_cast<TNative*>(UA_Array_new(size, getUaDataType<type>()));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

/// Allocate and copy iterator range to native array.
template <typename InputIt, Type type>
[[nodiscard]] auto* toNativeArrayAlloc(InputIt first, InputIt last) {
    using ValueType = typename std::iterator_traits<InputIt>::value_type;
    using NativeType = typename TypeConverter<ValueType>::NativeType;
    assertTypeCombination<ValueType, type>();
    const size_t size = std::distance(first, last);
    auto* result = allocNativeArray<NativeType, type>(size);
    for (size_t i = 0; i < size; ++i) {
        TypeConverter<ValueType>::toNative(*first++, result[i]);  // NOLINT
    }
    return result;
}

/// Allocate and copy to native array.
template <typename T, Type type>
[[nodiscard]] auto* toNativeArrayAlloc(const T* array, size_t size) {
    return toNativeArrayAlloc<const T*, type>(array, array + size);  // NOLINT
}

}  // namespace detail

/* ---------------------------- Implementation for native data types ---------------------------- */

namespace detail {

template <typename T, TypeIndex... typeIndexes>
struct TypeConverterNative {
    using ValueType = T;
    using NativeType = T;
    using ValidTypes = TypeIndexList<typeIndexes...>;

    static_assert(ValidTypes::size() >= 1);

    static void fromNative(const T& src, T& dst) {
        if constexpr (std::is_fundamental_v<T>) {
            dst = src;  // shallow copy
        } else {
            // just take first type -> underlying memory layout of all types should be the same
            constexpr auto typeIndexGuess = ValidTypes::toArray().at(0);
            // clear first
            UA_clear(&dst, getUaDataType<typeIndexGuess>());
            // deep copy
            const auto status = UA_copy(&src, &dst, getUaDataType<typeIndexGuess>());
            throwOnBadStatus(status);
        }
    }

    static void toNative(const T& src, T& dst) {
        fromNative(src, dst);
    }
};

}  // namespace detail

// NOLINTNEXTLINE
#define UAPP_TYPECONVERTER_NATIVE(NativeType, ...)                                                 \
    template <>                                                                                    \
    struct TypeConverter<NativeType> : detail::TypeConverterNative<NativeType, __VA_ARGS__> {};

UAPP_TYPECONVERTER_NATIVE(UA_Boolean, UA_TYPES_BOOLEAN)
UAPP_TYPECONVERTER_NATIVE(UA_SByte, UA_TYPES_SBYTE)
UAPP_TYPECONVERTER_NATIVE(UA_Byte, UA_TYPES_BYTE)
UAPP_TYPECONVERTER_NATIVE(UA_Int16, UA_TYPES_INT16)
UAPP_TYPECONVERTER_NATIVE(UA_UInt16, UA_TYPES_UINT16)
UAPP_TYPECONVERTER_NATIVE(UA_Int32, UA_TYPES_INT32)
UAPP_TYPECONVERTER_NATIVE(UA_UInt32, UA_TYPES_UINT32)
UAPP_TYPECONVERTER_NATIVE(UA_Int64, UA_TYPES_INT64)
UAPP_TYPECONVERTER_NATIVE(UA_UInt64, UA_TYPES_UINT64)
UAPP_TYPECONVERTER_NATIVE(UA_Float, UA_TYPES_FLOAT)
UAPP_TYPECONVERTER_NATIVE(UA_Double, UA_TYPES_DOUBLE)
static_assert(std::is_same_v<UA_String, UA_ByteString>);
static_assert(std::is_same_v<UA_String, UA_XmlElement>);
UAPP_TYPECONVERTER_NATIVE(UA_String, UA_TYPES_STRING, UA_TYPES_BYTESTRING, UA_TYPES_XMLELEMENT)
UAPP_TYPECONVERTER_NATIVE(UA_Guid, UA_TYPES_GUID)
UAPP_TYPECONVERTER_NATIVE(UA_NodeId, UA_TYPES_NODEID)
UAPP_TYPECONVERTER_NATIVE(UA_ExpandedNodeId, UA_TYPES_EXPANDEDNODEID)
UAPP_TYPECONVERTER_NATIVE(UA_QualifiedName, UA_TYPES_QUALIFIEDNAME)
UAPP_TYPECONVERTER_NATIVE(UA_LocalizedText, UA_TYPES_LOCALIZEDTEXT)
UAPP_TYPECONVERTER_NATIVE(UA_ExtensionObject, UA_TYPES_EXTENSIONOBJECT)

/* ------------------------------- Implementation for TypeWrapper ------------------------------- */

template <typename WrapperType>
struct TypeConverter<WrapperType, std::enable_if_t<detail::IsTypeWrapper<WrapperType>::value>> {
    using NativeConverter = TypeConverter<typename WrapperType::NativeType>;

    using ValueType = WrapperType;
    using NativeType = typename WrapperType::NativeType;
    using ValidTypes = TypeIndexList<WrapperType::getTypeIndex()>;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = WrapperType(src);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        NativeConverter::toNative(*src.handle(), dst);
    }
};

/* ---------------------------- Implementations for std library types --------------------------- */

template <>
struct TypeConverter<std::string> {
    using ValueType = std::string;
    using NativeType = UA_String;
    using ValidTypes = TypeIndexList<UA_TYPES_STRING, UA_TYPES_BYTESTRING, UA_TYPES_XMLELEMENT>;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = detail::toString(src);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        UA_clear(&dst, detail::getUaDataType<UA_TYPES_STRING>());
        dst = detail::allocUaString(src);
    }
};

template <typename Clock, typename Duration>
struct TypeConverter<std::chrono::time_point<Clock, Duration>> {
    using ValueType = std::chrono::time_point<Clock, Duration>;
    using NativeType = UA_DateTime;
    using ValidTypes = TypeIndexList<UA_TYPES_DATETIME>;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = DateTime(src).toTimePoint<Clock, Duration>();
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        dst = DateTime::fromTimePoint(src).get();
    }
};

}  // namespace opcua
