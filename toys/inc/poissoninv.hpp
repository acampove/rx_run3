/////////////////////////////////////////////////////////////////
//                                                             //
// This software was written by Mike Giles, 2014               //
// Modified by Da Yu Tou, September 2018                       //
//                                                             //
// It is copyright University of Oxford, and provided under    //
// the terms of the GNU GPLv3 license:                         //
// http://www.gnu.org/licenses/gpl.html                        //
//                                                             //
// Commercial users wanting to use the software under a more   //
// permissive license, such as BSD, should contact the author: //
// mike.giles@maths.ox.ac.uk                                   //
//                                                             //
/////////////////////////////////////////////////////////////////

// standard math header file

#ifndef POISSONINV_HPP
#define POISSONINV_HPP

#include <math.h>

// declare prototype for inverse Normal CDF function
// defined at the bottom of this header file
/**
 * \namespace PoissonInverse
 * \brief A namespace containing the inverse poisson functions.
 * \brief The important functions are poissinv(double, double) and poisscinv(double, double)
 */
namespace PoissonInverse {

    /**
     * \brief Calculates the inverse poisson given CDF and a mean.
     * \param U The value of the CDF
     * \param Lam The mean value of the poisson distribution.
     */
    double poissinv(double U, double Lam);

    /**
     * \brief Calculates the inverse poisson complement given CDF and a mean.
     * \param V The value of the CDF
     * \param Lam The mean value of the poisson distribution.
     */
    double poisscinv(double V, double Lam);

    double normcdfinv_as241(double);

    //
    // This double precision function computes the inverse
    // of the Poisson CDF
    //
    // u   = CDF value in range (0,1)
    // lam = Poisson rate
    //
    // For lam < 1e15,  max |error| no more than 1
    //  ave |error| < 1e-16*max(4,lam) for lam < 1e9
    //              < 1e-6             for lam < 1e15
    //
    // For lam > 1e15, the errors will be about 1 ulp.
    //

    // As described in the TOMS paper, there are two versions;
    // the first is optimised for MIMD execution, whereas
    // the second is designed for vector execution

    double        poissinv_core(double, double, double);
    double        poissinv_v(double U, double Lam);
    inline double poissinv_core(double U, double V, double Lam);
}   // namespace PoissonInverse

#endif