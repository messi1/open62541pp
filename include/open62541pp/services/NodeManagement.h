#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

#include "open62541pp/Common.h"  // ModellingRule
#include "open62541pp/NodeIds.h"  // ReferenceTypeId
#include "open62541pp/types/NodeId.h"

// forward declaration
namespace opcua {
class Variant;
}  // namespace opcua

namespace opcua::services {

/**
 * @defgroup NodeManagement NodeManagement service set
 * Add/delete nodes and references.
 * @ingroup Services
 */

/**
 * Add child object.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addObject(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& objectType = ObjectTypeId::BaseObjectType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
);

/**
 * Add child folder.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
inline void addFolder(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
) {
    addObject(serverOrClient, parentId, id, browseName, ObjectTypeId::FolderType, referenceType);
}

/**
 * Add child variable.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addVariable(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
);

/**
 * Add child property.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
inline void addProperty(
    T& serverOrClient, const NodeId& parentId, const NodeId& id, std::string_view browseName
) {
    addVariable(
        serverOrClient,
        parentId,
        id,
        browseName,
        VariableTypeId::PropertyType,
        ReferenceTypeId::HasProperty
    );
}

/**
 * Add child object type.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addObjectType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
);

/**
 * Add child variable type.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addVariableType(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const NodeId& variableType = VariableTypeId::BaseDataVariableType,
    const NodeId& referenceType = ReferenceTypeId::HasSubtype
);

#ifdef UA_ENABLE_METHODCALLS
/**
 * Method callback.
 * @ingroup NodeManagement
 */
using MethodCallback = std::function<void(const std::vector<Variant>&, std::vector<Variant>&)>;

// forward declaration
class Argument;

/**
 * Add child method.
 * Callbacks can not be set by clients. Servers can assign callbacks to method nodes afterwards.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addMethod(
    T& serverOrClient,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    MethodCallback callback,
    const std::vector<Argument>& inputArguments,
    const std::vector<Argument>& outputArguments,
    const NodeId& referenceType = ReferenceTypeId::HasComponent
);
#endif

/**
 * Add reference.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void addReference(
    T& serverOrClient,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward = true
);

/**
 * Add modelling rule.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
inline void addModellingRule(T& serverOrClient, const NodeId& id, ModellingRule rule) {
    addReference(
        serverOrClient,
        id,
        {0, static_cast<uint32_t>(rule)},
        ReferenceTypeId::HasModellingRule,
        true
    );
}

/**
 * Delete node.
 * @exception BadStatus
 * @ingroup NodeManagement
 */
template <typename T>
void deleteNode(T& serverOrClient, const NodeId& id, bool deleteReferences = true);

// TODO: deleteReferences

}  // namespace opcua::services
