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

/*! \file TargetCalculator.cpp

\brief The Mesquite::TargetCalculator class is the base class. Concrete classes are 
instantiated by the user, and often implemented by the user to give 
mesquite a measure of the perfect mesh. 

\author Thomas Leurent
\date   2004-04-30
*/


#include "TargetCalculator.hpp"
#include "PatchDataUser.hpp"
#include "MeshSet.hpp"
#include "MsqTimer.hpp"

using namespace Mesquite;

void TargetCalculator::reset_reference_meshset(MsqError &err)
{
  if (refMesh)
    refMesh->reset(err);
  MSQ_CHKERR(err);
} 


void TargetCalculator::compute_target_matrices_and_check_det(PatchData &pd, MsqError &err)
{
  MSQ_FUNCTION_TIMER( "TargetCalculator::compute_target_matrices_and_check_det" );

  // Compute the target matrices
  compute_target_matrices(pd, err); MSQ_ERRRTN(err);

  //checks that the determinant of each target matrix is positive.
  MsqMeshEntity* elems=pd.get_element_array(err); MSQ_ERRRTN(err);
  size_t num_elements=pd.num_elements();
  for (size_t i=0; i<num_elements; ++i) {
    MsqTag* tag = elems[i].get_tag();
    size_t num_corners = elems[i].vertex_count();
    for (size_t j=0; j<num_corners; ++j) {    
      if ( det(tag->target_matrix(j)) <= 0 ) {
        MSQ_SETERR(err)("A Target matrix has a non-positive determinant. "
                        "Please review your target calculator.",
                        MsqError::INVALID_STATE);
        return;
      }
    }
  }
}

  
void TargetCalculator::compute_default_target_matrices(PatchData &pd,
                                                       MsqError &err)
{
  MSQ_FUNCTION_TIMER( "TargetCalculator::compute_default_target_matrices" );
    
  // set on each element in the patch a tag containing an array of corner matrices
  // (the size of the array is adequate for each element, e.g. 4 for a quad).
  pd.allocate_target_matrices(err); MSQ_ERRRTN(err);
    
  MsqMeshEntity* elems=pd.get_element_array(err); MSQ_ERRRTN(err);
  size_t num_elements=pd.num_elements();

  Matrix3D tmp_tri, tmp_quad, tmp_tet, tmp_hex;
  initialize_default_target_matrices(tmp_tri, tmp_quad, tmp_tet, tmp_hex);
  
  // set the corner matrices to the correct value for each tag.
  for (size_t i=0; i<num_elements; ++i) {

    MsqTag* tag = elems[i].get_tag(); 
      
    EntityTopology type = elems[i].get_element_type();
    switch (type)
      {
      case TRIANGLE:
        tag->target_matrix(0) = tmp_tri; 
        tag->target_matrix(1) = tmp_tri; 
        tag->target_matrix(2) = tmp_tri; 
        break;
      case QUADRILATERAL:
        tag->target_matrix(0) = tmp_quad; 
        tag->target_matrix(1) = tmp_quad; 
        tag->target_matrix(2) = tmp_quad; 
        tag->target_matrix(3) = tmp_quad; 
        break;
      case TETRAHEDRON:
        tag->target_matrix(0) = tmp_tet; 
        tag->target_matrix(1) = tmp_tet; 
        tag->target_matrix(2) = tmp_tet; 
        tag->target_matrix(3) = tmp_tet; 
        break;
      case HEXAHEDRON:
        tag->target_matrix(0) = tmp_hex; 
        tag->target_matrix(1) = tmp_hex; 
        tag->target_matrix(2) = tmp_hex; 
        tag->target_matrix(3) = tmp_hex; 
        tag->target_matrix(4) = tmp_hex; 
        tag->target_matrix(5) = tmp_hex; 
        tag->target_matrix(6) = tmp_hex; 
        tag->target_matrix(7) = tmp_hex; 
        break;
      default:
        MSQ_SETERR(err)("Type not implemented.",MsqError::NOT_IMPLEMENTED);
        return;
      } //end switch
  } // end loop
}

  
void TargetCalculator::compute_reference_corner_matrices(PatchData &pd,
                                                         MsqError &err)
{
  MSQ_FUNCTION_TIMER( "TargetCalculator::compute_reference_corner_matrices" );

  PatchData ref_pd;
  if (refMesh)
    refMesh->get_next_patch(ref_pd, *originator, err); 
  else 
    pd.get_reference_mesh( ref_pd, err );
  MSQ_ERRRTN(err);
  
  // Make sure topology of ref_pd and pd are equal
  size_t num_elements=pd.num_elements();
  assert( num_elements == ref_pd.num_elements() );
  size_t num_vertices=pd.num_vertices();
  assert( num_vertices == ref_pd.num_vertices() );
    
  // Compute corner matrices for ref_pd and store in pd tags.
  MsqMeshEntity* elems = pd.get_element_array(err);
  MsqMeshEntity* elems_ref = ref_pd.get_element_array(err); MSQ_ERRRTN(err);
  TargetMatrix A[MSQ_MAX_NUM_VERT_PER_ENT];
  pd.allocate_target_matrices(err); MSQ_ERRRTN(err);
  for (size_t i=0; i<num_elements; ++i) {
    MsqTag* tag = elems[i].get_tag();
    int nve = elems[i].vertex_count();
    assert( nve = elems_ref[i].vertex_count() );
    elems_ref[i].compute_corner_matrices(ref_pd, A, nve, err); MSQ_ERRRTN(err);
    for (int j=0; j<nve; ++j) {
      tag->target_matrix(j) = A[j];
    }
  }
}


 void TargetCalculator::compute_guide_matrices(enum guide_type type, PatchData &ref_pd, size_t elem_ind,
                                           Matrix3D W_k[], int num, MsqError &err)
  {

    
    MsqMeshEntity* elems = ref_pd.get_element_array(err); MSQ_ERRRTN(err);
    size_t nve = elems[elem_ind].vertex_count();
    int i;

    switch(type) {
    case Ad:
      {
        Matrix3D tmp_tri, tmp_quad, tmp_tet, tmp_hex;
        initialize_default_target_matrices(tmp_tri, tmp_quad, tmp_tet, tmp_hex);
        EntityTopology elem_type = elems[elem_ind].get_element_type();
        switch (elem_type) {
        case TRIANGLE:
          for (i=0; i<3; ++i) W_k[i] = tmp_tri; 
          break;
        case QUADRILATERAL:
          for (i=0; i<4; ++i) W_k[i] = tmp_quad; 
          break;
        case TETRAHEDRON:
          for (i=0; i<4; ++i) W_k[i] = tmp_tet; 
          break;
        case HEXAHEDRON:
          for (i=0; i<8; ++i) W_k[i] = tmp_hex; 
          break;
        default:
          MSQ_SETERR(err)("Element type not implemented.",MsqError::NOT_IMPLEMENTED);
          return;
        }
      }
      return;
    case A0:
      elems[elem_ind].compute_corner_matrices(ref_pd, W_k, nve, err); MSQ_ERRRTN(err);
      return;
    case Ar:
      elems[elem_ind].compute_corner_matrices(ref_pd, W_k, nve, err); MSQ_ERRRTN(err);
      return;
    default:
      MSQ_SETERR(err)("Element type not implemented.",MsqError::NOT_IMPLEMENTED);
      return;
    }
    
  }

  //!
  Matrix3D TargetCalculator::compute_V_3D(const Matrix3D &A, MsqError &err)
  {
    Vector3D a1(A[0][0], A[1][0], A[2][0]); 
    Vector3D a2(A[0][1], A[1][1], A[2][1]); 
    Vector3D a3(A[0][2], A[1][2], A[2][2]); 

    double a1_norm = A.column_length(0); 
    Vector3D a1_x_a2 = a1 * a2;
    double a1_x_a2_norm = a1_x_a2.length(); 
    
    Matrix3D V;
    Vector3D v1, v2, v3;
    
    double a1_norm_inv = 1./a1_norm;
    double a1_x_a2_norm_inv = 1./a1_x_a2_norm;
    if (!finite(a1_norm_inv) || !finite(a1_x_a2_norm_inv)) {
      MSQ_SETERR(err)("Numerical error", MsqError::INVALID_STATE);
    }
    
    // note : % is the dot product
    v1 = a1_norm_inv * a1;
    v2 = ((-(a1%a2) * a1) + (a1_norm*a1_norm)*a2) * a1_norm_inv * a1_x_a2_norm_inv;
    v3 = a1_x_a2_norm_inv * a1_x_a2;

    V.set_column(0, v1);
    V.set_column(1, v2);
    V.set_column(2, v3);

    return V;
  }
