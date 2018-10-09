#pragma once
#include "TrustedState.h"

namespace slv2
{
    /**
     * @class   TrustedVState
     *
     * @brief   A trusted node state with vectors completed, but matrices still not. This class
     *          cannot be inherited
     *
     * @author  Alexander Avramenko
     * @date    09.10.2018
     *
     * @sa  T:TrustedState  
     *
     * ### remarks  Aae, 30.09.2018.
     */

    class TrustedVState final : public TrustedState
    {
    public:

        ~TrustedVState() override
        {}

        void on(SolverContext& context) override;

        Result onVector(SolverContext& context, const Credits::HashVector& vect, const PublicKey& sender) override;

        // onMatrix() behaviour is completely implemented in TrustesState

        const char * name() const override
        {
            return "TrustedV";
        }

    };

} // slv2
