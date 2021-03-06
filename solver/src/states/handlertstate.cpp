#include <consensus.hpp>
#include <lib/system/logger.hpp>
#include <solvercontext.hpp>
#include <states/handlertstate.hpp>

namespace cs {
void HandleRTState::on(SolverContext& context) {
    auto role = context.role();
    switch (role) {
        case Role::Trusted:
            context.request_role(Role::Trusted);
            break;
        case Role::Normal:
            context.request_role(Role::Normal);
            break;
        default:
            cserror() << name() << ": unknown role requested";
            break;
    }
}

}  // namespace cs
