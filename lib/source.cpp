#include "source.h"
#include "costhetagenerator.h"
#include "psigenerator.h"
#include "math.h"

Source::Source(BaseObject *parent) :
    BaseRandom(parent)
{
    walkTimeDistribution = NULL;
    cosThetaDistribution = NULL;
    psiDistribution = NULL;
    for (int i = 0; i < 3; ++i) {
        r0Distribution[i] = NULL;
    }
}

void Source::spinDirection(Walker *walker) const {
    double cosTheta = cosThetaDistribution->spin();
    double sinTheta = sqrt(1 - pow(cosTheta,2));
    double psi = psiDistribution->spin();
    double cosPsi = cos(psi);
    double sinPsi = sin(psi);

    walker->k0[0] = sinTheta*cosPsi;
    walker->k0[1] = sinTheta*sinPsi;
    walker->k0[2] = cosTheta;
}

void Source::spinPosition(Walker *walker) const {
    for (int i = 0; i < 3; ++i) {
        walker->r0[i] = r0Distribution[i]->spin();
    }
}

void Source::spinTime(Walker *walker) const {
    walker->walkTime = walkTimeDistribution->spin();    // overwrapping?
}

/**
 * @brief Constructs a new walker.
 * @return A pointer to the newly constructed walker.
 */

Walker* Source::constructWalker() const {
    Walker *walker = new Walker();

    spinPosition(walker); //the calling order is critical! GaussianRayBundleSource must spin the walker's position BEFORE calculating the direction vector
    spinDirection(walker);
    spinTime(walker);

    return walker;
}

void Source::setr0Distribution(AbstractDistribution **distrArray) {
    for (int i = 0; i < 3; ++i) {
        if(r0Distribution[i] != NULL)
            delete r0Distribution[i];
        r0Distribution[i] = distrArray[i];
        r0Distribution[i]->setParent(this);
    }
}

void Source::setk0Distribution(AbstractDistribution *cosThetaDistr, AbstractDistribution *psiDistr) {
    if(cosThetaDistribution != NULL)
        delete cosThetaDistribution;
    cosThetaDistribution = cosThetaDistr;
    cosThetaDistribution->setParent(this);
    if(psiDistribution != NULL)
        delete psiDistribution;
    psiDistribution = psiDistr;
    psiDistribution->setParent(this);
}

void Source::setWalkTimeDistribution(AbstractDistribution *distr) {
    walkTimeDistribution = distr;
    walkTimeDistribution->setParent(this);
}

void Source::setWavelength(double um) {
    wl = um;
}

double Source::wavelength() const {
    return wl;
}




PencilBeamSource::PencilBeamSource(BaseObject *parent) :
    Source(parent)
{

}

void PencilBeamSource::spinDirection(Walker *walker) const {
    walker->k0[2] = 1;
}

void PencilBeamSource::spinPosition(Walker *walker) const {

}

void PencilBeamSource::spinTime(Walker *walker) const {

}




/**
 * @brief Constructs a cylindrically symmetric gaussian 2D intensity profile in
 *        the \f$ z=0\f$ plane
 * @param FWHM full width half maximum of the profile
 * @param parent
 */

GaussianBeamSource::GaussianBeamSource(double FWHM, BaseObject *parent) :
    Source(parent)
{
    NormalDistribution *distr = new NormalDistribution(0,FWHM, this);
    for (int i = 0; i < 2; ++i) {
        r0Distribution[i] = distr;
    }
    r0Distribution[2] = new DeltaDistribution(0, this);
}

/**
 * @brief Constructs a non-cylindrically symmetric gaussian 2D intensity profile
 *        in the \f$ z=0\f$ plane
 * @param xFWHM full width half maximum of the profile along the \f$ x \f$ axis
 * @param yFWHM full width half maximum of the profile along the \f$ y \f$ axis
 * @param parent
 */

GaussianBeamSource::GaussianBeamSource(double xFWHM, double yFWHM, BaseObject *parent) :
    Source(parent)
{
    r0Distribution[0] = new NormalDistribution(0,xFWHM, this);
    r0Distribution[1] = new NormalDistribution(0,yFWHM, this);
    r0Distribution[2] = new DeltaDistribution(0, this);
}

/**
 * \copydoc Source::constructWalker()
 * \pre walkTimeDistribution has to be valid
 */

void GaussianBeamSource::spinDirection(Walker *walker) const {
    if(cosThetaDistribution != NULL || psiDistribution != NULL) //check if one of the k0Distributions has been overridden
        Source::spinDirection(walker);
    walker->k0[2] = 1;
}




IsotropicPointSource::IsotropicPointSource(double z0, BaseObject *parent) :
    Source(parent)
{
    depth = z0;
    cosThetaDistribution = new CosThetaGenerator(0);
    psiDistribution = new IsotropicPsiGenerator;
}

void IsotropicPointSource::spinPosition(Walker *walker) const {
    walker->r0[2] = depth;
}

void IsotropicPointSource::spinTime(Walker *walker) const {

}
