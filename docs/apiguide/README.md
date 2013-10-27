# Generic SIMD API Guide

The Generic SIMD API Guide is a simple tool to search the data types and APIs of the Generic SIMD library.


## API data
The api data is defined in apidata.js as a json object.

Each API json object has five attributes
- *name*: String, the API's name.
- *Lane*: Integer, could be only 4 or 8 right now.
- *Type*: String, the API's base(scalar) type, could be one of the following types
  + bool
  + int8_t
  + uint8_t
  + int16_t
  + uint16_t
  + int32_t
  + uint32_t
  + int64_t
  + uint64_t
  + float
  + double
- *Category*: String, could be
  + datatype: data type or constructor
  + math: arithmetic operations
  + bitop: bit operations
  + cmp: compare
  + load: load operation
  + store: store operation
  + cast: cast operation
  + other: other operations
- *Description*: String, detail description

Example
```json
{ name: "svec< 4, bool >",
  Lane:4, 
  Type: "bool",
  Category: "datatype",
  Description: "Data representation and operations on a vector of 4 boolean values. This is used in predicated vector operations. Specifically the ith value of svec<4,bool> indicates whether the ith lane of a predicated vector operation is enabled or not"
}
```