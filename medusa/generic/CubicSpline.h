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
 *  Created on: 09/12/2021
 *
 *  Author: Alessandro Maria Ricci
 *
 *  This library contains the implementation of the cubic spline used
 *  inside the PDF in the physics analysis. The constructor needs
 *  the knot vector and the list of the spline coefficients.
 *---------------------------------------------------------------------------*/

#ifndef MEDUSA_CUBIC_SPLINE_H
#define MEDUSA_CUBIC_SPLINE_H

// std
#include <initializer_list>

// Hydra
#include <hydra/detail/Config.h>
#include <hydra/Complex.h>
#include <hydra/detail/utility/CheckValue.h>

// Medusa
#include <medusa/generic/Constants.h>

// ROOT
#ifdef _ROOT_AVAILABLE_

#include <TROOT.h>
#include <TH1D.h>
#include <TF1.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TString.h>
#include <TStyle.h>

#endif //_ROOT_AVAILABLE_


namespace medusa {

    /*
    *  @class CubicSpline
    *  Class that provides the formulas used to implement the cubic spline in the physics analyzes.
    *  [Reference: "Simon Stemmle, PhD thesis, Heidelberg, Germany" and arXiv:1407.0748v1]
    * 
    *  The implementation of some methods is inside the CubicSpline.inl file.
    *
    *  nKnots = number of knots
    * 
    *  The intermediate functions K() and M() contain the derivatives
    *  up to the 6th order.
    */
    template<size_t nKnots>
    class CubicSpline
    {
        public:

        //-------------------------------------
        //           Constructors
        //-------------------------------------

        CubicSpline() = delete;

