
#ifndef CVA_KERNEL_HELPER_HPP

#define CVA_KERNEL_HELPER_HPP

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <limits>

#include <iostream>
#include <cstdlib>
#include <assert.h>

typedef float ValueType;

// PENGUIN: from ./includes/FinanceAPI/private/PrivateString.h
typedef std::wstring tstring;

// PENGUIN: fake CDate to be an integer type
typedef unsigned int CDate;

#define UNORDERED_MAP std::map

#define NUM_ITERS 10
#define NUM_VALUES_PER_LINE 10

/****************************************************************************************
 * extracted class/method definition
 ***************************************************************************************/

/************************************************************************
 * from ./AlgoFinance_WBC/src/includes/WBCCVAHelpersP.h
 *
 *************************************************************************/
enum CVAExposureBucketingType
  {
    END_OF_INTERVAL = 0,
    START_OF_INTERVAL,
    MID_OF_INTERVAL,
    MID_OF_INTERVAL_BASEL
  };

/************************************************************************
 * from ./includes/Finance/intern/FinanceOS.h
 *
 *************************************************************************/
//==============================NT========================================

#if defined(WIN32) || defined(__CYGWIN__)

#  ifdef FINANCE_EXPORTS  // Build a DLL on NT
#    define FINANCE_DECL __declspec(dllexport)
#    define FINANCE_EXTERN_DECL
#  else
#    ifdef FINANCE_STATIC   // Build a static lib on NT
#      define FINANCE_DECL
#      define FINANCE_EXTERN_DECL
#    else   // Build an NT application that uses the dll
#      define FINANCE_DECL __declspec(dllimport)
#      define FINANCE_EXTERN_DECL __declspec(dllimport)
#    endif
#  endif

#else //=======================UNIX===================================

#  define FINANCE_DECL
#  define FINANCE_EXTERN_DECL

#endif

/************************************************************************
 * from ./includes/Cube/intern/CubeOS.h
 *
 *************************************************************************/
//==============================NT========================================

#if defined(WIN32) || defined(__CYGWIN__)

#  ifdef CUBE_EXPORTS  // Build a DLL on NT
#    define CUBE_DECL __declspec(dllexport)
#    define CUBE_EXTERN_DECL
#  else
#    ifdef CUBE_STATIC   // Build a static lib on NT
#      define CUBE_DECL
#      define CUBE_EXTERN_DECL
#    else   // Build an NT application that uses the dll
#      define CUBE_DECL __declspec(dllimport)
#      define CUBE_EXTERN_DECL __declspec(dllimport)
#    endif
#  endif

#else //=======================UNIX===================================

#  define CUBE_DECL
#  define CUBE_EXTERN_DECL

#endif

/************************************************************************
 * from ./includes/Finance/intern/CVAP.h
 *
 *************************************************************************/
#define AMOUNT_CALC     0
#define RATE_CALC       1
#define AMOUNT_CALC_NET 2

/************************************************************************
 * CCVAValueArrayPointer (from includes/Finance/ValueArrayPrinter.h and 
 *                              ./AlgoFinance/src/ValueArrayPrinter.cpp) 
 *
 * Omitted methods not used in the kernel
 *************************************************************************/
template<unsigned CALC_TYPE, bool ENABLE>
class FINANCE_DECL CCVAValueArrayPrinter
{
public:
  // PENGUIN: removed unused fields
  CCVAValueArrayPrinter() {}
  void AddValue(ValueType v) {}
  void SumValue(ValueType val, unsigned idx) {}
  void PrintValues(unsigned) const {}
};

// specialized
template<unsigned CALC_TYPE>
class FINANCE_DECL CCVAValueArrayPrinter<CALC_TYPE, true>
{
public:
  CCVAValueArrayPrinter() {}

  void AddValue(ValueType v)
  {
    i_Values.push_back(v);
  }
  void SumValue(ValueType val, unsigned idx)
  {
    i_Values[idx] += val;
  }
  void PrintValues(unsigned) const
  {
    // not-yet-supported
    assert(false);
  }
private:
  std::vector<ValueType> i_Values;
};

/*********************************************************************
 * H5SheetAccessor from ./includes/Cube/H5SheetAccessor.h 
 *
 * Omitted methods not used in the kernel
 ********************************************************************/
class H5SheetAccessor
{
  struct IDataEntryAccessorSimpleReference
  {

  private:
    struct IReferenceHolder
    {
      unsigned i_Count;
      IReferenceHolder() : i_Count(0) {}

