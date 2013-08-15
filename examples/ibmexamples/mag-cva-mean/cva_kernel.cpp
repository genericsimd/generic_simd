        
/*******************************************************************************
 * Kernel extracted from CVA.cpp, CCVAHelper::Calculate when 
 *   func_args.CalculateAllScenarios == true, starting from the loop that 
 *   iterates over scenarios
 *
 * Changes made from original codes:
 *  - REMOVED AUDIT and MODE
 *
 ********************************************************************************/

/********************************************************************************
 * MACRO specialization
 *
 * NOTE: The following loop-invariant variables are used in control-flows inside loops
 *    - usingFutureValueCurve (boolean)
 *    - has_p_of_default (boolean)
 *    - bucketingConfig (enum w/ 4 possible values)
 *    - CALC_TYPE (enum w/ 3 possible values)
 *    - CWIEXP (0 or 1)
 *    - AUDIT (boolean, but we only care about false case)
 *
 * TODO:
 *  - define MACROS for different specialization
 ********************************************************************************/
//#define NDEBUG
#define NEW_TIMER

#define NUM_INPUT_TIMES 250
#define NUM_SCENARIOS 5000
#define NUM_ACTIVE_TIMES 1

#define R_1_VALUE 0.5
#define r_1_VALUE 0.6

/********************************************************************************/
#include "cva_kernel_helper.hpp"
#include "timing.h"

#include <stdio.h>
#include <cmath>
#include <algorithm>
#include <functional>

/******************************************************************************** 
 * from ./AlgoFinance/src/CVA.cpp.orig, CalcDiscountedExposure
 *
 ********************************************************************************/
// calculate the discounted exposure term - E(tj)*d(ti, tj)
// which depends on the CVA exposure bucketing configuration parameter
#ifdef BASE_VERSION
inline ValueType CalcDiscountedExposure(const H5SheetAccessor& sheet,
					const H5SheetAccessor& dc_curve,
					CVAExposureBucketingType bucketingConfig,
					bool usingFutureValueCurve,
					unsigned i, 
					unsigned j, 
					unsigned s)
#else
template<CVAExposureBucketingType bucketingConfig, bool usingFutureValueCurve>
inline ValueType CalcDiscountedExposure(const H5SheetAccessor& sheet,
					const H5SheetAccessor& dc_curve,
					unsigned i, 
					unsigned j, 
					unsigned s)
#endif
{
  switch (bucketingConfig)
    {
    case END_OF_INTERVAL:
      {
	ValueType E_tj = sheet.GetValue(0, s, j);
	ValueType d_ti_tj = dc_curve.GetValue(0, s, j);
	if (i > 0 && usingFutureValueCurve)
	  {
	    ValueType d_t0_ti = dc_curve.GetValue(0, s, i);
	    d_ti_tj = d_ti_tj / d_t0_ti;
	  }
	
	return E_tj * d_ti_tj;
      }
    case START_OF_INTERVAL:
      {
	ValueType E_tj_1 = sheet.GetValue(0, s, j-1);
	ValueType d_ti_tj_1 = dc_curve.GetValue(0, s, j-1);
	if (i > 0 && usingFutureValueCurve)
	  {
	    ValueType d_t0_ti = dc_curve.GetValue(0, s, i);
	    d_ti_tj_1 = d_ti_tj_1 / d_t0_ti;
	  }
#if !defined(NDEBUG)
	if ((d_ti_tj_1 - (ValueType)1.0) > (ValueType)0.00000000001)
	  printf("Discount values  from current->current should be 1, since there is no real discounting happening");
#endif
	
	return E_tj_1 * d_ti_tj_1;
      }
    case MID_OF_INTERVAL:
      {
	ValueType E_tj = sheet.GetValue(0, s, j);
	ValueType E_tj_1 = sheet.GetValue(0, s, j-1);
	ValueType d_ti_tj = dc_curve.GetValue(0, s, j);
	ValueType d_ti_tj_1 = dc_curve.GetValue(0, s, j-1);
	if (i > 0 && usingFutureValueCurve)
	  {
	    ValueType d_t0_ti = dc_curve.GetValue(0, s, i);
	    d_ti_tj = d_ti_tj / d_t0_ti;
	    d_ti_tj_1 = d_ti_tj_1 / d_t0_ti;
	  }
	
#if !defined(NDEBUG)
	if ((d_ti_tj_1 - (ValueType)1.0) > (ValueType)0.00000000001)
	  printf("Discount values  from current->current should be 1, since there is no real discounting happening");
#endif
	
	return ValueType((E_tj + E_tj_1) * (d_ti_tj + d_ti_tj_1) / 4.0);
      }
    case MID_OF_INTERVAL_BASEL:
      {
	ValueType E_tj = sheet.GetValue(0, s, j);
	ValueType E_tj_1 = sheet.GetValue(0, s, j-1);
	ValueType d_ti_tj = dc_curve.GetValue(0, s, j);
	ValueType d_ti_tj_1 = dc_curve.GetValue(0, s, j-1);
	if (i > 0 && usingFutureValueCurve)
	  {
	    ValueType d_t0_ti = dc_curve.GetValue(0, s, i);
	    d_ti_tj = d_ti_tj / d_t0_ti;
	    d_ti_tj_1 = d_ti_tj_1 / d_t0_ti;
	  }
#if !defined(NDEBUG)
	if ((d_ti_tj_1 - (ValueType)1.0) > (ValueType)0.00000000001)
	  printf("Discount values  from current->current should be 1, since there is no real discounting happening");
#endif
	
	return ValueType(((E_tj * d_ti_tj) + (E_tj_1 * d_ti_tj_1)) / 2.0);
      }
    default:
      /* PENGUIN: comment out exception handling and fake it with print error */ 
      std::cerr << "Unexpected CVA exposure bucketing configuration (" << bucketingConfig << ")" << std::endl;
      exit(1);
      /*
      throw CFinanceException(_T("CVA Calculate"),  _T("Unexpected CVA exposure bucketing configuration (") + MUS::ToString(bucketingConfig) + _T(")"));
      */
    }
};

