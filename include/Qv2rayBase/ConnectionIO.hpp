#pragma once
#include "Qv2rayBase.hpp"

namespace Qv2ray::core::connection::connectionIO
{
    QV2RAYBASE_EXPORT CONFIGROOT ConvertConfigFromFile(const QString &sourceFilePath, bool importComplex);
} // namespace Qv2ray::core::connection::connectionIO

using namespace Qv2ray::core::connection;
using namespace Qv2ray::core::connection::connectionIO;
