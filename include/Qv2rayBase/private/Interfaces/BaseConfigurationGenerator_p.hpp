#include "Interfaces/IConfigurationGenerator.hpp"

namespace Qv2rayBase::Interfaces
{
    class Qv2rayBasePrivateConfigurationGenerator : public IConfigurationGenerator
    {
      public:
        virtual ProfileContent ApplyRouting(const ProfileContent &profile, const RoutingObject &rid) override;
    };
} // namespace Qv2rayBase::_private
