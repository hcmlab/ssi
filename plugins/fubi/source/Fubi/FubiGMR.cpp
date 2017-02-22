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

#include "FubiGMR.h"

#include "FubiGMRUtils.h"

using namespace FubiGMRUtils;

FubiGMR::FubiGMR(const std::vector<std::vector<Fubi::Vec3f>>& convertedData, unsigned int numStates)
	: m_numStates(numStates), m_numDimensions(4)
{
	// Load the dataset from the provided FUBI data
	unsigned int rowSize = (unsigned int)convertedData[0].size();
	m_numData = rowSize * (unsigned int)convertedData.size();

	m_data = new  Matrix(m_numData, m_numDimensions, false);
	for (unsigned int j = 0; j < convertedData.size(); ++j)
	{
		const std::vector<Fubi::Vec3f>& dataRow = convertedData[j];
		Vector tempVec(m_numDimensions);
		unsigned int startIndex = j*rowSize;
		for (unsigned int i = 0; i < rowSize; ++i)
		{
			const Fubi::Vec3f& fvec = dataRow[i];
			tempVec[0] = (float)i;
			tempVec[1] = fvec.x;
			tempVec[2] = fvec.y;
			tempVec[3] = fvec.z;
			m_data->SetRow(tempVec, startIndex + i);
		}
	}

	// Intilize matrices with the given size
	m_priors = new float[m_numStates];
	m_sigma = new Matrix[m_numStates];
	m_mu = new Matrix(m_numStates, m_numDimensions);

	// Initialize temp vars for the mu and sigma calculations
	Matrix index(m_numStates, m_numData);
	int * valuesPerState = new int[m_numStates];
	Vector* mean = new Vector[m_numStates];
	for (int s = 0; s < m_numStates; s++)
	{
		mean[s].Resize(m_numDimensions, true);
		valuesPerState[s] = 0;
	}

	Matrix epsilonMat(m_numDimensions, m_numDimensions);
	for (int k = 0; k < m_numDimensions; k++)
		epsilonMat(k, k) = Fubi::Math::Epsilon;

	// Divide the dataset into slices of equal time for each state
	float scaleFac = m_numStates / (float)rowSize;
	for (int n = 0; n < m_numData; n++)
	{
		int s = Fubi::ftoi_t((*m_data)(n, 0)*scaleFac);
		mean[s] += m_data->GetRow(n);
		index(s, valuesPerState[s]) = (float)n;
		++(valuesPerState[s]);
	}

	// Compute mu and sigma per state
	for (int s = 0; s < m_numStates; s++)
	{
		m_mu->SetRow(mean[s] / (float)valuesPerState[s], s);
		m_sigma[s] = Matrix(m_numDimensions, m_numDimensions);
		// Initialize equal propability per state
		m_priors[s] = 1.0f / m_numStates;
		for (int ind = 0; ind < valuesPerState[s]; ind++)
		{
			for (int i = 0; i < m_numDimensions; i++) /* Computing covariance matrices */
			{
				for (int j = 0; j < m_numDimensions; j++)
				{
					m_sigma[s](i, j) += ((*m_data)(Fubi::ftoi_t(index(s, ind)), i) - (*m_mu)(s, i))
						* ((*m_data)(Fubi::ftoi_t(index(s, ind)), j) - (*m_mu)(s, j));
				}
			}
		}
		m_sigma[s] /= (float) valuesPerState[s];
		// Add epsilon mat to prevent non-inversibility
		m_sigma[s] += epsilonMat;
	}

	delete[] mean;
	delete[] valuesPerState;
}

FubiGMR::~FubiGMR()
{
	delete[] m_priors;
	delete[] m_sigma;
	delete m_mu;
	delete m_data;
}

void FubiGMR::calculateGMM(int maxIterations)
{
	// Threshold for changes to the log likeliness to stop the EM before maxIterations has been reached
	const float log_lik_threshold = 1e-8f;

	// Epsilon matrix prevents non-inversibitliy
	Matrix epsilonMat(m_numDimensions, m_numDimensions);
	for (int k = 0; k < m_numDimensions; k++)
		epsilonMat(k, k) = Fubi::Math::Epsilon;

	// Variables used in the EM loop
	float log_lik;
	float log_lik_old = -1e10f;
	Matrix pxi(m_numData, m_numStates);
	Matrix pix(m_numData, m_numStates);
	Vector E;
	float * sum_p = new float[m_numData];

	// The EM loop
	for (int iteration = 0; iteration < maxIterations; ++iteration)
	{
		// Compute the log likeliness
		float sum_log = 0;
		for (int i = 0; i < m_numData; ++i)
		{
			sum_p[i] = 0;
			Vector row;
			for (int j = 0; j < m_numStates; ++j)
			{
				float p = pdfState(m_data->GetRow(i, row), j);  // P(x|i)
				if (p == 0)
				{
					// Error: Null probability -> Abort;
					delete[] sum_p;
					return;
				}
				pxi(i, j) = p;
				sum_p[i] += p*m_priors[j];
			}
			sum_log += log(sum_p[i]);
		}
		for (int j = 0; j < m_numStates; j++)
		{
			for (int i = 0; i < m_numData; i++)
			{
				pix(i, j) = pxi(i, j)*m_priors[j] / sum_p[i]; // then P(i|x)
			}
		}
		log_lik = sum_log / m_numData;

		// Check how much the likeliness changed
		if (fabs((log_lik / log_lik_old) - 1) < log_lik_threshold)
		{
			// Threshold reached so stop the loop
			delete[] sum_p;
			return;
		}
		log_lik_old = log_lik;

		// Loop still running, so we update probablities, means and covariances for each state
		pix.SumRow(E);
		for (int j = 0; j < m_numStates; j++)
		{
			m_priors[j] = E(j) / m_numData;

			Vector tmu(m_numDimensions);
			for (int i = 0; i < m_numData; i++)
			{
				tmu += m_data->GetRow(i)*pix(i, j);
			}
			m_mu->SetRow(tmu / E(j), j);

			Matrix tmsigma(m_numDimensions, m_numDimensions);
			for (int i = 0; i < m_numData; i++)
			{
				Matrix Dif(m_numDimensions, 1);
				Dif.SetColumn((m_data->GetRow(i) - m_mu->GetRow(j)), 0);
				tmsigma += (Dif*Dif.Transpose())*pix(i, j);
			}
			m_sigma[j] = tmsigma / E(j) + epsilonMat;
		}
	}
	delete[] sum_p;
}

