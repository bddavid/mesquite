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
//    AUTHOR: Michael Brewer
//       ORG: Sandia National Labs
//    E-MAIL: mbrewer@sandia.gov
//
// ORIG-DATE: 03-Dec-02
//  LAST-MOD: 31-Mar-04 at 12:06:47 by Thomas Leurent
//
// DESCRIPTION:
// ============
/*! \file QualityMetricTest.cpp

Unit testing of various QualityMetrics primarily to test for
correct metric return values. 

\author Michael Brewer
\author Thomas Leurent

 */
// DESCRIP-END.
//

#include "Mesquite.hpp"
#include "PatchData.hpp"
#include "PatchDataInstances.hpp"
#include "QualityMetric.hpp"
#include "ConditionNumberQualityMetric.hpp"
#include "VertexConditionNumberQualityMetric.hpp"
#include "IdealWeightInverseMeanRatio.hpp"
#include "IdealWeightMeanRatio.hpp"
#include "AspectRatioGammaQualityMetric.hpp"
#include "MultiplyQualityMetric.hpp"
#include "EdgeLengthQualityMetric.hpp"
#include "UntangleBetaQualityMetric.hpp"
#include "cppunit/extensions/HelperMacros.h"
#include "SmoothnessQualityMetric.hpp"
#include "LocalSizeQualityMetric.hpp"
#include "I_DFT.hpp"
#include "I_DFT_WeakBarrier.hpp"
#include "I_DFT_NoBarrier.hpp"
#include "I_DFT_StrongBarrier.hpp"
#include "I_DFT_InverseMeanRatio.hpp"
#include "I_DFT_Generalized.hpp"
#include "PowerQualityMetric.hpp"
#include "ScalarAddQualityMetric.hpp"
#include "MeshImpl.hpp"
#include "PlanarDomain.hpp"
#include "ConcreteTargetCalculators.hpp"
#include <math.h>

using namespace Mesquite;

using std::cout;
using std::cerr;
using std::endl;

class QualityMetricTest : public CppUnit::TestFixture
{
private:  

  CPPUNIT_TEST_SUITE(QualityMetricTest);
    //Test condition number and generalized condition number metrics
  CPPUNIT_TEST (test_condition_number);
    //Test mean ratio and inverse mean ratio metrics
  CPPUNIT_TEST (test_mean_ratio);
    //Test apect ratio gamma (Tri's and Tet's only)
  CPPUNIT_TEST (test_aspect_ratio_gamma);
    //Test composite multiply
  CPPUNIT_TEST (test_composite_multiply);
  CPPUNIT_TEST (test_other_composites);
  
    //Test edge length metric
  CPPUNIT_TEST (test_edge_length_metric);  
    //Test averaging methods
  CPPUNIT_TEST (test_averaging_method);
    // test analytical gradients
  CPPUNIT_TEST (test_mean_ratio_tri_gradient_planar);
  CPPUNIT_TEST (test_mean_ratio_tri_gradient_nonplanar);
  CPPUNIT_TEST (test_mean_ratio_quad_gradient_planar);
  CPPUNIT_TEST (test_mean_ratio_quad_gradient_nonplanar);
  CPPUNIT_TEST (test_mean_ratio_tet_gradient);
  CPPUNIT_TEST (test_mean_ratio_hex_gradient);
  CPPUNIT_TEST (test_mean_ratio_pyr_gradient);
  CPPUNIT_TEST (test_mean_ratio_wdg_gradient);
    // test analytical Hessians
  CPPUNIT_TEST (test_mean_ratio_tri_hessian);
  CPPUNIT_TEST (test_mean_ratio_quad_hessian);
  CPPUNIT_TEST (test_mean_ratio_quad_hessian_linear);
  CPPUNIT_TEST (test_mean_ratio_quad_hessian_sum_squared);
  CPPUNIT_TEST (test_mean_ratio_quad_hessian_rms);
  CPPUNIT_TEST (test_mean_ratio_quad_hessian_harmonic);
  CPPUNIT_TEST (test_mean_ratio_quad_hessian_hms);
  CPPUNIT_TEST (test_mean_ratio_tet_hessian);
  CPPUNIT_TEST (test_mean_ratio_pyr_hessian);
  CPPUNIT_TEST (test_mean_ratio_wdg_hessian);
  CPPUNIT_TEST (test_mean_ratio_hex_hessian);
  CPPUNIT_TEST (test_mean_ratio_hex_hessian_linear);
  CPPUNIT_TEST (test_mean_ratio_hex_hessian_sum_squared);
  CPPUNIT_TEST (test_mean_ratio_hex_hessian_rms);
  CPPUNIT_TEST (test_mean_ratio_hex_hessian_harmonic);
  CPPUNIT_TEST (test_mean_ratio_hex_hessian_hms);
    //Test local size metric
  CPPUNIT_TEST (test_local_size_metric);
    //Test local size metric
  CPPUNIT_TEST (test_vertex_based_condition_number_metric);
  // test element gradient coming directly from the gradient
  // versus the one coming from the hessian
  CPPUNIT_TEST (test_mean_ratio_tri_grad_from_hessian);
  CPPUNIT_TEST (test_mean_ratio_quad_grad_from_hessian);
  CPPUNIT_TEST (test_mean_ratio_tet_grad_from_hessian);
  CPPUNIT_TEST (test_mean_ratio_pyr_grad_from_hessian);
  CPPUNIT_TEST (test_mean_ratio_wdg_grad_from_hessian);
  CPPUNIT_TEST (test_mean_ratio_hex_grad_from_hessian);

    //I_DFT
  CPPUNIT_TEST (test_i_dft_tet_gradient);
  CPPUNIT_TEST (test_i_dft_pyr_gradient);
  CPPUNIT_TEST (test_i_dft_wdg_gradient);
  CPPUNIT_TEST (test_i_dft_hex_gradient);
  CPPUNIT_TEST (test_i_dft_tri_gradient);
  CPPUNIT_TEST (test_i_dft_quad_gradient);
  
  CPPUNIT_TEST (test_i_dft_no_barrier_tri_gradient);
  CPPUNIT_TEST (test_i_dft_no_barrier_quad_gradient);
  
  CPPUNIT_TEST (test_i_dft_imr_tet_gradient);
  CPPUNIT_TEST (test_i_dft_imr_pyr_gradient);
  CPPUNIT_TEST (test_i_dft_imr_wdg_gradient);
  CPPUNIT_TEST (test_i_dft_imr_hex_gradient);
  
  CPPUNIT_TEST (test_i_dft_weak_barrier_tet_gradient);
  CPPUNIT_TEST (test_i_dft_weak_barrier_pyr_gradient);
  CPPUNIT_TEST (test_i_dft_weak_barrier_wdg_gradient);
  CPPUNIT_TEST (test_i_dft_weak_barrier_hex_gradient);
  
  CPPUNIT_TEST (test_i_dft_nobarrier_tet_gradient);
  CPPUNIT_TEST (test_i_dft_nobarrier_pyr_gradient);
  CPPUNIT_TEST (test_i_dft_nobarrier_wdg_gradient);
  CPPUNIT_TEST (test_i_dft_nobarrier_hex_gradient);
  
  CPPUNIT_TEST (test_i_dft_strong_barrier_tet_gradient);
  CPPUNIT_TEST (test_i_dft_strong_barrier_pyr_gradient);
  CPPUNIT_TEST (test_i_dft_strong_barrier_wdg_gradient);
  CPPUNIT_TEST (test_i_dft_strong_barrier_hex_gradient);
  
  CPPUNIT_TEST (test_i_dft_tet_hessian);
  CPPUNIT_TEST (test_i_dft_pyr_hessian);
  CPPUNIT_TEST (test_i_dft_wdg_hessian);
  CPPUNIT_TEST (test_i_dft_hex_hessian);
  CPPUNIT_TEST (test_i_dft_tri_hessian);
  CPPUNIT_TEST (test_i_dft_quad_hessian);
  
  CPPUNIT_TEST (test_i_dft_no_barrier_tri_hessian);
  CPPUNIT_TEST (test_i_dft_no_barrier_quad_hessian);
  
  CPPUNIT_TEST (test_i_dft_weak_barrier_tet_hessian);
  CPPUNIT_TEST (test_i_dft_weak_barrier_pyr_hessian);
  CPPUNIT_TEST (test_i_dft_weak_barrier_wdg_hessian);
  CPPUNIT_TEST (test_i_dft_weak_barrier_hex_hessian);
  
  CPPUNIT_TEST (test_i_dft_nobarrier_tet_hessian);
  CPPUNIT_TEST (test_i_dft_nobarrier_pyr_hessian);
  CPPUNIT_TEST (test_i_dft_nobarrier_wdg_hessian);
  CPPUNIT_TEST (test_i_dft_nobarrier_hex_hessian);
  
  CPPUNIT_TEST (test_i_dft_strong_barrier_tet_hessian);
  CPPUNIT_TEST (test_i_dft_strong_barrier_pyr_hessian);
  CPPUNIT_TEST (test_i_dft_strong_barrier_wdg_hessian);
  CPPUNIT_TEST (test_i_dft_strong_barrier_hex_hessian);
  
  CPPUNIT_TEST (test_i_dft_imr_tet_hessian);
  CPPUNIT_TEST (test_i_dft_imr_pyr_hessian);
  CPPUNIT_TEST (test_i_dft_imr_wdg_hessian);
  CPPUNIT_TEST (test_i_dft_imr_hex_hessian);
  
  CPPUNIT_TEST (test_i_dft_tet_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_pyr_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_wdg_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_quad_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_tri_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_hex_grad_from_hessian);
  
  CPPUNIT_TEST (test_i_dft_no_barrier_quad_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_no_barrier_tri_grad_from_hessian);
  
