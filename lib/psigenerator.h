#ifndef PSIGENERATOR_H
#define PSIGENERATOR_H
#include "distributions.h"

namespace MCPP {

/**
 * @brief The IsotropicPsiGenerator class generates the azimuthal scattering angle
 * uniformly in the interval \f$ \psi \in [0, 2\pi) \f$.
 */

class IsotropicPsiGenerator : public AbstractDistribution
{
public:
    IsotropicPsiGenerator(BaseObject *parent=NULL);

    virtual MCfloat spin() const;

private:
    virtual BaseObject* clone_impl() const;
};

}
#endif // PSIGENERATOR_H