    private:
      // PENGUIN: removed DataEntryAccessor
      IReferenceHolder(const IReferenceHolder &hdl) : i_Count(hdl.i_Count) /*, i_Accessor(hdl.i_Accessor)*/ {}
      IReferenceHolder& operator=(const IReferenceHolder &obj)
      {
	new (this) IReferenceHolder(obj);
	return *this;
      }
      
    };
    
    IReferenceHolder *i_DataAccessor;
    
  public:
    //IDataEntryAccessorSimpleReference() : i_DataAccessor(NULL) {}
    IDataEntryAccessorSimpleReference(const IDataEntryAccessorSimpleReference &obj) : i_DataAccessor(obj.i_DataAccessor)
    {
      Reference();
    }
    
    // PENGUIN: remove acc and info, will fake it
    IDataEntryAccessorSimpleReference(/* const HDF5CubeAccessor *acc , const HDF5CubeInstrumentInfo *info*/)
      : i_DataAccessor(NULL)
    {
      i_DataAccessor = new IReferenceHolder();
      i_DataAccessor->i_Count = 0;
      Reference();
    }
    
    ~IDataEntryAccessorSimpleReference()
    {
      Dereference();
    }
    
    void Dereference()
    {
      if( i_DataAccessor )
	{
	  assert( i_DataAccessor->i_Count > 0 );
	  --i_DataAccessor->i_Count;
	  if( i_DataAccessor->i_Count == 0 )
	    {
	      /* PENGUIN: comment it out, just fake it
	      DataEntryAccessor_clean(&i_DataAccessor->i_Accessor);
	      */
	      delete i_DataAccessor;
	    }
	  
	  i_DataAccessor = NULL;
	}
    }
    
    void Reference()
    {
      if( i_DataAccessor != NULL )
	{
	  ++i_DataAccessor->i_Count;
	}
    }
    
    IDataEntryAccessorSimpleReference& operator=(const IDataEntryAccessorSimpleReference &obj)
    {
      if(&obj != this)
	{
	  this->~IDataEntryAccessorSimpleReference();
	  new (this) IDataEntryAccessorSimpleReference(obj);
	}
      return *this;
    }
  };

public:
  H5SheetAccessor()
    : i_buffer(NULL), i_numScenarios(0), i_numTimes(0), i_numSheets(0), i_pointsPerSheet(0) {}
  
  H5SheetAccessor(const char*buffer, unsigned num_scenarios, unsigned num_times, unsigned num_sheets, size_t sheet_sz)
    : i_buffer(reinterpret_cast<const ValueType *>(buffer)), i_numScenarios(num_scenarios), i_numTimes(num_times), i_numSheets(num_sheets), i_pointsPerSheet(sheet_sz) {}
  
#ifdef TRANSPOSED_LAYOUT
  ValueType GetValue(unsigned sheet, unsigned scen_idx, unsigned t_idx) const 
  {
    const ValueType *begin_sheet = i_buffer + (sheet * i_pointsPerSheet);
    return begin_sheet[i_numTimes * scen_idx + t_idx];
  }
#else
  ValueType GetValue(unsigned sheet, unsigned scen_idx, unsigned t_idx) const 
  {
    const ValueType *begin_sheet = i_buffer + (sheet * i_pointsPerSheet);
    return begin_sheet[i_numScenarios*t_idx + scen_idx];
  }
#endif

private:
  const ValueType *i_buffer;
  unsigned i_numScenarios; 
  unsigned i_numTimes;     
  unsigned i_numSheets;    
  size_t i_pointsPerSheet;
  IDataEntryAccessorSimpleReference i_DataEntryAccessor;
};

/*********************************************************************
 * CSheetResolver from ./includes/Cube/SheetResolver.h 
 *
 * Omitted methods that are not used in the kernel
 ********************************************************************/
class CUBE_DECL CSheetResolver
{
protected:
  unsigned i_NumberOfScenarios; 
  unsigned i_NumberOfTimeSteps; 

public:
  /**
   * Builds a new one for the given dimensions.
   */
  inline CSheetResolver(unsigned numberOfScenarios, unsigned numberOfTimeSteps) 
    : i_NumberOfScenarios(numberOfScenarios),
      i_NumberOfTimeSteps(numberOfTimeSteps)
  {}
  
  /**
   * Returns the value for the given time and scenario indices.
   */
  inline ValueType & Value(
			   ValueType * sheet,
			   unsigned scenIdx,
			   unsigned timeIdx
			   )     const
  {
    return sheet[i_NumberOfScenarios*timeIdx + scenIdx];
  }
  
