/* ***************************************************************** 
    MESQUITE -- The Mesh Quality Improvement Toolkit

    Copyright 2004 Sandia Corporation and Argonne National
    Laboratory.  Under the terms of Contract DE-AC04-94AL85000 
    with Sandia Corporation, the U.S. Government retains certain 
    rights in this software.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License 
    (lgpl.txt) along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    diachin2@llnl.gov, djmelan@sandia.gov, mbrewer@sandia.gov, 
    pknupp@sandia.gov, tleurent@mcs.anl.gov, tmunson@mcs.anl.gov      
   
  ***************************************************************** */
#include "ParameterSet.hpp"
#include <cstring>

using namespace Mesquite;

#undef __FUNC__
#define __FUNC__ "ParameterSet::ParameterSet" 
ParameterSet::ParameterSet() 
    : mParameterArray(NULL),
      mNumParameters(0)
{
}

#undef __FUNC__
#define __FUNC__ "ParameterSet::~ParameterSet" 
ParameterSet::~ParameterSet()
{
  while (mNumParameters--)
  {
    delete [] mParameterArray[mNumParameters].name;
      // If it's a string, free the string value
    if (mParameterArray[mNumParameters].type == MSQ_STRING &&
        mParameterArray[mNumParameters].value.strVal != NULL)
    {
      delete [] mParameterArray[mNumParameters].value.strVal;
    }
  }
  
  delete mParameterArray;
}

#undef __FUNC__
#define __FUNC__ "ParameterSet::get_parameter_index" 
size_t ParameterSet::get_parameter_index(const char* name, MsqError &err)
{
    // Search for a parameter with the same name
  size_t i = 0;
  while (i < mNumParameters && strcmp(mParameterArray[i].name, name))
    i++;
  
  return i;
}

#undef __FUNC__
#define __FUNC__ "ParameterSet::remove_parameter" 
void ParameterSet::remove_parameter(const char* name, MsqError &err)
{
    // Make sure it exists
  size_t index = get_parameter_index(name);
  if (index == mNumParameters)
    return MSQ_FAILURE;
  
    // Free the string holding the name
  delete [] mParameterArray[index].name;
  
    // If it's a string, free the string value
  if (mParameterArray[index].type == MSQ_STRING &&
      mParameterArray[index].value.strVal != NULL)
  {
    delete [] mParameterArray[index].value.strVal;
  }
  
    // If it isn't at the end, move the last parameter
    // into the spot this one was using
  if (index != mNumParameters - 1)
  {
    mParameterArray[index] = mParameterArray[mNumParameters - 1];
  }
  
  mNumParameters--;
  
  return MSQ_SUCCESS;
}

#undef __FUNC__
#define __FUNC__ "ParameterSet::generic_add_parameter" 
void ParameterSet::generic_add_parameter(const char* name, MsqError &err)
{
    // Make sure it doesn't already exist
  if (get_parameter_index(name) != mNumParameters) {
    err.errorOn=true; return; }
  
    // Make the array big enough
  ParameterRecord* new_array = new ParameterRecord[mNumParameters + 1];
    // Copy the old into the new
  memcpy(new_array, mParameterArray, mNumParameters*sizeof(ParameterRecord));
    // Toss the old
  delete [] mParameterArray;
    // Save the new
  mNumParameters++;
  mParameterArray = new_array;
  
    // Now add the new parameter, with no initial value
  mParameterArray[mNumParameters - 1].name = new char[strlen(name) + 1];
  strcpy(mParameterArray[mNumParameters - 1].name, name);
  
  return;
}

#undef __FUNC__
#define __FUNC__ "ParameterSet::add_int_parameter" 
void ParameterSet::add_int_parameter(const char* name,
                                           int initial_value, MsqError &err)
{
  generic_add_parameter(name, err);
  
  if (!err.errorOn)
  {
    mParameterArray[mNumParameters - 1].type = MSQ_INT;
    mParameterArray[mNumParameters - 1].value.intVal = initial_value;
  }
  
  return;
}

#undef __FUNC__
#define __FUNC__ "ParameterSet::set_int_parameter" 
void ParameterSet::set_int_parameter(const char* name,
                                           int value, MsqError &err)
{
    // Make sure it exists
  size_t index = get_parameter_index(name);
  if (index == mNumParameters)
    return MSQ_FAILURE;
  
    // Make sure it's the right type
  if (mParameterArray[index].type != MSQ_INT)
    return MSQ_FAILURE;
  
    // Set the value
  mParameterArray[index].value.intVal = value;
  
  return MSQ_SUCCESS;
}

#undef __FUNC__
#define __FUNC__ "ParameterSet::get_int_parameter" 
void ParameterSet::get_int_parameter(const char* name,
                                     int* value, MsqError &err)
{
    // Make sure it exists
  size_t index = get_parameter_index(name);
  if (index == mNumParameters) {
    err.set_msg("parameter name undefined");
    return;
  }
  
    // Make sure it's the right type
  if (mParameterArray[index].type != MSQ_INT) {
    err.set_msg("wrong parameter type");
    return;
  }
  
    // Get the value
  *value = *reinterpret_cast<int*>(&(mParameterArray[index].value));
  
  return;
}
