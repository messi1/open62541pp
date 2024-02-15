#include "CustomDataTypes.h"
#include "open62541pp/TypeWrapper.h"

#include <utility>  // move

namespace opcua {

CustomDataTypes::CustomDataTypes(const UA_DataTypeArray** arrayConfig)
    : arrayConfig_(arrayConfig) {}

void CustomDataTypes::setCustomDataTypes(std::vector<DataType> dataTypes) {
    dataTypes_ = std::move(dataTypes);
    // NOLINTNEXTLINE
    array_ = std::unique_ptr<UA_DataTypeArray>(new UA_DataTypeArray{
        nullptr,  // next
        dataTypes_.size(),
        asNative(dataTypes_.data()),
    });
    *arrayConfig_ = array_.get();
}

}  // namespace opcua