  /**
   * Returns the value for the given time and scenario indices.
   */
  inline ValueType Value(
			 const ValueType * sheet,
			 unsigned scenIdx,
			 unsigned timeIdx
			 ) const
  {
    return sheet[i_NumberOfScenarios*timeIdx + scenIdx];
  }

  /**
   * Returns the line of values across scenarios, for a given time step.
   */
  inline ValueType * ScenarioLine(
				  ValueType * sheet,
				  unsigned timeIdx
				  ) const
  {
    return & sheet[i_NumberOfScenarios*timeIdx];
  }

  /**
   * PENGUIN: add the print out function
   */
  void DumpSheet(
		 const ValueType * sheet,
		 char* prefix
		 ) const
  {
    std::cout << prefix << " DUMP SHEET [timeSteps=" << i_NumberOfTimeSteps 
	      << ", scenarios=" << i_NumberOfScenarios << "]" << std::endl;
    for (int i=0; i<i_NumberOfTimeSteps; i++) 
      {
	std::cout << "  [" << i << "]: ";
	for (int j=0; j<i_NumberOfScenarios; j++) 
	  {
	    std::cout << Value(sheet, j, i) << " ";
	  }
	std::cout << std::endl;
      }
  }
};

/*********************************************************************
 * CSheetResolver from ./includes/Finance/intern/CVAP.h 
 *
 * Omitted methods that are not used in the kernel
 ********************************************************************/
template<unsigned CALC_TYPE>
struct CCVARateHelper
{
  static void CalcRate(ValueType &amount, const ValueType demoninator)
  {
    amount = demoninator == (ValueType)0 ? ((ValueType)0.0) : amount / demoninator;
  }

  static void AccumulateDenominator(ValueType &total, const ValueType m_value,
				    const ValueType r_1, const CDate &tj_1, const CDate &tj)
  {
    ValueType t_delta = ValueType(tj - tj_1)/ValueType(365.0);
    total += m_value * r_1 * t_delta;
  }
};

// specialization
/* When we calculate amount, all the rate code will
* be compile out due to optimization so every call
* related to rate will be gone.
*/
template<>
struct CCVARateHelper<AMOUNT_CALC>
{
  static void CalcRate(ValueType &amount, const ValueType demoninator) {}
  static void AccumulateDenominator(ValueType &total, const ValueType m_value,
				    const ValueType r_1, const CDate &tj, const CDate &tj_1)
  {}
};

template<>
struct CCVARateHelper<AMOUNT_CALC_NET>
{ 
  static void CalcRate(ValueType &amount, const ValueType demoninator) {}
  static void AccumulateDenominator(ValueType &total, const ValueType m_value,
				    const ValueType r_1, const CDate &tj, const CDate &tj_1)
  {}
};

/*********************************************************************
 * KahanUpdate from ./includes/Finance/intern/KahanSummation.h 
 *
 * Omitted methods that are not used in the kernel
 ********************************************************************/
template <class ValueType>
inline void
KahanUpdate(ValueType & sum, ValueType & correction, ValueType term)
{
          ValueType y = term - correction;
          ValueType t = sum + y;
          correction = (t - sum) - y;
          sum = t;
}

/*********************************************************************
 * KahanUpdate from ./includes/Consolidation/ValueAdder.h 
 *
 * Omitted methods that are not used in the kernel
 ********************************************************************/
#ifdef ENABLE_SSE
#include <xmmintrin.h>
#include <math.h>

template <class VALUE_TYPE>
class SSEArithmetics {};

/* ========== float arithmetic specialization =========== */

template<>
class SSEArithmetics<float>
{
public:
  typedef float ValueType;

  enum { elements = 4, mask = 0x3 };

  struct Chunk
  {
  private:
    __m128 i_Data;
    
    Chunk(const __m128 &data) : i_Data(data) { /* private ctor */ }
    
  public:
    
    /* loads 4 single-precision floating point values
     * r0 = data[0], r1 = data[1], r2 = data[2], r3 = data[3]
     */
    explicit Chunk(const ValueType *data) : i_Data( _mm_load_ps(data) ) { /* ctor */ }
    Chunk(const Chunk &other) : i_Data(other.i_Data) { /* copy constructor */ }
    explicit Chunk(const ValueType &val) : i_Data( _mm_set_ps1(val) ) {}
    
    
    /* fills i_Data with
     * data[0] = a0
     * data[1] = a1
     * data[2] = a2
     * data[3] = a3
     */
    void FillData(ValueType *data)
    {
      _mm_store_ps(data, i_Data);
    }
    
