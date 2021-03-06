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
/*!
  \file   QualityMetric.cpp
  \brief  

  \author Michael Brewer
  \author Thomas Leurent
  \date   2002-05-14
*/

#include "QualityMetric.hpp"
#include "MsqVertex.hpp"
#include "MsqMeshEntity.hpp"
#include "MsqDebug.hpp"
#include "MsqTimer.hpp"
#include "PatchData.hpp"

using namespace Mesquite;

#ifdef MSQ_USE_OLD_STD_HEADERS
#  include <vector.h>
#else
#  include <vector>
#endif

/*!
  \param pd: PatchData that contains the element which Hessian we want.

  \param el: this is the element for which the Hessian will be returned.
  
  \param free_vtces: base address of an array of pointers to the element
  vertices which are considered free for purposes of computing the hessian.
  The vertices within this array must be ordered in the same order
  as the vertices within the element, el. 
  Only the Hessian entries corresponding to a pair of free vertices
  will be non-zero.

  \param grad_vec: this is an array of nve Vector3D, where nve is the total
  number of vertices in the element. Only the entries corresponding to free
  vertices specified in free_vtces will be non-zero. The order is the same
  as the order of the vertices in el.

  \param hessian: this is a 1D array of Matrix3D that will contain the upper
  triangular part of the Hessian. It has size nve*(nve+1)/2, i.e. the number
  of entries in the upper triangular part of a nve*nve matrix.

  \param num_free_vtx: is the number df free vertices in the element.
  Essentially, this gives the size of free_vtces[].  The gradient array has
  the size of the number of vertices in the element, regardless.

  \param metric_value: Since the metric is computed, we return it.
  
  \return true if the element is valid, false otherwise. 
*/
bool QualityMetric::compute_element_hessian(PatchData &pd,
                                            MsqMeshEntity* el,
                                            MsqVertex* free_vtces[],
                                            Vector3D grad_vec[],
                                            Matrix3D hessian[],
                                            int num_free_vtx,
                                            double &metric_value,
                                            MsqError &err)
{
  // first, checks that free vertices order is consistent with the
  // element order. 
  msq_std::vector<size_t> elem_vtx_indices;
  msq_std::vector<size_t>::const_iterator v;
  el->get_vertex_indices(elem_vtx_indices);
  int i;
  v=elem_vtx_indices.begin();
  for (i=0; i<num_free_vtx; ++i) {
    while ( v!=elem_vtx_indices.end() &&
            *v != pd.get_vertex_index(free_vtces[i]) ) {
      ++v;
    }
    if ( v==elem_vtx_indices.end() ) {
      MSQ_SETERR(err)("free vertices cannot be given in a different"
                      "order than the element's.", MsqError::INTERNAL_ERROR);
      return false;
    }
  }
    
    
  bool ret=false;
  switch(hessianType)
    {
    case NUMERICAL_HESSIAN:
      ret = compute_element_numerical_hessian(pd, el, free_vtces, grad_vec, hessian,
                                              num_free_vtx, metric_value, err);
      MSQ_ERRZERO(err);
      break;
    case ANALYTICAL_HESSIAN:
      ret = compute_element_analytical_hessian(pd, el, free_vtces, grad_vec, hessian,
                                               num_free_vtx, metric_value, err);
      MSQ_ERRZERO(err);
      break;
    }
  return ret;
}
   
   
/*! If that function is not over-riden in the concrete class, the base
    class function makes it default to a numerical gradient.
    \param vertex  Vertex which is considered free for purposes of computing the gradient.
    \param grad_vec Vector where the gradient is stored.
    \param metric_value Since the metric is computed, we return it. 
    \return true if the element is valid, false otherwise.
*/
bool QualityMetric::compute_vertex_analytical_gradient(PatchData &pd,
                                                       MsqVertex &vertex,
                                                       MsqVertex* free_vtces[],
                                                       Vector3D grad_vec[],
                                                       int num_free_vtx,
                                                       double &metric_value,
                                                       MsqError &err)
{
  MSQ_PRINT(1)("QualityMetric has no analytical gradient defined. "
                            "Defaulting to numerical gradient.\n");
  set_gradient_type(NUMERICAL_GRADIENT);
  bool b = compute_vertex_numerical_gradient(pd, vertex, free_vtces, grad_vec,
                                           num_free_vtx, metric_value, err);
  return !MSQ_CHKERR(err) && b;
}

void QualityMetric::change_metric_type(MetricType /*t*/, MsqError &err)
{
  MSQ_SETERR(err)("This QualityMetric's MetricType can not be changed.",
                  MsqError::NOT_IMPLEMENTED);
}


/*! If that function is not over-riden in the concrete class, the base

    Parameters description, see QualityMetric::compute_element_gradient() .

    \return true if the element is valid, false otherwise.
*/
bool QualityMetric::compute_element_analytical_gradient(PatchData &pd,
                                             MsqMeshEntity* element,
                                             MsqVertex* free_vtces[], Vector3D grad_vec[],
                                             int num_free_vtx, double &metric_value,
                                             MsqError &err)
{
  MSQ_PRINT(1)("QualityMetric has no analytical gradient defined. "
                "Defaulting to numerical gradient.\n");
  set_gradient_type(NUMERICAL_GRADIENT);
  bool b = compute_element_numerical_gradient(pd, element, free_vtces, grad_vec, num_free_vtx, metric_value, err);
  return !MSQ_CHKERR(err) && b;
}


