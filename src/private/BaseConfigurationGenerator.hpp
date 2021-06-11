#include <Profile/Generator.hpp>

namespace Qv2rayBase::_private
{
    class Qv2rayBasePrivateConfigurationGenerator : public Profile::IConfigurationGenerator
    {
      public:
        virtual ProfileContent ApplyRouting(const ProfileContent &profile, const RoutingObject &rid) override;
    };
} // namespace Qv2rayBase::_private
