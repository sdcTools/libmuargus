// %module MuArgusCtrl
// directors="1" needed for being able to use the director feature
%module(directors="1") MuArgusCtrl

%include <typemaps.i>
%include <windows.i>
%include <arrays_java.i>
%include <various.i>
%include <std_string.i>

// By default treat method parameters of the following types as simple output variables in Java
// Arrays of size 1 are generated, so output can be retrieved (we can't pass the address of a variable in Java)
%apply bool *OUTPUT { bool * };
%apply long *OUTPUT { long * };
%apply double *OUTPUT { double * };
%apply char **STRING_OUT { char ** };

// Include this block in the generated C++ wrapper code (TauArgus_wrap.cpp)
%{
#include "IProgressListener.h"
#include "MuArgCtrl.h"
%}

%feature("director") IProgressListener;

// Turn on director wrapping IProgressListener. This makes it possible to extend this class in Java
// Generate wrapper code for the classes in these header files, so they can be called from Java
%include "IProgressListener.h"
%include "MuArgCtrl.h"