/******************************************************************************** 
 * from ./AlgoFinance/src/ExpectedTailExposure.cpp CalculateExpectedTailExposure
 ********************************************************************************/
#ifdef ENABLE_KERNEL
extern "C" void righttail_kernel(uint32_t, uint32_t, uint32_t, const float*, float*);
#endif

FINANCE_DECL void
CalculateExpectedTailExposure(
			      unsigned numScenarios, unsigned numTimes, ValueType * in,
			      ValueType * result, ValueType confidence, bool oneSided)
{
  CSheetResolver sr(numScenarios, numTimes);

  if (confidence < 0 || confidence > 1) {
    std::cerr << "Within ExpectedTailExposure: invalid confidence level, " << std::endl;
    exit(1);
    // throw CFinanceException(_T("Within ExpectedTailExposure: invalid confidence level, ") +
    // MUS::ToString(confidence) + _T("."));
  }

  // Assuming oneSided for now in this helper;
  ValueType doubleSidedConfidence = oneSided ? confidence : ValueType(1.0) - (ValueType(1.0) - confidence)*ValueType(0.5);
  unsigned threshold = static_cast<unsigned>(ceil(doubleSidedConfidence * numScenarios)) - 1;
  
#ifdef ENABLE_KERNEL
  righttail_kernel(threshold, numScenarios, numTimes, in, result);
#else
  std::vector<ValueType> copy(numScenarios, 0);
  
  for (unsigned t = 0; t < numTimes; ++t)
    {
      const ValueType * start = sr.ScenarioLine(in,t);
      for (unsigned s = 0; s < numScenarios; ++s)
        {
	  copy[s] = start[s];
        }
      
      std::nth_element<std::vector<ValueType>::iterator>(copy.begin(), copy.begin()+threshold, copy.end());
      
      ValueType sum(0.);
      
      for (unsigned s = threshold; s < numScenarios; ++s)
        {
	  sum += copy[s];
        }
      
      ValueType factor = ValueType(1.)/(numScenarios-threshold);
      result[t] = sum*factor;
    }
#endif
}

/***************************************************************************************
 * main kernel
 ***************************************************************************************/