/*! If that function is not over-riden in the concrete class, the base
  class function makes it default to a numerical hessian.
  
  For parameters description, see QualityMetric::compute_element_hessian() .
  
  \return true if the element is valid, false otherwise. 
*/
bool QualityMetric::compute_element_analytical_hessian(PatchData &pd,
                                             MsqMeshEntity* element,
                                             MsqVertex* free_vtces[], Vector3D grad_vec[],
                                             Matrix3D hessian[],
                                             int num_free_vtx, double &metric_value,
                                             MsqError &err)
{
  MSQ_PRINT(1)("QualityMetric has no analytical hessian defined. "
                "Defaulting to numerical hessian.\n");
  set_hessian_type(NUMERICAL_HESSIAN);
  bool b = compute_element_numerical_hessian(pd, element, free_vtces, grad_vec,
                                           hessian, num_free_vtx, metric_value, err);
  return !MSQ_CHKERR(err) && b;
}


/*!
  Note that for this function, grad_vec should be an array of size the
  number of vertices in el, not of size num_free_vtx.
*/
bool QualityMetric::compute_element_gradient_expanded(PatchData &pd,
                                                      MsqMeshEntity* el,
                                                      MsqVertex* free_vtces[],
                                                      Vector3D grad_vec[],
                                                      int num_free_vtx,
                                                      double &metric_value,
                                                      MsqError &err)
{
  int i, g, e;
  bool ret;
  Vector3D* grad_vec_nz = new Vector3D[num_free_vtx];
  ret = compute_element_gradient(pd, el, free_vtces, grad_vec_nz,
                                 num_free_vtx, metric_value, err);
  if (MSQ_CHKERR(err)) {
    delete [] grad_vec_nz;
    return false;
  }

  msq_std::vector<size_t> gv_i;
  gv_i.reserve(num_free_vtx);
  i=0;
  for (i=0; i<num_free_vtx; ++i) {
    gv_i.push_back( pd.get_vertex_index(free_vtces[i]) );
  }
     
  msq_std::vector<size_t> ev_i;
  el->get_vertex_indices(ev_i);

  bool inc;
  msq_std::vector<size_t>::iterator ev;
  msq_std::vector<size_t>::iterator gv;
  for (ev=ev_i.begin(), e=0; ev!=ev_i.end(); ++ev, ++e) {
    inc = false; g=0;
    gv = gv_i.begin();
    while (gv!=gv_i.end()) {
      if (*ev == *gv) {
        inc = true;
        break;
      }
      ++gv; ++g;
    }
    if (inc == true)
      grad_vec[e] = grad_vec_nz[g];
    else
      grad_vec[e] = 0;
  }
  
  delete []grad_vec_nz;
  return ret;
}
   
   
/*!
  Parameters description, see QualityMetric::compute_element_gradient() .
  
  \return true if the element is valid, false otherwise.
*/
bool QualityMetric::compute_element_numerical_gradient(PatchData &pd,
                                             MsqMeshEntity* element,
                                             MsqVertex* free_vtces[],
                                                       Vector3D grad_vec[],
                                             int num_free_vtx, double &metric_value,
                                             MsqError &err)
{
  MSQ_FUNCTION_TIMER( "QualityMetric::compute_element_numerical_gradient" );
    /*!TODO: (MICHAEL)  Try to inline this function (currenlty conflicts
      with MsqVertex.hpp).*/    
  MSQ_PRINT(3)("Computing Numerical Gradient\n");
  
  bool valid=this->evaluate_element(pd, element, metric_value, err);
  if (MSQ_CHKERR(err) || !valid)
    return false;

  const double delta_C = 10e-6;
  double delta = delta_C;
  const double delta_inv_C = 1. / delta; // avoids division in the loop. 
  double delta_inv = delta_inv_C;
  int counter=0;
  double pos=0.0;
  double metric_value1=0;
  const int reduction_limit = 15;
  for (int v=0; v<num_free_vtx; ++v) 
  {
    /* gradient in the x, y, z direction */
    for (int j=0;j<3;++j) 
    {
        //re-initialize variables.
      valid=false;
      delta = delta_C;
      delta_inv = delta_inv_C;
      counter=0;
        //perturb the node and calculate gradient.  The while loop is a
        //safety net to make sure the epsilon perturbation does not take
        //the element out of the feasible region.
      while(!valid && counter<reduction_limit){
        //save the original coordinate before the perturbation
        pos=(*free_vtces[v])[j];
          // perturb the coordinates of the free vertex in the j direction
          // by delta       
        (*free_vtces[v])[j]+=delta;
          //compute the function at the perturbed point location
        valid=this->evaluate_element(pd, element,  metric_value1, err);
        MSQ_CHKERR(err);
          //compute the numerical gradient
        grad_vec[v][j]=(metric_value1-metric_value)*delta_inv;
          // put the coordinates back where they belong
        (*free_vtces[v])[j] = pos;
        ++counter;
        delta*=0.1;
	delta_inv*=10.;
      }
      if(counter>=reduction_limit){
        MSQ_SETERR(err)("Perturbing vertex by delta caused an inverted element.",
                        MsqError::INTERNAL_ERROR);
        return false;
      }
      
    }
  }
  return true;
}