        // The constructor needs the knot vector and the list of the spline coefficients.
        // The dimension of the knot vector is given as a class parameter. The list must contain nKnots+2 values.
        // The user has to respect the parameter order.
        CubicSpline(const double (&knots)[nKnots], const std::initializer_list<double> SplineCoefficients)
        {
            // set knot vector and spline coefficients
            double b[nKnots+2];
            u[0] = knots[0];
            u[1] = knots[0];
            u[2] = knots[0];
            for(size_t i=0; i<nKnots; i++)
            {
                u[3+i] = knots[i];
                b[i] = *(SplineCoefficients.begin() + i);
            }
            b[nKnots] = *(SplineCoefficients.begin() + nKnots);
            b[nKnots+1] = *(SplineCoefficients.begin() + nKnots + 1);
            u[nKnots+3] = knots[nKnots-1];
            u[nKnots+4] = knots[nKnots-1];
            u[nKnots+5] = knots[nKnots-1];

            // calculate prefactors
            double P[nKnots], Q[nKnots], R[nKnots], S[nKnots];
            for(size_t i=0; i<nKnots; i++)
            {
                P[i] = (u[i+4]-u[i+1])*(u[i+4]-u[i+2])*(u[i+4]-u[i+3]);
                Q[i] = (u[i+5]-u[i+2])*(u[i+4]-u[i+2])*(u[i+4]-u[i+3]);
                R[i] = (u[i+5]-u[i+3])*(u[i+5]-u[i+2])*(u[i+4]-u[i+3]);
                S[i] = (u[i+6]-u[i+3])*(u[i+5]-u[i+3])*(u[i+4]-u[i+3]);
            }

            // calculate coefficients of normal polynomial
            double a0[4][nKnots], a1[4][nKnots], a2[4][nKnots], a3[4][nKnots];
            for(size_t i=0; i<nKnots-1; i++)
            {
                // Constant
                a0[0][i] = u[i+4]*u[i+4]*u[i+4]/P[i];

                a0[1][i] = -u[i+1]*u[i+4]*u[i+4]/P[i] - u[i+2]*u[i+4]*u[i+5]/Q[i] - u[i+3]*u[i+5]*u[i+5]/R[i];

                a0[2][i] = u[i+2]*u[i+2]*u[i+4]/Q[i] + u[i+2]*u[i+3]*u[i+5]/R[i] + u[i+3]*u[i+3]*u[i+6]/S[i];

                a0[3][i] = -u[i+3]*u[i+3]*u[i+3]/S[i];
    
                // Linear
                a1[0][i] = -3*u[i+4]*u[i+4]/P[i];

                a1[1][i] = (2*u[i+1]*u[i+4]+u[i+4]*u[i+4])/P[i]
                            + (u[i+2]*u[i+4]+u[i+2]*u[i+5]+u[i+4]*u[i+5])/Q[i]
                            + (2*u[i+3]*u[i+5]+u[i+5]*u[i+5])/R[i];

                a1[2][i] = -(2*u[i+2]*u[i+4]+u[i+2]*u[i+2])/Q[i]
                            -(u[i+2]*u[i+3]+u[i+2]*u[i+5]+u[i+3]*u[i+5])/R[i]
                            -(2*u[i+3]*u[i+6]+u[i+3]*u[i+3])/S[i];

                a1[3][i] = 3*u[i+3]*u[i+3]/S[i];
    
                // Quadratic
                a2[0][i] = 3*u[i+4]/P[i];

                a2[1][i] = -(2*u[i+4]+u[i+1])/P[i]
                            -(u[i+2]+u[i+4]+u[i+5])/Q[i]
                            -(2*u[i+5]+u[i+3])/R[i];

                a2[2][i] = (2*u[i+2]+u[i+4])/Q[i]
                            +(u[i+2]+u[i+5]+u[i+3])/R[i]
                            +(2*u[i+3]+u[i+6])/S[i];

                a2[3][i] = -3*u[i+3]/S[i];
    
                // Cubic
                a3[0][i] = -1./P[i];

                a3[1][i] = 1./P[i] + 1./Q[i] + 1./R[i];

                a3[2][i] = -1./Q[i] - 1./R[i] - 1./S[i];

                a3[3][i] = 1./S[i];

                // calculate polynomial coefficients for the current set of spline coefficients
                AS[0][i] = b[i]*a0[0][i] + b[i+1]*a0[1][i] + b[i+2]*a0[2][i] + b[i+3]*a0[3][i];
                AS[1][i] = b[i]*a1[0][i] + b[i+1]*a1[1][i] + b[i+2]*a1[2][i] + b[i+3]*a1[3][i];
                AS[2][i] = b[i]*a2[0][i] + b[i+1]*a2[1][i] + b[i+2]*a2[2][i] + b[i+3]*a2[3][i];
                AS[3][i] = b[i]*a3[0][i] + b[i+1]*a3[1][i] + b[i+2]*a3[2][i] + b[i+3]*a3[3][i];
                if(std::fabs(AS[0][i])<1e-9) AS[0][i]=0;
                if(std::fabs(AS[1][i])<1e-9) AS[1][i]=0;
                if(std::fabs(AS[2][i])<1e-9) AS[2][i]=0;
                if(std::fabs(AS[3][i])<1e-9) AS[3][i]=0;
            }

            // After last knot: Linear extrapolation
            // value of 2nd last spline at last knot
            double v=u[nKnots+2];
            AS[3][nKnots-1] = 0;
            AS[2][nKnots-1] = 0;

            // In case it should be constant:
            // AS[1][nKnots-1] = 0;
            // AS[0][nKnots-1] = AS[0][nKnots-2] + AS[1][nKnots-2]*v + AS[2][nKnots-2]*v*v + AS[3][nKnots-2]*v*v*v;

            // In case it should be linear:
            AS[1][nKnots-1] = AS[1][nKnots-2] + 2*AS[2][nKnots-2]*v + 3*AS[3][nKnots-2]*v*v;
            AS[0][nKnots-1] = AS[0][nKnots-2] + AS[1][nKnots-2]*v + AS[2][nKnots-2]*v*v + AS[3][nKnots-2]*v*v*v - AS[1][nKnots-1]*v;

            // Determine if and where this linear part gets negative
            if(AS[1][nKnots-1]<0)
            {
                NegativePart = true;
                xNegative = -AS[0][nKnots-1]/AS[1][nKnots-1];
            }
            else
            {
                NegativePart = false;
            }

            // Calculate the factorials: k!, j!, (k-n)! and (n-j)!
            factorial[0] = 1;
            factorial[1] = 1;
            factorial[2] = 2;
            factorial[3] = 6;
        }


