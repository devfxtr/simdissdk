/* -*- mode: c++ -*- */
/***************************************************************************
*****                                                                  *****
*****                   Classification: UNCLASSIFIED                   *****
*****                    Classified By:                                *****
*****                    Declassify On:                                *****
*****                                                                  *****
****************************************************************************
*
*
* Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
*               EW Modeling & Simulation, Code 5773
*               4555 Overlook Ave.
*               Washington, D.C. 20375-5339
*
* License for source code at https://simdis.nrl.navy.mil/License.aspx
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
*
*/
#include "simCore/Calc/Math.h"
#include "simCore/EM/Constants.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/Propagation.h"

namespace simCore
{

double getRcvdPowerFreeSpace(double rngMeters,
        double freqMhz,
        double powerWatts,
        double xmtGaindB,
        double rcvGaindB,
        double rcsSqm,
        double systemLossdB,
        bool oneWay)
{
  // Free Space Radar range equation
  double rcvPower = simCore::SMALL_DB_VAL;
  double lamdaSqrd = square(simCore::LIGHT_SPEED_AIR / (1e6 * freqMhz));
  if (oneWay == false)
  {
    // http://www.microwaves101.com/encyclopedia/Navy_Handbook.cfm  Section 4.4
    rcvPower = xmtGaindB + rcvGaindB - systemLossdB + simCore::linear2dB((rcsSqm * powerWatts * lamdaSqrd) / (simCore::RRE_CONSTANT * square(square(rngMeters))));
  }
  else
  {
    // http://www.microwaves101.com/encyclopedia/Navy_Handbook.cfm  Section 4.3
    rcvPower = xmtGaindB + rcvGaindB - systemLossdB + simCore::linear2dB((powerWatts * lamdaSqrd) / (square(4. * M_PI * rngMeters)));
  }
  return rcvPower;
}

double getRcvdPowerBlake(double rngMeters,
        double freqMhz,
        double powerWatts,
        double xmtGaindB,
        double rcvGaindB,
        double rcsSqm,
        double ppfdB,
        double systemLossdB,
        bool oneWay)
{
  double rcvPower = getRcvdPowerFreeSpace(rngMeters, freqMhz, powerWatts, xmtGaindB, rcvGaindB, rcsSqm, systemLossdB, oneWay);
  // Received signal power calculation from Blake's equation 1.18 (p 12)
  // Radar Range-Performance Analysis (1986)
  // Lamont V. Blake, ISBN 0-89006-224-2
  // Use free space value, then apply propagation factor
  return (oneWay == false) ? (rcvPower + (4. * ppfdB)) : (rcvPower + (2. * ppfdB));
}

FrequencyBandUsEcm toUsEcm(double freqMhz)
{
  // As defined in https://en.wikipedia.org/wiki/Radio_spectrum

  if (freqMhz < 0.0)
    return USECM_FREQ_OUT_OF_BOUNDS;

  if (freqMhz < 250.0)
    return USECM_FREQ_A;

  if (freqMhz < 500.0)
    return USECM_FREQ_B;

  if (freqMhz < 1000.0)
    return USECM_FREQ_C;

  if (freqMhz < 2000.0)
    return USECM_FREQ_D;

  if (freqMhz < 3000.0)
    return USECM_FREQ_E;

  if (freqMhz < 4000.0)
    return USECM_FREQ_F;

  if (freqMhz < 6000.0)
    return USECM_FREQ_G;

  if (freqMhz < 8000.0)
    return USECM_FREQ_H;

  if (freqMhz < 10000.0)
    return USECM_FREQ_I;

  if (freqMhz < 20000.0)
    return USECM_FREQ_J;

  if (freqMhz < 40000.0)
    return USECM_FREQ_K;

  if (freqMhz < 60000.0)
    return USECM_FREQ_L;

  if (freqMhz < 100000.0)
    return USECM_FREQ_M;

  return USECM_FREQ_OUT_OF_BOUNDS;
}

void getFreqMhzRange(FrequencyBandUsEcm usEcm, double* minFreqMhz, double* maxFreqMhz)
{
  double minFreq = 0.0;
  double maxFreq = 0.0;

  switch (usEcm)
  {
  case USECM_FREQ_A:
    minFreq = 0.0;
    maxFreq = 250;
    break;
  case USECM_FREQ_B:
    minFreq = 250;
    maxFreq = 500;
    break;
  case USECM_FREQ_C:
    minFreq = 500;
    maxFreq = 1000;
    break;
  case USECM_FREQ_D:
    minFreq = 1000;
    maxFreq = 2000;
    break;
  case USECM_FREQ_E:
    minFreq = 2000;
    maxFreq = 3000;
    break;
  case USECM_FREQ_F:
    minFreq = 3000;
    maxFreq = 4000;
    break;
  case USECM_FREQ_G:
    minFreq = 4000;
    maxFreq = 6000;
    break;
  case USECM_FREQ_H:
    minFreq = 6000;
    maxFreq = 8000;
    break;
  case USECM_FREQ_I:
    minFreq = 8000;
    maxFreq = 10000;
    break;
  case USECM_FREQ_J:
    minFreq = 10000;
    maxFreq = 20000;
    break;
  case USECM_FREQ_K:
    minFreq = 20000;
    maxFreq = 40000;
    break;
  case USECM_FREQ_L:
    minFreq = 40000;
    maxFreq = 60000;
    break;
  case USECM_FREQ_M:
    minFreq = 60000;
    maxFreq = 100000;
    break;
  case USECM_FREQ_OUT_OF_BOUNDS:
    break;
  default:
    assert(0); // New band added to enum, but not here
    break;
  }

  if (minFreqMhz != NULL)
    *minFreqMhz = minFreq;
  if (maxFreqMhz != NULL)
    *maxFreqMhz = maxFreq;

  return;
}

FrequencyBandIEEE toIeeeBand(double freqMhz, bool useMM)
{
  // As defined in https://en.wikipedia.org/wiki/Radio_spectrum#IEEE

  if (freqMhz < 3.0)
    return IEEE_FREQ_OUT_OF_BOUNDS;

  if (freqMhz < 30.0)
    return IEEE_FREQ_HF;

  if (freqMhz < 300.0)
    return IEEE_FREQ_VHF;

  if (freqMhz < 1000.0)
    return IEEE_FREQ_UHF;

  if (freqMhz < 2000.0)
    return IEEE_FREQ_L;

  if (freqMhz < 4000.0)
    return IEEE_FREQ_S;

  if (freqMhz < 8000.0)
    return IEEE_FREQ_C;

  if (freqMhz < 12000.0)
    return IEEE_FREQ_X;

  if (freqMhz < 18000.0)
    return IEEE_FREQ_KU;

  if (freqMhz < 27000.0)
    return IEEE_FREQ_K;

  if (useMM && freqMhz >= 30000.0 && freqMhz < 300000.0)
    return IEEE_FREQ_MM;

  if (freqMhz < 40000.0)
    return IEEE_FREQ_KA;

  if (freqMhz < 75000.0)
    return IEEE_FREQ_V;

  if (freqMhz < 110000.0)
    return IEEE_FREQ_W;

  if (freqMhz < 300000.0)
    return IEEE_FREQ_G;

  return IEEE_FREQ_OUT_OF_BOUNDS;
}

void getFreqMhzRange(FrequencyBandIEEE ieeeEcm, double* minFreqMhz, double* maxFreqMhz)
{
  double minFreq = 0.0;
  double maxFreq = 0.0;

  switch (ieeeEcm)
  {
  case IEEE_FREQ_OUT_OF_BOUNDS:
    break;
  case IEEE_FREQ_HF:
    minFreq = 3.0;
    maxFreq = 30.0;
    break;
  case IEEE_FREQ_VHF:
    minFreq = 30.0;
    maxFreq = 300.0;
    break;
  case IEEE_FREQ_UHF:
    minFreq = 300.0;
    maxFreq = 1000.0;
    break;
  case IEEE_FREQ_L:
    minFreq = 1000.0;
    maxFreq = 2000.0;
    break;
  case IEEE_FREQ_S:
    minFreq = 2000.0;
    maxFreq = 4000.0;
    break;
  case IEEE_FREQ_C:
    minFreq = 4000.0;
    maxFreq = 8000.0;
    break;
  case IEEE_FREQ_X:
    minFreq = 8000.0;
    maxFreq = 12000.0;
    break;
  case IEEE_FREQ_KU:
    minFreq = 12000.0;
    maxFreq = 18000.0;
    break;
  case IEEE_FREQ_K:
    minFreq = 18000.0;
    maxFreq = 27000.0;
    break;
  case IEEE_FREQ_KA:
    minFreq = 27000.0;
    maxFreq = 40000.0;
    break;
  case IEEE_FREQ_V:
    minFreq = 40000.0;
    maxFreq = 75000.0;
    break;
  case IEEE_FREQ_W:
    minFreq = 75000.0;
    maxFreq = 110000.0;
    break;
  case IEEE_FREQ_G:
    minFreq = 110000.0;
    maxFreq = 300000.0;
    break;
  case IEEE_FREQ_MM:
    minFreq = 30000.0;
    maxFreq = 300000.0;
    break;
  default:
    assert(0); // New band added to enum, but not here
    break;
  }

  if (minFreqMhz != NULL)
    *minFreqMhz = minFreq;
  if (maxFreqMhz != NULL)
    *maxFreqMhz = maxFreq;

  return;
}

}

