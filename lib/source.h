#ifndef SOURCE_H
#define SOURCE_H

#include "walker.h"
#include "distributions.h"

namespace MCPP {

/**
 * @brief Base Source class: constructs and initializes a Walker according to
 * given source term modeling.
 */

class Source : public BaseRandom
{
public:
    Source(BaseObject *parent=NULL);
    ~Source();

    void spin(Walker *walker) const;

#ifdef SWIG
    %apply SWIGTYPE *DISOWN {AbstractDistribution *x0Distribution, AbstractDistribution *y0Distribution};
#endif
    void setr0Distribution(AbstractDistribution *x0Distribution, AbstractDistribution *y0Distribution, double _z0);

#ifdef SWIG
    %apply SWIGTYPE *DISOWN {AbstractDistribution *cosThetaDistr, AbstractDistribution *psiDistr};
#endif
    void setk0Distribution(AbstractDistribution *cosThetaDistr, AbstractDistribution *psiDistr);

#ifdef SWIG
    %apply SWIGTYPE *DISOWN {AbstractDistribution *distr };
#endif
    void setWalkTimeDistribution(AbstractDistribution *distr);

    void setWavelength(double um);
    MCfloat wavelength() const;
    MCfloat z0() const;

protected:
    AbstractDistribution *r0Distribution[3];
    AbstractDistribution *cosThetaDistribution;
    AbstractDistribution *psiDistribution;
    AbstractDistribution *walkTimeDistribution;
    virtual void spinDirection(Walker *walker) const;
    virtual void spinPosition(Walker *walker) const;
    void spinTime(Walker *walker) const;
    virtual BaseObject *clone_impl() const;
    void cloneInto(Source *src) const;
    MCfloat _z0;

private:
    MCfloat wl;
};




/**
 * @brief Pencil beam source \f$ \delta(\bm{r}) \, \delta (t) \, \delta
 * (\bm{k}_z) \f$, with \f$ \bm{k}_z = (0,0,1) \f$.
 */

class PencilBeamSource : public Source
{
public:
    PencilBeamSource(BaseObject *parent=NULL);
    ~PencilBeamSource();

private:
    virtual void spinDirection(Walker *walker) const;
    virtual void spinPosition(Walker *walker) const;
    virtual BaseObject *clone_impl() const;
};




/**
 * @brief 2-dimensional \f$ xy \f$ Gaussian intensity profile
 *
 * Basic modeling of a Gaussian beam. Injection points are generated on the
 * \f$ z=0 \f$ plane accordingly to given FWHM values specified. Injection
 * directions are set to \f$ \bm{k} = \delta (\bm{k}_z) \f$ regardless of the
 * injection point (see GaussianRayBundleSource for a more elaborate modeling).
 * Time distribution is left unspecified, and must be set serparately.
 */

class GaussianBeamSource : public Source
{
public:
    GaussianBeamSource(double FWHM, BaseObject *parent=NULL);
    GaussianBeamSource(double xFWHM, double yFWHM, BaseObject *parent=NULL);
    ~GaussianBeamSource();

private:
    virtual BaseObject *clone_impl() const;

    void init(MCfloat xFWHM, MCfloat yFWHM);
    MCfloat xFWHM, yFWHM;
};





/**
 * @brief Isotropic point source \f$ \delta (0,0,z_0) \delta (t) \f$.
 *
 * @warning Place this source carefully, i.e. not within, say, a non-scattering
 * layer with high-to-low refractive index boundary conditions (because of total
 * internal reflection the simulation would eventually get stuck indefinitely
 * propagating trapped walkers).
 */

class IsotropicPointSource : public Source
{
public:
    IsotropicPointSource(double z0, BaseObject *parent=NULL);
    ~IsotropicPointSource();

private:
    virtual BaseObject *clone_impl() const;

    virtual void describe_impl() const;
};

}
#endif // SOURCE_H
