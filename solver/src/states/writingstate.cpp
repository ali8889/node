#include <consensus.hpp>
#include <solvercontext.hpp>
#include <writingstate.hpp>

namespace cs {

void WritingState::on(SolverContext& context) {
    // simply try to spawn next round
    csdebug() << name() << ": spawn next round";
    context.sendRoundTable();
}

}  // namespace cs