    Chunk operator+(const Chunk &other) const
    {
      return _mm_add_ps(i_Data, other.i_Data);
    }
    
    Chunk operator-(const Chunk &other) const
    {
      return _mm_sub_ps(i_Data, other.i_Data);
    }
    
    Chunk operator*(const Chunk &other) const
    {
      return _mm_mul_ps(i_Data, other.i_Data);
    }
    
    Chunk operator/(const Chunk &other) const
    {
      return _mm_div_ps(i_Data, other.i_Data);
    }
    
    Chunk inverse(void) const
    {
      return _mm_rcp_ps(i_Data);
    }
    
    Chunk& operator=(const Chunk &other)
    {
      i_Data = other.i_Data;
      return *this;
    }
    
    Chunk sqrt(void)
    {
      return _mm_sqrt_ps(i_Data);
    }
    
    void operator+=(const Chunk &other)
    {
      i_Data = _mm_add_ps(i_Data, other.i_Data);
    }
    
    void operator-=(const Chunk &other)
    {
      i_Data = _mm_sub_ps(i_Data, other.i_Data);
    }
    
    void operator*=(const Chunk &other)
    {
      i_Data = _mm_mul_ps(i_Data, other.i_Data);
    }
    
    void operator/=(const Chunk &other)
    {
      i_Data = _mm_div_ps(i_Data, other.i_Data);
    }
    
    // unary operator
    Chunk operator-()
    {
      Chunk tmp = _mm_set1_ps(-1);
      return (*this * tmp);
    }

    Chunk abs(void) const
    {
      unsigned int x = 0x7fffffff;
      float &f = * (float *)( &x );
      __m128 tmp = _mm_set1_ps(f);
      return _mm_and_ps(i_Data, tmp);
    }
    
    Chunk min(const Chunk &other) const
    {
      return _mm_min_ps(i_Data, other.i_Data);
    }
    
    Chunk min(const ValueType &val) const
    {
      __m128 tmp = _mm_set_ps1(val);
      return _mm_min_ps(i_Data, tmp);
    }
    
    Chunk max(const Chunk &other) const
    {
      return _mm_max_ps(i_Data, other.i_Data);
    }
    
    Chunk max(const ValueType &val) const
    {
      __m128 tmp = _mm_set_ps1(val);
      return _mm_max_ps(i_Data, tmp);
    }
    
  };
};
#endif

#ifdef ENABLE_SIMD
#ifdef __ALTIVEC__
#include <power_vsx4.h>
using namespace vsx;
#else
#ifdef __SSE4_2__
#include <sse4.h>
using namespace sse;
#else
#include <generic4.h>
using namespace generic;
#endif //__SSE4_2__
#endif //__ALTIVEC__
#include <math.h>

template <class VALUE_TYPE>
class SIMDArithmetics {};

/* ========== float arithmetic specialization =========== */

template<>
class SIMDArithmetics<float>
{
public:
  typedef float ValueType;

  enum { elements = 4, mask = 0x3 };

  struct Chunk
  {
  private:
    svec4_f i_Data;

    Chunk(const svec4_f &data) : i_Data(data) { /* private ctor */ }

  public:

    /* loads 4 single-precision floating point values
     * r0 = data[0], r1 = data[1], r2 = data[2], r3 = data[3]
     */
    explicit Chunk(const ValueType *data) : i_Data( svec4_f::load((svec4_f*)data) ) { /* ctor */ }
    Chunk(const Chunk &other) : i_Data(other.i_Data) { /* copy constructor */ }
    explicit Chunk(const ValueType &val) : i_Data( svec4_f(val) ) {}


    /* fills i_Data with
     * data[0] = a0
     * data[1] = a1
     * data[2] = a2
     * data[3] = a3
     */
    void FillData(ValueType *data)
    {
      i_Data.store((svec4_f*)data);
    }

    Chunk operator+(const Chunk &other) const
    {
      return i_Data + other.i_Data;
    }

    Chunk operator-(const Chunk &other) const
    {
      return i_Data - other.i_Data;
    }

    Chunk operator*(const Chunk &other) const
    {
      return i_Data * other.i_Data;
    }

    Chunk operator/(const Chunk &other) const
    {
      return i_Data / other.i_Data;
    }

    Chunk inverse(void) const
    {
      return i_Data.rcp();
    }

    Chunk& operator=(const Chunk &other)
    {
      i_Data = other.i_Data;
      return *this;
    }

