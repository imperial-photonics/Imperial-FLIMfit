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

#pragma once


#include "InstrumentResponseFunction.h"

class AbstractConvolver;
class TransformedDataParameters;

class GaussianIrf : public InstrumentResponseFunction
{
public:
   GaussianIrf(const std::vector<GaussianParameters>& gaussian_params = {});
   GaussianIrf(const GaussianParameters& gaussian_params);

   std::shared_ptr<AbstractConvolver> getConvolver(std::shared_ptr<TransformedDataParameters> dp);

   void setFrameSigma(const std::vector<double>& frame_sigma);

   bool isSpatiallyVariant();
   bool arePositionsEquivalent(PixelIndex idx1, PixelIndex idx2);
   bool hasSpatiallyVaryingSigma(); 

   double getSigma(PixelIndex idx);

   double calculateMean();

   std::vector<GaussianParameters> gaussian_params;

private:

   bool frame_varying_sigma = false;
   std::vector<double> frame_sigma;

   template<class Archive>
   void serialize(Archive & ar, const unsigned int version);

   friend class boost::serialization::access;
};

template<class Archive>
void GaussianIrf::serialize(Archive & ar, const unsigned int version)
{
   ar & boost::serialization::base_object<InstrumentResponseFunction>(*this);
   ar & gaussian_params;
   ar & frame_varying_sigma;
   ar & frame_sigma;
}

BOOST_CLASS_VERSION(GaussianIrf, 1)

