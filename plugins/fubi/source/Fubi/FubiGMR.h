// ****************************************************************************************
//
// FubiGMR
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
// 
// This code is based on the gmm-gmr implementation of Florent D'halluin, Sylvain Calinon,
// LASA Lab, EPFL, CH - 1015 Lausanne, Switzerland,
// http://www.calinon.ch, http://lasa.epfl.ch
// which was published in:
// @article{ Calinon07SMC,
//  title = "On Learning, Representing and Generalizing a Task in a Humanoid Robot",
//  author = "S. Calinon and F. Guenter and A. Billard",
//  journal = "IEEE Transactions on Systems, Man and Cybernetics, Part B.
//  Special issue on robot learning by observation, demonstration and
//  imitation",
//  year = "2007",
//  volume = "37",
//  number = "2",
//  pages = "286--298"
// }
// ****************************************************************************************

#pragma once

#include "FubiMath.h"

namespace FubiGMRUtils
{
	class Matrix;
	class Vector;
}

class FubiGMR
{
public:
	FubiGMR(const std::vector<std::vector<Fubi::Vec3f>>& convertedData, unsigned int numStates);
	~FubiGMR();

	/*
	* \brief Perform expectation/maximization on the imported data set with the given maximum of iterations
	* to retrieve the gaussian mixture model
	*/
	void calculateGMM(int maxIterations);
	
	/*
	* \brief Calculate the gaussian mixture regression and save it to the provides vectors
	*/
	void calculateGMR(std::vector<Fubi::Vec3f>& means, std::vector<Fubi::Matrix3f>& inverseCovs);

private:
	/*
	* \brief get the probability density for a given vector using the provided mu and sigma
	*/
	float pdfState(FubiGMRUtils::Vector& v, const FubiGMRUtils::Vector& mu, const FubiGMRUtils::Matrix& sigma);
	/*
	* \brief get the probability density for a given state and a given vector, only using the indicated components
	*/
	float pdfState(FubiGMRUtils::Vector& v, FubiGMRUtils::Vector& components, int state);
	/* 
	 * \brief get the probability density for a given state and a given vector
	 */
	float pdfState(FubiGMRUtils::Vector& v, int state);

	int m_numStates, m_numData, m_numDimensions;
	FubiGMRUtils::Matrix* m_mu;
	FubiGMRUtils::Matrix* m_sigma;
	float* m_priors;
	FubiGMRUtils::Matrix* m_data;
};