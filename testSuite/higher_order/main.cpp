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
// -*- Mode : c++; tab-width: 3; c-tab-always-indent: t; indent-tabs-mode: nil; c-basic-offset: 3 -*-
//
//   SUMMARY: 
//     USAGE:
//
// ORIG-DATE: 19-Feb-02 at 10:57:52
//  LAST-MOD: 23-Jul-03 at 18:09:51 by Thomas Leurent
//
//
// DESCRIPTION:
// ============
/*! \file main.cpp

describe main.cpp here

 */
// DESCRIP-END.
//
#ifndef MSQ_USE_OLD_IO_HEADERS
#include <iostream>
using std::cout;
using std::endl;
#else
#include <iostream.h>
#endif

#ifndef MSQ_USE_OLD_C_HEADERS
#include <cstdlib>
#else
#include <stdlib.h>
#endif


#include "Mesquite.hpp"
#include "MeshImpl.hpp"
#include "MsqError.hpp"
#include "Vector3D.hpp"
#include "InstructionQueue.hpp"
#include "PatchData.hpp"
#include "TerminationCriterion.hpp"
#include "QualityAssessor.hpp"

// algorythms
#include "IdealWeightInverseMeanRatio.hpp"
#include "ConditionNumberQualityMetric.hpp"
#include "LPtoPTemplate.hpp"
#include "LInfTemplate.hpp"
#include "FeasibleNewton.hpp"
#include "ConjugateGradient.hpp"

#include "PlanarDomain.hpp"
using namespace Mesquite;

/* This is the input mesh topology
     (0)------(16)-----(1)------(17)-----(2)------(18)-----(3)
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
     (19)      0       (20)      1       (21)      2       (22)
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
     (4)------(23)-----(5)------(24)-----(6)------(25)-----(7)
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
     (26)      3       (27)      4       (28)      5       (29)
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
     (8)------(30)-----(9)------(31)-----(10)-----(32)-----(11)
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
     (33)      6       (34)      7       (35)      8       (36)
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
      |                 |                 |                 |
     (12)-----(37)-----(13)-----(38)-----(14)-----(39)-----(15)
*/


const char LINEAR_INPUT_FILE_NAME[]       = "linear_input.vtk";
const char QUADRATIC_INPUT_FILE_NAME[]    = "quadratic_input.vtk";
const char EXPECTED_LINAR_FILE_NAME[]     = "expected_linear_output.vtk";
const char EXPECTED_QUADRATIC_FILE_NAME[] = "expected_quadratic_output.vtk";
const char OUTPUT_FILE_NAME[]             = "smoothed_qudratic_mesh.vtk";
const unsigned NUM_CORNER_VERTICES = 16;
const unsigned NUM_MID_NODES = 24;
const double SPATIAL_COMPARE_TOLERANCE = 1e-6;

void compare_nodes( size_t start_index,
                    size_t end_index,
                    Mesh* mesh1,
                    Mesh* mesh2,
                    MsqError& err )
{
  size_t i, num_verts = end_index - start_index;
  msq_std::vector<MsqVertex> verts1(num_verts), verts2(num_verts);
  msq_std::vector<Mesh::VertexHandle> handles1(num_verts), handles2(num_verts);

/* VertexIterator skips higher-order nodes.
   For now, just assume index == handle

  msq_std::vector<Mesh::VertexHandle>::iterator handle_iter1, handle_iter2;
  
    // Skip start_index vertices
  VertexIterator* iter1 = mesh1->vertex_iterator( err ); MSQ_ERRRTN(err);
  VertexIterator* iter2 = mesh2->vertex_iterator( err ); MSQ_ERRRTN(err);
  for (i = 0; i < start_index; ++i)
  {
    if (iter1->is_at_end())
    {
      MSQ_SETERR(err)("start index out of range for first mesh set", MsqError::INVALID_ARG);
      return;
    }
    if (iter2->is_at_end())
    {
      MSQ_SETERR(err)("start index out of range for second mesh set", MsqError::INVALID_ARG);
      return;
    }
    iter1->operator++();
    iter2->operator++();
  }
  
    // Get handles for vertices
  handle_iter1 = handles1.begin();
  handle_iter2 = handles2.begin();
  for (i = start_index; i < end_index; ++i)
  {
    if (iter1->is_at_end())
    {
      MSQ_SETERR(err)("end index out of range for first mesh set", MsqError::INVALID_ARG);
      return;
    }
    *handle_iter1 = iter1->operator*();
    iter1->operator++();
    ++handle_iter1;
    
    if (iter2->is_at_end())
    {
      MSQ_SETERR(err)("end index out of range for second mesh set", MsqError::INVALID_ARG);
      return;
    }
    *handle_iter2 = iter2->operator*();
    iter2->operator++();
    ++handle_iter2;
  }
*/
  for (i = start_index; i < end_index; ++i)
    handles1[i-start_index] = handles2[i-start_index] = (void*)i;
  
  
    // Get coordinates from handles
  mesh1->vertices_get_coordinates( &handles1[0], &verts1[0], num_verts, err );
  MSQ_ERRRTN(err);
  mesh2->vertices_get_coordinates( &handles2[0], &verts2[0], num_verts, err );
  MSQ_ERRRTN(err);
  
    // Compare coordinates
  for (i = 1; i <= num_verts; ++i)
  {
    const double diff = (verts1[i-1] - verts2[i-1]).length();
    if (diff > SPATIAL_COMPARE_TOLERANCE)
    {
      MSQ_SETERR(err)(MsqError::INTERNAL_ERROR, 
                      "%u%s vertices differ.",
                      (unsigned)i,
                      i%10 == 1 ? "st" :
                      i%10 == 2 ? "nd" :
                      i%10 == 3 ? "rd" : "th");
      return;
    }
  }
}
  
  // code copied from testSuite/algorithm_test/main.cpp