    Chunk sqrt(void)
    {
      return i_Data.sqrt();
    }

    void operator+=(const Chunk &other)
    {
      i_Data = i_Data + other.i_Data;
    }

    void operator-=(const Chunk &other)
    {
      i_Data = i_Data - other.i_Data;
    }

    void operator*=(const Chunk &other)
    {
      i_Data = i_Data * other.i_Data;
    }

    void operator/=(const Chunk &other)
    {
      i_Data = i_Data / other.i_Data;
    }

    // unary operator
    Chunk operator-()
    {
      Chunk tmp = svec4_f(-1);
      return (*this * tmp);
    }

    Chunk abs(void) const
    {
      return i_Data.abs();
    }

    Chunk min(const Chunk &other) const
    {
      return svec_min(i_Data, other.i_Data);
    }

    Chunk min(const ValueType &val) const
    {
      svec4_f tmp(val);
      return svec_min(i_Data, tmp);
    }

    Chunk max(const Chunk &other) const
    {
      return svec_max(i_Data, other.i_Data);
    }

    Chunk max(const ValueType &val) const
    {
      svec4_f tmp(val);
      return svec_max(i_Data, tmp);
    }

  };
};
#endif


/*********************************************************************
 * CStandardMeanCalculator from ./includes/Finance/intern/MeanP.h 
 *
 * Omitted methods that are not used in the kernel
 ********************************************************************/
struct FINANCE_DECL CStandardMeanCalculator
{
  static inline void Calculate(unsigned numScenarios, unsigned numTimes, const ValueType *in, ValueType *result)
  {
    const ValueType factor((ValueType)1.0/numScenarios);
    for (unsigned t = 0; t < numTimes; ++t)
      {
	ValueType sum = 0;
	ValueType correction = 0;
	for (unsigned s = 0; s < numScenarios; ++s, ++in)
	  {
	    KahanUpdate(sum, correction, *in);
	  }
	result[t] = sum*factor;
      }
  }
};

struct FINANCE_DECL CStandardMeanExposureCalculator
{
  static inline void Calculate(unsigned numScenarios, unsigned numTimes, const ValueType *in, ValueType *result)
  {
    const ValueType factor((ValueType)1.0/numScenarios);
    for (unsigned t = 0; t < numTimes; ++t)
      {
	ValueType sum = 0;
	ValueType correction = 0;
	for (unsigned s = 0; s < numScenarios; ++s, ++in)
	  {
	    ValueType in_val = *in;
	    KahanUpdate(sum, correction, (in_val > 0 ? in_val : 0));
	  }
	result[t] = sum*factor;
      }
  }
};

#ifdef ENABLE_SSE
struct CSSEMeanCalculator
{
  typedef typename SSEArithmetics<ValueType>::Chunk Chunk;
  enum { elements = SSEArithmetics<ValueType>::elements, two_elements = SSEArithmetics<ValueType>::elements*2, mask = SSEArithmetics<ValueType>::mask };
  
  static inline void Calculate(unsigned numScenarios, unsigned numTimes, const ValueType *in, ValueType *result)
  {
    // if number of scenarios is greater than chunk AND divisible by either 2 or 4 depends on the precision
    if( numScenarios > two_elements && (numScenarios & mask) == 0 )
      {
	// SIMD required to aligned in the 16 byte boundary
	assert( ((size_t)in % 16) == 0 );
	
	const ValueType factor((ValueType)1.0/numScenarios);
	ValueType tmp[elements] __attribute__ ((aligned(16)));
	for(unsigned t=0; t < numTimes; ++t)
	  {
	    ValueType final_sum = 0;
	    Chunk sum_chunk(in);
	    Chunk correction((ValueType)0);
	    unsigned num = numScenarios - elements;
	    in += elements;
	    for(; num >= elements; in += elements, num -= elements )
	      {
		Chunk ele(in);
		KahanUpdate<Chunk>(sum_chunk, correction, ele);
	      }
	    
	    
	    sum_chunk.FillData( &tmp[0]);
	    ValueType c = 0;
	    for(unsigned i=0; i < elements; ++i)
	      KahanUpdate(final_sum, c, tmp[i]);
	    
	    assert( num == 0 );
	    
	    result[t] = final_sum*factor;
	  }
      }
    else
      CStandardMeanCalculator::Calculate(numScenarios, numTimes, in, result);
  }
};

struct CSSEMeanExposureCalculator
{
  typedef typename SSEArithmetics<ValueType>::Chunk Chunk;
  enum { elements = SSEArithmetics<ValueType>::elements, two_elements = SSEArithmetics<ValueType>::elements*2, mask = SSEArithmetics<ValueType>::mask };
  
