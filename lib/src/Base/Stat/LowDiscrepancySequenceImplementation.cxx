//                                               -*- C++ -*-
/**
 *  @brief This class is a top-level class for low discrepancy sequences
 *
 *  Copyright 2005-2021 Airbus-EDF-IMACS-ONERA-Phimeca
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "openturns/LowDiscrepancySequenceImplementation.hxx"
#include "openturns/SpecFunc.hxx"
#include "openturns/Exception.hxx"
#include "openturns/PersistentObjectFactory.hxx"

#ifdef OPENTURNS_HAVE_PRIMESIEVE
#include <primesieve.hpp>
#endif

BEGIN_NAMESPACE_OPENTURNS


/**
 * @class LowDiscrepancySequenceImplementation
 */


CLASSNAMEINIT(LowDiscrepancySequenceImplementation)

static const Factory<LowDiscrepancySequenceImplementation> Factory_LowDiscrepancySequenceImplementation;

/* Constructor with parameters */
LowDiscrepancySequenceImplementation::LowDiscrepancySequenceImplementation(const UnsignedInteger dimension)
  : PersistentObject()
  , dimension_(dimension)
  , LCGState_(0)
{
  initialize(dimension);
}


/* Virtual constructor */
LowDiscrepancySequenceImplementation * LowDiscrepancySequenceImplementation::clone() const
{
  return new LowDiscrepancySequenceImplementation(*this);
}


/* Initialize the sequence */
void LowDiscrepancySequenceImplementation::initialize(const UnsignedInteger dimension)
{
  if (!(dimension > 0)) throw InvalidArgumentException(HERE) << "Dimension must be > 0.";
  dimension_ = dimension;
  LCGState_ = ResourceMap::GetAsUnsignedInteger("LowDiscrepancySequence-ScramblingSeed");
}


/* Dimension accessor*/
UnsignedInteger LowDiscrepancySequenceImplementation::getDimension() const
{
  return dimension_;
}


/* Generate a low discrepancy vector of numbers uniformly distributed over [0, 1) */
Point LowDiscrepancySequenceImplementation::generate() const
{
  throw NotYetImplementedException(HERE) << "In LowDiscrepancySequenceImplementation::generate()";
}


/* Generate a sample of pseudo-random vectors of numbers uniformly distributed over [0, 1) */
Sample LowDiscrepancySequenceImplementation::generate(const UnsignedInteger size) const
{
  Sample sequenceSample(size, dimension_);
  for (UnsignedInteger i = 0; i < size ; ++i) sequenceSample[i] = generate();
  return sequenceSample;
}


/* Compute the star discrepancy of a sample uniformly distributed over [0, 1) */
Scalar LowDiscrepancySequenceImplementation::ComputeStarDiscrepancy(const Sample & sample)
{
  // computationnaly heavy function : O(N²), let N the size of the sample
  const UnsignedInteger size = sample.getSize();

  // discrepancy is the maximum of the local discrepancy
  const Point lowerPoint(sample.getDimension());
  Scalar discrepancy = 0.0;
  for(UnsignedInteger i = 0; i < size; ++i)
  {
    const Scalar local = ComputeLocalDiscrepancy(sample, Interval(lowerPoint, sample[i]));
    if (local > discrepancy)
      discrepancy = local;
  }
  return discrepancy;
}

/* Scrambling seed accessor */
void LowDiscrepancySequenceImplementation::setScramblingState(const UnsignedInteger state)
{
  LCGState_ = state;
}

Unsigned64BitsInteger LowDiscrepancySequenceImplementation::getScramblingState() const
{
  return LCGState_;
}



/* String converter */
String LowDiscrepancySequenceImplementation::__repr__() const
{
  OSS oss;
  oss << "class=" << LowDiscrepancySequenceImplementation::GetClassName()
      << " dimension=" << dimension_
      << " LCGState=" << LCGState_;
  return oss;
}

/** Method save() stores the object through the StorageManager */
void LowDiscrepancySequenceImplementation::save(Advocate & adv) const
{
  PersistentObject::save(adv);

  adv.saveAttribute("dimension_", dimension_);
  adv.saveAttribute("LCGState_", LCGState_);
}

/** Method load() reloads the object from the StorageManager */
void LowDiscrepancySequenceImplementation::load(Advocate & adv)
{
  PersistentObject::load(adv);

  adv.loadAttribute("dimension_", dimension_);
  adv.loadAttribute("LCGState_", LCGState_);
}