InstructionQueue* create_instruction_queue(MsqError& err)
{
  
    // creates an intruction queue
  InstructionQueue* queue1 = new InstructionQueue;

  // creates a mean ratio quality metric ...
  ShapeQualityMetric* mean = new IdealWeightInverseMeanRatio(err); MSQ_ERRZERO(err);
//   mean->set_gradient_type(QualityMetric::NUMERICAL_GRADIENT);
//   mean->set_hessian_type(QualityMetric::NUMERICAL_HESSIAN);
  mean->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
  mean->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
  
  LPtoPTemplate* obj_func = new LPtoPTemplate(mean, 1, err); MSQ_ERRZERO(err);
  obj_func->set_gradient_type(ObjectiveFunction::ANALYTICAL_GRADIENT);
    //obj_func->set_hessian_type(ObjectiveFunction::ANALYTICAL_HESSIAN);
  
  // creates the optimization procedures
//   ConjugateGradient* pass1 = new ConjugateGradient( obj_func, err );
  FeasibleNewton* pass1 = new FeasibleNewton( obj_func );

  //perform optimization globally
  pass1->set_patch_type(PatchData::GLOBAL_PATCH, err,1 ,1); MSQ_ERRZERO(err);
  
  //QualityAssessor* mean_qa = new QualityAssessor(mean,QualityAssessor::AVERAGE);

    //**************Set termination criterion****************

  //perform 1 pass of the outer loop (this line isn't essential as it is
  //the default behavior).
  TerminationCriterion* tc_outer = new TerminationCriterion;
  tc_outer->add_criterion_type_with_int(TerminationCriterion::NUMBER_OF_ITERATES, 1000, err);  MSQ_ERRZERO(err);
  pass1->set_outer_termination_criterion(tc_outer);
  
  //perform the inner loop until a certain objective function value is
  //reached.  The exact value needs to be determined (about 18095).
  //As a safety, also stop if the time exceeds 10 minutes (600 seconds).
  TerminationCriterion* tc_inner = new TerminationCriterion;
  tc_inner->add_criterion_type_with_double(TerminationCriterion::VERTEX_MOVEMENT_ABSOLUTE, 1e-6, err); MSQ_ERRZERO(err);
  pass1->set_inner_termination_criterion(tc_inner);
  
  // adds 1 pass of pass1 to mesh_set1
  //queue1->add_quality_assessor(mean_qa,err); MSQ_ERRZERO(err);
  queue1->set_master_quality_improver(pass1, err); MSQ_ERRZERO(err);
  //queue1->add_quality_assessor(mean_qa,err); MSQ_ERRZERO(err);

  return queue1;
}

