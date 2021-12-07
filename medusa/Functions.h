/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2020 Antonio Augusto Alves Junior, Davide Brundu,
 *                      Andrea Contu, Francesca Dordei, Piera Muzzetto,
 *                      Alessandro Maria Ricci
 *
 *   This file is part of Medusa Library.
 *
 *   Medusa is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Medusa is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Medusa.  If not, see <http://www.gnu.org/licenses/>.
 *
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 *  Created on: 06/12/2021
 *
 *  Author: Alessandro Maria Ricci
 *
 *  This library contains generic functions.
 *---------------------------------------------------------------------------*/

#ifndef MEDUSA_FUNCTIONS_H_
#define MEDUSA_FUNCTIONS_H_

// std
#include <cmath>
#include <complex>
#include <tuple>
#include <limits>
#include <stdexcept>
#include <assert.h>
#include <utility>
#include <ratio>

// Hydra
#include <hydra/detail/Config.h>
#include <hydra/Complex.h>
#include <hydra/Types.h>
#include <hydra/Vector4R.h>

// Medusa
#include <medusa/Faddeeva.h>

// macro
#define sqrt2 hydra::math_constants::sqrt2


namespace medusa {
    namespace functions {

        // This function returns the cosine of the decay theta angle in the helicity basis.
        // The decay angle calculated is that between the flight direction of the daughter meson, "D",
        // in the rest frame of "Q" (the parent of "D"), with respect to "Q"'s flight direction
        // in "P"'s (the parent of "Q") rest frame P == B0, Q = dimuon, D = muon
        // The formulas used here can be found also in EvtGen [EvtGen/src/EvtKine.cpp] (https://evtgen.hepforge.org/).
        __hydra_dual__
        inline double cos_decay_angle(hydra::Vector4R const& p, hydra::Vector4R const& q, hydra::Vector4R const& d)
        {
            // P == B0, Q = Jpsi, D = muon
            double pd = p*d;
            double pq = p*q;
            double qd = q*d;
            double mp2 = p.mass2();
            double mq2 = q.mass2();
            double md2 = d.mass2();

            return (pd * mq2 - pq * qd) / ::sqrt((pq * pq - mq2 * mp2) * (qd * qd - mq2 * md2));
        }


        // Evaluate the angle phi between two decay planes, formed by particles d2&d3 and h1&h2 correspondingly.
        // The angle is evaluated in the rest frame of "mother" particles (defined as d2+d3+h1+h2)
        // It is calculated as the angle formed by the h1 3vector projection on an x-y plane defined by d2(=x), h1+h2 (=z)
        // For LHCb convention with B0->h+h-mu+mu- ==> d2 = h-, d3=h+, h1 = mu+, h2=mu-
        // The formulas used here can be found also in EvtGen [EvtGen/src/EvtKine.cpp] (https://evtgen.hepforge.org/).
        __hydra_dual__
        inline double phi_plane_angle(hydra::Vector4R d2, hydra::Vector4R d3, hydra::Vector4R h1, hydra::Vector4R h2)
        {
            hydra::Vector4R Mother = d2 + d3 + h1 + h2;
            d2.applyBoostTo(Mother, /*inverse boost? == */ true);
            d3.applyBoostTo(Mother, /*inverse boost? == */ true);
            h1.applyBoostTo(Mother, /*inverse boost? == */ true);
            h2.applyBoostTo(Mother, /*inverse boost? == */ true);

            hydra::Vector4R D = d2 + d3;

            hydra::Vector4R d1_perp = d2 - (D.dot(d2) / D.dot(D)) * D; // d2 will be mu^+
            hydra::Vector4R h1_perp = h1 - (D.dot(h1) / D.dot(D)) * D;

            // orthogonal to both D and d1_perp
            // hydra::Vector4R d1_prime = D.cross(d1_perp);
            hydra::Vector4R d1_prime = d1_perp.cross(D);


            d1_perp  = d1_perp / d1_perp.d3mag();
            d1_prime = d1_prime / d1_prime.d3mag();

            double cos_phi, sin_phi;

            cos_phi = d1_perp.dot(h1_perp);   //cos_chi
            sin_phi = d1_prime.dot(h1_perp);  //sin_chi

            double phi = ::atan2(sin_phi, cos_phi);
  
            return (phi>=0)? phi : phi + 2*PI;
        }


        // Convolution of exp( -a*t )*cosh( b*t ) or exp( -a*t )*sinh( b*t ) with the Gaussian
        // [tag > 0 -> cosh | tag < 0 -> sinh] (Reference: arXiv:1906.08356v4)
        __hydra_dual__
        inline double Convoluted_exp_sinhcosh(double time, double a, double b, double mu, double sigma, int tag)
        {
            double x = (time - mu)/(sigma*sqrt2);

            double z1 = (a - b)*sigma/sqrt2;
            double z2 = (a + b)*sigma/sqrt2;

            double faddeeva_z1 = ::exp( z1*z1 - 2*z1*x ) * faddeeva::erfc(z1-x);
            double faddeeva_z2 = ::exp( z2*z2 - 2*z2*x ) * faddeeva::erfc(z2-x);

            double Convolution = 0;

            if(tag > 0) Convolution = 0.25*(faddeeva_z1 + faddeeva_z2);

            if(tag < 0) Convolution = 0.25*(faddeeva_z1 - faddeeva_z2);

            return Convolution;
        }