/*!
  Note that for this function, grad_vec should be an array of size the
  number of vertices in el, not of size num_free_vtx. Entries that do not correspond
  with the vertices argument array will be null.

  For parameters description, see QualityMetric::compute_element_hessian() .
  
  \return true if the element is valid, false otherwise. 
*/
bool QualityMetric::compute_element_numerical_hessian(PatchData &pd,
                                             MsqMeshEntity* element,
                                             MsqVertex* free_vtces[],
                                             Vector3D grad_vec[],
                                             Matrix3D hessian[],
                                             int num_free_vtx, double &metric_value,
                                             MsqError &err)
{
  MSQ_FUNCTION_TIMER( "QualityMetric::compute_element_numerical_hessian" );
  MSQ_PRINT(3)("Computing Numerical Hessian\n");
  
  bool valid=this->compute_element_gradient_expanded(pd, element, free_vtces, grad_vec,
                                    num_free_vtx, metric_value, err); 
  if (MSQ_CHKERR(err) || !valid)
    return false;
  const double delta_C =  10e-6;
  double delta = delta_C;
  const double delta_inv_C = 1./delta;
  double delta_inv = delta_inv_C;
  const int reduction_limit = 15;
  int counter;
  double vj_coord=0.0;
  short nve = element->vertex_count();
  Vector3D* grad_vec1 = new Vector3D[nve];
  Vector3D fd;
  msq_std::vector<size_t> ev_i;
  element->get_vertex_indices(ev_i);
  short w, v, i, j, sum_w, mat_index, k;

  int fv_ind=0; // index in array free_vtces .

  // loop over all vertices in element.
  for (v=0; v<nve; ++v) {
    
    // finds out whether vertex v in the element is fixed or free,
    // as according to argument free_vtces[]
    bool free_vertex = false;
    for (k=0; k<num_free_vtx; ++k) {
      if ( ev_i[v] == pd.get_vertex_index(free_vtces[k]) )
        free_vertex = true;
    }

    // If vertex is fixed, enters null blocks for that column.
    // Note that null blocks for the row will be taken care of by
    // the gradient null entries. 
    if (free_vertex==false) {
      for (w=0; w<nve; ++w) {
        if (v>=w) {
          sum_w = w*(w+1)/2; // 1+2+3+...+w
          mat_index = w*nve+v-sum_w;
          hessian[mat_index] = 0.;
        }
      }
    }
    else  {
    // If vertex is free, use finite difference on the gradient to find the Hessian.
      for (j=0;j<3;++j) {
        counter=0;
        double delta = delta_C;
        delta_inv = delta_inv_C;
        valid = false;
        while (!valid && counter<reduction_limit){
          ++counter;
          
            // perturb the coordinates of the vertex v in the j direction by
            // delta
          vj_coord = (*free_vtces[fv_ind])[j];
          (*free_vtces[fv_ind])[j]+=delta;
            //compute the gradient at the perturbed point location
          valid = this->compute_element_gradient_expanded(pd, element, free_vtces,
                              grad_vec1, num_free_vtx, metric_value, err);
          if( MSQ_CHKERR(err) )
            return false;
          
          if( !valid){
              (*free_vtces[fv_ind])[j]-=delta;
            delta *= 0.1;
            delta_inv *=10.0;
          }          
        }
        if( !valid){
          MSQ_SETERR(err)("Algorithm did not successfully compute element's "
                           "Hessian.\n",MsqError::INTERNAL_ERROR);
          return false;
        }
        //compute the numerical Hessian
        for (w=0; w<nve; ++w) {
          if (v>=w) {
            //finite difference to get some entries of the Hessian
            fd = (grad_vec1[w]-grad_vec[w])*delta_inv;
            // For the block at position w,v in a matrix, we need the corresponding index
            // (mat_index) in a 1D array containing only upper triangular blocks.
            sum_w = w*(w+1)/2; // 1+2+3+...+w
            mat_index = w*nve+v-sum_w;
          
            for (i=0; i<3; ++i)
              hessian[mat_index][i][j] = fd[i];   
     
          }
        }
        // put the coordinates back where they belong
        (*free_vtces[fv_ind])[j] = vj_coord;
      }
      ++fv_ind;
    }
  }

  delete[] grad_vec1;

  return true;
}


/*!  Numerically calculates the gradient of a vertex-based QualityMetric
  value on the given free vertex.  The metric is evaluated at MsqVertex
  'vertex', and the gradient is calculated with respect to the degrees
  of freedom associated with MsqVertices in the 'vertices' array.
*/
bool QualityMetric::compute_vertex_numerical_gradient(PatchData &pd,
                                                      MsqVertex &vertex,
                                                      MsqVertex* free_vtces[],
                                                      Vector3D grad_vec[],
                                                      int num_free_vtx,
                                                      double &metric_value,
                                                      MsqError &err)
{
   /*!TODO: (MICHAEL)  Try to inline this function (currenlty conflicts
      with MsqVertex.hpp).*/    
  MSQ_PRINT(2)("Computing Gradient (QualityMetric's numeric, vertex based.\n");
  
  bool valid=this->evaluate_vertex(pd, &(vertex), metric_value, err);
  if (MSQ_CHKERR(err) || !valid)
    return false;
  
  const double delta = 10e-6;
  const double delta_inv = 1./delta;
  double metric_value1=0;
  double pos;
  int v=0;
  for (v=0; v<num_free_vtx; ++v) 
  {
    /* gradient in the x, y, z direction */
    int j=0;
    for (j=0;j<3;++j) 
    {
      //save the original coordinate before the perturbation
      pos=(*free_vtces[v])[j];
      // perturb the coordinates of the free vertex in the j direction by delta
      (*free_vtces[v])[j]+=delta;
      //compute the function at the perturbed point location
      this->evaluate_vertex(pd, &(vertex),  metric_value1, err); MSQ_ERRZERO(err);
      //compute the numerical gradient
      grad_vec[v][j]=(metric_value1-metric_value)*delta_inv;
      // put the coordinates back where they belong
      (*free_vtces[v])[j] = pos;
    }
  }
  return true;  
}

     
       //!Evaluate the metric for a vertex
bool QualityMetric::evaluate_vertex(PatchData& /*pd*/, MsqVertex* /*vertex*/,
                                  double& /*value*/, MsqError &err)
        {
          MSQ_SETERR(err)("No implementation for a "
                      "vertex-version of this metric.",
                      MsqError::NOT_IMPLEMENTED);
          return false;
        }
     
       //!Evaluate the metric for an element