/* Compute the local discrepancy of a sample, given a multidimensionnal interval */
Scalar LowDiscrepancySequenceImplementation::ComputeLocalDiscrepancy(const Sample & sample,
    const Interval & interval)
{
  if (sample.getDimension() != interval.getDimension()) throw InvalidArgumentException(HERE) << "Error: the sample must have the same dimension as the given interval.";
  // calculate number of inner points
  const UnsignedInteger size = sample.getSize();
  UnsignedInteger inPoints = 0;
  for(UnsignedInteger j = 0; j < size; ++j)
    if (interval.numericallyContains(sample[j])) ++inPoints;
  // The local discrepancy is the absolute difference between the fraction of points
  // that fall into the given interval and its volume
  return std::abs(static_cast<Scalar>(inPoints) / size - interval.getVolume());
}

Bool LowDiscrepancySequenceImplementation::IsPrimesieveAvailable()
{
#ifdef OPENTURNS_HAVE_PRIMESIEVE
  return true;
#else
  return false;
#endif
}

/* Get the needed prime numbers */
/* Get the n first prime numbers */
LowDiscrepancySequenceImplementation::Unsigned64BitsIntegerCollection LowDiscrepancySequenceImplementation::GetFirstPrimeNumbers(const UnsignedInteger n)
{
  if (n == 0) throw InvalidArgumentException(HERE) << "Error: cannot ask for no prime number";
#ifdef OPENTURNS_HAVE_PRIMESIEVE
  Unsigned64BitsIntegerCollection result(n);
  primesieve::iterator it;
  for (UnsignedInteger i = 0; i < n; ++i)
    result[i] = it.next_prime();
  return result;
#else
  static const UnsignedInteger MaxPrime(1600);
  static const Unsigned64BitsInteger Table[MaxPrime] =
  {
    2,    3,    5,    7,   11,   13,   17,   19,   23,   29,
    31,   37,   41,   43,   47,   53,   59,   61,   67,   71,
    73,   79,   83,   89,   97,  101,  103,  107,  109,  113,
    127,  131,  137,  139,  149,  151,  157,  163,  167,  173,
    179,  181,  191,  193,  197,  199,  211,  223,  227,  229,
    233,  239,  241,  251,  257,  263,  269,  271,  277,  281,
    283,  293,  307,  311,  313,  317,  331,  337,  347,  349,
    353,  359,  367,  373,  379,  383,  389,  397,  401,  409,
    419,  421,  431,  433,  439,  443,  449,  457,  461,  463,
    467,  479,  487,  491,  499,  503,  509,  521,  523,  541,
    547,  557,  563,  569,  571,  577,  587,  593,  599,  601,
    607,  613,  617,  619,  631,  641,  643,  647,  653,  659,
    661,  673,  677,  683,  691,  701,  709,  719,  727,  733,
    739,  743,  751,  757,  761,  769,  773,  787,  797,  809,
    811,  821,  823,  827,  829,  839,  853,  857,  859,  863,
    877,  881,  883,  887,  907,  911,  919,  929,  937,  941,
    947,  953,  967,  971,  977,  983,  991,  997, 1009, 1013,
    1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
    1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151,
    1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
    1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291,
    1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373,
    1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451,
    1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
    1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583,
    1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657,
    1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733,
    1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811,
    1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889,
    1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987,
    1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053,
    2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129,
    2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213,
    2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287,
    2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357,
    2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423,
    2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531,
    2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617,
    2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687,
    2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741,
    2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819,
    2833, 2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903,
    2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999,
    3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079,
    3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181,
    3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257,
    3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331,
    3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413,
    3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511,
    3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571,
    3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643,
    3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727,
    3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821,
    3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907,
    3911, 3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989,
    4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057,
    4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139,
    4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231,
    4241, 4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297,
    4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409,
    4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493,
    4507, 4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583,
    4591, 4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657,
    4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751,
    4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831,
    4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937,
    4943, 4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003,
    5009, 5011, 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087,
    5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179,
    5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279,
    5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387,
    5393, 5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443,
    5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507, 5519, 5521,
    5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639,
    5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693,
    5701, 5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791,
    5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857,
    5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939,
    5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053,
    6067, 6073, 6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133,
    6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221,
    6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301,
    6311, 6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367,
    6373, 6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473,
    6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569, 6571,
    6577, 6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661, 6673,
    6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761,
    6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833,
    6841, 6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917,
    6947, 6949, 6959, 6961, 6967, 6971, 6977, 6983, 6991, 6997,
    7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103,
    7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207,
    7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297,
    7307, 7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411,
    7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499,
    7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561,
    7573, 7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643,
    7649, 7669, 7673, 7681, 7687, 7691, 7699, 7703, 7717, 7723,
    7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823, 7829,
    7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919,
    7927, 7933, 7937, 7949, 7951, 7963, 7993, 8009, 8011, 8017,
    8039, 8053, 8059, 8069, 8081, 8087, 8089, 8093, 8101, 8111,
    8117, 8123, 8147, 8161, 8167, 8171, 8179, 8191, 8209, 8219,
    8221, 8231, 8233, 8237, 8243, 8263, 8269, 8273, 8287, 8291,
    8293, 8297, 8311, 8317, 8329, 8353, 8363, 8369, 8377, 8387,
    8389, 8419, 8423, 8429, 8431, 8443, 8447, 8461, 8467, 8501,
    8513, 8521, 8527, 8537, 8539, 8543, 8563, 8573, 8581, 8597,
    8599, 8609, 8623, 8627, 8629, 8641, 8647, 8663, 8669, 8677,
    8681, 8689, 8693, 8699, 8707, 8713, 8719, 8731, 8737, 8741,
    8747, 8753, 8761, 8779, 8783, 8803, 8807, 8819, 8821, 8831,
    8837, 8839, 8849, 8861, 8863, 8867, 8887, 8893, 8923, 8929,
    8933, 8941, 8951, 8963, 8969, 8971, 8999, 9001, 9007, 9011,
    9013, 9029, 9041, 9043, 9049, 9059, 9067, 9091, 9103, 9109,
    9127, 9133, 9137, 9151, 9157, 9161, 9173, 9181, 9187, 9199,
    9203, 9209, 9221, 9227, 9239, 9241, 9257, 9277, 9281, 9283,
    9293, 9311, 9319, 9323, 9337, 9341, 9343, 9349, 9371, 9377,
    9391, 9397, 9403, 9413, 9419, 9421, 9431, 9433, 9437, 9439,
    9461, 9463, 9467, 9473, 9479, 9491, 9497, 9511, 9521, 9533,
    9539, 9547, 9551, 9587, 9601, 9613, 9619, 9623, 9629, 9631,
    9643, 9649, 9661, 9677, 9679, 9689, 9697, 9719, 9721, 9733,
    9739, 9743, 9749, 9767, 9769, 9781, 9787, 9791, 9803, 9811,
    9817, 9829, 9833, 9839, 9851, 9857, 9859, 9871, 9883, 9887,
    9901, 9907, 9923, 9929, 9931, 9941, 9949, 9967, 9973, 10007,
    10009, 10037, 10039, 10061, 10067, 10069, 10079, 10091, 10093, 10099,
    10103, 10111, 10133, 10139, 10141, 10151, 10159, 10163, 10169, 10177,
    10181, 10193, 10211, 10223, 10243, 10247, 10253, 10259, 10267, 10271,
    10273, 10289, 10301, 10303, 10313, 10321, 10331, 10333, 10337, 10343,
    10357, 10369, 10391, 10399, 10427, 10429, 10433, 10453, 10457, 10459,
    10463, 10477, 10487, 10499, 10501, 10513, 10529, 10531, 10559, 10567,
    10589, 10597, 10601, 10607, 10613, 10627, 10631, 10639, 10651, 10657,
    10663, 10667, 10687, 10691, 10709, 10711, 10723, 10729, 10733, 10739,
    10753, 10771, 10781, 10789, 10799, 10831, 10837, 10847, 10853, 10859,
    10861, 10867, 10883, 10889, 10891, 10903, 10909, 10937, 10939, 10949,
    10957, 10973, 10979, 10987, 10993, 11003, 11027, 11047, 11057, 11059,
    11069, 11071, 11083, 11087, 11093, 11113, 11117, 11119, 11131, 11149,
    11159, 11161, 11171, 11173, 11177, 11197, 11213, 11239, 11243, 11251,
    11257, 11261, 11273, 11279, 11287, 11299, 11311, 11317, 11321, 11329,
    11351, 11353, 11369, 11383, 11393, 11399, 11411, 11423, 11437, 11443,
    11447, 11467, 11471, 11483, 11489, 11491, 11497, 11503, 11519, 11527,
    11549, 11551, 11579, 11587, 11593, 11597, 11617, 11621, 11633, 11657,
    11677, 11681, 11689, 11699, 11701, 11717, 11719, 11731, 11743, 11777,
    11779, 11783, 11789, 11801, 11807, 11813, 11821, 11827, 11831, 11833,
    11839, 11863, 11867, 11887, 11897, 11903, 11909, 11923, 11927, 11933,
    11939, 11941, 11953, 11959, 11969, 11971, 11981, 11987, 12007, 12011,
    12037, 12041, 12043, 12049, 12071, 12073, 12097, 12101, 12107, 12109,
    12113, 12119, 12143, 12149, 12157, 12161, 12163, 12197, 12203, 12211,
    12227, 12239, 12241, 12251, 12253, 12263, 12269, 12277, 12281, 12289,
    12301, 12323, 12329, 12343, 12347, 12373, 12377, 12379, 12391, 12401,
    12409, 12413, 12421, 12433, 12437, 12451, 12457, 12473, 12479, 12487,
    12491, 12497, 12503, 12511, 12517, 12527, 12539, 12541, 12547, 12553,
    12569, 12577, 12583, 12589, 12601, 12611, 12613, 12619, 12637, 12641,
    12647, 12653, 12659, 12671, 12689, 12697, 12703, 12713, 12721, 12739,
    12743, 12757, 12763, 12781, 12791, 12799, 12809, 12821, 12823, 12829,
    12841, 12853, 12889, 12893, 12899, 12907, 12911, 12917, 12919, 12923,
    12941, 12953, 12959, 12967, 12973, 12979, 12983, 13001, 13003, 13007,
    13009, 13033, 13037, 13043, 13049, 13063, 13093, 13099, 13103, 13109,
    13121, 13127, 13147, 13151, 13159, 13163, 13171, 13177, 13183, 13187,
    13217, 13219, 13229, 13241, 13249, 13259, 13267, 13291, 13297, 13309,
    13313, 13327, 13331, 13337, 13339, 13367, 13381, 13397, 13399, 13411,
    13417, 13421, 13441, 13451, 13457, 13463, 13469, 13477, 13487, 13499
  };
  Unsigned64BitsIntegerCollection result(n);
  if (n <= MaxPrime)
    std::copy(&Table[0], &Table[n], result.begin());
  else
  {
    // Upper bound of the nth prime number, valid for n>=6, see https://en.wikipedia.org/wiki/Prime-counting_function
    const Unsigned64BitsInteger upperBound(std::ceil(n * std::log(n * std::log(n))));
    Indices is_prime(upperBound + 1, 1);
    // Use the Sieve of Eratosthenes for the prime numbers. There is no significant gain in terms of CPU or RAM trying to reuse Table
    for (UnsignedInteger p = 2; p * p <= upperBound; ++p)
    {
      // If flags[p] is equal to 1, it is a prime
      if (is_prime[p])
      {
        // Update all multiples of p
        for (UnsignedInteger i = 2 * p; i <= upperBound; i += p)
          is_prime[i] = 0;
      } // flags[p] == 1
    } // p
    UnsignedInteger index = 0;
    for (UnsignedInteger i = 2; i <= upperBound; ++i)
    {
      if (is_prime[i])
      {
        result[index] = i;
        ++index;
        // Exit the loop if we have found enough primes
        if (index == n) break;
      } // is_prime
    } // i
  } // n > MaxPrime
  return result;
#endif
}

/* Compute the least prime number greater or equal to n */
Unsigned64BitsInteger LowDiscrepancySequenceImplementation::GetNextPrimeNumber(const UnsignedInteger n)
{
  if (n == 0) return 2;
#ifdef OPENTURNS_HAVE_PRIMESIEVE
  primesieve::iterator it;
  it.skipto(n - 1);
  return it.next_prime();
#else
  const Unsigned64BitsIntegerCollection primes(GetFirstPrimeNumbers(static_cast<UnsignedInteger>(n / SpecFunc::LambertW(n))));
  UnsignedInteger i = 0;
  while (primes[i] < n)
    ++i;
  return primes[i];
#endif
}

END_NAMESPACE_OPENTURNS
