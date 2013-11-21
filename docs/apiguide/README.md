# Generic SIMD API Guide

The Generic SIMD API Guide is a simple tool to search the data types and APIs of the Generic SIMD library.

The tool is a pure static html tool based on [filter.js framework](https://github.com/jiren/filter.js).

The tool uses one search box and three checkbox filters 
- Lane: filter API json object's Lane attribute
- Type: filter API json object's Type attribute
- category: filter API json object's Category attribute

The search box is a full text search of all the json object's text. So if you search "add", you should get the result containting "address".

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
- *Example*: String. Optional. Example code.

Example
```json
{ name: "svec< 4, bool >",
  Lane:4, 
  Type: "bool",
  Category: "datatype",
  Description: "Data representation and operations on a vector of 4 boolean values. This is used in predicated vector operations. Specifically the ith value of svec<4,bool> indicates whether the ith lane of a predicated vector operation is enabled or not",
  Example: "Sample code. Use <br> for line break"
}
```
