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
/*! \file EdgeLengthQualityMetric.cpp
  \author Michael Brewer
  \date 2002-05-14
  Evaluates the lengths of the edges attached to the given vertex.
  By default, the averaging method is set to SUM.
*/


#include "EdgeLengthQualityMetric.hpp"
#include <math.h>
#include <list>
#include "Vector3D.hpp"
#include "QualityMetric.hpp"
#include "MsqVertex.hpp"
#include "PatchData.hpp"
#include "MsqMessage.hpp"
using namespace Mesquite;

MSQ_USE(vector);

#undef __FUNC__
#define __FUNC__ "EdgeLengthQualityMetric::evaluate_node"

bool EdgeLengthQualityMetric::evaluate_vertex(PatchData &pd, MsqVertex* vert,
                                             double &fval, MsqError &err)
{
  fval=0.0;
  size_t this_vert = pd.get_vertex_index(vert);
  size_t other_vert;
  vector<size_t> adj_verts;
  Vector3D edg;
  pd.get_adjacent_vertex_indices(this_vert,adj_verts,err);
  int num_sample_points=adj_verts.size();
  double *metric_values=new double[num_sample_points];
  MsqVertex* verts = pd.get_vertex_array(err);
  int point_counter=0;
  while(!adj_verts.empty()){
    other_vert=adj_verts.back();
    adj_verts.pop_back();
    edg[0]=verts[this_vert][0]-verts[other_vert][0];
    edg[1]=verts[this_vert][1]-verts[other_vert][1];
    edg[2]=verts[this_vert][2]-verts[other_vert][2];    
    metric_values[point_counter]=edg.length();
    ++point_counter;
  }
  fval=average_metrics(metric_values,num_sample_points,err);
  delete[] metric_values;
  
  return true;
  
}

