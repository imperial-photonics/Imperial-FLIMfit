//=========================================================================
//
// Copyright (C) 2013 Imperial College London.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// This software tool was developed with support from the UK 
// Engineering and Physical Sciences Council 
// through  a studentship from the Institute of Chemical Biology 
// and The Wellcome Trust through a grant entitled 
// "The Open Microscopy Environment: Image Informatics for Biological Sciences" (Ref: 095931).
//
// Author : Sean Warren
//
//=========================================================================

#include "FretDecayGroup.h"
#include "ParameterConstraints.h"


#include <stdio.h>
#include <boost/lexical_cast.hpp>

FretDecayGroup::FretDecayGroup(int n_donor_exponential, int n_fret_populations, bool include_donor_only) :
   MultiExponentialDecayGroupPrivate(n_donor_exponential, true, "FRET Decay"),
   n_fret_populations(n_fret_populations),
   include_donor_only(include_donor_only),
   kappa_factor(n_kappa)
{   
   std::vector<ParameterFittingType> fixed_or_global = { Fixed, FittedGlobally };
   std::vector<ParameterFittingType> fixed = { Fixed };
   A_parameter = std::make_shared<FittingParameter>("A", 1, 1, fixed, Fixed);
   Q_parameter = std::make_shared<FittingParameter>("Q", 1, 0, 4, 1, fixed_or_global, Fixed);
   Qsigma_parameter = std::make_shared<FittingParameter>("Qsigma", 0.1, 0, 4, 1, fixed_or_global, Fixed);
   tauA_parameter = std::make_shared<FittingParameter>("tauA", 4000, 500, 8000, 1e-3, fixed_or_global, Fixed);
   
   setupParameters();
}

FretDecayGroup::FretDecayGroup(const FretDecayGroup& obj) : 
   MultiExponentialDecayGroupPrivate(obj),
   kappa_factor(n_kappa)
{
   n_fret_populations = obj.n_fret_populations;
   include_donor_only = obj.include_donor_only;
   include_acceptor = obj.include_acceptor;
   A_parameter = obj.A_parameter;
   Q_parameter = obj.Q_parameter;
   Qsigma_parameter = obj.Qsigma_parameter;
   tauA_parameter = obj.tauA_parameter;
   tauT_parameters = obj.tauT_parameters;
   acceptor_channel_factors = obj.acceptor_channel_factors;

   setupParameters();
   init();
}

void FretDecayGroup::setupParameters()
{
   setupParametersMultiExponential();

   parameters.push_back(A_parameter);
   resizeLifetimeParameters(tauT_parameters, n_fret_populations, "tauT_");

   if (include_acceptor)
   {
      parameters.push_back(Q_parameter);
      parameters.push_back(Qsigma_parameter);
      parameters.push_back(tauA_parameter);
   }

   channel_factor_names.clear();
   channel_factor_names.push_back("Donor");
   if (include_acceptor)
      channel_factor_names.push_back("Acceptor");

   parametersChanged();
}

void FretDecayGroup::init()
{
   //for (auto& b : beta_parameters)
   //   if (b->isFittedGlobally())
   //      throw std::runtime_error("Fitting beta parameters in FRET model not currently implemented");

   MultiExponentialDecayGroupPrivate::init();

   n_lin_components = n_fret_populations + include_donor_only;

   for (auto& p : tauT_parameters)
      n_nl_parameters += p->isFittedGlobally();
   
   if (include_acceptor)
   {
      n_nl_parameters += Q_parameter->isFittedGlobally();
      n_nl_parameters += Qsigma_parameter->isFittedGlobally();
      n_nl_parameters += tauA_parameter->isFittedGlobally();
   }

   fret_buffer.clear();
   fret_buffer.resize(n_fret_populations);
   for (int i = 0; i < n_fret_populations; i++)
   {
      fret_buffer[i].resize(n_kappa);
      for (int j = 0; j < n_kappa; j++)
         fret_buffer[i][j] = AbstractConvolver::make_vector(n_exponential, dp);
   }
 
   if (include_acceptor)
      acceptor_buffer = AbstractConvolver::make(dp);
   
   norm_acceptor_channel_factors.resize(dp->n_chan, 1);
   normaliseChannelFactors(acceptor_channel_factors, norm_acceptor_channel_factors);
}

