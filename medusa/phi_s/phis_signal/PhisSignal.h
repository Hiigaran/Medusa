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
 *  PhisSignal.h
 *
 *  Created on: 14/06/2021
 *      Author: Alessandro Maria Ricci
 * 
 *  Reference: arXiv:1906.08356v4
 *---------------------------------------------------------------------------*/

#ifndef PHIS_SIGNAL_H_
#define PHIS_SIGNAL_H_


// std
#include <cmath>
#include <tuple>
#include <limits>
#include <stdexcept>
#include <assert.h>
#include <utility>
#include <ratio>


// Hydra
#include <hydra/detail/Config.h>
#include <hydra/detail/BackendPolicy.h>
#include <hydra/Types.h>
#include <hydra/Function.h>
#include <hydra/Pdf.h>
#include <hydra/Tuple.h>
#include <hydra/functions/Utils.h>
#include <hydra/functions/Math.h>


// Medusa
#include <medusa/phi_s/Parameters.h>
#include <medusa/generic/Functions.h>


namespace medusa {

    /*
    *  @class PhisSignal
    *  Functor that provides the time dependent formula used in phi_s analysis in the only-signal model
    *  (see Eq. (9) in arXiv:1906.08356v4)
    * 
    *  The implementation of the method Update_ATCoefficients() is inside the detail/ folder
    *
    *  B0sbar   = boolean to specify wether B is B0sbar or not
    *  ArgTypes = argument types of the functor
    */
    template<bool B0sbar,
             typename ArgTypeTime,
             typename ArgTypeCosThetah,
             typename ArgTypeCosThetal,
             typename ArgTypePhi,
             typename Signature=double(ArgTypeTime, ArgTypeCosThetah, ArgTypeCosThetal, ArgTypePhi) >
    class PhisSignal: public hydra::BaseFunctor< PhisSignal< B0sbar, ArgTypeTime, ArgTypeCosThetah, ArgTypeCosThetal, ArgTypePhi >, Signature, 17>
    {

        using ThisBaseFunctor = hydra::BaseFunctor< PhisSignal<B0sbar, ArgTypeTime, ArgTypeCosThetah, ArgTypeCosThetal, ArgTypePhi >, Signature, 17 >;

        using hydra::BaseFunctor< PhisSignal< B0sbar, ArgTypeTime, ArgTypeCosThetah, ArgTypeCosThetal, ArgTypePhi >, Signature, 17 >::_par;


        public:

        //-------------------------------------
        //           Constructors
        //-------------------------------------

        PhisSignal() = delete;

        // ctor with list of hydra::Parameter
        // the user has to respect the parameter order
        PhisSignal(hydra::Parameter const& A_02,          hydra::Parameter const& A_perp2,     hydra::Parameter const& A_S2, 
                   hydra::Parameter const& DeltaGamma_sd, hydra::Parameter const& DeltaGamma,  hydra::Parameter const& DeltaM,
                   hydra::Parameter const& phi_0,         hydra::Parameter const& phi_par0,    hydra::Parameter const& phi_perp0,
                   hydra::Parameter const& phi_S0,        hydra::Parameter const& lambda_0,    hydra::Parameter const& lambda_par0,       
                   hydra::Parameter const& lambda_perp0,  hydra::Parameter const& lambda_S0,   hydra::Parameter const& delta_par0,
                   hydra::Parameter const& delta_perp0,   hydra::Parameter const& delta_Sperp):
        ThisBaseFunctor({A_02, A_perp2, A_S2,
                         DeltaGamma_sd, DeltaGamma, DeltaM,
                         phi_0, phi_par0, phi_perp0, phi_S0,
                         lambda_0, lambda_par0, lambda_perp0, lambda_S0,
                         delta_par0, delta_perp0, delta_Sperp })
        {
            Update();
        }


        // ctor with array of hydra::Parameter
        // the user has to respect the parameter order as the main ctor
        explicit PhisSignal( const hydra::Parameter (&Hs)[17] ):
        ThisBaseFunctor{ Hs[0], Hs[1], Hs[2],  Hs[3],  Hs[4],  Hs[5],  Hs[6], Hs[7],
                         Hs[8], Hs[9], Hs[10], Hs[11], Hs[12], Hs[13], Hs[14], Hs[15], Hs[16] }
        {
            Update();
        }