bool QualityMetric::evaluate_element(PatchData& /*pd*/,
                                   MsqMeshEntity* /*element*/,
                                   double& /*value*/, MsqError &err)
        {
          MSQ_SETERR(err)("No implementation for a element-version of this "
                          "metric.", MsqError::NOT_IMPLEMENTED);
          return false;
        }
        
double QualityMetric::average_corner_gradients( EntityTopology type,
                                  uint32_t fixed_vertices,
                                  unsigned num_corner,
                                  double corner_values[],
                                  const Vector3D corner_grads[],
                                  Vector3D* vertex_grads,
                                  MsqError& err )
{
  const unsigned num_vertex = TopologyInfo::corners( type );
  const unsigned dim = TopologyInfo::dimension(type);
  const unsigned per_vertex = dim+1;
  
  unsigned i, j, num_adj;
  const unsigned *adj_idx, *rev_idx;
  
    // NOTE: This function changes the corner_values array such that
    //       it contains the gradient coefficients.
  double avg = average_metric_and_weights( corner_values, num_corner, err );
  MSQ_ERRZERO(err);

  for (i = 0; i < num_vertex; ++i)
  {
    if (fixed_vertices & (1<<i))  // skip fixed vertices
      continue;
    
    adj_idx = TopologyInfo::adjacent_vertices( type, i, num_adj );
    rev_idx = TopologyInfo::reverse_vertex_adjacency_offsets( type, i, num_adj );
    if (i < num_corner) // not all vertices are corners (e.g. pyramid)
      *vertex_grads = corner_values[i] * corner_grads[per_vertex*i];
    else
      *vertex_grads = 0;
    for (j = 0; j < num_adj; ++j)
    {
      const unsigned v = adj_idx[j], c = rev_idx[j]+1;
      if (v >= num_corner) // if less corners than vertices (e.g. pyramid apex)
        continue;
      *vertex_grads += corner_values[v] * corner_grads[per_vertex*v+c];
    }
    ++vertex_grads;
  }

  return avg;
}

uint32_t QualityMetric::fixed_vertex_bitmap( PatchData& pd,
                                             MsqMeshEntity* elem,
                                             MsqVertex* free_list[],
                                             unsigned num_free )
{
  uint32_t result = ~(uint32_t)0;
  unsigned num_vtx = elem->vertex_count();
  const size_t* vertices = elem->get_vertex_index_array();
  for (unsigned i = num_vtx - 1; i >= 0 && num_free; --i)
  {
    if (&pd.vertex_by_index(vertices[i]) == free_list[num_free-1])
    {
      result &= ~(uint32_t)(1<<i);
      --num_free;
    }
  }
  return result;
}
  

void QualityMetric::zero_fixed_gradients( EntityTopology elem_type,
                                          uint32_t fixed_vertices,
                                          Vector3D grads[] )
{
  const unsigned num_vertex = TopologyInfo::corners( elem_type );
  for (unsigned i = 0; i < num_vertex; ++i)
    if (fixed_vertices & (1 << i))
      grads[i] = 0;
}

void QualityMetric::copy_free_gradients( EntityTopology type,
                                      uint32_t fixed_vertices,
                                      const Vector3D* src_gradients,
                                      Vector3D* tgt_gradients )
{
  const unsigned num_vertex = TopologyInfo::corners( type );
  for (unsigned i = 0; i < num_vertex; ++i)
    if (!(fixed_vertices & (1 << i)))
    {
      *tgt_gradients = src_gradients[i];
      ++tgt_gradients;
    }
}

void QualityMetric::zero_fixed_hessians( EntityTopology elem_type,
                                         uint32_t fixed,
                                         Matrix3D hessians[] )
{
  const unsigned num_vertex = TopologyInfo::corners( elem_type );
  for (unsigned i = 0; i < num_vertex; ++i)
    for (unsigned j = i; j < num_vertex; ++j)
      if (fixed & ((1<<i)|(1<<j)))
        hessians[num_vertex*i - i*(i+1)/2 + j].zero();
}      

double inverse( double d ) { return 1.0 / d; }
double square( double d ) { return d * d ; };
double invsqr( double d ) { return 1.0 / (d*d); }
double invsqrt( double d ) { return 1.0 / ::sqrt(d); }
typedef double (*fptr)(double);