void FretDecayGroup::setNumExponential(int n_exponential_)
{
   n_exponential = n_exponential_;
   setupParameters();
}

void FretDecayGroup::setNumFretPopulations(int n_fret_populations_)
{
   n_fret_populations = n_fret_populations_;
   setupParameters();
}

void FretDecayGroup::setIncludeDonorOnly(bool include_donor_only_)
{
   include_donor_only = include_donor_only_;
   setupParameters();
}

void FretDecayGroup::setIncludeAcceptor(bool include_acceptor_)
{
   include_acceptor = include_acceptor_;
   setupParameters();
}

const std::vector<double>& FretDecayGroup::getChannelFactors(int index)
{
   if (index == 0)
      return channel_factors;
   else if (index == 1)
      return acceptor_channel_factors;

   throw std::runtime_error("Bad channel factor index");
}

void FretDecayGroup::setChannelFactors(int index, const std::vector<double>& channel_factors_)
{
   if (index == 0)
      channel_factors = channel_factors_;
   else if (index == 1)
      acceptor_channel_factors = channel_factors_;
   else
      throw std::runtime_error("Bad channel factor index");
}



/*
   Set up matrix indicating which parmeters affect which column.
   Each row of the matrix corresponds to a variable
*/
void FretDecayGroup::setupIncMatrix(std::vector<int>& inc, int& inc_row, int& inc_col)
{
   int n_fret_group = n_fret_populations + include_donor_only;
   int inc_col0 = inc_col;

   int* id = &inc[0];

   // Set diagonal elements of incidence matrix for variable tau's   
   for (int i = 0; i < n_exponential; i++)
   {
      if (tau_parameters[i]->isFittedGlobally())
      {
         for (int j = 0; j < n_fret_group; j++)
            inc[inc_row + (inc_col + j) * 12] = 1;
         inc_row++;
      }
   }

   // Set diagonal elements of incidence matrix for variable beta's   
   if (n_exponential > 1)
      for (int i = 0; i < n_exponential; i++)
      {
         if (beta_parameters[i]->isFittedGlobally())
         {
            for (int j = 0; j < n_fret_group; j++)
               inc[inc_row + (inc_col + j) * 12] = 1;
            inc_row++;
         }
      }

   if (include_donor_only)
      inc_col++;

   // Set elements of incidence matrix for E derivatives
   for (int i = 0; i < n_fret_populations; i++)
   {
      if (tauT_parameters[i]->isFittedGlobally())
      {
         inc[inc_row + (inc_col + i) * 12] = 1;
         inc_row++;
      }
   }

   // Set elements of incidence matrix for Q 
   if (include_acceptor && Q_parameter->isFittedGlobally())
   {
      for (int i = include_donor_only; i < n_fret_group; i++)
         inc[inc_row + (inc_col0 + i) * 12] = 1;
      inc_row++;
   }

   // Elements for direct acceptor
   if (include_acceptor && Qsigma_parameter->isFittedGlobally())
   {
      for (int i = 0; i < n_fret_group; i++)
         inc[inc_row + (inc_col0 + i) * 12] = 1;
      inc_row++;
   }

   // Elements for tauA derivatives
   if (include_acceptor && tauA_parameter->isFittedGlobally())
   {
      for (int i = 0; i < n_fret_group; i++)
         inc[inc_row + (inc_col0 + i) * 12] = 1;
      inc_row++;
   }

   inc_col += n_fret_populations;
}