  CPPUNIT_TEST (test_i_dft_imr_tet_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_imr_pyr_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_imr_wdg_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_imr_hex_grad_from_hessian);
  
  CPPUNIT_TEST (test_i_dft_weak_barrier_tet_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_weak_barrier_pyr_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_weak_barrier_wdg_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_weak_barrier_hex_grad_from_hessian);
  
  CPPUNIT_TEST (test_i_dft_strong_barrier_tet_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_strong_barrier_pyr_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_strong_barrier_wdg_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_strong_barrier_hex_grad_from_hessian);
  
  CPPUNIT_TEST (test_i_dft_nobarrier_tet_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_nobarrier_pyr_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_nobarrier_wdg_grad_from_hessian);
  CPPUNIT_TEST (test_i_dft_nobarrier_hex_grad_from_hessian);

  CPPUNIT_TEST (test_untangle_metric);

  CPPUNIT_TEST_SUITE_END();
  
private:
  
  PatchData triPatch;
  PatchData refTriPatch;
  PatchData refQuadPatch;
  PatchData quadPatch;
  PatchData refTetPatch;
  PatchData tetPatch;
  PatchData pyrPatch;
  PatchData wdgPatch;
  PatchData refPyrPatch;
  PatchData refWdgPatch;
  PatchData hexPatch;
  PatchData refHexPatch;
  PatchData invertedTri;
  PatchData invertedTet;
  PatchData idealTri;
  PatchData idealTet;
  PatchData idealPyr;
  PatchData idealWdg;
    //Tol used for double comparisons
  double qualTol;
  int pF;//PRINT_FLAG
public:
  void setUp()
  {
      //pF=1;//PRINT_FLAG IS ON
    pF=0;//PRINT_FLAG IS OFF
    MsqPrintError err(cout); 
    
    qualTol = 1.e-12;
    
    DeformingDomainGuides843 guide_843(NULL);

     /* Our triangular patch is made of two tris.  tri_1 is a perfect
        equilateral (the ideal for most metrics).  tri_2 is an arbitrary
        triangle.
     */
    create_qm_two_tri_patch_with_domain(triPatch, err);CPPUNIT_ASSERT(!err);
   
    create_qm_two_tri_patch_with_domain(refTriPatch, err);CPPUNIT_ASSERT(!err);

    guide_843.compute_target_matrices(triPatch, refTriPatch, err);
    CPPUNIT_ASSERT(!err);
    
 
     /* Our quad patch is made of two quads.  quad_1 is a perfect
        square (the ideal for most metrics).  quad_2 is an arbitrary
        quad.
     */
    create_qm_two_quad_patch_with_domain(quadPatch,err);CPPUNIT_ASSERT(!err);
    
    create_qm_two_quad_patch_with_domain(refQuadPatch,err);
    CPPUNIT_ASSERT(!err);
    
    guide_843.compute_target_matrices(quadPatch, refQuadPatch, err);
    CPPUNIT_ASSERT(!err);


     /* Our tet patch is made of two tets.  tet_1 is a perfect
        equilateral (the ideal for most metrics).  tet_2 is an arbitrary
        tet.
     */
    create_qm_two_tet_patch(tetPatch,err);CPPUNIT_ASSERT(!err);
    
    create_qm_two_tet_patch(refTetPatch,err);CPPUNIT_ASSERT(!err);
    
    guide_843.compute_target_matrices(tetPatch, refTetPatch, err);
    CPPUNIT_ASSERT(!err);
    
    /* pyramid patches */
    create_qm_two_pyr_patch(pyrPatch, err); CPPUNIT_ASSERT(!err);
    
    create_qm_two_pyr_patch(refPyrPatch, err); CPPUNIT_ASSERT(!err);

    guide_843.compute_target_matrices(pyrPatch, refPyrPatch, err);
    CPPUNIT_ASSERT(!err);
    
    /* wedge patches */
    create_qm_two_wdg_patch(wdgPatch, err); CPPUNIT_ASSERT(!err);
    
    create_qm_two_wdg_patch(refWdgPatch, err); CPPUNIT_ASSERT(!err);

    guide_843.compute_target_matrices(wdgPatch, refWdgPatch, err);
    CPPUNIT_ASSERT(!err);


     /* Our hex patch is made of two hexes.  hex_1 is a perfect
        unit cube (the ideal for most metrics).  hex_2 is an arbitrary
        hex.
     */
    create_qm_two_hex_patch(hexPatch,err);CPPUNIT_ASSERT(!err);

    create_qm_two_hex_patch(refHexPatch,err);CPPUNIT_ASSERT(!err);

    guide_843.compute_target_matrices(hexPatch, refHexPatch, err);
    CPPUNIT_ASSERT(!err);


       //'ideal' inverted tet
     create_one_inverted_tet_patch(invertedTet, err);CPPUNIT_ASSERT(!err);
       //ideal tri
     create_one_tri_patch(idealTri, err);CPPUNIT_ASSERT(!err);
       //ideal tet
     create_one_tet_patch(idealTet, err);CPPUNIT_ASSERT(!err);
      // ideal pyramid
     create_one_pyr_patch(idealPyr, err); CPPUNIT_ASSERT(!err);
      // ideal wedge
     create_one_wdg_patch(idealWdg, err); CPPUNIT_ASSERT(!err);
  }

  void tearDown()
  {
    destroy_patch_with_domain(triPatch);
    destroy_patch_with_domain(quadPatch);
    destroy_patch_with_domain(refTriPatch);
    destroy_patch_with_domain(refQuadPatch);
    destroy_patch_with_domain(tetPatch);
    destroy_patch_with_domain(hexPatch);
    destroy_patch_with_domain(pyrPatch);
    destroy_patch_with_domain(wdgPatch);
    destroy_patch_with_domain(refTetPatch);
    destroy_patch_with_domain(refHexPatch);
    destroy_patch_with_domain(refPyrPatch);
    destroy_patch_with_domain(refWdgPatch);
  }
  
public:
  QualityMetricTest()
    {}

   void test_condition_number()
   {
     bool v_flag;
       //START WITH TRI's
     MsqPrintError err(cout);
     double val;
     MsqMeshEntity* elems;
     MsqVertex* verts = triPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems=triPatch.get_element_array(err);
     CPPUNIT_ASSERT(!err);
     ShapeQualityMetric *met = new ConditionNumberQualityMetric;
       //Check condition number of ideal tri
     v_flag=met->evaluate_element(triPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(v_flag==true);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     
       //SECOND: QUAD's
     verts = quadPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = quadPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check condition number of ideal quad
     v_flag=met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(v_flag==true);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     
       //THIRD TET's
     verts = tetPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = tetPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check condition number of ideal tet
     v_flag=met->evaluate_element(tetPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(v_flag==true);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);

       //FOURTH HEX's
     verts = hexPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = hexPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check condition number of ideal hex
     v_flag=met->evaluate_element(hexPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(v_flag==true);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     
      // PYRAMID
     verts = pyrPatch.get_vertex_array(err);  CPPUNIT_ASSERT(!err);
     elems = pyrPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
      // Check contition number of ideal pyramid
     v_flag = met->evaluate_element( pyrPatch, &elems[0], val, err ); 
     CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(v_flag == true);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val, 1.0, qualTol);
     
      // WEDGE
     verts = wdgPatch.get_vertex_array(err);  CPPUNIT_ASSERT(!err);
     elems = wdgPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
      // Check contition number of ideal prism
     v_flag = met->evaluate_element( wdgPatch, &elems[0], val, err ); 
     CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(v_flag == true);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val, 1.0, qualTol);

     delete met;
   }
    //******************** utility functions ***********************      


     // this function will change the number of free vertices so that
     // different parts of the i_dft compuation gets exercised
     // if which_case == 1, compare numerical and analytical gradients
     // if which_case == 2, compare numerical and analytical hessians
     // if which_case == 3, compare the anaylical gradients from the
     //                     gradient call and the Hessian call.
  void test_i_dft_fix_vertices(PatchData &this_patch,
                               QualityMetric *i_dft_metric,
                               int which_case)
  {
    MsqError err;
      //I_DFT i_dft_metric;
    if(which_case == 1)
      test_i_dft_gradient(this_patch, i_dft_metric);
    else if(which_case ==2)
      test_metric_hessian(this_patch, i_dft_metric);
    else{
      i_dft_metric->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
      i_dft_metric->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);      
      test_metric_grad_from_hessian(quadPatch,i_dft_metric,err);
    }
      //now do some things to test a single free vertex case
    
    MsqVertex* vertices=this_patch.get_vertex_array(err);
    size_t num_verts = this_patch.num_vertices();
    bool* flags = new bool[num_verts];
    CPPUNIT_ASSERT(!err);
    unsigned i;
    for(i=0; i<num_verts; ++i){
      if(vertices[i].is_free_vertex()){
        vertices[i].set_hard_fixed_flag();
        flags[i] = true;
      }
      else
        flags[i] =false;
    }
      //change each vertex to free... one at a time
    for(i=0; i<num_verts; ++i){
      vertices[i].remove_vertex_flag(MsqVertex::MSQ_HARD_FIXED);
      if(which_case == 1)
        test_i_dft_gradient(this_patch, i_dft_metric);
      else if (which_case == 2)
        test_metric_hessian(this_patch, i_dft_metric);
      else
        test_metric_grad_from_hessian(quadPatch,i_dft_metric,err);
      vertices[i].set_hard_fixed_flag();
    }
    for(i=0; i<num_verts; ++i){
      if(flags[i]){
        vertices[i].remove_vertex_flag(MsqVertex::MSQ_HARD_FIXED);
      }
    }   
      