#ifdef BASE_VERSION
template<unsigned CALC_TYPE, bool CWIEXP>
#else
template<unsigned CALC_TYPE, bool CWIEXP, CVAExposureBucketingType bucketingConfig, bool usingFutureValueCurve>
#endif
void 
CVA_CalculateDefault_Kernel(
			    int numActiveTimes,
			    int numScenarios,
			    int numInputTimes,
#ifdef BASE_VERSION
			    CVAExposureBucketingType bucketingConfig, 
			    bool usingFutureValueCurve,
#endif
			    ValueType * resultSheet,
			    ValueType * qdHolder,
			    H5SheetAccessor &p_of_default_other_tree,
			    H5SheetAccessor &p_of_default,
			    H5SheetAccessor &sheet, 
			    H5SheetAccessor &dc_curve,
			    ValueType R_1, 
			    ValueType r_1, 
			    CDate *tps
			    )
{
  for(unsigned i=0; i < numActiveTimes; ++i)
    {
      /* assume dc_curve is passed in from the caller
      const H5SheetAccessor *temp = marketConditions.CVADiscountSheet(i);
      bool has_discount_curve = temp != 0;
      if(temp) dc_curve = *temp;
      */

      // if( AUDIT ) ...
      /* assume p_of_default and p_of_default_other_tree is passed in from caller
      if( !has_p_of_default )
	{
	  // Re-calculate Probabilities of Default is set to "For Every time-step"
	  p_of_default = GetPDSheetAccessor(marketConditions, le, credit_default, i);
	  
	  if (CALC_TYPE == AMOUNT_CALC_NET)
	    {
	      // Re-calculate Probabilities of Default is set to "For Every time-step"
	      p_of_default_other_tree = GetPDSheetAccessor(marketConditions, leOther, credit_default_other, i);
	    }
	}
      */ 
      ValueType sum = 0.0;
      ValueType denominator = 0.0;
      ValueType sp = 1.; // survival probability
      if (CALC_TYPE == AMOUNT_CALC_NET)
	{
	  for (unsigned spIdx = 1; spIdx <=i; spIdx++)
	    {
	      ValueType pd = p_of_default_other_tree.GetValue(0, 0 /*(pdScenIdx == PD_SCENARIO_BASE ? 0 : pdScenIdx)*/, spIdx);
	      sp = sp - pd;
	    }
	}
      
      CCVAValueArrayPrinter<CALC_TYPE, false> m_value_printer;
      CCVAValueArrayPrinter<CALC_TYPE, false> u_ti_tj_printer;
      CCVAValueArrayPrinter<CALC_TYPE, false> sp_printer;
      CCVAValueArrayPrinter<CALC_TYPE, false> cva_printer;
      
      //if( AUDIT ) ...
	
      for(unsigned j=i+1; j < numInputTimes; ++j)
	{
	  ValueType pd = p_of_default.GetValue(0, 0 /*( pdScenIdx == PD_SCENARIO_BASE ? 0 : pdScenIdx)*/, j);
	  ValueType qd = pd * R_1;
	  
#if defined(ENABLE_SSE) | defined(ENABLE_SIMD)
	  ValueType m_value __attribute__ ((aligned(16)));
#else
	  ValueType m_value;
#endif
	  // Calculate M value
	  std::vector<ValueType, CMemAlignAllocator<ValueType> >  mInput(numScenarios);
	  
	  for (unsigned s = 0; s < numScenarios; s++)
	    {
#ifdef BASE_VERSION
	      mInput[s] = CalcDiscountedExposure(sheet, dc_curve, bucketingConfig, usingFutureValueCurve, i, j, s);
#else
	      mInput[s] = CalcDiscountedExposure<bucketingConfig, usingFutureValueCurve>(sheet, dc_curve, i, j, s);
#endif
	    }
	  if (CWIEXP == 0)
	    {
	      CMeanCalculator::Calculate(numScenarios, 1, &mInput[0], &m_value);
	    }
	  else
	    {
	      if (CWIEXP == 1) std::transform(mInput.begin(), mInput.end(), mInput.begin(), std::negate<ValueType>());
	      
	      ValueType confidence = (ValueType)1.0 - qd;
	      CalculateExpectedTailExposure(numScenarios, 1, &mInput[0], &m_value, confidence, true);
	      if (CWIEXP == 1) m_value = ValueType(-1.) * m_value;
	    }
	  
	  if (qdHolder != 0 && i == 0 && j == numInputTimes -1)
	    {
	      qdHolder[0] = qd;
	    }
	  
	  m_value_printer.AddValue(m_value);
	  
	  if (CALC_TYPE == AMOUNT_CALC_NET)
	    {
	      sum += m_value * qd * r_1  * sp;
	      u_ti_tj_printer.AddValue(qd*sp*r_1);
	      sp_printer.AddValue(sp);
	      cva_printer.AddValue(m_value*qd*sp*r_1);
	      
	      pd = p_of_default_other_tree.GetValue(0, 0 /*(pdScenIdx == PD_SCENARIO_BASE ? 0 : pdScenIdx)*/, j);
	      
	      sp = sp - pd; // prepare for next j
	    }
	  else
	    {
	      sum += m_value * qd * r_1;
	      
	      if( CALC_TYPE == AMOUNT_CALC )
		{
		  u_ti_tj_printer.AddValue(qd*r_1);
		  cva_printer.AddValue(m_value*qd*r_1);
		}
	      else
		{
		  u_ti_tj_printer.AddValue((((ValueType)(tps[j]-tps[j-1]))/((ValueType)(365.0)))*r_1);
		  cva_printer.AddValue(m_value*((((ValueType)(tps[j]-tps[j-1]))/((ValueType)(365.0)))*r_1));
		}
	    }
	  CCVARateHelper<CALC_TYPE>::AccumulateDenominator(denominator, m_value, r_1, tps[j-1], tps[j]);
	}
      
      m_value_printer.PrintValues(i);
      u_ti_tj_printer.PrintValues(i);
      sp_printer.PrintValues(i);
      cva_printer.PrintValues(i);
      
      CCVARateHelper<CALC_TYPE>::CalcRate(sum, denominator);
      
      resultSheet[i] = sum;
    }
}