int FretDecayGroup::setVariables(const double* param_values)
{
   int idx = MultiExponentialDecayGroupPrivate::setVariables(param_values);

   A = A_parameter->initial_value;

   tau_fret.resize(n_fret_populations, std::vector<std::vector<double>>(n_kappa, std::vector<double>(n_exponential)));
   tau_transfer.resize(n_fret_populations, std::vector<double>(n_kappa));

   // Set tau's for FRET
   for (int i = 0; i<n_fret_populations; i++)
   {
      double tau_transfer_0 = tauT_parameters[i]->getValue<double>(param_values, idx);
      tau_transfer_0 = std::max(tau_transfer_0, 50.0);
      
      for (int k = 0; k < n_kappa; k++)
      {
         tau_transfer[i][k] = tau_transfer_0 / kappa_factor.f[k];
         for (int j = 0; j<n_exponential; j++)
         {
            double rate = 1 / tau[j] + 1 / tau_transfer[i][k];
            fret_buffer[i][k][j]->compute(rate, irf_idx, t0_shift);
            tau_fret[i][k][j] = 1 / rate;
         }
      }
   }

   if (include_acceptor)
   {
      a_star.resize(n_fret_populations, std::vector<std::vector<double>>(n_kappa, std::vector<double>(n_exponential)));
      
      Q = Q_parameter->getValue<double>(param_values, idx);
      Qsigma = Qsigma_parameter->getValue<double>(param_values, idx);
      tauA = tauA_parameter->getValue<double>(param_values, idx);

      tauA = std::max(tauA, 50.0);

      
      acceptor_buffer->compute(1 / tauA, irf_idx, t0_shift);

      for (int i = 0; i < n_fret_populations; i++)
         for(int k = 0; k < n_kappa; k++)
            for (int j = 0; j < n_exponential; j++)
            {
               double kd = 1 / tau[j];
               double ka = 1 / tauA;
               double kt = 1 / tau_transfer[i][k];

               a_star[i][k][j] = kt / (kd + kt - ka);
            }
   }

   return idx;
}

int FretDecayGroup::getNonlinearOutputs(float* nonlin_variables, float* output, int& nonlin_idx)
{
   int output_idx = MultiExponentialDecayGroupPrivate::getNonlinearOutputs(nonlin_variables, output, nonlin_idx);

   output[output_idx++] = A_parameter->getValue<float>(nonlin_variables, nonlin_idx);

   float* output_tauT = output + output_idx;
   for (int i = 0; i < n_fret_populations; i++)
      output[output_idx++] = std::max(tauT_parameters[i]->getValue<float>(nonlin_variables, nonlin_idx), 50.f);

   if (include_acceptor)
   {
      output[output_idx++] = Q_parameter->getValue<float>(nonlin_variables, nonlin_idx);
      output[output_idx++] = Qsigma_parameter->getValue<float>(nonlin_variables, nonlin_idx);
      output[output_idx++] = std::max(tauA_parameter->getValue<float>(nonlin_variables, nonlin_idx), 50.f);
   }

   float* tau = output;
   float* beta = output + n_exponential;
   for (int i = 0; i < n_fret_populations; i++)
   {
      float E = 0;
      for (int j = 0; j < n_exponential; j++)
      {
         float tauF = 1.0f / (1.0f / tau[j] + 1.0f / output_tauT[i]);
         float Ei = 1.0f - tauF / tau[j];
         float b = (n_exponential > 1) ? beta[j] : 1; // we only have beta reported if n_exp > 1
         E += Ei * b;
      }
      output[output_idx++] = E;
   }

   return output_idx;
}


int FretDecayGroup::getLinearOutputs(float* lin_variables, float* output, int& lin_idx)
{
   int output_idx = 0;

   int n_fret_group = include_donor_only + n_fret_populations;
   output_idx += normaliseLinearParameters(lin_variables, n_fret_group, output + output_idx, lin_idx);

   return output_idx;
}

