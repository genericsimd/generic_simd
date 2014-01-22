#Programming Guide
 
<b>For detailed interface specification, refer to [Generic SIMD intrinsics library API] (http://genericsimd.github.io/generic_simd/index.html) </b>

##Data Types

The library supports templaterized SIMD vector types, *svec<N,STYPE>*,
where *N* specifies elements per vector and has to be power of two.
*STYPE* specifies scalar type of vector element: *bool*, *char*, "unsigned
char*, *short*, *unsigned short*, *int*, *unsigned int*, *long long*,
*unsigned long long*, *float*, and *double*.

Currently the library supports only N = 4

- *svec<4,bool>*: vector of 4 boolean 
- *svec<4,int8_t>, svec<4,uint8_t>: vector of 4 signed/unsigned 8-bit int 
- *svec<4,int16_t>*, *svec<4,uint16_t>*: vector of 4 signed/unsigned 16-bit int 
- *svec<4,int32_t>*, *svec<4,int32_t>*: vector of 4 signed/unsigned 32-bit int 
- *svec<4,int64_t>*, *svec<4,uint64_t>*: vector 4 signed/unsigned 32-bit int 
- *svec<4,float>*: vector of 4 float 
- *svec<4,double>*: vector of 4 double 
- *svec<4,void*>*: vector of 4 pointers

In the rest of the document we use VTYPE to indicate SIMD vector types.

##Operations

###Constructor

- Default constructor returns a vector with undefined value. e.g. "svec<4,int32_t> v;" 
  You can modify it's elements by "[]" operator. 
- Construct a SIMD vector with four scalar values. e.g. "svec<4,int32_t> v(1,2,3,4)" 
- Construct a SIMD vector with one scalar value. e.g. "svec<4,int32_t> v(100)". 

All the four values in the SIMD vector is 100. 


###Extract/insert single vector element

"[]" operator is used to get and set the elements.
```c++
svec<4,int32_t> v(1,2,3,4);
int a = v[2]; // extracts the 3rd element of the vector (i.e., element index starts from 0), a is 3 now
v[3] = 10;    // assigns 10 to the 3rd element of the vector, v is [1,2,3,10] now
```

Due to the current limitation, bool vector's setter must use "-1" as true in the right hand side.
```c++
svec<4,bool> m(0); // construct a vector of boolean with all elements initialized to false
m[0] = -1;     // after assignment, 1st element of m is true.
```

###Load and Store

Store a vector to location p through instance method store(VTYPE *).

Load a vector from location p through class static method VTYPE::(VTYPE *).
e.g. "svec<4,int32_t>::load(an_address)" will return a new svec<4,int32_t> vector.

Load a scalar value from an address and splat it into the whole vector could be done through class static method VTYPE::load_and_splat(STYPE *)

There is another method called VTYPE::load_const(STYPE*), which has similar semantics.

###Compare Operations

Compare two vectors, and return a svec<4,bool> vector.

Operators: == != for all types

Operators: >, >=, <, <= for all types except svec<4,bool>.

###Bit operations

svec<4,bool> has operator ~ to reverse the boolean value.

Binary bit operators &, |, ^ are available for all integer vector types.

Logical operators !, &&, || are available for svec<4,bool> type.

###Math operations

Support all types except svec<4,bool>.

Unary operator "-" is used to get the neg value for non-boolean vectors

Binary operators +, -, *, / can support VTYPE op VTYPE, VTYPE op STYPE, STYPE op VTYPE.

Binary operators >>, <<, % can support VTYPE op VTYPE, VTYPE op STYPE over all integer types. 

\>> and << for shift, and % for remainder.

Please note shift by a vector can only has unsigned integer vector in the right hand.

###Instance methods operations

broadcast(), rotate(), shuffle() support all types exclude svec<4,bool>().

round(), floor(), ceil(), sqrt(), rcp(), rsqrt(), exp(), log(), pow(VTYPE) support svec<4,float>, and svec<4,double>.

All above will return a new vector.

reduce_add(), reduce_max(), reduce_min() do a vector scope's reduction, and return a scalar value.

any_true(), all_true(), none_true() do a svec<4,bool> vector's reduction, and return a boolean scalar value.

###Gather and Scatter

Please refer the detail document for how to use gather and scatter.
E.g. svec<4,int32_t> type

- svec<4,int32_t>::gather()
- svec<4,int32_t>::scatter()
- svec<4,int32_T>::gather_base_offsets()
- svec<4,int32_t>::scatter_base_offsets()
- svec<4,int32_t>::gather_stride()
- svec<4,int32_t>::scatter_stride()

**Note** The current power processor has no gather/scatter instructions. The software based implementation is slow right now, especially the gather_base_offsets() and scatter_base_offsets().

In case of regular stride style gather/scatter, it's better to use gather_stride() and scatter_stride().

###Multiply-Add and Multiply-Sub

VTYPE svec_madd(VTYPE a, VTYPE b, VTYPE c) returns a * b + c;

VTYPE svec_msub(VTYPE a, VTYPE b, VTYPE c) returns a * b - c;

VTYPE svec_nmsub(VTYPE a, VTYPE b, VTYPE c) returns -(a * b - c);

###Select operation

The prototype is svec_select(svec<4,bool> mask, VTYPE a, VTYPE b), and return a new vector whose elements are selected from _a_ or _b_ based on the mask. True from _a_ and false from _b_.

There is another select svec_select(bool cond, VTYPE a, VTYPE b), which is the same as "cond ? a : b".

###Type cast operation

The prototype is svec_cast<TO_VTYPE>(FROM_VTYPE). It supports all combinations of type cast. Each element's cast semantics is the same as scalar cast.

###Operation with mask

load, store, gatter, scatter, compare operations have a masked version.
Please refer the detail document for detail.