float FubiGMR::pdfState(Vector& v, const Vector& mu, const Matrix& sigma)
{
	Matrix inverseSigma;
	float sigmaDeterminant;
	sigma.Inverse(inverseSigma, &sigmaDeterminant);
	if (sigmaDeterminant != 0)
	{
		Vector dif = v - mu;
		double p = (double)(dif*(inverseSigma*dif));
		p = exp(-0.5*p) / sqrt(pow(Fubi::Math::TwoPi, m_numDimensions)*fabs(sigmaDeterminant));
		return  (p < Fubi::Math::MinPosFloat)
			? Fubi::Math::MinPosFloat : (float)p;
	}
	// Failed inverting sigma matrix
	return 0;
}

float FubiGMR::pdfState(Vector& v, int state)
{
	return pdfState(v, m_mu->GetRow(state), m_sigma[state]);
}

float FubiGMR::pdfState(Vector& v, Vector& components, int state)
{
	return pdfState(v, m_mu->GetRow(state).GetSubVector(components), m_sigma[state].GetMatrixSpace(components, components));
}

void FubiGMR::calculateGMR(std::vector<Fubi::Vec3f>& means, std::vector<Fubi::Matrix3f>& inverseCovs)
{
	// Helping vars for extracting the right columns (t, x, y, z)
	unsigned int outDataSize = (unsigned int)means.size();
	const int outDim = 3;
	Vector inComponents(1), outComponents(outDim);
	inComponents(0) = 0;
	for (unsigned int i = 0; i < 3; i++)
		outComponents(i) = (float)(i + 1);
	Matrix inData(outDataSize, 1);
	float* inArray = inData.Array();
	for (unsigned int i = 0; i < outDataSize; ++i)
	{
		inArray[i] = (float)i;
	}

	// Calculate probabilites for each data point
	Matrix Pxi(outDataSize, m_numStates);
	for (unsigned int i = 0; i < outDataSize; i++)
	{
		float norm_f = 0.0f;
		Vector row;
		for (int s = 0; s < m_numStates; s++)
		{
			float p_i = m_priors[s] * pdfState(inData.GetRow(i, row), inComponents, s);
			Pxi(i, s) = p_i;
			norm_f += p_i;
		}
		Pxi.SetRow(Pxi.GetRow(i) / norm_f, i);
	}

	// Calculate sigmas + variance for each state
	Matrix* subSigma = new Matrix[m_numStates];
	Matrix* subSigmaVar = new Matrix[m_numStates];
	for (int s = 0; s < m_numStates; s++)
	{
		Matrix isubSigmaIn;
		Matrix subSigmaOut;
		m_sigma[s].GetMatrixSpace(inComponents, inComponents, subSigmaOut);
		subSigmaOut.Inverse(isubSigmaIn);
		m_sigma[s].GetMatrixSpace(outComponents, inComponents, subSigmaOut);
		subSigma[s] = subSigmaOut*isubSigmaIn;
		m_sigma[s].GetMatrixSpace(outComponents, outComponents, subSigmaOut);
		m_sigma[s].GetMatrixSpace(inComponents, outComponents, isubSigmaIn);
		subSigmaVar[s] = subSigmaOut - subSigma[s] * isubSigmaIn;
	}

	// Now calculate the final mus and sigmas for each data point
	Matrix subMuIn;
	Matrix subMuOut;
	m_mu->GetColumnSpace(outComponents, subMuOut);
	m_mu->GetColumnSpace(inComponents, subMuIn);
	for (unsigned int i = 0; i < outDataSize; i++)
	{
		// Calculate mu and sigma for the current data point
		Matrix sigmaOut(outDim, outDim);
		Vector muOut(outDim, true);
		for (int s = 0; s < m_numStates; s++)
		{
			const float pIS = Pxi(i, s);
			muOut += (subMuOut.GetRow(s) + (subSigma[s] * (inData.GetRow(i) - subMuIn.GetRow(s)))) * pIS;
			sigmaOut += subSigmaVar[s] * (pIS*pIS);
		}

		// Copy them to the output vars
		Fubi::Matrix3f& inverseCov = inverseCovs[i];
		Fubi::Vec3f& mean = means[i];
		for (unsigned int j = 0; j < outDim; j++)
		{
			mean[j] = muOut[j];
			for (unsigned int k = 0; k < outDim; k++)
				inverseCov.c[j][k] = sigmaOut(j, k);
		}
		// Don't forget to invert the covariance matrix!
		inverseCov = inverseCov.inverted();
	}

	delete[] subSigma;
	delete[] subSigmaVar;
}