std::vector<std::string> FretDecayGroup::getLinearOutputParamNames()
{
   std::vector<std::string> names;
   names.push_back("I_0");

   int n = n_fret_populations + include_donor_only;

   if (n > 1)
   {
      if (include_donor_only)
         names.push_back("gamma_0");

      if (n)
         for (int i = 0; i < n_fret_populations; i++)
         {
            std::string name = "gamma_" + boost::lexical_cast<std::string>(i + 1);
            names.push_back(name);
         }
   }
   return names;
}

std::vector<std::string> FretDecayGroup::getNonlinearOutputParamNames()
{
   auto names = AbstractDecayGroup::getNonlinearOutputParamNames();
   
   for (int i = 0; i < n_fret_populations; i++)
   {
      std::string name = "E_" + boost::lexical_cast<std::string>(i + 1);
      names.push_back(name);
   }

   return names;
}


int FretDecayGroup::calculateModel(double* a, int adim, double& kap, int bin_shift)
{
   int col = 0;

   if (include_donor_only)
   {
      memset(a + col*adim, 0, adim*sizeof(*a));
      addDecayGroup(buffer, 1, a + col*adim, adim, kap, bin_shift);
      if (include_acceptor)   
         acceptor_buffer->addDecay(Qsigma, norm_acceptor_channel_factors, reference_lifetime, a + col*adim, bin_shift);
      col++;
   }

   for (int i = 0; i < fret_buffer.size(); i++)
   {
      memset(a + col*adim, 0, adim*sizeof(*a));
      for(int k=0; k<n_kappa; k++)
         addDecayGroup(fret_buffer[i][k], kappa_factor.p[k], a + col*adim, adim, kap, bin_shift);
      
      if (include_acceptor)
      {
         addAcceptorContribution(i, Q, a + col*adim, adim, kap, bin_shift);
         acceptor_buffer->addDecay(Qsigma, norm_acceptor_channel_factors, reference_lifetime, a + col*adim, bin_shift);
      }
      
//TODO      kap += kappaLim(tau_transfer[i]);

      col++;
   }

   if (include_acceptor)
      kap += kappaLim(tauA);

   // Scale for brightness
   for (int i = 0; i < col*adim; i++)
      a[i] *= A;

   return col;
}

void FretDecayGroup::addAcceptorContribution(int i, double factor, double* a, int adim, double& kap, int bin_shift)
{
   if (include_acceptor)
   {
      double a_star_sum = 0;
      for (int j = 0; j < n_exponential; j++)
         for (int k = 0; k < n_kappa; k++)
         {
            double f = factor * kappa_factor.p[k] * beta[j] * a_star[i][k][j];
            fret_buffer[i][k][j]->addDecay(-f, norm_acceptor_channel_factors, reference_lifetime, a, bin_shift); // rise time
            a_star_sum += f;
         }
      
      acceptor_buffer->addDecay(a_star_sum, norm_acceptor_channel_factors, reference_lifetime, a, bin_shift);
   }
}

void FretDecayGroup::addAcceptorDerivativeContribution(int i, int j, int k, double fact, double* b, int bdim, double& kap_derv)
{
   if (include_acceptor)
   {
      double f = fact * a_star[i][k][j];
      fret_buffer[i][k][j]->addDecay(-f, norm_acceptor_channel_factors, reference_lifetime, b); // rise time
      acceptor_buffer->addDecay(f, norm_acceptor_channel_factors, reference_lifetime, b);
   }
}


