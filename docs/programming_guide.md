#Programming Guide
 
<b>For detailed interface specification, refer to [Generic SIMD intrinsics library API] (http://pengwuibm.github.io/generic_simd/index.html) </b>

##Data Types
The library supports the following SIMD vector types. 

- *svec4_i1*: vector of 4 boolean 
- *svec4_i8*, *svec4_u8*: vector of 4 signed/unsigned char 
- *svec4_i16*, *svec4_u16*: vector of 4 signed/unsigned short 
- *svec4_i32*, *svec4_u32*: vector of 4 signed/unsigned integer 
- *svec4_i64*, *svec4_u64*: vector 4 signed/unsigned long long 
- *svec4_f*: vector of 4 float 
- *svec4_d*: vector of 4 double 

where "i" indicates signed types and "u" unsigned types. 

In the rest of the document we use VTYPE to indicate a SIMD vector type, and STYPE to indicate a scalar type.

##Operations

###Constructor

- Default constructor returns a vector with undefined value. e.g. "svec4_i32 v;" 
  You can modify it's elements by "[]" operator. 
- Construct a SIMD vector with four scalar values. e.g. "svec4_i32 v(1,2,3,4)" 
- Construct a SIMD vector with one scalar value. e.g. "svec4_i32 v(100)". 

All the four values in the SIMD vector is 100. 


###Extract/insert single vector element

"[]" operator is used to get and set the elements.
```c++
svec4_i32 v(1,2,3,4);
int a = v[2]; // extracts the 3rd element of the vector (i.e., element index starts from 0), a is 3 now
v[3] = 10;    // assigns 10 to the 3rd element of the vector, v is [1,2,3,10] now
```

Due to the current limitation, bool vector's setter must use "-1" as true in the right hand side.
```c++
svec4_i1 m(0); // construct a vector of boolean with all elements initialized to false
m[0] = -1;     // after assignment, 1st element of m is true.
```

###Load and Store

Store a vector to location p through instance method store(VTYPE *).

Load a vector from location p through class static method VTYPE::(VTYPE *).
e.g. "svec4_i32::load(an_address)" will return a new svec4_i32 vector.

Load a scalar value from an address and splat it into the whole vector could be done through class static method VTYPE::load_and_splat(STYPE *)

There is another method called VTYPE::load_const(STYPE*), which has similar semantics.

###Compare Operations

Compare two vectors, and return a svec4_i1 vector.

Operators: == != for all types

Operators: >, >=, <, <= for all types except svec4_i1.

###Bit operations

svec4_i1 has operator ~ to reverse the boolean value.

Binary bit operators &, |, ^ are available for all integer vector types.

Logical operators !, &&, || are available for svec4_i1 type.

###Math operations

Support all types except svec4_i1.

Unary operator "-" is used to get the neg value for non-boolean vectors

Binary operators +, -, *, / can support VTYPE op VTYPE, VTYPE op STYPE, STYPE op VTYPE.

Binary operators >>, <<, % can support VTYPE op VTYPE, VTYPE op STYPE over all integer types. 

\>> and << for shift, and % for remainder.

Please note shift by a vector can only has unsigned integer vector in the right hand.

###Instance methods operations

broadcast(), rotate(), shuffle() support all types exclude svec4_i1().

round(), floor(), ceil(), sqrt(), rcp(), rsqrt(), exp(), log(), pow(VTYPE) support svec4_f, and svec4_d.

All above will return a new vector.

reduce_add(), reduce_max(), reduce_min() do a vector scope's reduction, and return a scalar value.

any_true(), all_true(), none_true() do a svec4_i1 vector's reduction, and return a boolean scalar value.

###Gather and Scatter

Please refer the detail document for how to use gather and scatter.
E.g. svec4_i32 type

- vsx::svec4_i32::gather()
- vsx::svec4_i32::scatter()
- vsx::svec4_i32::gather_base_offsets()
- vsx::svec4_i32::scatter_base_offsets()
- vsx::svec4_i32::gather_stride()
- vsx::svec4_i32::scatter_stride()

**Note** The current power processor has no gather/scatter instructions. The software based implementation is slow right now, especially the gather_base_offsets() and scatter_base_offsets().

In case of regular stride style gather/scatter, it's better to use gather_stride() and scatter_stride().

###Multiply-Add and Multiply-Sub

VTYPE svec_madd(VTYPE a, VTYPE b, VTYPE c) returns a * b + c;

VTYPE svec_msub(VTYPE a, VTYPE b, VTYPE c) returns a * b - c;

VTYPE svec_nmsub(VTYPE a, VTYPE b, VTYPE c) returns -(a * b - c);

###Select operation

The prototype is svec_select(svec4_i1 mask, VTYPE a, VTYPE b), and return a new vector whose elements are selected from _a_ or _b_ based on the mask. True from _a_ and false from _b_.

There is another select svec_select(bool cond, VTYPE a, VTYPE b), which is the same as "cond ? a : b".

###Type cast operation

The prototype is svec_cast<TO_VTYPE>(FROM_VTYPE). It supports all combinations of type cast. Each element's cast semantics is the same as scalar cast.

###Operation with mask

load, store, gatter, scatter, compare operations have a masked version.
Please refer the detail document for detail.