        // The constructor needs the knot and coefficient vectors.
        // The dimension of the knot vector is given as a class parameter. The coefficient vector must contain nKnots+2 values.
        // The user has to respect the parameter order.
        CubicSpline(const double (&knots)[nKnots], double (&SplineCoefficients)[nKnots+2])
        {
            // set knot vector and spline coefficients
            double b[nKnots+2];
            u[0] = knots[0];
            u[1] = knots[0];
            u[2] = knots[0];
            for(size_t i=0; i<nKnots; i++)
            {
                u[3+i] = knots[i];
                b[i] = SplineCoefficients[i];
            }
            b[nKnots] = SplineCoefficients[nKnots];
            b[nKnots+1] = SplineCoefficients[nKnots+1];
            u[nKnots+3] = knots[nKnots-1];
            u[nKnots+4] = knots[nKnots-1];
            u[nKnots+5] = knots[nKnots-1];

            // calculate prefactors
            double P[nKnots], Q[nKnots], R[nKnots], S[nKnots];
            for(size_t i=0; i<nKnots; i++)
            {
                P[i] = (u[i+4]-u[i+1])*(u[i+4]-u[i+2])*(u[i+4]-u[i+3]);
                Q[i] = (u[i+5]-u[i+2])*(u[i+4]-u[i+2])*(u[i+4]-u[i+3]);
                R[i] = (u[i+5]-u[i+3])*(u[i+5]-u[i+2])*(u[i+4]-u[i+3]);
                S[i] = (u[i+6]-u[i+3])*(u[i+5]-u[i+3])*(u[i+4]-u[i+3]);
            }

            // calculate coefficients of normal polynomial
            double a0[4][nKnots], a1[4][nKnots], a2[4][nKnots], a3[4][nKnots];
            for(size_t i=0; i<nKnots-1; i++)
            {
                // Constant
                a0[0][i] = u[i+4]*u[i+4]*u[i+4]/P[i];

                a0[1][i] = -u[i+1]*u[i+4]*u[i+4]/P[i] - u[i+2]*u[i+4]*u[i+5]/Q[i] - u[i+3]*u[i+5]*u[i+5]/R[i];

                a0[2][i] = u[i+2]*u[i+2]*u[i+4]/Q[i] + u[i+2]*u[i+3]*u[i+5]/R[i] + u[i+3]*u[i+3]*u[i+6]/S[i];

                a0[3][i] = -u[i+3]*u[i+3]*u[i+3]/S[i];
    
                // Linear
                a1[0][i] = -3*u[i+4]*u[i+4]/P[i];

                a1[1][i] = (2*u[i+1]*u[i+4]+u[i+4]*u[i+4])/P[i]
                            + (u[i+2]*u[i+4]+u[i+2]*u[i+5]+u[i+4]*u[i+5])/Q[i]
                            + (2*u[i+3]*u[i+5]+u[i+5]*u[i+5])/R[i];

                a1[2][i] = -(2*u[i+2]*u[i+4]+u[i+2]*u[i+2])/Q[i]
                            -(u[i+2]*u[i+3]+u[i+2]*u[i+5]+u[i+3]*u[i+5])/R[i]
                            -(2*u[i+3]*u[i+6]+u[i+3]*u[i+3])/S[i];

                a1[3][i] = 3*u[i+3]*u[i+3]/S[i];
    
                // Quadratic
                a2[0][i] = 3*u[i+4]/P[i];

                a2[1][i] = -(2*u[i+4]+u[i+1])/P[i]
                            -(u[i+2]+u[i+4]+u[i+5])/Q[i]
                            -(2*u[i+5]+u[i+3])/R[i];

                a2[2][i] = (2*u[i+2]+u[i+4])/Q[i]
                            +(u[i+2]+u[i+5]+u[i+3])/R[i]
                            +(2*u[i+3]+u[i+6])/S[i];

                a2[3][i] = -3*u[i+3]/S[i];
    
                // Cubic
                a3[0][i] = -1./P[i];

                a3[1][i] = 1./P[i] + 1./Q[i] + 1./R[i];

                a3[2][i] = -1./Q[i] - 1./R[i] - 1./S[i];

                a3[3][i] = 1./S[i];

                // calculate polynomial coefficients for the current set of spline coefficients
                AS[0][i] = b[i]*a0[0][i] + b[i+1]*a0[1][i] + b[i+2]*a0[2][i] + b[i+3]*a0[3][i];
                AS[1][i] = b[i]*a1[0][i] + b[i+1]*a1[1][i] + b[i+2]*a1[2][i] + b[i+3]*a1[3][i];
                AS[2][i] = b[i]*a2[0][i] + b[i+1]*a2[1][i] + b[i+2]*a2[2][i] + b[i+3]*a2[3][i];
                AS[3][i] = b[i]*a3[0][i] + b[i+1]*a3[1][i] + b[i+2]*a3[2][i] + b[i+3]*a3[3][i];
                if(std::fabs(AS[0][i])<1e-9) AS[0][i]=0;
                if(std::fabs(AS[1][i])<1e-9) AS[1][i]=0;
                if(std::fabs(AS[2][i])<1e-9) AS[2][i]=0;
                if(std::fabs(AS[3][i])<1e-9) AS[3][i]=0;
            }

            // After last knot: Linear extrapolation
            // value of 2nd last spline at last knot
            double v=u[nKnots+2];
            AS[3][nKnots-1] = 0;
            AS[2][nKnots-1] = 0;

            // In case it should be constant:
            // AS[1][nKnots-1] = 0;
            // AS[0][nKnots-1] = AS[0][nKnots-2] + AS[1][nKnots-2]*v + AS[2][nKnots-2]*v*v + AS[3][nKnots-2]*v*v*v;

            // In case it should be linear:
            AS[1][nKnots-1] = AS[1][nKnots-2] + 2*AS[2][nKnots-2]*v + 3*AS[3][nKnots-2]*v*v;
            AS[0][nKnots-1] = AS[0][nKnots-2] + AS[1][nKnots-2]*v + AS[2][nKnots-2]*v*v + AS[3][nKnots-2]*v*v*v - AS[1][nKnots-1]*v;

            // Determine if and where this linear part gets negative
            if(AS[1][nKnots-1]<0)
            {
                NegativePart = true;
                xNegative = -AS[0][nKnots-1]/AS[1][nKnots-1];
            }
            else
            {
                NegativePart = false;
            }

            // Calculate the factorials: k!, j!, (k-n)! and (n-j)!
            factorial[0] = 1;
            factorial[1] = 1;
            factorial[2] = 2;
            factorial[3] = 6;
        }