/***********************************************************************************
 *
 * build data structures used to drive the kernel
 *
 ************************************************************************************/
class Parameter
{
public:
  int numActiveTimes; 
  int numInputTimes;
  int numScenarios;   

  ValueType R_1;
  ValueType r_1;

  ValueType * resultSheet;
  ValueType * qdHolder;
  H5SheetAccessor p_of_default_other_tree;
  H5SheetAccessor p_of_default;
  H5SheetAccessor sheet;
  H5SheetAccessor dc_curve;
  CDate *tps;
  
  int resultSheetSize;

  // Note: all buffers contain 1 sheet (due to legacy implementation)
  const char* allocateBuffer(int numTimes, int numScenarios, bool maxIsOne)
  {
    int length =  numTimes * numScenarios;
    ValueType* buffer = (ValueType *)malloc(sizeof(ValueType) * length);
#ifdef TRANSPOSED_LAYOUT
    for (int j=0; j<numScenarios; j++) {
      for (int i=0; i<numTimes; i++) {
	buffer[j*numTimes + i] = (i+1)*(j+1)/21.0;
      }
    }
#else
    for (int i=0; i<numTimes; i++) {
      for (int j=0; j<numScenarios; j++) {
	buffer[i*numScenarios + j] = (i+1)*(j+1)/21.0;
      }
    }
#endif

    if (maxIsOne) {
      // make sure the number is within [0, 1)
      for (int i=0; i<numTimes * numScenarios; i++) {
	ValueType v = buffer[i];
	if (v < 0) v = -v;
	if (v >= 1) {
	  buffer[i] = fmod(v, 99)/99;
	}
      }
    }
    return (const char*)buffer;
  }

  // i ==> iterating over 0 to numActiveTimes
  // scen ==> iterating over 0 to scenario 
  // j ==> iterating over i+1 to innerLoopTimes
  Parameter() :
    numActiveTimes(NUM_ACTIVE_TIMES),
    numInputTimes(NUM_INPUT_TIMES),
    numScenarios(NUM_SCENARIOS),
    //numPointsPerSheet(numInputTimes * numScenarios),
    R_1(R_1_VALUE),
    r_1(r_1_VALUE),
    p_of_default(allocateBuffer(numInputTimes, numScenarios, true), 
		 numScenarios, numInputTimes, 1, numScenarios * numInputTimes),
    p_of_default_other_tree(allocateBuffer(numInputTimes, numScenarios, true), 
			    numScenarios, numInputTimes, 1, numScenarios * numInputTimes),
    sheet(allocateBuffer(numInputTimes, numScenarios, false), 
	  numScenarios, numInputTimes, 1, numScenarios * numInputTimes),
    dc_curve(allocateBuffer(numInputTimes, numScenarios, false), 
	     numScenarios, numInputTimes, 1, numScenarios * numInputTimes)
  {
    resultSheetSize = numInputTimes * numActiveTimes;
    resultSheet = new ValueType[resultSheetSize]; 
    memset(resultSheet, 0, resultSheetSize*sizeof(ValueType));

    qdHolder = new ValueType[numInputTimes]; 
    for (int i=0; i<numInputTimes; i++) qdHolder[i] = i*i/13.0;

    tps = new CDate[numInputTimes];
    for (int i=0; i<numInputTimes; i++) tps[i] = i+1;
  }

  void resetResultSheet()
  {
    memset(resultSheet, 0, resultSheetSize*sizeof(ValueType));
  }

  ValueType* dupResultSheet()
  {
    ValueType* result = new ValueType[resultSheetSize]; 
    memcpy(result, resultSheet, resultSheetSize*sizeof(ValueType));

    return result;
  }

  int verifyResultSheet(ValueType* expectedResult)
  {
    int nerror = 0;
    for (int i=0; i<resultSheetSize; i++) {
      if (resultSheet[i] != expectedResult[i]) {
	printf("Unexpected results at iter %d: my=%f, expected=%f\n", i, resultSheet[i], expectedResult[i]);
	nerror++;
      }
    }
    return nerror;
  }