  static inline void Calculate(unsigned numScenarios, unsigned numTimes, const ValueType *in, ValueType *result)
  {
    // if number of scenarios is greater than chunk AND divisible by either 2 or 5 depends on the precision
    if( numScenarios > two_elements && (numScenarios & mask) == 0 )
      {
	// SIMD required to aligned in the 16 byte boundary
	assert( ((size_t)in % 16) == 0 );
	
	const Chunk zero((ValueType)0);
	const ValueType factor((ValueType)1.0/numScenarios);
	ValueType tmp[elements] __attribute__ ((aligned(16)));
	for(unsigned t=0; t < numTimes; ++t)
	  {
	    ValueType final_sum = 0;
	    Chunk sum_chunk( Chunk(in).max(zero) );  // apply exposure
	    Chunk correction((ValueType)0);
	    unsigned num = numScenarios - elements;
	    in += elements;
	    for(; num >= elements; in += elements, num -= elements )
	      {
		Chunk ele( Chunk(in).max(zero) ); // apply exposure
		KahanUpdate<Chunk>(sum_chunk, correction, ele); // apply sumation
	      }
	    
	    
	    sum_chunk.FillData( &tmp[0]);
	    ValueType c = 0;
	    for(unsigned i=0; i < elements; ++i)
	      KahanUpdate(final_sum, c, tmp[i]);
	    
	    assert( num == 0 );
	    
	    result[t] = final_sum*factor;
	  }
      }
    else
      CStandardMeanExposureCalculator::Calculate(numScenarios, numTimes, in, result);
  }
};
#endif

#ifdef ENABLE_SIMD
struct CSIMDMeanCalculator
{
  typedef typename SIMDArithmetics<ValueType>::Chunk Chunk;
  enum { elements = SIMDArithmetics<ValueType>::elements, two_elements = SIMDArithmetics<ValueType>::elements*2, mask = SIMDArithmetics<ValueType>::mask };

  static inline void Calculate(unsigned numScenarios, unsigned numTimes, const ValueType *in, ValueType *result)
  {
    // if number of scenarios is greater than chunk AND divisible by either 2 or 4 depends on the precision
    if( numScenarios > two_elements && (numScenarios & mask) == 0 )
      {
    // SIMD required to aligned in the 16 byte boundary
    assert( ((size_t)in % 16) == 0 );

    const ValueType factor((ValueType)1.0/numScenarios);
    ValueType tmp[elements] __attribute__ ((aligned(16)));
    for(unsigned t=0; t < numTimes; ++t)
      {
        ValueType final_sum = 0;
        Chunk sum_chunk(in);
        Chunk correction((ValueType)0);
        unsigned num = numScenarios - elements;
        in += elements;
        for(; num >= elements; in += elements, num -= elements )
          {
        Chunk ele(in);
        KahanUpdate<Chunk>(sum_chunk, correction, ele);
          }


        sum_chunk.FillData( &tmp[0]);
        ValueType c = 0;
        for(unsigned i=0; i < elements; ++i)
          KahanUpdate(final_sum, c, tmp[i]);

        assert( num == 0 );

        result[t] = final_sum*factor;
      }
      }
    else
      CStandardMeanCalculator::Calculate(numScenarios, numTimes, in, result);
  }
};

struct CSIMDMeanExposureCalculator
{
  typedef typename SIMDArithmetics<ValueType>::Chunk Chunk;
  enum { elements = SIMDArithmetics<ValueType>::elements, two_elements = SIMDArithmetics<ValueType>::elements*2, mask = SIMDArithmetics<ValueType>::mask };

  static inline void Calculate(unsigned numScenarios, unsigned numTimes, const ValueType *in, ValueType *result)
  {
    // if number of scenarios is greater than chunk AND divisible by either 2 or 5 depends on the precision
    if( numScenarios > two_elements && (numScenarios & mask) == 0 )
      {
    // SIMD required to aligned in the 16 byte boundary
    assert( ((size_t)in % 16) == 0 );

    const Chunk zero((ValueType)0);
    const ValueType factor((ValueType)1.0/numScenarios);
    ValueType tmp[elements] __attribute__ ((aligned(16)));
    for(unsigned t=0; t < numTimes; ++t)
      {
        ValueType final_sum = 0;
        Chunk sum_chunk( Chunk(in).max(zero) );  // apply exposure
        Chunk correction((ValueType)0);
        unsigned num = numScenarios - elements;
        in += elements;
        for(; num >= elements; in += elements, num -= elements )
          {
        Chunk ele( Chunk(in).max(zero) ); // apply exposure
        KahanUpdate<Chunk>(sum_chunk, correction, ele); // apply sumation
          }


        sum_chunk.FillData( &tmp[0]);
        ValueType c = 0;
        for(unsigned i=0; i < elements; ++i)
          KahanUpdate(final_sum, c, tmp[i]);

        assert( num == 0 );

        result[t] = final_sum*factor;
      }
      }
    else
      CStandardMeanExposureCalculator::Calculate(numScenarios, numTimes, in, result);
  }
};
#endif