double QualityMetric::average_corner_hessians( EntityTopology type,
                                     uint32_t fixed_vertices,
                                     unsigned num_corner,
                                     const double corner_values[],
                                     const Vector3D corner_grads[],
                                     const Matrix3D corner_hessians[],
                                     Vector3D vertex_grads[],
                                     Matrix3D vertex_hessians[],
                                     MsqError& err )
{
  const unsigned num_vertex = TopologyInfo::corners( type );
  const unsigned num_hess = num_vertex * (num_vertex+1) / 2;
  const unsigned dim = TopologyInfo::dimension(type);
  const unsigned g_per_v = dim+1;
  const unsigned h_per_v = g_per_v * (g_per_v + 1) / 2;
  Matrix3D outer;
  
  unsigned i, j, k, l, num_adj, num_adj2;
  const unsigned *adj_idx, *rev_idx, *adj_idx2, *rev_idx2;
  double avg = 0.0;  // the output average metric value
  const double inv = 1.0 / num_corner;
  
  
    // Initialize all hessians to zero
  for (i = 0; i < num_hess; ++i)
    vertex_hessians[i].zero();
  
  switch (avgMethod)
  {
  case SUM:

    for (i = 0; i < num_corner; ++i)
      avg += corner_values[i];

    for (i = 0; i < num_vertex; ++i)
    {
      if (fixed_vertices & (1<<i))  // skip fixed vertices
      {
        vertex_grads[i] = 0;
        continue;
      }

      adj_idx = TopologyInfo::adjacent_vertices( type, i, num_adj );
      rev_idx = TopologyInfo::reverse_vertex_adjacency_offsets( type, i, num_adj );
      if (i < num_corner) // not all vertices are corners (e.g. pyramid)
        vertex_grads[i] = corner_grads[g_per_v*i];
      else
        vertex_grads[i] = 0;
      for (j = 0; j < num_adj; ++j)
      {
        unsigned v = adj_idx[j], c = rev_idx[j]+1;
        if (v >= num_corner)
          continue; // not all vertices are corners (e.g. pyramid)
        vertex_grads[i] += corner_grads[g_per_v*v+c];
      }
    
        // For each column of the hessians matrix (i->row)
      for (j = i; j < num_vertex; ++j)
      {
        if (fixed_vertices & (1<<j))  // skip fixed vertices
          continue;
       
          // vertices adjacent to adjacent vertex
        adj_idx2 = TopologyInfo::adjacent_vertices( type, j, num_adj2 );
        rev_idx2 = TopologyInfo::reverse_vertex_adjacency_offsets( type, j, num_adj2 );

          // The index of the hessian calculated in this iteration.
        unsigned loc = num_vertex*i - i*(i+1)/2 + j;

          // For this vertex and each adjacent vertex
        for (k = 0; k <= num_adj; k++)
        {
          unsigned v, r, c;  // vertex, and row and column in vertex hessians
          if (k == 0) // for k = 0, this vertex
          {
            v = i;
            r = 0;
          }
          else        // otherwise k-1-th adjacent vertex
          {
            v = adj_idx[k-1];
            r = rev_idx[k-1] + 1;
          }
          
          if (v >= num_corner) // not all vertices are corners (e.g. pyramid)
            continue; 
          else if (j == v) // for this vertex
            c = 0;
          else        // otherwise find offset of shared adjacent vertex
          {
            for (l = 0; l < num_adj2 && adj_idx2[l] != v; ++l);
            if (l == num_adj2)
              continue;
            
            c = rev_idx2[l] + 1;
          }
          
          if (r <= c)
          {
            unsigned h = h_per_v*v + g_per_v*r - r*(r+1)/2 + c;
            vertex_hessians[loc] += corner_hessians[h];
          }
          else
          {
            unsigned h = h_per_v*v + g_per_v*c - c*(c+1)/2 + r;
            vertex_hessians[loc].plus_transpose_equal( corner_hessians[h] );
          }
        } // for (k = adjacent_vertex)
      } // for(j = column)
    } // for (i = vertex)
    
    break;

  
  case SUM_SQUARED:

    for (i = 0; i < num_corner; ++i)
      avg += corner_values[i] * corner_values[i];

    for (i = 0; i < num_vertex; ++i)
    {
      if (fixed_vertices & (1<<i))  // skip fixed vertices
      {
        vertex_grads[i] = 0;
        continue;
      }

      adj_idx = TopologyInfo::adjacent_vertices( type, i, num_adj );
      rev_idx = TopologyInfo::reverse_vertex_adjacency_offsets( type, i, num_adj );
      if (i < num_corner) // not all vertices are corners (e.g. pyramid)
        vertex_grads[i] = 2.0 * corner_values[i] * corner_grads[g_per_v*i];
      else
        vertex_grads[i] = 0;
      for (j = 0; j < num_adj; ++j)
      {
        unsigned v = adj_idx[j], c = rev_idx[j]+1;
        if (v >= num_corner)
          continue; // not all vertices are corners (e.g. pyramid)
        vertex_grads[i] += 2.0 * corner_values[v] * corner_grads[g_per_v*v+c];
      }
    
        // For each column of the hessians matrix (i->row)
      for (j = i; j < num_vertex; ++j)
      {
        if (fixed_vertices & (1<<j))  // skip fixed vertices
          continue;
       
          // vertices adjacent to adjacent vertex
        adj_idx2 = TopologyInfo::adjacent_vertices( type, j, num_adj2 );
        rev_idx2 = TopologyInfo::reverse_vertex_adjacency_offsets( type, j, num_adj2 );

          // The index of the hessian calculated in this iteration.
        unsigned loc = num_vertex*i - i*(i+1)/2 + j;
        
          // For this vertex and each adjacent vertex
        for (k = 0; k <= num_adj; k++)
        {
          unsigned v, r, c;  // vertex, and row and column in vertex hessians
          if (k == 0) // for k = 0, this vertex
          {
            v = i;
            r = 0;
          }
          else        // otherwise k-1-th adjacent vertex
          {
            v = adj_idx[k-1];
            r = rev_idx[k-1] + 1;
          }
          
          if (v >= num_corner) // not all vertices are corners (e.g. pyramid)
            continue; 
          else if (j == v) // for this vertex
            c = 0;
          else        // otherwise find offset of shared adjacent vertex
          {
            for (l = 0; l < num_adj2 && adj_idx2[l] != v; ++l);
            if (l == num_adj2)
              continue;
            
            c = rev_idx2[l] + 1;
          }
          
          outer.outer_product( corner_grads[g_per_v*v+r], corner_grads[g_per_v*v+c] );
          if (r <= c)
          {
            unsigned h = h_per_v*v + g_per_v*r - r*(r+1)/2 + c;
            outer += corner_values[v]*corner_hessians[h];
          }
          else
          {
            unsigned h = h_per_v*v + g_per_v*c - c*(c+1)/2 + r;
            outer.plus_transpose_equal( corner_values[v]*corner_hessians[h] );
          }
          outer *= 2.0;
          vertex_hessians[loc] += outer;
        } // for (k = adjacent_vertex)
      } // for(j = column)
    } // for (i = vertex)
    
    break;

  
  case LINEAR:

    for (i = 0; i < num_corner; ++i)
      avg += corner_values[i];
    avg *= inv;
    
    for (i = 0; i < num_vertex; ++i)
    {
      if (fixed_vertices & (1<<i))  // skip fixed vertices
      {
        vertex_grads[i] = 0;
        continue;
      }

      adj_idx = TopologyInfo::adjacent_vertices( type, i, num_adj );
      rev_idx = TopologyInfo::reverse_vertex_adjacency_offsets( type, i, num_adj );
      if (i < num_corner) // not all vertices are corners (e.g. pyramid)
        vertex_grads[i] = corner_grads[g_per_v*i];
      else
        vertex_grads[i] = 0;
      for (j = 0; j < num_adj; ++j)
      {
        unsigned v = adj_idx[j], c = rev_idx[j]+1;
        if (v >= num_corner)
          continue; // not all vertices are corners (e.g. pyramid)
        vertex_grads[i] += corner_grads[g_per_v*v+c];
      }
      vertex_grads[i] *= inv;
    
        // For each column of the hessians matrix (i->row)
      for (j = i; j < num_vertex; ++j)
      {
        if (fixed_vertices & (1<<j))  // skip fixed vertices
          continue;
       
          // vertices adjacent to adjacent vertex
        adj_idx2 = TopologyInfo::adjacent_vertices( type, j, num_adj2 );
        rev_idx2 = TopologyInfo::reverse_vertex_adjacency_offsets( type, j, num_adj2 );

          // The index of the hessian calculated in this iteration.
        unsigned loc = num_vertex*i - i*(i+1)/2 + j;
        
          // For this vertex and each adjacent vertex
        for (k = 0; k <= num_adj; k++)
        {
          unsigned v, r, c;  // vertex, and row and column in vertex hessians
          if (k == 0) // for k = 0, this vertex
          {
            v = i;
            r = 0;
          }
          else        // otherwise k-1-th adjacent vertex
          {
            v = adj_idx[k-1];
            r = rev_idx[k-1] + 1;
          }
          
          if (v >= num_corner) // not all vertices are corners (e.g. pyramid)
            continue; 
          else if (j == v) // for this vertex
            c = 0;
          else        // otherwise find offset of shared adjacent vertex
          {
            for (l = 0; l < num_adj2 && adj_idx2[l] != v; ++l);
            if (l == num_adj2)
              continue;
            
            c = rev_idx2[l] + 1;
          }
          
          if (r <= c)
          {
            unsigned h = h_per_v*v + g_per_v*r - r*(r+1)/2 + c;
            vertex_hessians[loc] += corner_hessians[h];
          }
          else
          {
            unsigned h = h_per_v*v + g_per_v*c - c*(c+1)/2 + r;
            vertex_hessians[loc].plus_transpose_equal( corner_hessians[h] );
          }
        } // for (k = adjacent_vertex)
          
        vertex_hessians[loc] *= inv;
      } // for(j = column)
    } // for (i = vertex)
    
    break;


  default: // common case for RMS, Harmonic, & HMS - handle error case here too.
  {
    double p, t;
    double g_factor[8], h_factor[8];

      // calculate average metric value and fill g_factor and h_factor
    switch( avgMethod )
    {
      case RMS:
        p = 2.0;
        t = 2.0 * inv;
        for (i = 0; i < num_corner; ++i)
        {
          avg += corner_values[i] * corner_values[i];
          g_factor[i] = t * corner_values[i];
          h_factor[i] = t;
        }
        t = inv * avg;
        avg = msq_stdc::sqrt( t );
        break;

      case HARMONIC:
        p = -1.0;
        for (i = 0; i < num_corner; ++i)
        {
          t = 1.0 / corner_values[i];
          avg += t;
          g_factor[i] = -inv * t * t;
          h_factor[i] = -2.0 * g_factor[i] * t;
        }
        t = inv * avg;
        avg = 1.0 / t;
        break;

      case HMS:
        p = -2.0;
        for (i = 0; i < num_corner; ++i)
        {
          t = 1.0 / corner_values[i];
          avg += t * t;
	        g_factor[i] = -2.0 * inv * t * t * t;
	        h_factor[i] = -3.0 * g_factor[i] * t;
        }
        t = inv * avg;
        avg = 1.0 / msq_stdc::sqrt( t );
        break;

      default:
        MSQ_SETERR(err)("averaging method not available.",MsqError::INVALID_STATE);
        return 0.0;
    }
    
      // average gradients
    for (i = 0; i < num_vertex; ++i)
    {
      if (fixed_vertices & (1<<i))  // skip fixed vertices
      {
        vertex_grads[i] = 0;
        continue;
      }

      adj_idx = TopologyInfo::adjacent_vertices( type, i, num_adj );
      rev_idx = TopologyInfo::reverse_vertex_adjacency_offsets( type, i, num_adj );
      if (i < num_corner) // not all vertices are corners (e.g. pyramid)
        vertex_grads[i] = g_factor[i] * corner_grads[g_per_v*i];
      else
        vertex_grads[i] = 0;
      for (j = 0; j < num_adj; ++j)
      {
        unsigned v = adj_idx[j], c = rev_idx[j]+1;
        if (v >= num_corner)
          continue; // not all vertices are corners (e.g. pyramid)
        vertex_grads[i] += g_factor[v] * corner_grads[g_per_v*v+c];
      }
    }
    
      // average Hessians
    const double f1 = avg / ( p * t );
    const double f2 = (1.0 / p - 1) * f1 / t;
    for (i = 0; i < num_vertex; ++i)
    {
      if (fixed_vertices & (1<<i))  // skip fixed vertices
        continue;

      adj_idx = TopologyInfo::adjacent_vertices( type, i, num_adj );
      rev_idx = TopologyInfo::reverse_vertex_adjacency_offsets( type, i, num_adj );
    
        // For each column of the hessians matrix (i->row)
      for (j = i; j < num_vertex; ++j)
      {
        if (fixed_vertices & (1<<j))  // skip fixed vertices
          continue;
       
          // vertices adjacent to adjacent vertex
        adj_idx2 = TopologyInfo::adjacent_vertices( type, j, num_adj2 );
        rev_idx2 = TopologyInfo::reverse_vertex_adjacency_offsets( type, j, num_adj2 );

          // The index of the hessian calculated in this iteration.
        unsigned loc = num_vertex*i - i*(i+1)/2 + j;
        
          // For this vertex and each adjacent vertex
        for (k = 0; k <= num_adj; k++)
        {
          unsigned v, r, c;  // vertex, and row and column in vertex hessians
          if (k == 0) // for k = 0, this vertex
          {
            v = i;
            r = 0;
          }
          else        // otherwise k-1-th adjacent vertex
          {
            v = adj_idx[k-1];
            r = rev_idx[k-1] + 1;
          }
          
          if (v >= num_corner) // not all vertices are corners (e.g. pyramid)
            continue; 
          else if (j == v) // for this vertex
            c = 0;
          else        // otherwise find offset of shared adjacent vertex
          {
            for (l = 0; l < num_adj2 && adj_idx2[l] != v; ++l);
            if (l == num_adj2)
              continue;
            
            c = rev_idx2[l] + 1;
          }
          
          outer.outer_product( corner_grads[g_per_v*v+r], corner_grads[g_per_v*v+c] );
          outer *= h_factor[v];
          if (r <= c)
          {
            unsigned h = h_per_v*v + g_per_v*r - r*(r+1)/2 + c;
            outer += g_factor[v]*corner_hessians[h];
          }
          else
          {
            unsigned h = h_per_v*v + g_per_v*c - c*(c+1)/2 + r;
            outer.plus_transpose_equal( g_factor[v]*corner_hessians[h] );
          }
          vertex_hessians[loc] += outer;
          
        } // for (k = adjacent_vertex)
        
        vertex_hessians[loc] *= f1;
        outer.outer_product( vertex_grads[i], vertex_grads[j] );
        outer *= f2;
        vertex_hessians[loc] += outer;
        
      } // for(j = column)
      
      vertex_grads[i] *= f1;
    } // for (i = vertex)
  } // block for "default" case
  break;
  
  } // outer switch
  
  return avg;
}


