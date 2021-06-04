#include "Generation.hpp"

namespace Qv2ray::core::connection::generation::misc
{
    QJsonObject GenerateAPIEntry(const QString &tag, bool withHandler, bool withLogger, bool withStats)
    {
        QJsonObject root;
        QJsonArray services;

        services << "ReflectionService";

        if (withHandler)
            services << "HandlerService";

        if (withLogger)
            services << "LoggerService";

        if (withStats)
            services << "StatsService";

        root["services"] = services;
        root["tag"] = tag;
        return root;
    }
} // namespace Qv2ray::core::connection::generation::misc