struct FINANCE_DECL CMeanCalculator
{
  static inline void Calculate(unsigned numScenarios, unsigned numTimes, const ValueType *in, ValueType *result)
  {
#ifdef ENABLE_SSE
    CSSEMeanCalculator::Calculate(numScenarios, numTimes, in, result);
#else
#ifdef ENABLE_SIMD
    CSIMDMeanCalculator::Calculate(numScenarios, numTimes, in, result);
#else
    CStandardMeanCalculator::Calculate(numScenarios, numTimes, in, result);
#endif //ENABLE_SIMD
#endif //ENABLE_SSE
  }
};

struct FINANCE_DECL CMeanExposureCalculator
{
  static inline void Calculate(unsigned numScenarios, unsigned numTimes, const ValueType *in, ValueType *result)
  {
#ifdef ENABLE_SSE
    CSSEMeanExposureCalculator::Calculate(numScenarios, numTimes, in, result);
#else
#ifdef ENABLE_SIMD
    CSIMDMeanExposureCalculator::Calculate(numScenarios, numTimes, in, result);
#else
    CStandardMeanExposureCalculator::Calculate(numScenarios, numTimes, in, result);
#endif //ENABLE_SIMD
#endif //ENABLE_SSE
  }
};

/*********************************************************************
 * CStandardMeanCalculator from ./includes/Finance/intern/ExpectedTailExposure.h 
 *
 * Omitted methods that are not used in the kernel
 ********************************************************************/
FINANCE_DECL void CalculateExpectedTailExposure(
						unsigned numScenarios, unsigned numTimes,  const ValueType * in,
						ValueType * result, ValueType confidence, bool oneSided);

#if 0
/*********************************************************************
 * H5CatalogueAccessor from ./includes/Cube/H5CatalogueAccessor.h 
 *
 * Omitted methods that are not used in the kernel
 ********************************************************************/
/* ========= H5CatalogueAccessor wrapper ============*/
class H5CatalogueAccessor
{
  mutable HDF5CubeAccessor *     i_pCubeAccessor;
  const HDF5CubeInfo *           i_pInfo;
  const HDF5CubeDefinition *     i_pDefinition;
  
  tstring i_fileName;
  CSimpleLockable i_Lock;
  
  mutable std::vector<CDate>     i_Dates;
  mutable std::vector<tstring>   i_DefaultScenarioNames;
  mutable std::vector<ValueType> i_DefaultScenarioWeights;

  /* prevent explicit copying */
  H5CatalogueAccessor& operator=(const H5CatalogueAccessor &);
  
public:
  H5CatalogueAccessor(const tstring &filename) : i_pCubeAccessor(NULL),
						 i_pInfo(NULL), i_pDefinition(NULL),
						 i_fileName(filename)
  {
    HDF5CubeError_t ret = HDF5CubeAccessor_create(filename.c_str(), &i_pCubeAccessor);
    if( ret != HDF5_CUBE_OK )
      throw CBadConditions(HDF5CubeAccessor_getErrorBuffer(i_pCubeAccessor));
    
    if( i_pCubeAccessor->version > (float)HDF5CUBE_CURRENT_VERSION )
      {
	/* log a warning message that we are dealing with forward compatable */
	TheMessageCenter::PostWarning(_T("Catalogue Accessor"),
				      _T("Input cube version is newer than the current MAG's supported cube version.  Input cube version is ") +
				      MUS::ToString(i_pCubeAccessor->version) + _T(", MAG's supported cube version is ") + MUS::ToString((float)HDF5CUBE_CURRENT_VERSION));
	
      }
    
    i_pInfo = HDF5CubeAccessor_getInfo(i_pCubeAccessor);
    i_pDefinition = HDF5CubeAccessor_getDefinition(i_pCubeAccessor);
    HDF5CubeAccessor_setStyle(i_pCubeAccessor, H5CubeUtilityServices::GetStyle<ValueType>());
  }
  