        // ctor with other CubicSpline instance (copy ctor)
        __hydra_dual__
        CubicSpline(CubicSpline<nKnots> const& other)
        {
            u[0] = other.GetKnots()[0];
            u[1] = other.GetKnots()[1];
            u[2] = other.GetKnots()[2];
            for(size_t i=0; i<nKnots; i++)
            {
                u[3+i] = other.GetKnots()[3+i];

                AS[0][i] = other.GetOverCoeff(0,i);
                AS[1][i] = other.GetOverCoeff(1,i);
                AS[2][i] = other.GetOverCoeff(2,i);
                AS[3][i] = other.GetOverCoeff(3,i);
            }
            u[nKnots+3] = other.GetKnots()[nKnots+3];
            u[nKnots+4] = other.GetKnots()[nKnots+4];
            u[nKnots+5] = other.GetKnots()[nKnots+5];

            NegativePart = other.GetNegativePart();
            xNegative = other.GetxNegative();

            for(size_t i=0; i<4; i++)
            {
                factorial[i] = other.GetFactorial()[i];
            }
        }


        //-------------------------------------
        //       Operator= overloading
        //-------------------------------------

        __hydra_dual__
        CubicSpline<nKnots>& operator=(CubicSpline<nKnots> const& other)
        {
            if(this == &other) return *this;

            u[0] = other.GetKnots()[0];
            u[1] = other.GetKnots()[1];
            u[2] = other.GetKnots()[2];
            for(size_t i=0; i<nKnots; i++)
            {
                u[3+i] = other.GetKnots()[3+i];

                AS[0][i] = other.GetOverCoeff(0,i);
                AS[1][i] = other.GetOverCoeff(1,i);
                AS[2][i] = other.GetOverCoeff(2,i);
                AS[3][i] = other.GetOverCoeff(3,i);
            }
            u[nKnots+3] = other.GetKnots()[nKnots+3];
            u[nKnots+4] = other.GetKnots()[nKnots+4];
            u[nKnots+5] = other.GetKnots()[nKnots+5];

            NegativePart = other.GetNegativePart();
            xNegative = other.GetxNegative();

            for(size_t i=0; i<4; i++)
            {
                factorial[i] = other.GetFactorial()[i];
            }

            return *this;
        }