int main()
{     
  MsqPrintError err(cout);
  
    // Create geometry
  Vector3D z(0,0,1), o(0,0,0);
  PlanarDomain geom(z,o);  
  
    // Read in linear input mesh
  cout << "Reading " << LINEAR_INPUT_FILE_NAME << endl;
  MeshImpl* linear_in = new MeshImpl;
  linear_in->read_vtk( LINEAR_INPUT_FILE_NAME, err );
  if (MSQ_CHKERR(err)) return 1;
  
    // Read in expected linear results
  cout << "Reading " << EXPECTED_LINAR_FILE_NAME << endl;
  MeshImpl* linear_ex = new MeshImpl;
  linear_ex->read_vtk( EXPECTED_LINAR_FILE_NAME, err );
  if (MSQ_CHKERR(err)) return 1;
  
    // Read in one copy of quadratic input mesh
  cout << "Reading " << QUADRATIC_INPUT_FILE_NAME << endl;
  MeshImpl* quadratic_in_1 = new MeshImpl;
  quadratic_in_1->read_vtk( QUADRATIC_INPUT_FILE_NAME, err );
  if (MSQ_CHKERR(err)) return 1;
 
    // Read in second copy of quadratic input mesh
  cout << "Reading " << QUADRATIC_INPUT_FILE_NAME << " again" << endl;
  MeshImpl* quadratic_in_2 = new MeshImpl;
  quadratic_in_2->read_vtk( QUADRATIC_INPUT_FILE_NAME, err );
  if (MSQ_CHKERR(err)) return 1;
  
    // Read in expected quadratic results
  cout << "Reading " << EXPECTED_QUADRATIC_FILE_NAME << endl;
  MeshImpl* quadratic_ex = new MeshImpl;
  quadratic_ex->read_vtk( EXPECTED_QUADRATIC_FILE_NAME, err );
  if (MSQ_CHKERR(err)) return 1;
  

    // Smooth linear mesh and check results
  cout << "Smoothing linear elements" << endl;
  InstructionQueue* q1 = create_instruction_queue( err );
  if (MSQ_CHKERR(err)) return 1;
  q1->run_instructions( linear_in, &geom, err ); 
  if (MSQ_CHKERR(err)) return 1;
  cout << "Checking results" << endl;
  compare_nodes( 0, NUM_CORNER_VERTICES, linear_in, linear_ex, err );
  if (MSQ_CHKERR(err)) return 1;
  delete q1;
  
    // Smooth only corner vertices of quadratic mesh and check results
  cout << "Smoothing quadratic elements as linear elements" << endl;
  InstructionQueue* q2 = create_instruction_queue( err );
  if (MSQ_CHKERR(err)) return 1;
  q2->disable_automatic_midnode_adjustment();
  q2->run_instructions( quadratic_in_1, &geom, err ); 
  if (MSQ_CHKERR(err)) return 1;
    // Make sure corner vertices are the same as in the linear case
  cout << "Checking results" << endl;
  compare_nodes( 0, NUM_CORNER_VERTICES, quadratic_in_1, linear_ex, err );
  if (MSQ_CHKERR(err)) return 1;
    // Make sure mid-side vertices are unchanged.
  compare_nodes( NUM_CORNER_VERTICES, NUM_CORNER_VERTICES + NUM_MID_NODES,
                quadratic_in_1, quadratic_in_2, err );
  if (MSQ_CHKERR(err)) return 1;
  delete q2;
  
    // Smooth corner vertices and adjust mid-side nodes
  cout << "Smoothing quadratic elements" << endl;
  InstructionQueue* q3 = create_instruction_queue( err );
  if (MSQ_CHKERR(err)) return 1;
  q3->enable_automatic_midnode_adjustment();
  q3->run_instructions( quadratic_in_2, &geom, err ); 
  if (MSQ_CHKERR(err)) return 1;
    // Make sure corner vertices are the same as in the linear case
  cout << "Checking results" << endl;
  compare_nodes( 0, NUM_CORNER_VERTICES, quadratic_in_2, linear_ex, err );
  if (MSQ_CHKERR(err)) return 1;
    // Make sure mid-side vertices are updated correctly
  compare_nodes( NUM_CORNER_VERTICES, NUM_CORNER_VERTICES + NUM_MID_NODES,
                quadratic_in_2, quadratic_ex, err );
  if (MSQ_CHKERR(err)) return 1;
  delete q3;
  
  quadratic_ex->write_vtk("smoothed_mesh.vtk", err); 
  if (MSQ_CHKERR(err)) return 1;
  return 0;
}
