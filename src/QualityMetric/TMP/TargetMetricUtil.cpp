/* ***************************************************************** 
    MESQUITE -- The Mesh Quality Improvement Toolkit

    Copyright 2007 Sandia National Laboratories.  Developed at the
    University of Wisconsin--Madison under SNL contract number
    624796.  The U.S. Government and the University of Wisconsin
    retain certain rights to this software.

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

    (2007) kraftche@cae.wisc.edu    

  ***************************************************************** */


/** \file TargetMetricUtil.cpp
 *  \brief 
 *  \author Jason Kraftcheck 
 */

#include "Mesquite.hpp"
#include "TargetMetricUtil.hpp"
#include "MsqMatrix.hpp"
#include "SamplePoints.hpp"
#include "PatchData.hpp"
#include "ElementQM.hpp"
#include "ElemSampleQM.hpp"

namespace Mesquite {


void surface_to_2d( const MsqMatrix<3,2>& App,
                    const MsqMatrix<3,2>& Wp,
                    MsqMatrix<2,2>& A,
                    MsqMatrix<2,2>& W )
{
  MsqMatrix<3,1> Wp1 = Wp.column(0);
  MsqMatrix<3,1> Wp2 = Wp.column(1);
  MsqMatrix<3,1> nwp = Wp1 * Wp2;
  nwp *= 1.0/length(nwp);

  MsqMatrix<3,1> z[2];
  z[0] = Wp1 * (1.0 / length( Wp1 ));
  z[1] = nwp * z[0];
  MsqMatrix<3,2> Z(z);
  W = transpose(Z) * Wp;

  MsqMatrix<3,1> npp = App.column(0) * App.column(1);
  npp *= 1.0 / length(npp);
  double dot = npp % nwp;
  MsqMatrix<3,1> nr = (dot >= 0.0) ? nwp : -nwp;
  MsqMatrix<3,1> v = nr * npp;
  double vlen = length(v);
  if (vlen > DBL_EPSILON) {
    v *= 1.0 / length(v);
    MsqMatrix<3,1> r1[3] = { v, npp, v * npp }, r2[3] = { v, nr, v * nr };
    MsqMatrix<3,3> R1( r1 ), R2( r2 );
    MsqMatrix<3,3> RT = R2 * transpose(R1);
    MsqMatrix<3,2> Ap = RT * App;
    A = transpose(Z) * Ap;
  }
  else {
    A = transpose(Z) * App;
  }
}

void get_sample_pt_evaluations( PatchData& pd,
                                const SamplePoints* pts,
                                msq_std::vector<size_t>& handles,
                                bool free,
                                MsqError& err )
{
  handles.clear();
  msq_std::vector<size_t> elems;
  ElementQM::get_element_evaluations( pd, elems, free, err ); MSQ_ERRRTN(err);
  for (msq_std::vector<size_t>::iterator i = elems.begin(); i != elems.end(); ++i)
  {
    EntityTopology type = pd.element_by_index( *i ).get_element_type();
    int num_samples = pts->num_sample_points( type );
    for (int j = 0; j < num_samples; ++j)
      handles.push_back( ElemSampleQM::handle(j, *i) );
  }
}
                   
void get_elem_sample_points( PatchData& pd,
                             const SamplePoints* pts,
                             size_t elem,
                             msq_std::vector<size_t>& handles,
                             MsqError& err )
{
  EntityTopology type = pd.element_by_index( elem ).get_element_type();
  int num_samples = pts->num_sample_points( type );
  handles.resize( num_samples );
  for (int j = 0; j < num_samples; ++j)
    handles[j] = ElemSampleQM::handle(j, elem);
}

} // namespace Mesquite