double QualityMetric::average_metric_and_weights( double metrics[],
                                                  int count, 
                                                  MsqError& err )
{
  double avg = 0.0;
  int i, tmp_count;
  double f;
  
  switch (avgMethod)
  {
  
  case MINIMUM:
    avg = metrics[0];
    for (i = 1; i < count; ++i)
      if (metrics[i] < avg)
        avg = metrics[i];
    
    tmp_count = 0;
    for (i = 0; i < count; ++i)
    {
      if( metrics[i] - avg <= MSQ_MIN )
      {
        metrics[i] = 1.0;
        ++tmp_count;
      }
      else
      {
        metrics[i] = 0.0;
      }
    }
    
    f = 1.0 / tmp_count;
    for (i = 0; i < count; ++i)
      metrics[i] *= f;
      
    break;

  
  case MAXIMUM:
    avg = metrics[0];
    for (i = 1; i < count; ++i)
      if (metrics[i] > avg)
        avg = metrics[i];
    
    tmp_count = 0;
    for (i = 0; i < count; ++i)
    {
      if( avg - metrics[i] <= MSQ_MIN )
      {
        metrics[i] = 1.0;
        ++tmp_count;
      }
      else
      {
        metrics[i] = 0.0;
      }
    }
    
    f = 1.0 / tmp_count;
    for (i = 0; i < count; ++i)
      metrics[i] *= f;
      
    break;

  
  case SUM:
    for (i = 0; i < count; ++i)
    {
      avg += metrics[i];
      metrics[i] = 1.0;
    }
      
    break;

  
  case SUM_SQUARED:
    for (i = 0; i < count; ++i)
    {
      avg += (metrics[i]*metrics[i]);
      metrics[i] *= 2;
    }
      
    break;

  
  case LINEAR:
    f = 1.0 / count;
    for (i = 0; i < count; ++i)
    {
      avg += metrics[i];
      metrics[i] = f;
    }
    avg *= f;
      
    break;

  
  case GEOMETRIC:
    for (i = 0; i < count; ++i)
      avg += log(metrics[i]);
    avg = exp( avg/count );
    
    f = avg / count;
    for (i = 0; i < count; ++i)
      metrics[i] = f / metrics[i];
      
    break;

  
  case RMS:
    for (i = 0; i < count; ++i)
      avg += metrics[i] * metrics[i];
    avg = sqrt( avg / count );
    
    f = 1. / (avg*count);
    for (i = 0; i < count; ++i)
      metrics[i] *= f;
      
    break;

  
  case HARMONIC:
    for (i = 0; i < count; ++i)
      avg += 1.0 / metrics[i];
    avg = count / avg;
  
    for (i = 0; i < count; ++i)
      metrics[i] = (avg * avg) / (count * metrics[i] * metrics[i]);
      
    break;

  
  case HMS:
    for (i = 0; i < count; ++i)
      avg += 1. / (metrics[i] * metrics[i]);
    avg = sqrt( count / avg );
    
    f = avg*avg*avg / count;
    for (i = 0; i < count; ++i)
      metrics[i] = f / (metrics[i] * metrics[i] * metrics[i]);
      
    break;

  
  default:
    MSQ_SETERR(err)("averaging method not available.",MsqError::INVALID_STATE);
  }
  
  return avg;
}

   

   /*! 
     average_metrics takes an array of length num_value and averages the
     contents using averaging method 'method'.
   */