    delete [] flags;
      
      
  }
  
    /*!       
      \param pd: this PatchData must have at least two elements.
  */
  void test_metric_grad_from_hessian(PatchData &pd, QualityMetric* this_metric,
                                     MsqError &err )
  {
    int i, max_nve = MSQ_MAX_NUM_VERT_PER_ENT;
    Vector3D* grad1 = new Vector3D[max_nve];
    Vector3D* grad2 = new Vector3D[max_nve];
    Matrix3D* hessian = new Matrix3D[max_nve*(max_nve+1)/2];
    double QM_val0, QM_val1, QM_val2;
    bool valid;
    
    MsqMeshEntity* elems = pd.get_element_array(err);CPPUNIT_ASSERT(!err);
    MsqVertex* vertices =  pd.get_vertex_array(err);CPPUNIT_ASSERT(!err);

    std::vector<size_t> elem_vtx_indices;
    elems[1].get_vertex_indices(elem_vtx_indices);
    int nve = elem_vtx_indices.size(); // number of vertices in element.
    MsqVertex** all_vtces = new MsqVertex*[nve];
    for (i=0; i<nve; ++i) {
      all_vtces[i] = &vertices[elem_vtx_indices[i]];
    }

    // 1 **** test with all vertices free
    // creates a mean ratio quality metric ...
    valid = this_metric->evaluate_element (pd, &elems[1], QM_val0, err);
    CPPUNIT_ASSERT(!err); CPPUNIT_ASSERT(valid);
    
    valid = this_metric->compute_element_gradient (pd, &elems[1],
                                                   all_vtces, grad1,
                                                   nve, QM_val1, err);
    CPPUNIT_ASSERT(!err); CPPUNIT_ASSERT(valid);
    CPPUNIT_ASSERT(finite(QM_val0));
    CPPUNIT_ASSERT(finite(QM_val1));
                   
    CPPUNIT_ASSERT_DOUBLES_EQUAL(QM_val0, QM_val1, 1e-12);
    
    valid = this_metric->compute_element_hessian(pd, &elems[1], all_vtces,
                                                 grad2, hessian,
                                                 nve, QM_val2,
                                                 err); CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(valid);
    CPPUNIT_ASSERT(finite(QM_val0));
    CPPUNIT_ASSERT(finite(QM_val1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(QM_val1, QM_val2, 1e-12);
    if(pF){
      std::cout << "\n\nGradient from compute_gradient()\n";
      for (i=0; i<nve; ++i)
        std::cout << grad1[i];

      std::cout << "\nGradient from compute_hessian()\n";
      for (i=0; i<nve; ++i)
        std::cout << grad2[i];
    }
    
    
    // test gradients
    for (i=0; i<nve; ++i){
      for (int j=0; j<3; ++j){
        CPPUNIT_ASSERT(finite(grad1[i][j]));
        CPPUNIT_ASSERT(finite(grad2[i][j]));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(grad1[i][j], grad2[i][j], 1e-12);
      }
    }
    delete []grad1;
    delete []grad2;
    delete []hessian;
    delete []all_vtces;
  }
      //for I_DFT test to make sure the numerical and analytical gradients
    // are equivalent
void test_i_dft_gradient(PatchData &pd, QualityMetric* this_metric)
  {
    MsqPrintError err(cout); 
	int i;
    Vector3D* grad_num = new Vector3D[2];
    Vector3D* grad_ana = new Vector3D[2];
    double metric_value;
    bool valid;
    size_t nfv = pd.num_free_vertices(err);
    if(nfv > 2)
       nfv =2;
    MsqMeshEntity* elems = pd.get_element_array(err);CPPUNIT_ASSERT(!err);
    MsqVertex* vertices =  pd.get_vertex_array(err);CPPUNIT_ASSERT(!err);

    std::vector<size_t> bad_elem_vertex_indices;
    elems[1].get_vertex_indices(bad_elem_vertex_indices);
    MsqVertex* two_vtces[2];
    two_vtces[0] = &vertices[bad_elem_vertex_indices[0]];
    two_vtces[1] = &vertices[bad_elem_vertex_indices[2]];
    
    // creates an I_DFT quality metric ...
    //QualityMetric* i_dft_metric = new I_DFT();
    CPPUNIT_ASSERT(!err);
    this_metric->set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);

    this_metric->set_gradient_type(QualityMetric::NUMERICAL_GRADIENT);
    valid = this_metric->compute_element_gradient (pd, &elems[1], two_vtces,
                                                    grad_num, nfv, metric_value,
                                                    err); CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(valid);
    if(pF){
      std::cout<<"\nI_DFT gradient test.\n";
      std::cout << "NUMERICAL GRADIENT\n";
      for (i=0; i<2; ++i)
        for (int j=0; j<3; ++j)
          std::cout << grad_num[i][j] << std::endl;
    }

    this_metric->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    valid = this_metric->compute_element_gradient (pd, &elems[1], two_vtces,
                                                    grad_ana, nfv, metric_value,
                                                    err); CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(valid);
    if(pF){
      std::cout << "ANALYTICAL GRADIENT\n";
      for (i=0; i<2; ++i)
        for (int j=0; j<3; ++j)
          std::cout << grad_ana[i][j] << std::endl;
    }
    for (i=0; i<2; ++i){
      for (int j=0; j<3; ++j){
        CPPUNIT_ASSERT(finite(grad_ana[i][j]));
        CPPUNIT_ASSERT(finite(grad_num[i][j]));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(grad_num[i][j], grad_ana[i][j], 0.001);
      }
    }
    

    // same test, but free vertices order differ from vertices order in element. 
    two_vtces[0] = &vertices[bad_elem_vertex_indices[2]];
    two_vtces[1] = &vertices[bad_elem_vertex_indices[0]];
    this_metric->set_gradient_type(QualityMetric::NUMERICAL_GRADIENT);
    valid = this_metric->compute_element_gradient (pd, &elems[1], two_vtces,
                                                    grad_num, 2, metric_value,
                                                    err); CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(valid);

    this_metric->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    valid = this_metric->compute_element_gradient (pd, &elems[1], two_vtces,
                                                    grad_ana, 2, metric_value,
                                                    err); CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(valid);

    for (i=0; i<2; ++i){
      for (int j=0; j<3; ++j){
        CPPUNIT_ASSERT(finite(grad_ana[i][j]));
        CPPUNIT_ASSERT(finite(grad_num[i][j]));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(grad_num[i][j], grad_ana[i][j], 0.001);
      }
    }
      //delete i_dft_metric;        
    delete []grad_num;
    delete []grad_ana;
  }
    //***************** end utility functions ************************
   void test_mean_ratio()
   {
       //START WITH TRI's
     MsqPrintError err(cout);
     bool valid;
     double val;
     MsqMeshEntity* elems;
     MsqVertex* verts = triPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems=triPatch.get_element_array(err); 
     CPPUNIT_ASSERT(!err);
     ShapeQualityMetric *met = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
     ShapeQualityMetric *imet = new IdealWeightMeanRatio;
       //Check mean ratio of ideal tri
     met->evaluate_element(triPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nMEAN TRI " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
       //Check inverse mean ratio of ideal tri (INVERSE)
     imet->evaluate_element(triPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nInv MEAN TRI " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
       //SECOND: QUAD's
     verts = quadPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = quadPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check mean ratio of ideal quad
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nMEAN QUAD " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
       //Check inverse mean ratio of ideal quad (INVERSE)
     imet->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nInv MEAN QUAD " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);

       //THIRD TET's
     verts = tetPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = tetPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check mean ratio of ideal tet
     met->evaluate_element(tetPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nMEAN TET " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);

       //Check inverse mean ratio of ideal tet (INVERSE)
     imet->evaluate_element(tetPatch,&elems[0],val,err); CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nInv MEAN TET " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);

       //FOURTH HEX's
     verts = hexPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = hexPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check mean ratio of ideal hex
     valid = met->evaluate_element(hexPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(valid==true);
     if(pF) cout << "\nMEAN HEX " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
       //Check inverse mean ratio of ideal hex (INVERSE)
     valid = imet->evaluate_element(hexPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(valid==true);
     if(pF) cout << "\nInv MEAN HEX " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);

       //test for pyramids
     verts = pyrPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = pyrPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check mean ratio of ideal pyramid
     valid = met->evaluate_element(pyrPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(valid==true);
     if(pF) cout << "\nMEAN PYR " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
       //Check inverse mean ratio of ideal hex (INVERSE)
     valid = imet->evaluate_element(pyrPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(valid==true);
     if(pF) cout << "\nInv MEAN HEX " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     
      // test prism
     verts = wdgPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = wdgPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check mean ratio of ideal pyramid
     valid = met->evaluate_element(wdgPatch,&elems[0],val,err);
     CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(valid==true);
     if(pF) cout << "\nMEAN WDG " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
       //Check inverse mean ratio of ideal hex (INVERSE)
     valid = imet->evaluate_element(wdgPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(valid==true);
     if(pF) cout << "\nInv MEAN HEX " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);

     delete met;
     delete imet;
   }
  
  void test_aspect_ratio_gamma()
   {
       //START WITH TRI's
     MsqPrintError err(cout); 
     double val;
     bool valid=false;
     MsqMeshEntity* elems;
     MsqVertex* verts = triPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems=triPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(!err);
     ShapeQualityMetric *met = new AspectRatioGammaQualityMetric;
       //Check aspect ratio gamma of ideal tri
     valid=met->evaluate_element(triPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     CPPUNIT_ASSERT(valid==true);
       //THIRD TET's
     verts = tetPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = tetPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check aspect ratio gamma of ideal tet
     valid=met->evaluate_element(tetPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT(valid==true);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     delete met;
   }

  void test_composite_multiply()
   {
       //START WITH TRI's
     MsqPrintError err(cout); 
     double val;
     MsqMeshEntity* elems;
     MsqVertex* verts = triPatch.get_vertex_array(err);CPPUNIT_ASSERT(!err);
     elems=triPatch.get_element_array(err);
     CPPUNIT_ASSERT(!err);
     ShapeQualityMetric *mmet = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
     ShapeQualityMetric *cmet = new ConditionNumberQualityMetric;
     MultiplyQualityMetric *met = new MultiplyQualityMetric(mmet,
                                                            cmet,
                                                            err);
       //Check ideal tri
     met->evaluate_element(triPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nMULT TRI " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
       //SECOND: QUAD's
     verts = quadPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = quadPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check ideal quad
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nMULT QUAD " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
       //THIRD TET's
     verts = tetPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = tetPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check ideal tet
     
     if(!met->evaluate_element(tetPatch,&elems[0],val,err)){
      cout << "\nMULT RETURNING FALSE FOR TET";
     }
     if(pF){
       cout << "\nMULT TET " << val;
       double tetm=0;
       if(!mmet->evaluate_element(tetPatch,&elems[0],tetm,err))
          cout << "\nMMET TET " << tetm;
       if(!cmet->evaluate_element(tetPatch,&elems[0],tetm,err))
          cout << "\nCMET TET " << tetm;
     }
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
       //FOURTH HEX's
     verts = hexPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = hexPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check ideal hex
     met->evaluate_element(hexPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nMULT HEX " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     delete met;
     delete mmet;
     delete cmet;
   }
  void test_other_composites()
     {
       MsqPrintError err(cout); 
       double val;
       double temp_val;
       MsqMeshEntity* elems;
       MsqVertex* verts = triPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
       elems=triPatch.get_element_array(err);
       CPPUNIT_ASSERT(!err);
         //vertex based
       SmoothnessQualityMetric *m1 = new EdgeLengthQualityMetric;
       PowerQualityMetric *pow_m1 = new PowerQualityMetric(m1,2,err); CPPUNIT_ASSERT(!err);
       ScalarAddQualityMetric *sa_m1 = new ScalarAddQualityMetric(m1,2,err); CPPUNIT_ASSERT(!err);
       m1->evaluate_vertex(triPatch,&verts[2],val,err);CPPUNIT_ASSERT(!err); 
       pow_m1->evaluate_vertex(triPatch,&verts[2],temp_val,err);
       CPPUNIT_ASSERT(!err);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(val*val,temp_val,qualTol);
       sa_m1->evaluate_vertex(triPatch,&verts[2],temp_val,err); CPPUNIT_ASSERT(!err);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(val+2.0,temp_val,qualTol);
         //element based
       ShapeQualityMetric *m2 = new ConditionNumberQualityMetric;
       PowerQualityMetric *pow_m2 = new PowerQualityMetric(m2,2,err); CPPUNIT_ASSERT(!err);
       ScalarAddQualityMetric *sa_m2 = new ScalarAddQualityMetric(m2,2,err); CPPUNIT_ASSERT(!err);
       m2->evaluate_element(triPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
       pow_m2->evaluate_element(triPatch,&elems[0],temp_val,err);
       CPPUNIT_ASSERT(!err);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(val*val,temp_val,qualTol);
       sa_m2->evaluate_element(triPatch,&elems[0],temp_val,err); CPPUNIT_ASSERT(!err);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(val+2.0,temp_val,qualTol);
         //element based with a negative power
       PowerQualityMetric *pow_mneg2 = new PowerQualityMetric(m2,-2,err); CPPUNIT_ASSERT(!err);
       m2->evaluate_element(triPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
       pow_mneg2->evaluate_element(triPatch,&elems[0],temp_val,err);
       CPPUNIT_ASSERT(!err);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0/(val*val),temp_val,qualTol);
       delete m1;
       delete m2;
       delete pow_m1;
       delete pow_m2;
       delete sa_m1;
       delete sa_m2;
       delete pow_mneg2;
       
     }
  

  void test_edge_length_metric()
   {
       //START WITH TRI's
     MsqPrintError err(cout); 
     double val;
     MsqMeshEntity* elems;
     MsqVertex* verts = triPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems=triPatch.get_element_array(err);
     CPPUNIT_ASSERT(!err);
     SmoothnessQualityMetric *met = new EdgeLengthQualityMetric;
       //SmoothnessQualityMetric *lam_met = new EdgeLengthQualityMetric(EdgeLengthQualityMetric::DIVIDE_BY_LAMBDA);
       //Check aspect ratio gamma of ideal tri patch
       //Vert[2] has two edges connected of length 1
     met->evaluate_vertex(triPatch,&verts[2],val,err);CPPUNIT_ASSERT(!err);
     if(pF)
       cout << "\nEdge Length Metric tris (should be 2) " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,2.0,qualTol);
       //lam_met->evaluate_vertex(triPatch,&verts[2],val,err);CPPUNIT_ASSERT(!err);
       //if(pF)
       //PRINT_INFO("\nEdge Length Metric tris (should be 4) %f", val);
       //CPPUNIT_ASSERT_DOUBLES_EQUAL(val,4.0,qualTol);
       //Vert[1] has two edges connected of length 1
       //THIRD TET's
     verts = tetPatch.get_vertex_array(err); CPPUNIT_ASSERT(!err);
     elems = tetPatch.get_element_array(err); CPPUNIT_ASSERT(!err);
       //Check aspect ratio gamma of ideal tet
     met->evaluate_vertex(tetPatch,&verts[0],val,err);CPPUNIT_ASSERT(!err);
     if(pF) cout << "\nEdge Length Metric tets (should be 3) " << val;
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,3.0,qualTol);
     delete met;
       //delete lam_met;
   }
  
  void test_local_size_metric()
     {
       VolumeQualityMetric *met = new LocalSizeQualityMetric;
       MsqPrintError err(cout); 
         //Test the metric on a single elemnt patch
       PatchData p1;
       create_one_tri_patch(p1, err);
       MsqVertex* vert1=p1.get_vertex_array(err);
       double first_val=-1;
       bool first_bool=met->evaluate_vertex(p1,&vert1[0],first_val,err); CPPUNIT_ASSERT(!err);
       if(pF) cout << "\nLocal size ideal tri " << first_val;
       CPPUNIT_ASSERT_DOUBLES_EQUAL(first_val, 1.0, qualTol);
       CPPUNIT_ASSERT(first_bool==true);
       first_bool=met->evaluate_vertex(p1,&vert1[1],first_val,err); CPPUNIT_ASSERT(!err);
       if(pF) cout << "\nLocal size ideal tri " << first_val;
       CPPUNIT_ASSERT_DOUBLES_EQUAL(first_val, 1.0, qualTol);
       CPPUNIT_ASSERT(first_bool==true);
         //Test on a patch with two ideal tris
       PatchData p2;
       create_two_tri_patch(p2, err); CPPUNIT_ASSERT(!err);
       MsqVertex* vert2=p2.get_vertex_array(err); CPPUNIT_ASSERT(!err);
       double second_val;
       bool second_bool=met->evaluate_vertex(p2,&vert2[0],second_val,err); CPPUNIT_ASSERT(!err);
         //Two neighboring tris with equal area should have local size of 1.0
       if(pF) cout << "\nLocal Size Metric two ideal tris " << second_val;
       CPPUNIT_ASSERT_DOUBLES_EQUAL(second_val, 1.0, qualTol);
       CPPUNIT_ASSERT(second_bool==true);
         /*
        //Test on a patch with two tris one not ideal
       PatchData p3;
       create_qm_two_tri_patch(p3, err);
       MsqMeshEntity* elem3=p3.get_element_array(err);
       double third_val;
       bool third_bool=met->evaluate_element(p3,&elem3[0],third_val,err);
         //Two neighboring tris with equal area should have asm of 0.0
       CPPUNIT_ASSERT(third_val>0.0);
       CPPUNIT_ASSERT(third_val<1.0);
       CPPUNIT_ASSERT(third_bool==true);
         */
       delete met;
     }
  void test_vertex_based_condition_number_metric()
     {
       ShapeQualityMetric *met = new VertexConditionNumberQualityMetric;
       MsqPrintError err(cout);
         //Test the metric on a single elemnt patch
       PatchData p1;
       create_one_tri_patch(p1, err); CPPUNIT_ASSERT(!err);
       MsqVertex* vert1=p1.get_vertex_array(err); CPPUNIT_ASSERT(!err);
       double first_val=-1;
       bool first_bool=met->evaluate_vertex(p1,&vert1[0],first_val,err);
       if(pF) cout << "\nVertex Condition No. Metric ideal tri " << first_val;
       CPPUNIT_ASSERT_DOUBLES_EQUAL(first_val, 1.0, qualTol);
       CPPUNIT_ASSERT(first_bool==true);
       first_bool=met->evaluate_vertex(p1,&vert1[1],first_val,err);
       if(pF) cout << "\nVertex Condition No. Metric ideal tri " << first_val;
       CPPUNIT_ASSERT_DOUBLES_EQUAL(first_val, 1.0, qualTol);
       CPPUNIT_ASSERT(first_bool==true);
         //Test on a patch with two ideal tris
       PatchData p2;
       create_two_tri_patch(p2, err); CPPUNIT_ASSERT(!err);
       MsqVertex* vert2=p2.get_vertex_array(err); CPPUNIT_ASSERT(!err);
       double second_val;
       bool second_bool=met->evaluate_vertex(p2,&vert2[0],second_val,err);
         //Two neighboring tris with equal area should have local size of 1.0
       if(pF) cout << "\nVertex Condition No. Metric ideal tris " << second_val;
       CPPUNIT_ASSERT_DOUBLES_EQUAL(second_val, 1.0, qualTol);
       CPPUNIT_ASSERT(second_bool==true);
       delete met;
     }
  
  void test_untangle_metric()
     {
       UntangleQualityMetric *met = new UntangleBetaQualityMetric(0.0);
       MsqPrintError err(cout); 
       double val;
         //Test the metric on a single elemnt patch
       MsqMeshEntity* elem1=idealTri.get_element_array(err); CPPUNIT_ASSERT(!err);
       bool first_bool=met->evaluate_element(idealTri,&elem1[0],val,err); CPPUNIT_ASSERT(!err);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(val, 0.0, qualTol);
       CPPUNIT_ASSERT(first_bool==true);
       elem1=idealTet.get_element_array(err); CPPUNIT_ASSERT(!err);
       first_bool=met->evaluate_element(idealTet,&elem1[0],val,err); CPPUNIT_ASSERT(!err);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(val, 0.0, qualTol);
       CPPUNIT_ASSERT(first_bool==true);
         //elem1=invertedTri.get_element_array(err);
         //first_bool=met->evaluate_element(invertedTri,&elem1[0],val,err);
         //std::cout<<"\nINVERTED TRI "<<val<<"\n";
         //CPPUNIT_ASSERT_DOUBLES_EQUAL(first_val, 0.0, MSQ_MIN);
         //CPPUNIT_ASSERT(first_bool==true);
       elem1=invertedTet.get_element_array(err); CPPUNIT_ASSERT(!err);
       first_bool=met->evaluate_element(invertedTet,&elem1[0],val,err); CPPUNIT_ASSERT(!err);
         //std::cout<<"\nINVERTED TET "<<val<<"\n";
         //Michael:: double check to make sure that 2.0 is correct here.
       CPPUNIT_ASSERT_DOUBLES_EQUAL(val, 2.0, qualTol);
       CPPUNIT_ASSERT(first_bool==true);

       elem1=idealPyr.get_element_array(err); CPPUNIT_ASSERT(!err);
       first_bool=met->evaluate_element(idealPyr,&elem1[0],val,err); CPPUNIT_ASSERT(!err);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(val, 0.0, qualTol);
       CPPUNIT_ASSERT(first_bool==true);

       elem1=idealWdg.get_element_array(err); CPPUNIT_ASSERT(!err);
       first_bool=met->evaluate_element(idealWdg,&elem1[0],val,err); CPPUNIT_ASSERT(!err);
       CPPUNIT_ASSERT_DOUBLES_EQUAL(val, 0.0, qualTol);
       CPPUNIT_ASSERT(first_bool==true);

       delete met;
     }  
  
  void test_averaging_method()
     {
         //USE QUAD's
     MsqPrintError err(cout);
     double val;
     MsqMeshEntity* elems;
     elems=quadPatch.get_element_array(err);
     CPPUNIT_ASSERT(!err);
     ShapeQualityMetric *met = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
       //Check mean ratio of ideal quad
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     met->set_averaging_method(QualityMetric::GEOMETRIC, err);
       //Check mean ratio of ideal quad GEOMETRIC
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     met->set_averaging_method(QualityMetric::HARMONIC, err);
       //Check mean ratio of ideal quad HARMONIC
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     met->set_averaging_method(QualityMetric::LINEAR, err);
       //Check mean ratio of ideal quad LINEAR
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     met->set_averaging_method(QualityMetric::MAXIMUM, err);
       //Check mean ratio of ideal quad MAXIMUM
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     met->set_averaging_method(QualityMetric::MINIMUM, err);
       //Check mean ratio of ideal quad MINIMUM
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     met->set_averaging_method(QualityMetric::RMS, err);
       //Check mean ratio of ideal quad RMS
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     met->set_averaging_method(QualityMetric::SUM, err);
       //Check mean ratio of ideal SUM (NOTICE:: should be 4.0)
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,4.0,qualTol);
     met->set_averaging_method(QualityMetric::MAX_OVER_MIN, err);
       //Check mean ratio of ideal MAX_OVER_MIN (NOTICE:: should be 1.0)
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     
     met->set_averaging_method(QualityMetric::MAX_MINUS_MIN, err);
       //Check mean ratio of ideal MAX_MINUS_MIN (NOTICE:: should be 0.0)
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,0.0,qualTol);
     met->set_averaging_method(QualityMetric::STANDARD_DEVIATION, err);
       //Check mean ratio of ideal STANDARD_DEVIATION (NOTICE:: should be 0.0)
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,0.0,qualTol);
     
     met->set_averaging_method(QualityMetric::SUM_OF_RATIOS_SQUARED, err);
       //Check mean ratio of ideal SUM_OF_RATIOS_SQR (NOTICE:: should be 1.0)
     met->evaluate_element(quadPatch,&elems[0],val,err);CPPUNIT_ASSERT(!err);
     CPPUNIT_ASSERT_DOUBLES_EQUAL(val,1.0,qualTol);
     delete met;
   }

  void test_mean_ratio_gradient(PatchData &pd)
  {
    MsqPrintError err(cout); 
    Vector3D* grad_num = new Vector3D[2];
    Vector3D* grad_ana = new Vector3D[2];
    double metric_value;
    bool valid;
	int i;

    MsqMeshEntity* elems = pd.get_element_array(err);CPPUNIT_ASSERT(!err);
    MsqVertex* vertices =  pd.get_vertex_array(err);CPPUNIT_ASSERT(!err);

    std::vector<size_t> bad_elem_vertex_indices;
    elems[1].get_vertex_indices(bad_elem_vertex_indices);
    MsqVertex* two_vtces[2];
    two_vtces[0] = &vertices[bad_elem_vertex_indices[0]];
    two_vtces[1] = &vertices[bad_elem_vertex_indices[2]];
    
    // creates a mean ratio quality metric ...
    ShapeQualityMetric* mean_ratio = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
    mean_ratio->set_averaging_method(QualityMetric::SUM, err);

    mean_ratio->set_gradient_type(QualityMetric::NUMERICAL_GRADIENT);
    valid = mean_ratio->compute_element_gradient (pd, &elems[1], two_vtces,
                                          grad_num, 2, metric_value, err); CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(valid);
    if(pF){
      std::cout << "NUMERICAL GRADIENT\n";
      for (int i=0; i<2; ++i)
        for (int j=0; j<3; ++j)
          std::cout << grad_num[i][j] << std::endl;
    }
    
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    valid = mean_ratio->compute_element_gradient (pd, &elems[1], two_vtces,
                                                  grad_ana, 2, metric_value, err);
    CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(valid);
    if(pF){
      std::cout << "ANALYTICAL GRADIENT\n";
      for (int i=0; i<2; ++i)
        for (int j=0; j<3; ++j)
          std::cout << grad_ana[i][j] << std::endl;
    }
    
    for (i=0; i<2; ++i)
      for (int j=0; j<3; ++j)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(grad_num[i][j], grad_ana[i][j], 0.001);
    
    delete mean_ratio;
    delete []grad_num;
    delete []grad_ana;
  }


  
  void test_mean_ratio_tri_gradient_planar()
  {
    if(pF) cout << "\ntest_mean_ratio_tri_gradient_planar()\n";
    triPatch.vertex_by_index(3).z(0.);
    test_mean_ratio_gradient(triPatch);
  }
       
  void test_mean_ratio_tri_gradient_nonplanar()
  {
    
    if(pF) cout << "\ntest_mean_ratio_tri_gradient_nonplanar()\n";
    test_mean_ratio_gradient(triPatch);
  }
       
  void test_mean_ratio_quad_gradient_planar()
  {
    if(pF) cout << "\ntest_mean_ratio_quad_gradient_planar()\n";
    quadPatch.vertex_by_index(4).z(0.);
    quadPatch.vertex_by_index(5).z(0.);
    test_mean_ratio_gradient(quadPatch);
  }
       
  void test_mean_ratio_quad_gradient_nonplanar()
  {
    if(pF) cout << "\ntest_mean_ratio_quad_gradient_nonplanar()\n";
    test_mean_ratio_gradient(quadPatch);
  }
       
  void test_mean_ratio_hex_gradient()
  {
    if(pF) cout << "\ntest_mean_ratio_hex_gradient()\n";
    test_mean_ratio_gradient(hexPatch);
  }
  void test_mean_ratio_tet_gradient()
  {
    if(pF) cout << "\ntest_mean_ratio_tet_gradient()\n";
    test_mean_ratio_gradient(tetPatch);
  }
  void test_mean_ratio_pyr_gradient()
  {
    if(pF) cout << "\ntest_mean_ratio_pyr_gradient()\n";
    test_mean_ratio_gradient(pyrPatch);
  }
  void test_mean_ratio_wdg_gradient()
  {
    if(pF) cout << "\ntest_mean_ratio_wdg_gradient()\n";
    test_mean_ratio_gradient(wdgPatch);
  }
  
  void test_i_dft_tri_gradient()
  {
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(triPatch, &i_dft_metric, 1);
  }
  void test_i_dft_quad_gradient()
  {
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(quadPatch,&i_dft_metric, 1);
  }
  void test_i_dft_no_barrier_tri_gradient()
  {
    I_DFT_NoBarrier i_dft_metric;
    test_i_dft_fix_vertices(triPatch,&i_dft_metric, 1);
  }
  void test_i_dft_no_barrier_quad_gradient()
  {
    I_DFT_NoBarrier i_dft_metric;
    test_i_dft_fix_vertices(quadPatch,&i_dft_metric, 1);
  }  
  void test_i_dft_tet_gradient()
  {
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(tetPatch,&i_dft_metric, 1);
  }     
  void test_i_dft_pyr_gradient()
  {
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(pyrPatch, &i_dft_metric, 1);
  }     
  void test_i_dft_wdg_gradient()
  {
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(wdgPatch, &i_dft_metric, 1);
  }     
  void test_i_dft_hex_gradient()
  {
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(hexPatch,&i_dft_metric, 1);
  }        
  
  void test_i_dft_imr_hex_gradient()
  {
    I_DFT_InverseMeanRatio i_dft_metric;
    test_i_dft_gradient(hexPatch, &i_dft_metric);
  }           
  void test_i_dft_imr_tet_gradient()
  {
    I_DFT_InverseMeanRatio i_dft_metric;
    test_i_dft_gradient(tetPatch, &i_dft_metric);
  }
  void test_i_dft_imr_pyr_gradient()
  {
    I_DFT_InverseMeanRatio i_dft_metric;
    test_i_dft_gradient(pyrPatch, &i_dft_metric);
  }
  void test_i_dft_imr_wdg_gradient()
  {
    I_DFT_InverseMeanRatio i_dft_metric;
    test_i_dft_gradient(wdgPatch, &i_dft_metric);
  }
  
  void test_i_dft_strong_barrier_hex_gradient()
  {
    I_DFT_StrongBarrier i_dft_metric;
    test_i_dft_gradient(hexPatch, &i_dft_metric);
  }           
  void test_i_dft_strong_barrier_tet_gradient()
  {
    I_DFT_StrongBarrier i_dft_metric;
    test_i_dft_gradient(tetPatch, &i_dft_metric);
  } 
  void test_i_dft_strong_barrier_pyr_gradient()
  {
    I_DFT_StrongBarrier i_dft_metric;
    test_i_dft_gradient(pyrPatch, &i_dft_metric);
  } 
  void test_i_dft_strong_barrier_wdg_gradient()
  {
    I_DFT_StrongBarrier i_dft_metric;
    test_i_dft_gradient(wdgPatch, &i_dft_metric);
  } 
  void test_i_dft_weak_barrier_hex_gradient()
  {
    I_DFT_WeakBarrier i_dft_metric;
    test_i_dft_gradient(hexPatch, &i_dft_metric);
  }           
  void test_i_dft_weak_barrier_tet_gradient()
  {
    I_DFT_WeakBarrier i_dft_metric;
    test_i_dft_gradient(tetPatch, &i_dft_metric);
  }
  void test_i_dft_weak_barrier_pyr_gradient()
  {
    I_DFT_WeakBarrier i_dft_metric;
    test_i_dft_gradient(pyrPatch, &i_dft_metric);
  }
  void test_i_dft_weak_barrier_wdg_gradient()
  {
    I_DFT_WeakBarrier i_dft_metric;
    test_i_dft_gradient(wdgPatch, &i_dft_metric);
  }
  void test_i_dft_nobarrier_hex_gradient()
  {
    I_DFT_NoBarrier i_dft_metric;
    test_i_dft_gradient(hexPatch, &i_dft_metric);
  }           
  void test_i_dft_nobarrier_tet_gradient()
  {
    I_DFT_NoBarrier i_dft_metric;
    test_i_dft_gradient(tetPatch, &i_dft_metric);
  }
  void test_i_dft_nobarrier_pyr_gradient()
  {
    I_DFT_NoBarrier i_dft_metric;
    test_i_dft_gradient(pyrPatch, &i_dft_metric);
  }
  void test_i_dft_nobarrier_wdg_gradient()
  {
    I_DFT_NoBarrier i_dft_metric;
    test_i_dft_gradient(wdgPatch, &i_dft_metric);
  }
  
  /*! This tests the QualityMetric hessian, comparing analytical
      and numerical versions. 
      
      \param pd: this PatchData must have at least two arguments.
      \param *met: pointer to a metric which will be used in the
      test.  NOTE:  the test may change the current type of gradient
      or hessian evaluations used for the metric.
  */
  void test_metric_hessian(PatchData &pd, QualityMetric *met)
  {
    MsqPrintError err(cout); 
    int i, m, max_nve = MSQ_MAX_NUM_VERT_PER_ENT;
    Vector3D* grad_num = new Vector3D[max_nve];
    Vector3D* grad_ana = new Vector3D[max_nve];
    Matrix3D* hessian_num = new Matrix3D[max_nve*(max_nve+1)/2];
    Matrix3D* hessian_ana = new Matrix3D[max_nve*(max_nve+1)/2];
    double metric_value;
    double metric_value2;
    

    MsqMeshEntity* elems = pd.get_element_array(err);CPPUNIT_ASSERT(!err);
    MsqVertex* vertices =  pd.get_vertex_array(err);CPPUNIT_ASSERT(!err);

    std::vector<size_t> elem_vtx_indices;
    elems[1].get_vertex_indices(elem_vtx_indices);
    int nve = elem_vtx_indices.size(); // number of vertices in element.
    MsqVertex** all_vtces = new MsqVertex*[nve];
    for (i=0; i<nve; ++i) {
      all_vtces[i] = &vertices[elem_vtx_indices[i]];
    }

    all_vtces[0] = &vertices[elem_vtx_indices[0]];
    all_vtces[1] = &vertices[elem_vtx_indices[2]];
    all_vtces[2] = NULL;
    met->set_hessian_type(QualityMetric::NUMERICAL_HESSIAN);
    bool ret_bool=met->compute_element_hessian(pd, &elems[1],
                                               all_vtces, grad_num,
                                               hessian_num, 2,
                                               metric_value,err);
    CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(ret_bool==true);
    if(pF){
      std::cout << "\nNUMERICAL Metric value "<<metric_value<<".\n";
     std::cout << "NUMERICAL GRADIENT for element with two free vertices.\n";
     for (i=0; i<4; ++i)
       for (int j=0; j<3; ++j)
         std::cout << grad_num[i][j] << std::endl;
     std::cout << "NUMERICAL HESSIAN for element with two free vertices.\n";
     for (i=0; i<nve*(nve+1)/2; ++i)
       std::cout << hessian_num[i] << std::endl;
    }
    
    met->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    ret_bool=met->compute_element_hessian(pd, &elems[1], all_vtces,
                                          grad_ana, hessian_ana, 2,
                                          metric_value2,err);
    CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(ret_bool==true);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(metric_value, metric_value2, 0.001);
    if(pF){
      std::cout << "\nANALYTICAL Metric value "<<metric_value<<".\n";
      std::cout << "ANALYTICAL GRADIENT for element with two free vertices.\n";
      for (i=0; i<4; ++i)
        for (int j=0; j<3; ++j)
          std::cout << grad_ana[i][j] << std::endl;
      std::cout << "ANALYTICAL HESSIAN for element with two free vertices.\n";
      for (i=0; i<nve*(nve+1)/2; ++i)
        std::cout << hessian_ana[i] << std::endl;
    }
    
    // test returned gradients
    for (m=0; m<nve; ++m)
      for (i=0; i<3; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(grad_num[m][i], grad_ana[m][i], 0.001);    

    // test returned Hessians
    for (m=0; m<nve*(nve+1)/2; ++m)
      for (i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
          CPPUNIT_ASSERT_DOUBLES_EQUAL(hessian_num[m][i][j], hessian_ana[m][i][j], 0.02);


    
    delete[] all_vtces;
    delete[] grad_num;
    delete[] grad_ana;
    delete[] hessian_num;
    delete[] hessian_ana;
  }
  

  
  /*! This tests the QualityMetric hessian, comparing analytical
      and numerical versions. Two comparisons are performed, one for
      elements with free vertices only, and one for an element that
      includes fixed vertices.
      
      \param pd: this PatchData must have at least two arguments.
  */
  void test_mean_ratio_hessian(PatchData &pd)
  {
    MsqPrintError err(cout); 
    int m, max_nve = MSQ_MAX_NUM_VERT_PER_ENT;
    Vector3D* grad_num = new Vector3D[max_nve];
    Vector3D* grad_ana = new Vector3D[max_nve];
    Matrix3D* hessian_num = new Matrix3D[max_nve*(max_nve+1)/2];
    Matrix3D* hessian_ana = new Matrix3D[max_nve*(max_nve+1)/2];
    double metric_value;

    MsqMeshEntity* elems = pd.get_element_array(err);CPPUNIT_ASSERT(!err);
    MsqVertex* vertices =  pd.get_vertex_array(err);CPPUNIT_ASSERT(!err);

    std::vector<size_t> elem_vtx_indices;
    elems[1].get_vertex_indices(elem_vtx_indices);
    int nve = elem_vtx_indices.size(); // number of vertices in element.
    MsqVertex** all_vtces = new MsqVertex*[nve];
    for (int i=0; i<nve; ++i) {
      all_vtces[i] = &vertices[elem_vtx_indices[i]];
    }

    // 1 **** test with all vertices free
    // creates a mean ratio quality metric ...
    ShapeQualityMetric* mean_ratio = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
    

//    mean_ratio->set_gradient_type(QualityMetric::NUMERICAL_GRADIENT);
    mean_ratio->set_hessian_type(QualityMetric::NUMERICAL_HESSIAN);
    mean_ratio->compute_element_hessian(pd, &elems[1], all_vtces,
                                        grad_num, hessian_num, nve, metric_value,
                                        err); CPPUNIT_ASSERT(!err);
    if(pF){
      std::cout << "GRADIENT for element with all  vertices free.\n";
      for (int i=0; i<nve; ++i)
        for (int j=0; j<3; ++j)
          std::cout << grad_num[i][j] << std::endl;

      std::cout << "NUMERICAL HESSIAN for element with all  vertices free.\n";
      for (int i=0; i<nve*(nve+1)/2; ++i)
        std::cout << hessian_num[i] << std::endl;
    }
    
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    mean_ratio->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    mean_ratio->compute_element_hessian(pd, &elems[1], all_vtces,
                                        grad_ana, hessian_ana, nve, metric_value,
                                        err); CPPUNIT_ASSERT(!err);
    if(pF){
      std::cout << "ANALYTICAL HESSIAN for element with all  vertices free.\n";
      for (int i=0; i<nve*(nve+1)/2; ++i)
        std::cout << hessian_ana[i] << std::endl;
    }
    

      // test returned gradients
    for (m=0; m<nve; ++m)
      for (int i=0; i<3; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(grad_num[m][i], grad_ana[m][i], 0.001);    

    // test returned Hessians
    for (m=0; m<nve*(nve+1)/2; ++m)
      for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j){
            //PRINT_INFO("\nm=%i,i=%i,j=%i",m,i,j);
            //PRINT_INFO("\nNumerical = %f, Analytical = %f",hessian_num[m][i][j], hessian_ana[m][i][j]);
          CPPUNIT_ASSERT_DOUBLES_EQUAL(hessian_num[m][i][j], hessian_ana[m][i][j], 0.003);
        }
    
    
    // 2 **** same test as 1, but gives the free vertices in an order
    //        different than the order within the elements.
    //        Test check that an error is set. 

    // swaps free vertices 0 and 2.
    MsqVertex* swap;
    swap = all_vtces[2];
    all_vtces[2] = all_vtces[0];
    all_vtces[0] = swap;
    
    mean_ratio->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    bool ret_bool = mean_ratio->compute_element_hessian(pd, &elems[1],
                                                        all_vtces,
                                                        grad_ana, hessian_ana,
                                                        nve, metric_value,
                                                        err);
    
    CPPUNIT_ASSERT(ret_bool == false);
    CPPUNIT_ASSERT(err == true);
    err.clear();

    
    // 3 **** same test as 1, but with only 2 free vertices in the element. 
    all_vtces[0] = &vertices[elem_vtx_indices[0]];
    all_vtces[1] = &vertices[elem_vtx_indices[2]];
    all_vtces[2] = NULL;
    mean_ratio->set_hessian_type(QualityMetric::NUMERICAL_HESSIAN);
    ret_bool=mean_ratio->compute_element_hessian(pd, &elems[1], all_vtces,
                                                 grad_num, hessian_num, 2,
                                                 metric_value,
                                                 err); CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(ret_bool==true);
    if(pF){
      std::cout << "GRADIENT for element with two free vertices.\n";
      for (int i=0; i<4; ++i)
        for (int j=0; j<3; ++j)
          std::cout << grad_num[i][j] << std::endl;

      std::cout << "NUMERICAL HESSIAN for element with two free vertices.\n";
      for (int i=0; i<nve*(nve+1)/2; ++i)
        std::cout << hessian_num[i] << std::endl;
    }
    
    mean_ratio->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    ret_bool=mean_ratio->compute_element_hessian(pd, &elems[1], all_vtces,
                                                 grad_ana, hessian_ana, 2,
                                                 metric_value,
                                                 err); CPPUNIT_ASSERT(!err);
    CPPUNIT_ASSERT(ret_bool==true);
    if(pF){
      std::cout << "ANALYTICAL HESSIAN for element with two free vertices.\n";
      for (int i=0; i<nve*(nve+1)/2; ++i)
        std::cout << hessian_ana[i] << std::endl;
    }
    
    // test returned gradients
    for (m=0; m<nve; ++m)
      for (int i=0; i<3; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(grad_num[m][i], grad_ana[m][i], 0.001);    

    // test returned Hessians
    for (m=0; m<nve*(nve+1)/2; ++m)
      for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
          CPPUNIT_ASSERT_DOUBLES_EQUAL(hessian_num[m][i][j], hessian_ana[m][i][j], 0.003);

       
    delete mean_ratio;
    delete[] all_vtces;
    delete[] grad_num;
    delete[] grad_ana;
    delete[] hessian_num;
    delete[] hessian_ana;
  }
       
  void test_mean_ratio_tri_hessian()
    {
      MsqPrintError err(cout); 
      test_mean_ratio_hessian(triPatch);
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
        //make sure the code handles this case correctly
      mean_rat->set_averaging_method(QualityMetric::SUM_SQUARED,
                                     err);
      test_metric_hessian(triPatch, mean_rat);
      delete mean_rat;
    }
  
        
  
  void test_mean_ratio_quad_hessian()
  {
    test_mean_ratio_hessian(quadPatch);
  }
  void test_mean_ratio_quad_hessian_linear()
    {
      MsqPrintError err(cout); 
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::LINEAR,
                                   err);
      test_metric_hessian(quadPatch, mean_rat);
      delete mean_rat;
    }
  void test_mean_ratio_quad_hessian_sum_squared()
    {
      MsqPrintError err(cout);
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::SUM_SQUARED,
                                     err);
      test_metric_hessian(quadPatch, mean_rat);
      delete mean_rat;
    }
   void test_mean_ratio_quad_hessian_rms()
    {
      MsqPrintError err(cout);
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::RMS,
                                     err);
      test_metric_hessian(quadPatch, mean_rat);
      delete mean_rat;
    }
  void test_mean_ratio_quad_hessian_harmonic()
    {
      MsqPrintError err(cout); 
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::HARMONIC,
                                     err);
      test_metric_hessian(quadPatch, mean_rat);
      delete mean_rat;
    }
  void test_mean_ratio_quad_hessian_hms()
    {
      MsqPrintError err(cout); 
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::HMS,
                                     err);
      test_metric_hessian(quadPatch, mean_rat);
      delete mean_rat;
    }
  void test_mean_ratio_tet_hessian()
  {
    if(pF) cout << "\ntest_mean_ratio_tet_hessian()\n";
    test_mean_ratio_hessian(tetPatch);
  }
  void test_mean_ratio_pyr_hessian()
  {
    if(pF) cout << "\ntest_mean_ratio_pyr_hessian()\n";
    test_mean_ratio_hessian(pyrPatch);
  }
  void test_mean_ratio_wdg_hessian()
  {
    if(pF) cout << "\ntest_mean_ratio_wdg_hessian()\n";
    test_mean_ratio_hessian(wdgPatch);
  }
  
  void test_mean_ratio_hex_hessian()
  {
    if(pF) cout << "\ntest_mean_ratio_hex_hessian()\n";
    test_mean_ratio_hessian(hexPatch);
    MsqPrintError err(cout);
    QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
    mean_rat->set_averaging_method(QualityMetric::SUM_SQUARED,
                                   err);
    test_metric_hessian(hexPatch, mean_rat);
    delete mean_rat;
  }
 void test_mean_ratio_hex_hessian_linear()
    {
      MsqPrintError err(cout); 
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::LINEAR,
                                     err);
      test_metric_hessian(hexPatch, mean_rat);
      delete mean_rat;
    }  
  void test_i_dft_hex_hessian()
    {
      if(pF)
        std::cout<<"\nTesting hex Hessian for I_DFT.\n";
      I_DFT i_dft_metric;
      test_i_dft_fix_vertices(hexPatch, &i_dft_metric,2);
    }
  void test_i_dft_tet_hessian()
    {
      if(pF)
        std::cout<<"\nTesting tet Hessian for I_DFT.\n";
      I_DFT i_dft_metric;
      test_i_dft_fix_vertices(tetPatch, &i_dft_metric,2);
    }
  void test_i_dft_pyr_hessian()
    {
      if(pF)
        std::cout<<"\nTesting pyr Hessian for I_DFT.\n";
      I_DFT i_dft_metric;
      test_i_dft_fix_vertices(pyrPatch, &i_dft_metric, 2);
    }
  void test_i_dft_wdg_hessian()
    {
      if(pF)
        std::cout<<"\nTesting wdg Hessian for I_DFT.\n";
      I_DFT i_dft_metric;
      test_i_dft_fix_vertices(wdgPatch, &i_dft_metric, 2);
    }
  void test_i_dft_quad_hessian()
    {
      if(pF)
        std::cout<<"\nTesting quad Hessian for I_DFT.\n";
      I_DFT i_dft_metric;
      test_i_dft_fix_vertices(quadPatch, &i_dft_metric,2);
    }
  void test_i_dft_tri_hessian()
    {
      if(pF)
        std::cout<<"\nTesting tri Hessian for I_DFT.\n";
      I_DFT i_dft_metric;
      test_i_dft_fix_vertices(triPatch, &i_dft_metric, 2);
    }
  void test_i_dft_no_barrier_quad_hessian()
    {
      if(pF)
        std::cout<<"\nTesting quad Hessian for I_DFT.\n";
      I_DFT_NoBarrier i_dft_metric;
      test_i_dft_fix_vertices(quadPatch, &i_dft_metric, 2);
    }
  void test_i_dft_no_barrier_tri_hessian()
    {
      if(pF)
        std::cout<<"\nTesting tri Hessian for I_DFT.\n";
      I_DFT_NoBarrier i_dft_metric;
      test_i_dft_fix_vertices(triPatch, &i_dft_metric, 2);
    }
  void test_i_dft_weak_barrier_hex_hessian()
    {
      if(pF)
        std::cout<<"\nTesting hex Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_WeakBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
     
      test_metric_hessian(hexPatch, &i_dft_metric);
    }
  void test_i_dft_weak_barrier_tet_hessian()
    {
      if(pF)
        std::cout<<"\nTesting tet Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_WeakBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(tetPatch, &i_dft_metric);
    }  
  void test_i_dft_weak_barrier_pyr_hessian()
    {
      if(pF)
        std::cout<<"\nTesting PYR Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_WeakBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(pyrPatch, &i_dft_metric);
    }  
  void test_i_dft_weak_barrier_wdg_hessian()
    {
      if(pF)
        std::cout<<"\nTesting WDG Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_WeakBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(wdgPatch, &i_dft_metric);
    }  
  void test_i_dft_imr_hex_hessian()
    {
      if(pF)
        std::cout<<"\nTesting hex Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_InverseMeanRatio i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
     
      test_metric_hessian(hexPatch, &i_dft_metric);
    }
  void test_i_dft_imr_tet_hessian()
    {
      if(pF)
        std::cout<<"\nTesting tet Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_InverseMeanRatio i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(tetPatch, &i_dft_metric);
    }  
  void test_i_dft_imr_pyr_hessian()
    {
      if(pF)
        std::cout<<"\nTesting pyramid Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_InverseMeanRatio i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(pyrPatch, &i_dft_metric);
    }  
  void test_i_dft_imr_wdg_hessian()
    {
      if(pF)
        std::cout<<"\nTesting wedge Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_InverseMeanRatio i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(wdgPatch, &i_dft_metric);
    }  
  void test_i_dft_strong_barrier_hex_hessian()
    {
      if(pF)
        std::cout<<"\nTesting hex Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_StrongBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
     
      test_metric_hessian(hexPatch, &i_dft_metric);
    }
  void test_i_dft_strong_barrier_tet_hessian()
    {
      if(pF)
        std::cout<<"\nTesting tet Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_StrongBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(tetPatch, &i_dft_metric);
    }
   void test_i_dft_strong_barrier_pyr_hessian()
    {
      if(pF)
        std::cout<<"\nTesting PYR Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_StrongBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(pyrPatch, &i_dft_metric);
    }
   void test_i_dft_strong_barrier_wdg_hessian()
    {
      if(pF)
        std::cout<<"\nTesting wedge Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_StrongBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(wdgPatch, &i_dft_metric);
    }
 void test_i_dft_nobarrier_hex_hessian()
    {
      if(pF)
        std::cout<<"\nTesting hex Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_NoBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
     
      test_metric_hessian(hexPatch, &i_dft_metric);
    }
  void test_i_dft_nobarrier_tet_hessian()
    {
      if(pF)
        std::cout<<"\nTesting tet Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_NoBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(tetPatch, &i_dft_metric);
    }  
  void test_i_dft_nobarrier_pyr_hessian()
    {
      if(pF)
        std::cout<<"\nTesting PYR Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_NoBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(pyrPatch, &i_dft_metric);
    }  
  void test_i_dft_nobarrier_wdg_hessian()
    {
      if(pF)
        std::cout<<"\nTesting wedge Hessian for I_DFT.\n";
      MsqPrintError err(cout); 
 
      I_DFT_NoBarrier i_dft_metric;
      CPPUNIT_ASSERT(!err);
      i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
      CPPUNIT_ASSERT(!err);
      test_metric_hessian(wdgPatch, &i_dft_metric);
    }  

  void test_mean_ratio_hex_hessian_sum_squared()
    {
      MsqPrintError err(cout);
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::SUM_SQUARED,
                                     err);
      test_metric_hessian(hexPatch, mean_rat);
      delete mean_rat;
    }
  
  void test_mean_ratio_hex_hessian_rms()
    {
      MsqPrintError err(cout); 
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::RMS,
                                     err);
      test_metric_hessian(hexPatch, mean_rat);
      delete mean_rat;
    }
 void test_mean_ratio_hex_hessian_harmonic()
    {
      MsqPrintError err(cout);
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::HARMONIC,
                                     err);
      test_metric_hessian(hexPatch, mean_rat);
      delete mean_rat;
    }
  void test_mean_ratio_hex_hessian_hms()
    {
      MsqPrintError err(cout);
      QualityMetric* mean_rat = new IdealWeightInverseMeanRatio(err);CPPUNIT_ASSERT(!err);
      mean_rat->set_averaging_method(QualityMetric::HMS,
                                     err);
      test_metric_hessian(hexPatch, mean_rat);
      delete mean_rat;
    }



  void test_mean_ratio_tri_grad_from_hessian()
  {
    MsqPrintError err(cout);
    ShapeQualityMetric* mean_ratio = new IdealWeightInverseMeanRatio(err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    mean_ratio->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(triPatch,mean_ratio,err);
    delete mean_ratio;
  }
  
  void test_mean_ratio_quad_grad_from_hessian()
  {
    MsqPrintError err(cout);    
    ShapeQualityMetric* mean_ratio = new IdealWeightInverseMeanRatio(err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    mean_ratio->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(quadPatch,mean_ratio,err);
    delete mean_ratio;
  }

  void test_mean_ratio_tet_grad_from_hessian()
  {
    MsqPrintError err(cout);
    ShapeQualityMetric* mean_ratio = new IdealWeightInverseMeanRatio(err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    mean_ratio->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(tetPatch,mean_ratio,err);
    delete mean_ratio;
  }

  void test_mean_ratio_pyr_grad_from_hessian()
  {
    MsqPrintError err(cout);
    ShapeQualityMetric* mean_ratio = new IdealWeightInverseMeanRatio(err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    mean_ratio->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(pyrPatch,mean_ratio,err);
    delete mean_ratio;
  }

  void test_mean_ratio_wdg_grad_from_hessian()
  {
    MsqPrintError err(cout);
    ShapeQualityMetric* mean_ratio = new IdealWeightInverseMeanRatio(err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    mean_ratio->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(wdgPatch,mean_ratio,err);
    delete mean_ratio;
  }
  
  
  void test_mean_ratio_hex_grad_from_hessian()
  {
    MsqPrintError err(cout);
    ShapeQualityMetric* mean_ratio = new IdealWeightInverseMeanRatio(err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    mean_ratio->set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(hexPatch,mean_ratio,err);
    delete mean_ratio;
  }


  
  void test_i_dft_tet_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_tet_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(tetPatch, &i_dft_metric, 3);
  }
  
  void test_i_dft_pyr_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_pyr_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(pyrPatch, &i_dft_metric, 3);
  }
  
  void test_i_dft_wdg_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_wdg_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(wdgPatch, &i_dft_metric, 3);
  }

  void test_i_dft_hex_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_hex_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(hexPatch,  &i_dft_metric, 3);
  }
    
  void test_i_dft_tri_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_tri_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(triPatch,  &i_dft_metric, 3);
  }

  void test_i_dft_quad_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_quad_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT i_dft_metric;
    test_i_dft_fix_vertices(quadPatch,  &i_dft_metric, 3);
  }
  void test_i_dft_no_barrier_tri_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_no_barrier_tri_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_NoBarrier i_dft_metric;
    test_i_dft_fix_vertices(triPatch,  &i_dft_metric, 3);
  }

  void test_i_dft_no_barrier_quad_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_no_barrier_quad_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_NoBarrier i_dft_metric;
    test_i_dft_fix_vertices(quadPatch,  &i_dft_metric, 3);
  }
  
  void test_i_dft_strong_barrier_tet_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_strong_barrier_tet_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_StrongBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(tetPatch,&i_dft_metric,err);
  }
  
  void test_i_dft_strong_barrier_pyr_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_strong_barrier_pyr_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_StrongBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(pyrPatch,&i_dft_metric,err);
  }
  
  void test_i_dft_strong_barrier_wdg_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_strong_barrier_wdg_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_StrongBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(wdgPatch,&i_dft_metric,err);
  }

  void test_i_dft_strong_barrier_hex_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_strong_barrier_hex_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_StrongBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(hexPatch,&i_dft_metric,err);
  }
  
  void test_i_dft_nobarrier_tet_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_nobarrier_tet_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_NoBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(tetPatch,&i_dft_metric,err);
  }
  
  void test_i_dft_nobarrier_pyr_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_nobarrier_pyr_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_NoBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(pyrPatch,&i_dft_metric,err);
  }
  
  void test_i_dft_nobarrier_wdg_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_nobarrier_wdg_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_NoBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(wdgPatch,&i_dft_metric,err);
  }

  void test_i_dft_nobarrier_hex_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_nobarrier_hex_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_NoBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(hexPatch,&i_dft_metric,err);
  }
  void test_i_dft_weak_barrier_tet_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_weak_barrier_tet_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_WeakBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(tetPatch,&i_dft_metric,err);
  }
  void test_i_dft_weak_barrier_pyr_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_weak_barrier_pyr_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_WeakBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(pyrPatch,&i_dft_metric,err);
  }
  void test_i_dft_weak_barrier_wdg_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_weak_barrier_wdg_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_WeakBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(wdgPatch,&i_dft_metric,err);
  }

  void test_i_dft_weak_barrier_hex_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_weak_barrier_hex_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_WeakBarrier i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(hexPatch,&i_dft_metric,err);
  }
  
  void test_i_dft_imr_tet_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_imr_tet_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_InverseMeanRatio i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(tetPatch,&i_dft_metric,err);
  }
  
  void test_i_dft_imr_pyr_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_imr_pyr_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_InverseMeanRatio i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(pyrPatch,&i_dft_metric,err);
  }
  
  void test_i_dft_imr_wdg_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_imr_wdg_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_InverseMeanRatio i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(wdgPatch,&i_dft_metric,err);
  }

  void test_i_dft_imr_hex_grad_from_hessian()
  {
    if(pF)
      std::cout<<"\nTesting I_DFT metrics (test_i_dft_imr_hex_grad_from_hessian).\n";
    MsqPrintError err(cout);
    I_DFT_InverseMeanRatio i_dft_metric;
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_averaging_method(QualityMetric::SUM, err);
    CPPUNIT_ASSERT(!err);
    i_dft_metric.set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    i_dft_metric.set_hessian_type(QualityMetric::ANALYTICAL_HESSIAN);
    test_metric_grad_from_hessian(hexPatch,&i_dft_metric,err);
  }
  
  
};


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(QualityMetricTest, "QualityMetricTest");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(QualityMetricTest, "Unit");