  // NOTE, this is NOT a copy constructor..morphe current object into the given object
  H5CatalogueAccessor(H5CatalogueAccessor &m) : i_pCubeAccessor(m.i_pCubeAccessor),
						i_pInfo(m.i_pInfo),
						i_pDefinition(m.i_pDefinition),
						i_fileName(m.i_fileName)
  {
    m.i_pCubeAccessor = NULL;
    m.i_pInfo = NULL;
    m.i_pDefinition = NULL;
  }

  ~H5CatalogueAccessor()
  {
    if( i_pCubeAccessor != NULL )
      HDF5CubeAccessor_close( &i_pCubeAccessor );
    
    i_pCubeAccessor = NULL;
    i_pInfo = NULL;
    i_pDefinition = NULL;
  }
  
  bool GetEntry(const CQueryEntry &q, H5CatalogueEntry &entry) const
  {
    const HDF5CubeInstrumentInfo *info = H5CubeUtilityServices::GetInstrumentInfo(i_Lock, i_pCubeAccessor, q.GetKey(), q.GetType());
    if( info == NULL )
      return false;
    
    assert( HDF5CubeInstrumentInfo_getFlags(info) & q.GetType() );
    H5CatalogueEntry temp(info, i_pCubeAccessor, HDF5CubeAccessor_getSheetEntry(i_pCubeAccessor, info));
    entry = temp;
    return true;
  }
    
  bool GetSheetAccessor(const CQueryEntry &q, H5SheetAccessor<ValueType> &sheet) const
  {
    H5CatalogueEntry entry(0, 0, 0);
    
    if(GetEntry(q, entry))
      {
	H5SheetAccessor temp(i_pCubeAccessor, entry.i_pInfo);
	sheet = temp;
	return true;
      }
    else
      return false;
  }
};
#endif

/*********************************************************************
 *  ./includes/HDF5Cube/MagMalloc.h 
 *
 ********************************************************************/
#ifndef MAG_MALLOC_H
#define MAG_MALLOC_H

#if defined(USE_DMALLOC)
# include "dmalloc.h"
#endif

#if !defined(OSX)
#include <malloc.h>
#else
#include <memory.h>
#endif

#ifdef _aix_
#include <stdlib.h>
#endif

#if defined(ENABLE_SSE) || defined(ENABLE_SIMD)
#define BOUNDARY_ALIGN_SZ 16
#else
#define BOUNDARY_ALIGN_SZ 8
#endif

#if  defined(_aix_) || defined(WIN32) || defined(OSX)
#define MagMalloc(alignment_sz, sz) malloc(sz)
#else
#define MagMalloc(alignment_sz, sz) memalign(alignment_sz, sz)
#endif

#endif

/*********************************************************************
 * CMemAlignAllocator from ./includes/Cube/MemAlignAllocator.h
 ********************************************************************/
template <class T>
class CMemAlignAllocator
{
public:
  // typedefs
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T value_type;
  
  // converter from CMemAlignAllocator<T> to CMemAlignAllocator<U>
  template<class U> struct rebind { typedef CMemAlignAllocator<U> other; };
  
  // ctor and dtor
  inline explicit CMemAlignAllocator() {}
  inline explicit CMemAlignAllocator(const CMemAlignAllocator<T> &) {}

  template<class U> inline explicit CMemAlignAllocator(const CMemAlignAllocator<U> &) {}
  //template<class U> inline CMemAlignAllocator(CMemAlignAllocator<U> &) {}

  inline ~CMemAlignAllocator() {}
  
  // address
  inline pointer address(reference r) { return &r; }
  const_pointer address(const_reference r) const { return &r; }
  
  // mem allocation
  pointer allocate(size_type sz,
		   std::allocator<void>::const_pointer hint = 0)
  {
    return reinterpret_cast<pointer>( MagMalloc(BOUNDARY_ALIGN_SZ, sz*sizeof(T)) );
  }
  
  void deallocate(pointer p, size_type sz) { ::free(p); }
 
  size_type max_size() const 
  { 
    return (std::numeric_limits<size_type>::max() / sizeof(T));
  }
  
  void construct(pointer p, const T& val) { new(p) T(val); }
  void destroy(pointer p) { p->~T(); }
  
  
  bool operator==(const CMemAlignAllocator<T> &) { return true; }
  
  bool operator!=(const CMemAlignAllocator<T> &a) { return !operator==(a); }
};

#endif
