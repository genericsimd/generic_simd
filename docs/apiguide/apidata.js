apidata = [
{ name: "svec< 4, bool >",
  Lane:4, 
  Type: "bool",
  Category: "datatype",
  Description: "Data representation and operations on a vector of 4 boolean values. This is used in predicated vector operations. Specifically the ith value of svec<4,bool> indicates whether the ith lane of a predicated vector operation is enabled or not"
},
{ name: "svec< 4, bool >::svec()",
  Lane:4, 
  Type: "bool",
  Category: "datatype",
  Description: "Default constructor.<br>Return a vector of 4 undefined values"
},
{ name: "svec< 4, bool >::svec(uint32_t a)",
  Lane:4, 
  Type: "bool",
  Category: "datatype",
  Description: "Constructor.<br> Return a vector of 4 mask/booleans: {a,a,a,a}.<br><b>Note:</b>a must be either 0 or -1."
},
{ name: "svec< 4, bool >::svec(uint32_t a, uint32_t b, uint32_t c, uint32_t d)",
  Lane:4, 
  Type: "bool",
  Category: "datatype",
  Description: "Constructor.<br> Return a vector of a vector of 4 mask/booleans: {a,b,c,d}.<br><b>Note:</b>a,b,c,d must be either 0 or -1."
},
{ name: "svec< 4, bool >::operator[](int index)",
  Lane:4, 
  Type: "bool",
  Category: "other",
  Description: "Set or get the vector element specified by index.",
  Example: "svec<4,bool> mask(0,-1,-1,0);<br>bool a = mask[0];//a is false<br>mask[2] = 0; //mask is now{0,-1,0,0}"
},
{ name: "svec< 4, bool > svec< 4, bool >::operator==(svec<4,bool> a)",
  Lane:4, 
  Type: "bool",
  Category: "cmp",
  Description: "Element-wise compare equal. Return a bool vector.",
  Example: "a == b"
},
{ name: "svec< 4, bool > svec< 4, bool >::operator!=(svec<4,bool> a)",
  Lane:4, 
  Type: "bool",
  Category: "cmp",
  Description: "Element-wise compare not equal. Return a bool vector",
  Example: "a != b"
},
];