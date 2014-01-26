#ifndef PSIGENERATOR_H
#define PSIGENERATOR_H
#include "distributions.h"

/**
 * @brief The IsotropicPsiGenerator class generates the azimuthal scattering angle
 * uniformly in the interval \f$ \psi \in [0, 2\pi) \f$.
 */

class IsotropicPsiGenerator : public UniformDistribution
{
public:
    IsotropicPsiGenerator(BaseObject *parent=NULL);
};

#endif // PSIGENERATOR_H