int FretDecayGroup::calculateDerivatives(double* b, int bdim, double_iterator& kap_derv)
{
   int col = 0;
   for (int i = 0; i < n_exponential; i++)
   {
      if (tau_parameters[i]->isFittedGlobally())
      {
         if (include_donor_only)
            col += addLifetimeDerivative(i, b + col*bdim, bdim);
         col += addLifetimeDerivativesForFret(i, b + col*bdim, bdim, kap_derv);
         addLifetimeKappaDerivative(i, kap_derv);
      }
   }

   // TODO: if we add kappa here, need to sort out col -> iv referencing
   col += addContributionDerivativesForFret(b + col*bdim, bdim, kap_derv);
   col += addFretEfficiencyDerivatives(b + col*bdim, bdim, kap_derv);

   if (include_acceptor)
   {
      col += addAcceptorIntensityDerivatives(b + col*bdim, bdim, kap_derv);
      col += addDirectAcceptorDerivatives(b + col*bdim, bdim, kap_derv);
      col += addAcceptorLifetimeDerivatives(b + col*bdim, bdim, kap_derv);
   }

   // Scale for brightness
   for (int i = 0; i < col*bdim; i++)
      b[i] *= A;


   return col;
}



int FretDecayGroup::addLifetimeDerivativesForFret(int j, double* b, int bdim, double_iterator& kap_derv)
{
   int col = 0;
   int idx = 0;

   // d(fret)/d(tau)
   for (int i = 0; i<n_fret_populations; i++)
   {
      memset(b + idx, 0, bdim*sizeof(*b));

      double fact = beta[j] / (tau[j] * tau[j]);
      
      for (int k = 0; k < n_kappa; k++)
      {
         double fact_k = fact * kappa_factor.p[k];

         fret_buffer[i][k][j]->addDerivative(fact_k, norm_channel_factors, reference_lifetime, b + idx);

         if (include_acceptor)
         {
            double acceptor_fact = Q * a_star[i][k][j] * fact_k;
            fret_buffer[i][k][j]->addDerivative(-acceptor_fact, norm_acceptor_channel_factors, reference_lifetime, b + idx);

            acceptor_fact *= tau_transfer[i][k];
            addAcceptorDerivativeContribution(i, j, k, acceptor_fact, b + idx, bdim, kap_derv[col]);
         }
      }

      col++;
      idx += bdim;
   }

   return col;
}

int FretDecayGroup::addContributionDerivativesForFret(double* b, int bdim, double_iterator& kap_derv)
{
   if (n_exponential < 2)
      return 0;

   int col = 0;
   int n_fret_group = n_fret_populations + include_donor_only;

   int ji = 0;
   for (int j = 0; j < n_exponential; j++)
      if (beta_parameters[j]->isFittedGlobally())
      {
         for (int i = 0; i < n_fret_group; i++)
         {
            memset(b + col*bdim, 0, bdim * sizeof(*b));
            int qi = ji;
            for (int q = j; q < n_exponential; q++)
               if (!beta_parameters[q]->isFixed())
               {
                  for (int k = 0; k < n_kappa; k++)
                  {
                     double factor = beta_derv(n_beta_free, ji, qi, beta_param_values) * (1 - fixed_beta);
                     if (i == 0 && k == 0 && include_donor_only)
                     {
                        buffer[q]->addDecay(factor, norm_channel_factors, reference_lifetime, b + col*bdim);
                     }
                     else if (i > 0 || !include_donor_only)
                     {
                        double factor_k = factor * kappa_factor.p[k];
                        int fret_idx = i - include_donor_only;
                        fret_buffer[fret_idx][k][q]->addDecay(factor_k, norm_channel_factors, reference_lifetime, b + col*bdim);
                        if (include_acceptor)
                        {
                           fret_buffer[fret_idx][k][q]->addDecay(-Q * factor_k * a_star[fret_idx][k][q], norm_acceptor_channel_factors, reference_lifetime, b + col * bdim); // rise time
                           acceptor_buffer->addDecay(Q * factor_k * a_star[fret_idx][k][q], norm_acceptor_channel_factors, reference_lifetime, b + col * bdim);
                        }
                     }
                     qi++;
                  }
               }
            col++;
         }
         kap_derv++;
         ji++;
      }

   return col;
}