double QualityMetric::average_metrics(const double metric_values[],
                                        const int& num_values, MsqError &err)
{
    //MSQ_MAX needs to be made global?
  //double MSQ_MAX=1e10;
  double total_value=0.0;
  double temp_value=0.0;
  int i=0;
  int j=0;
    //if no values, return zero
  if (num_values<=0){
    return 0.0;
  }

  switch(avgMethod){
    case GEOMETRIC:
       total_value=1.0;
       for (i=0;i<num_values;++i){
         total_value*=(metric_values[i]);
       }
       total_value=pow(total_value, (1/((double) num_values)));
       break;

    case HARMONIC:
         //ensure no divide by zero, return zero
       for (i=0;i<num_values;++i){
         if(metric_values[i]<MSQ_MIN){
           if(metric_values[i]>MSQ_MIN){
             return 0.0;
           }
         }
         total_value+=(1/metric_values[i]);
       }
         //ensure no divide by zero, return MSQ_MAX_CAP
       if(total_value<MSQ_MIN){
         if(total_value>MSQ_MIN){
           return MSQ_MAX_CAP;
         }
       }
       total_value=num_values/total_value;
       break;

    case LINEAR:
       for (i=0;i<num_values;++i){
         total_value+=metric_values[i];
       }
       total_value/= (double) num_values;
       break;

    case MAXIMUM:
       total_value = metric_values[0];
       for (i = 1; i < num_values; ++i){
         if (metric_values[i] > total_value){
           total_value = metric_values[i];
         }
       }
       break;

    case MINIMUM:
       total_value = metric_values[0];
       for (i = 1; i < num_values; ++i){
         if (metric_values[i] < total_value) {
           total_value = metric_values[i];
         }
       }
       break;

    case NONE:
       MSQ_SETERR(err)("Averaging method set to NONE", MsqError::INVALID_ARG);
       break;

    case RMS:
       for (i=0;i<num_values;++i){
         total_value+=(metric_values[i]*metric_values[i]);
       }
       total_value/= (double) num_values;
       total_value=sqrt(total_value);
       break;

    case HMS:
       //ensure no divide by zero, return zero
       for (i=0;i<num_values;++i){
         if (metric_values[i]*metric_values[i] < MSQ_MIN) {
           return 0.0;
         }
         total_value += (1.0/(metric_values[i]*metric_values[i]));
       }

       //ensure no divide by zero, return MSQ_MAX_CAP
       if (total_value < MSQ_MIN) {
         return MSQ_MAX_CAP;
       }
       total_value = sqrt(num_values/total_value);
       break;

    case STANDARD_DEVIATION:
       total_value=0;
       temp_value=0;
       for (i=0;i<num_values;++i){
         temp_value+=metric_values[i];
         total_value+=(metric_values[i]*metric_values[i]);
       }
       temp_value/= (double) num_values;
       temp_value*=temp_value;
       total_value/= (double) num_values;
       total_value-=temp_value;
       break;

    case SUM:
       for (i=0;i<num_values;++i){
         total_value+=metric_values[i];
       }
       break;

    case SUM_SQUARED:
       for (i=0;i<num_values;++i){
         total_value+= (metric_values[i]*metric_values[i]);
       }
       break;

    case MAX_MINUS_MIN:
       //total_value used to store the maximum
         //temp_value used to store the minimum
       temp_value=MSQ_MAX_CAP;
       for (i=0;i<num_values;++i){
         if(metric_values[i]<temp_value){
           temp_value=metric_values[i];
         }
         if(metric_values[i]>total_value){
           total_value=metric_values[i];
         }
       }

         //ensure no divide by zero, return MSQ_MAX_CAP
       if (temp_value < MSQ_MIN) {
         return MSQ_MAX_CAP;
       }
       total_value-=temp_value;
       break;

    case MAX_OVER_MIN:
         //total_value used to store the maximum
         //temp_value used to store the minimum
       temp_value=MSQ_MAX_CAP;
       for (i=0;i<num_values;++i){
         if(metric_values[i]<temp_value){
           temp_value=metric_values[i];
         }
         if(metric_values[i]>total_value){
           total_value=metric_values[i];
         }
       }

         //ensure no divide by zero, return MSQ_MAX_CAP
       if (temp_value < MSQ_MIN) {
         return MSQ_MAX_CAP;
       }
       total_value/=temp_value;
       break;

    case SUM_OF_RATIOS_SQUARED:
       for (j=0;j<num_values;++j){
         //ensure no divide by zero, return MSQ_MAX_CAP
         if (metric_values[j] < MSQ_MIN) {
           return MSQ_MAX_CAP;
         }
         for (i=0;i<num_values;++i){
           total_value+=((metric_values[i]/metric_values[j])*
                         (metric_values[i]/metric_values[j]));
         }
       }
       total_value/=(double)(num_values*num_values);
       break;

    default:
         //Return error saying Averaging Method mode not implemented
       MSQ_SETERR(err)("Requested Averaging Method Not Implemented", MsqError::NOT_IMPLEMENTED);
       return 0;
  }
  return total_value;
}



double QualityMetric::weighted_average_metrics(const double coef[],
                                                      const double metric_values[],
                                                      const int& num_values, MsqError &err)
{
  //MSQ_MAX needs to be made global?
  //double MSQ_MAX=1e10;
  double total_value=0.0;
  int i=0;
  //if no values, return zero
  if (num_values<=0){
    return 0.0;
  }

  switch(avgMethod){

  case LINEAR:
    for (i=0;i<num_values;++i){
      total_value += coef[i]*metric_values[i];
    }
    total_value /= (double) num_values;
    break;

  default:
    //Return error saying Averaging Method mode not implemented
    MSQ_SETERR(err)("Requested Averaging Method Not Implemented",MsqError::NOT_IMPLEMENTED);
    return 0;
  }
  return total_value;
}
   
