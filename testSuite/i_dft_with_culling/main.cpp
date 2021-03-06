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
//
//   SUMMARY: 
//     USAGE:
//
// ORIG-DATE: 13-Apr-04 at 10:57:52
//  LAST-MOD:  9-Jun-04 at 15:33:59 by Thomas Leurent
//
//
// DESCRIPTION:
// ============
/*! \file main.cpp

describe main.cpp here

 */
// DESCRIP-END.
//

#ifdef MSQ_USE_OLD_IO_HEADERS
#  include <iostream.h>
#else
#  include <iostream>
   using std::cout;
   using std::endl;
#endif

#include <stdlib.h>


#include "Mesquite.hpp"
#include "MeshImpl.hpp"
#include "MsqError.hpp"
#include "InstructionQueue.hpp"
#include "TerminationCriterion.hpp"
#include "QualityAssessor.hpp"
#include "PlanarDomain.hpp"

// algorithms
#include "I_DFT.hpp"
#include "I_DFT_NoBarrier.hpp"

#include "sI_DFT.hpp"
#include "RI_DFT.hpp"
#include "sRI_DFT.hpp"
#include "ConcreteTargetCalculators.hpp"
#include "LPtoPTemplate.hpp"
#include "FeasibleNewton.hpp"
#include "ConjugateGradient.hpp"
using namespace Mesquite;

int main()
{
  Mesquite::MsqPrintError err(cout);

  Mesquite::MeshImpl *ini_mesh = new Mesquite::MeshImpl;
  ini_mesh->read_vtk("../../meshFiles/2D/VTK/equil_tri2.vtk", err);

  Vector3D pnt(0,0,0);
  Vector3D s_norm(0,0,1);
  PlanarDomain* msq_geom = new PlanarDomain(s_norm, pnt);

  // creates an intruction queue
  InstructionQueue queue1;

  // creates a DFT measure ...
  I_DFT mu;
//  mu.set_gradient_type(QualityMetric::NUMERICAL_GRADIENT);
//   mu.set_hessian_type(QualityMetric::NUMERICAL_HESSIAN);
   mu.set_averaging_method(QualityMetric::LINEAR, err); 
  if (err) return 1;
 
  Mesquite::MeshImpl *ref_mesh = new Mesquite::MeshImpl;
  ref_mesh->read_vtk("../../meshFiles/2D/VTK/equil_tri2.vtk", err);
  DeformingDomainGuides843 target( ref_mesh, msq_geom );
  queue1.add_target_calculator( &target, err );
  if (err) return 1;
 
  // ... and builds an objective function with it
  LPtoPTemplate* obj_func = new LPtoPTemplate(&mu, 1, err);
  obj_func->set_gradient_type(ObjectiveFunction::ANALYTICAL_GRADIENT);

  // creates the steepest descentfeas newt optimization procedures
  //FeasibleNewton* pass1 = new FeasibleNewton( obj_func );
  ConjugateGradient* pass1 = new ConjugateGradient( obj_func, err );
  pass1->set_patch_type(PatchData::ELEMENTS_ON_VERTEX_PATCH, err,1,1);

  // **************Set stopping criterion****************
  TerminationCriterion tc_inner;
  tc_inner.add_criterion_type_with_int(TerminationCriterion::NUMBER_OF_ITERATES,1,err);
    //tc_inner.add_criterion_type_with_double(TerminationCriterion::VERTEX_MOVEMENT_ABSOLUTE,.001,err);
  tc_inner.set_culling_type(TerminationCriterion::VERTEX_MOVEMENT_ABSOLUTE,1,err);
  pass1->set_inner_termination_criterion(&tc_inner);
   
  TerminationCriterion tc_outer;
  tc_outer.add_criterion_type_with_int(TerminationCriterion::NUMBER_OF_ITERATES,50,err);
  pass1->set_outer_termination_criterion(&tc_outer);

  // adds 1 pass of pass1 to mesh_set1
  queue1.set_master_quality_improver(pass1, err); 
  if (err) return 1;

  // launches optimization on Lagrange mesh
  queue1.run_instructions(ini_mesh, msq_geom, err); 
  if (err) return 1;
 
  ini_mesh->write_vtk("smo_mesh.vtk",err);
  if (err) return 1;
    //ini_mesh->write_exodus("smo_mesh.exo", err); 
    //if (err) return 1;
 
  print_timing_diagnostics(cout);
  return 0;
 
}