        // Convolution of exp( -a*t )*cos( b*t ) or exp( -a*t )*sin( b*t ) with the Gaussian
        // [tag > 0 -> cos | tag < 0 -> sin] (Reference: arXiv:1906.08356v4)
        __hydra_dual__
        inline double Convoluted_exp_sincos(double time, double a, double b, double mu, double sigma, int tag)
        {
            const hydra::complex<double> I(0.0, 1.0); // Imaginary unit

            double x = (time - mu)/(sigma*sqrt2);

            hydra::complex<double> z1( a*sigma/sqrt2, -b*sigma/sqrt2 );
            hydra::complex<double> z2( a*sigma/sqrt2,  b*sigma/sqrt2 );

            hydra::complex<double> faddeeva_z1 = hydra::exp( z1*z1 - 2*z1*x ) * faddeeva::erfc(z1-x);
            hydra::complex<double> faddeeva_z2 = hydra::exp( z2*z2 - 2*z2*x ) * faddeeva::erfc(z2-x);

            hydra::complex<double> result = 0;
            
            if(tag > 0) result = faddeeva_z1 + faddeeva_z2;

            if(tag < 0) result = (faddeeva_z1 - faddeeva_z2)/I;

            double Convolution = 0.25*result.real();

            return Convolution;
        }


        // Convolution of exp( -a*t )*cosh( b*t ) or exp( -a*t )*sinh( b*t ) with the Gaussian
        // integrated in the time [tag > 0 -> cosh | tag < 0 -> sinh] (Reference: arXiv:1906.08356v4)
        __hydra_dual__
        inline double Integrated_convoluted_exp_sinhcosh(double time, double a, double b, double mu, double sigma,
                                                                                double LowerLimit, double UpperLimit, int tag)
        {
            double x1 = (LowerLimit - mu)/(sigma*sqrt2);
            double x2 = (UpperLimit - mu)/(sigma*sqrt2);

            double z1 = (a - b)*sigma/sqrt2;
            double z2 = (a + b)*sigma/sqrt2;

            double cumulative_z1 = ( faddeeva::erf(x2) - ::exp( z1*z1 - 2*z1*x2 ) * faddeeva::erfc(z1-x2) -
                                            ( faddeeva::erf(x1) - ::exp( z1*z1 - 2*z1*x1 ) * faddeeva::erfc(z1-x1) ) ) / z1;

            double cumulative_z2 = ( faddeeva::erf(x2) - ::exp( z2*z2 - 2*z2*x2 ) * faddeeva::erfc(z2-x2) -
                                            ( faddeeva::erf(x1) - ::exp( z2*z2 - 2*z2*x1 ) * faddeeva::erfc(z2-x1) ) ) / z2;

            double Integrated_convolution = 0;

            if(tag > 0) Integrated_convolution = cumulative_z1 + cumulative_z2;

            if(tag < 0) Integrated_convolution = cumulative_z1 - cumulative_z2;

            return Integrated_convolution;
        }


        // Convolution of exp( -a*t )*cos( b*t ) or exp( -a*t )*sin( b*t ) with the Gaussian
        // integrated in the time [tag > 0 -> cos | tag < 0 -> sin] (Reference: arXiv:1906.08356v4)
        __hydra_dual__
        inline double Integrated_convoluted_exp_sincos(double time, double a, double b, double mu, double sigma,
                                                                                double LowerLimit, double UpperLimit, int tag)
        {
            const hydra::complex<double> I(0.0, 1.0); // Imaginary unit

            double x1 = (LowerLimit - mu)/(sigma*sqrt2);
            double x2 = (UpperLimit - mu)/(sigma*sqrt2);

            hydra::complex<double> z1( a*sigma/sqrt2, -b*sigma/sqrt2 );
            hydra::complex<double> z2( a*sigma/sqrt2,  b*sigma/sqrt2 );

            hydra::complex<double> cumulative_z1 = faddeeva::erf(x2) - hydra::exp( z1*z1 - 2*z1*x2 ) * faddeeva::erfc(z1-x2) -
                                                    ( faddeeva::erf(x1) - hydra::exp( z1*z1 - 2*z1*x1 ) * faddeeva::erfc(z1-x1) );

            hydra::complex<double> cumulative_z2 = faddeeva::erf(x2) - hydra::exp( z2*z2 - 2*z2*x2 ) * faddeeva::erfc(z2-x2) -
                                                    ( faddeeva::erf(x1) - hydra::exp( z2*z2 - 2*z2*x1 ) * faddeeva::erfc(z2-x1) );

            hydra::complex<double> result = 0;

            if(tag > 0) result  = cumulative_z1/z1 + cumulative_z2/z2;
            
            if(tag < 0) result = ( cumulative_z1/z1 - cumulative_z2/z2 ) / I;

            double Integrated_convolution = result.real();

            return Integrated_convolution;
        }

    } // namespace functions
} // namespace medusa

#endif // MEDUSA_FUNCTIONS_H