  void dumpResultSheet(char* filename)
  {
    FILE* file = fopen(filename, "w");
    fprintf(file, "DUMP RESULT SHEET: size=%d \n", resultSheetSize);
    for (int i=0; i<resultSheetSize; i+=NUM_VALUES_PER_LINE) {
      for (int j=0; j<NUM_VALUES_PER_LINE; j++) {
	fprintf(file, "%f ", resultSheet[i+j]);
      }
      fprintf(file, "\n");
    }
  }

  ~Parameter()
  {
    delete[] resultSheet;
    delete[] qdHolder;
    delete[] tps;
  }

  // TODO: add destructor to free the malloced values
};

/************************************************************************************
 *
 * main driver
 *
 ************************************************************************************/
#ifdef BASE_VERSION
#define RUN_KERNEL(myArgs,CWIEXP,USING_FUTURE_VALUE_CURVE)		\
  CVA_CalculateDefault_Kernel<AMOUNT_CALC_NET,CWIEXP>(			\
						      myArgs.numActiveTimes, \
						      myArgs.numScenarios, \
						      myArgs.numInputTimes, \
						      END_OF_INTERVAL,	\
						      USING_FUTURE_VALUE_CURVE, \
						      myArgs.resultSheet, \
						      myArgs.qdHolder,	\
						      myArgs.p_of_default, \
						      myArgs.p_of_default_other_tree, \
						      myArgs.sheet,	\
						      myArgs.dc_curve,	\
						      myArgs.R_1,	\
						      myArgs.r_1,	\
						      myArgs.tps)
#else
#define RUN_KERNEL(myArgs,CWIEXP,USING_FUTURE_VALUE_CURVE)		\
  CVA_CalculateDefault_Kernel<AMOUNT_CALC_NET,CWIEXP,END_OF_INTERVAL,USING_FUTURE_VALUE_CURVE>( \
											       myArgs.numActiveTimes, \
											       myArgs.numScenarios, \
											       myArgs.numInputTimes, \
											       myArgs.resultSheet, \
											       myArgs.qdHolder, \
											       myArgs.p_of_default, \
											       myArgs.p_of_default_other_tree, \
											       myArgs.sheet, \
											       myArgs.dc_curve, \
											       myArgs.R_1, \
											       myArgs.r_1, \
											       myArgs.tps)
#endif

static const char* getKernelName(const char* base)
{
  static char buffer[256];
  char* ptr = buffer;
  strcpy(buffer, base);
#ifdef BASE_VERSION
  strcat(ptr, "_base");
#endif
#ifdef TRANSPOSED_LAYOUT
  strcat(ptr, "_transposed");
#endif
#ifdef ENABLE_SSE
  strcat(ptr, "_sse");
#endif
#ifdef ENABLE_SIMD
  strcat(ptr, "_simd");
#endif
  return (const char*)ptr;
}

template<bool CWIEXP, bool USING_FUTURE_VALUE_CURVE> 
static void run_kernel(const char* kernelName, Parameter& myArgs)
{
  myArgs.resetResultSheet();

  /* warm up */
  RUN_KERNEL(myArgs, CWIEXP, USING_FUTURE_VALUE_CURVE);
  myArgs.dumpResultSheet((char*)"RESULT_SHEET.txt");

  ValueType* expectedResult = myArgs.dupResultSheet();

  /* serial code */
  reset_and_start_stimer();
  for (int k = 0; k<NUM_ITERS; k++) {
    myArgs.resetResultSheet();
    RUN_KERNEL(myArgs, CWIEXP, USING_FUTURE_VALUE_CURVE);
  }
  double sumSerial = get_elapsed_seconds() * 1000;

  /* verify and printout */
  int nerrors = myArgs.verifyResultSheet(expectedResult);
  if (nerrors == 0) {
    printf("[%15s]:\t\t%.3f ms per CVA\n", kernelName, sumSerial/NUM_ITERS);
  } else {
    printf("[%15s]: verification failed with %d errors\n", kernelName, nerrors);
  }

  delete[] expectedResult;
}

int main()
{
  Parameter myArgs;
  run_kernel<0,0>(getKernelName((const char*)"cva_mean"), myArgs);
/*
  run_kernel<0,1>(getKernelName((const char*)"cva_mean_futureCurve"), myArgs);
  run_kernel<1,0>(getKernelName((const char*)"cva_tail"), myArgs);
  run_kernel<1,1>(getKernelName((const char*)"cva_tail_futureCurve"), myArgs);
*/

  return 1;
}