        // ctor with array of double
        // the user has to respect the parameter order as the main ctor
        explicit PhisSignal( const double (&Hs)[17] ):
        ThisBaseFunctor{ Hs[0], Hs[1], Hs[2],  Hs[3],  Hs[4],  Hs[5],  Hs[6], Hs[7],
                         Hs[8], Hs[9], Hs[10], Hs[11], Hs[12], Hs[13], Hs[14], Hs[15], Hs[16] }
        {
            Update();
        }


        // ctor with other PhisSignal instance (copy ctor)
        __hydra_dual__
        PhisSignal(PhisSignal<B0sbar, ArgTypeTime, ArgTypeCosThetah, ArgTypeCosThetal, ArgTypePhi> const& other):
        ThisBaseFunctor(other)
        {
    	    #pragma unroll 10
    	    for(size_t i=0; i<10; i++)
    	    {   
                // angular coefficients a_k, b_k, c_k, d_k in  Eq. (10) and (11) in arXiv:1906.08356v4
                // polarization factor N_k in Eq. (9) in arXiv:1906.08356v4
    		    A.k[i] = other.GetA().k[i];
    		    B.k[i] = other.GetB().k[i];
    		    C.k[i] = other.GetC().k[i];
    		    D.k[i] = other.GetD().k[i];
    		    N.k[i] = other.GetN().k[i];
    	    }
        }


        //-------------------------------------
        //       Operator= overloading
        //-------------------------------------

        __hydra_dual__
        PhisSignal<B0sbar, ArgTypeTime, ArgTypeCosThetah, ArgTypeCosThetal, ArgTypePhi>& 
        operator=( PhisSignal<B0sbar, ArgTypeTime, ArgTypeCosThetah, ArgTypeCosThetal, ArgTypePhi> const& other)
        {
            if(this == &other) return *this;
            ThisBaseFunctor::operator=(other);

		    #pragma unroll 10
            for(size_t i=0; i<10; i++)
            {
                // angular coefficients a_k, b_k, c_k, d_k in  Eq. (10) and (11) in arXiv:1906.08356v4
                // polarization factor N_k in Eq. (9) in arXiv:1906.08356v4
        	    A.k[i] = other.GetA().k[i];
        	    B.k[i] = other.GetB().k[i];
        	    C.k[i] = other.GetC().k[i];
        	    D.k[i] = other.GetD().k[i];
        	    N.k[i] = other.GetN().k[i];
            }

            return *this;
        }


        //-------------------------------------
        //         Service functions
        //-------------------------------------

        // update the values of the angular coefficients a_k, b_k, c_k, d_k and the polarization factor N_k
        // by using the formulas in Table 3 in arXiv:1906.08356v4
        void Update(void) override
        {
            Update_ATCoefficients();
            Update_NFactors();
        }


        // evaluate the sum in Eq. (9) in arXiv:1906.08356v4
        __hydra_dual__ 
        inline double Evaluate(ArgTypeTime time, ArgTypeCosThetah costheta_h, ArgTypeCosThetal costheta_l, ArgTypePhi phi) const
        {
            /*
		     0: A_0^2,
		     1: A_perp^2,
		     2: A_S^2,
		     3: DeltaGamma_sd,
		     4: DeltaGamma,
		     5: DeltaM
             */

            double A_par2 = 1 - _par[0] - _par[1];
            
            double UnnormPDF = 0.;

            // This is a safety mechanism that is necessary when A_par2 takes negative values (see Update_NFactors()).
            // UnnormPDF = 0 enables the Hydra safety mechanism for whom FCN = FcnMaxValue (see main function).
            if(A_par2 < 0)
            {
                return UnnormPDF;
            }

            auto F = parameters::AngularFunctions(costheta_h, costheta_l, phi);

            double T1 = 0.5 * time * _par[4];
            double T2 = time * _par[5];

            double chT1 = ::cosh(T1);
            double shT1 = ::sinh(T1);
            double cT2 = ::cos(T2);
            double sT2 = ::sin(T2);

            #pragma unroll 10
            for(size_t i=0; i<10; i++)
            {
            	UnnormPDF += F.fk[i]*N.k[i]*Time_Factor(i, time, chT1, shT1, cT2, sT2);
            }


            // This macro controls if result is NaN. If yes, it prints a warning with the parameter values for whom we obtain a NaN.
            hydra::CHECK_VALUE(UnnormPDF, "par[0]=%f, par[1]=%f, par[2]=%f, par[3]=%f, par[4]=%f, par[5]=%f, "
                                          "par[6]=%f, par[7]=%f, par[8]=%f, par[9]=%f, par[10]=%f, par[11]=%f, "
                                          "par[12]=%f, par[13]=%f, par[14]=%f, par[15]=%f, par[16]=%f",
                                          _par[0], _par[1], _par[2], _par[3], _par[4], _par[5],
                                          _par[6], _par[7], _par[8], _par[9], _par[10], _par[11],
                                          _par[12], _par[13], _par[14], _par[15], _par[16]);

            // This is a safety mechanism that is necessary when the functor takes negative values due to the numerical errors.
            // Don't use the function ::abs(), because it changes the values of about 10^{-03} - 10^{-04} units.
            // Don't disable this mechanism, otherwise, there could be anomalous behaviors in the fcn computation.
            if(UnnormPDF < 0)
            {
                return UnnormPDF = 0.;
            }
            else
            {
                return UnnormPDF;
            }
        }