        //-----------------------------------------------
        //      Methods to compute the cubic spline
        //-----------------------------------------------

        // Find the respective knot number
        __hydra_dual__
        inline size_t findKnot(double x) const
        {
            size_t j = 0;
            for(size_t i=0; i<nKnots; i++)
            {
                if(x>=u[3+i]) j=i;
            }
            return j;
        }


        // Evaluate the cubic spline
        __hydra_dual__
        inline double CSplineEval(double x) const
        {
            // Should return 0 here. But small positive number is better for the fitter
            // Change this for your purpose 
            if(x>xNegative && NegativePart) return 1e-3;

            size_t j = findKnot(x);
            return AS[0][j] + AS[1][j]*x + AS[2][j]*x*x + AS[3][j]*x*x*x;
        }


        //-------------------------------
        //      Methods to integrate
        //-------------------------------

        // Integrate in t the cubic spline times the convolution of exp( -a*t )*cosh( b*t ) or exp( -a*t )*sinh( b*t )
        // with the Gaussian [tag = true -> cosh | tag = false -> sinh] (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline double Integrate_cspline_times_convolved_exp_sinhcosh(double a, double b, double mu, double sigma,
                                                                            double LowerLimit, double UpperLimit, bool tag) const;


        // Integrate in t the cubic spline times the convolution of exp( -a*t )*cos( b*t ) or exp( -a*t )*sin( b*t )
        // with the Gaussian [tag = true -> cos | tag = false -> sin] (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline double Integrate_cspline_times_convolved_exp_sincos(double a, double b, double mu, double sigma,
                                                                        double LowerLimit, double UpperLimit, bool tag) const;


        //-----------------------------------------
        //      Methods to generate plots
        //-----------------------------------------

        #ifdef _ROOT_AVAILABLE_

        //create a histogram for plotting
        TH1D* CreateHistogramPlot(std::string name, std::string names, size_t nBins, double from, double to)
        {
            TH1D* h = new TH1D(name.c_str(),names.c_str(),nBins,from,to);

            for(size_t i=0; i<nBins; i++)
            {
                h->SetBinContent(i+1,CSplineEval(from+(i+0.5)*(to-from)/nBins));
            }
            return h;
        }

        #endif //_ROOT_AVAILABLE_


        //-------------------------------------
        //           Selectors
        //-------------------------------------

        // get factorials: k!, j!, (k-n)! and (n-j)!
        __hydra_dual__
        const int* GetFactorial() const {return factorial;}

        // get knots
        __hydra_dual__
        const double* GetKnots() const {return u;}
  
        // get the overall polynomial coefficients
        __hydra_dual__
        double GetOverCoeff(size_t i, size_t j) const {return AS[i][j];}

        // get the info wether this spline will get negative at some point
        __hydra_dual__
        bool GetNegativePart() const {return NegativePart;}
        
        __hydra_dual__
        double GetxNegative() const {return xNegative;}



        private:


        //-------------------------------------------------
        //        Methods to help the integratation
        //-------------------------------------------------

        // Integrate (in x) the 3rd order polynomial times the convolution of exp( -a*t )*cosh( b*t ) or exp( -a*t )*sinh( b*t )
        // with the Gaussian [tag = true -> cosh | tag = false -> sinh] (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline double Integrate_3_order_polynomial_times_convolved_exp_sinhcosh(size_t bin, double Kz1[4], double Kz2[4],
                                                        double Mz1[4], double Mz2[4], double mu, double sigma, double powS[4], double powM[4], bool tag) const;


        // Integrate (in x) the 3rd order polynomial times the convolution of exp( -a*t )*cos( b*t ) or exp( -a*t )*sin( b*t )
        // with the Gaussian [tag = true -> cos | tag = false -> sin] (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline double Integrate_3_order_polynomial_times_convolved_exp_sincos(size_t bin, hydra::complex<double> Kz1[4], hydra::complex<double> Kz2[4],
                                                hydra::complex<double> Mz1[4], hydra::complex<double> Mz2[4], double mu, double sigma, double powS[4], double powM[4], bool tag) const;


        // Integrate (in x) t^k times the convolution of exp( -a*t )*cosh( b*t ) or exp( -a*t )*sinh( b*t )
        // with the Gaussian [tag = true -> cosh | tag = false -> sinh] (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline double Integrate_t_to_k_times_convolved_exp_sinhcosh(size_t k, double Kz1[4], double Kz2[4],
                                                        double Mz1[4], double Mz2[4], double mu, double sigma, double powS[4], double powM[4], bool tag) const;


        // Integrate (in x) t^k times the convolution of exp( -a*t )*cos( b*t ) or exp( -a*t )*sin( b*t )
        // with the Gaussian [tag = true -> cos | tag = false -> sin] (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline double Integrate_t_to_k_times_convolved_exp_sincos(size_t k, hydra::complex<double> Kz1[4], hydra::complex<double> Kz2[4],
                                                hydra::complex<double> Mz1[4], hydra::complex<double> Mz2[4], double mu, double sigma, double powS[4], double powM[4], bool tag) const;


        //------------------------------------------------------------
        //       Intermediate functions used in the integration
        //------------------------------------------------------------
        
        // K_n(z) for z as double (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline double K(double z, size_t n) const;


        // K_n(z) for z as complex (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline hydra::complex<double> K(hydra::complex<double> z, size_t n) const;


        // M_n(x1, x2; z) for z as double (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline double M(double x1, double x2, double z, size_t n) const;


        // M_n(x1, x2; z) for z as complex (Reference: arXiv:1407.0748v1)
        __hydra_dual__
        inline hydra::complex<double> M(double x1, double x2, hydra::complex<double> z, size_t n) const;


        //-------------------------
        //        Attributes
        //-------------------------

        // factorials: k!, j!, (k-n)! and (n-j)!
        int factorial[4];

        // knots
        double u[nKnots+6];

        // Overall coefficients of the polynomials. For the range x>=Knot[i] && x<Knot[i+1]
        // the spline is then given by: y=AS[0][i]+AS[1][i]*x+AS[2][i]*x*x+AS[3][i]*x*x*x
        double AS[4][nKnots];

        // The spline might get negative values due to the linear extrapolation in/after the last sector
        // Avoid this by checking if we are there and setting the spline to 0
        bool NegativePart;
        double xNegative;
    };

} // namespace medusa

#include <medusa/generic/CubicSpline.inl>

#endif // MEDUSA_CUBIC_SPLINE_H