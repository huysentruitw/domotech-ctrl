#pragma once

#include <Filter.h>

#include <memory>
#include <optional>

enum class CreateFilterStatus
{
    NoError = 0,

    FilterAlreadyExists = 1,
    FailedToStoreFilter = 2,
    FailedToParseConnections = 3,
    UnknownFilterType = 4,

    PinDirectionsMismatch = 10,
    UnknownRemoteModule = 11,
    LocalPinIndexOutOfRange = 12,
    RemotePinIndexOutOfRange = 13,
    PinConnectionFailed = 14,
};

struct CreateFilterResult
{
    CreateFilterStatus Status;
    std::optional<size_t> FailedAtMappingIndex = std::nullopt;
};