int FretDecayGroup::addFretEfficiencyDerivatives(double* b, int bdim, double_iterator& kap_derv)
{
   int col = 0;
   int idx = 0;

   for (int i = 0; i<n_fret_populations; i++)
   {
      if (tauT_parameters[i]->isFittedGlobally())
      {
         memset(b + idx, 0, bdim*sizeof(*b));

         for (int k = 0; k < n_kappa; k++)
         {
            for (int j = 0; j < n_exponential; j++)
            {
               double fact = beta[j] / (kappa_factor.f[k] * tau_transfer[i][k] * tau_transfer[i][k]) * kappa_factor.p[k];
               fret_buffer[i][k][j]->addDerivative(fact, norm_channel_factors, reference_lifetime, b + idx);

               if (include_acceptor)
               {
                  double acceptor_fact = -Q * a_star[i][k][j] * fact;
                  fret_buffer[i][k][j]->addDerivative(acceptor_fact, norm_acceptor_channel_factors, reference_lifetime, b + idx);

                  acceptor_fact = beta[j] * Q * a_star[i][k][j] * (1 / tauA - 1 / tau[j]) * kappa_factor.p[k] / kappa_factor.f[k];
                  addAcceptorDerivativeContribution(i, j, k, acceptor_fact, b + idx, bdim, *kap_derv);
               }

            }
         }

//         *kap_derv = -kappaLim(tau_transfer[i]);


         col++;
         idx += bdim;
         kap_derv++;
      }
   }

   return col;
}

int FretDecayGroup::addDirectAcceptorDerivatives(double* b, int bdim, double_iterator& kap_derv)
{
   int col = 0;
   int idx = 0;

   if (Qsigma_parameter->isFittedGlobally())
   {
      int n_fret_group = n_fret_populations + include_donor_only;
      for (int i = 0; i < n_fret_group; i++)
      {
         memset(b + idx, 0, bdim*sizeof(*b));
         acceptor_buffer->addDecay(1.0, norm_acceptor_channel_factors, reference_lifetime, b + idx);

         col++;
         idx += bdim;
      }
      kap_derv++;
   }

   return col;
}


int FretDecayGroup::addAcceptorIntensityDerivatives(double* b, int bdim, double_iterator& kap_derv)
{
   int col = 0;
   int idx = 0;

   if (Q_parameter->isFittedGlobally())
   {
      for (int i = 0; i < n_fret_populations; i++)
      {
         memset(b + idx, 0, bdim*sizeof(*b));
         addAcceptorContribution(i, 1.0, b + idx, bdim, kap_derv[col]);
         
         col++;
         idx += bdim;
      }
      kap_derv++;
   }

   return col;
}


int FretDecayGroup::addAcceptorLifetimeDerivatives(double* b, int bdim, double_iterator& kap_derv)
{
   int col = 0;
   int idx = 0;

   if (tauA_parameter->isFittedGlobally())
   {
      if (include_donor_only)
      {
         memset(b + idx, 0, bdim*sizeof(*b));
         acceptor_buffer->addDerivative(Qsigma / (tauA * tauA), norm_acceptor_channel_factors, reference_lifetime, b + idx);
         
         col++;
         idx += bdim;
      }

      for (int i = 0; i < n_fret_populations; i++)
      {
         memset(b + idx, 0, bdim * sizeof(*b));

         for (int j = 0; j < n_exponential; j++)
         {
            for (int k = 0; k < n_kappa; k++)
            {
               double fact = beta[j] * Q * a_star[i][k][j] / (tauA * tauA) * kappa_factor.p[k];
               acceptor_buffer->addDerivative(fact, norm_acceptor_channel_factors, reference_lifetime, b + idx);

               fact *= -tau_transfer[i][k];
               addAcceptorDerivativeContribution(i, j, k, fact, b + idx, bdim, *kap_derv);
            }
         }

         acceptor_buffer->addDerivative(Qsigma / (tauA * tauA), norm_acceptor_channel_factors, reference_lifetime, b + idx);

         col++;
         idx += bdim;

      }
      kap_derv++;
   }

   return col;
}