        //-------------------------------------
        //           Selectors
        //-------------------------------------

        __hydra_dual__
	    const parameters::AngularTimeCoefficients& GetA() const {return A;}

        __hydra_dual__
	    const parameters::AngularTimeCoefficients& GetB() const {return B;}

        __hydra_dual__
	    const parameters::AngularTimeCoefficients& GetC() const {return C;}

        __hydra_dual__
	    const parameters::AngularTimeCoefficients& GetD() const {return D;}

        __hydra_dual__
	    const parameters::NFactors& GetN() const {return N;}



        private:


        // update the values of the angular coefficients a_k, b_k, c_k, d_k
        // by using the formulas in Table 3 in arXiv:1906.08356v4
        // (see implementation in Update_ATCoefficients.inl)
        void Update_ATCoefficients();


        // update the values of the polarization factor N_k
        // by using the formulas in Table 3 in arXiv:1906.08356v4
        void Update_NFactors()
        {
    	    /*
    	    0: A_0^2,
            1: A_perp^2,
    	    2: A_S^2
    	    */

           double A_par2 = 1 - _par[0] - _par[1];

           if(A_par2 >= 0)
           {
    	        N.k[0] = _par[0];                      //A_0*A_0 ;
    	        N.k[1] = A_par2;                       //A_par*A_par
    	        N.k[2] = _par[1];                      //A_perp*A_perp;
    	        N.k[3] = ::sqrt(_par[1]*A_par2);       //A_perp*A_par;
    	        N.k[4] = ::sqrt(_par[0]*A_par2);       //A_0*A_par;
    	        N.k[5] = ::sqrt(_par[0]*_par[1]);      //A_0*A_perp;
    	        N.k[6] = _par[2];                      //A_S*A_S;
    	        N.k[7] = ::sqrt(_par[2]*A_par2);       //A_S*A_par;
    	        N.k[8] = ::sqrt(_par[2]*_par[1]);      //A_S*A_perp;
    	        N.k[9] = ::sqrt(_par[2]*_par[0]);      //A_S*A_0;
           }
        }


        // time factors h_k(t|Bs0) and h_k(t|Bs0bar) in Eq. (10) and (11) in arXiv:1906.08356v4
        __hydra_dual__
        inline double Time_Factor(int index, double time, double chT1, double shT1, double cT2, double sT2) const
        {
		    /*
		    0: A_0,
		    1: A_perp,
		    2: A_S,
		    3: DeltaGamma_sd,
		    4: DeltaGamma,
		    5: DeltaM,
		    */

    	    static const double f = 0.238732414638; // 3./(4*PI)

            double time_factor = f * ::exp( -(_par[3] + 0.65789) * time) *
                                        ( A.k[index]*chT1 + B.k[index]*shT1 +
                                            ( C.k[index]*cT2 + D.k[index]*sT2 )*CP );

            return time_factor;
        }



        constexpr static int CP = (B0sbar ? -1 : 1);

        parameters::NFactors N;                     // polarization factor N_k
        parameters::AngularTimeCoefficients A;      // angular coefficient a_k
        parameters::AngularTimeCoefficients B;      // angular coefficient b_k
        parameters::AngularTimeCoefficients C;      // angular coefficient c_k
        parameters::AngularTimeCoefficients D;      // angular coefficient d_k

    };

} // namespace medusa

// Medusa
#include <medusa/phi_s/phis_signal/details/Update_ATCoefficients.inl>
#include <medusa/phi_s/phis_signal/details/IntegrationFormula.inl>

#endif /* PHIS_SIGNAL_H_ */