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

/*! \file QualityMetric.hpp
    \brief
Header file for the Mesquite::QualityMetric class

  \author Thomas Leurent
  \author Michael Brewer
  \date   2002-05-01
 */

#ifndef QualityMetric_hpp
#define QualityMetric_hpp

#ifndef MSQ_USE_OLD_C_HEADERS
#include <cmath>
#include <cstring>
#else
#include <math.h>
#include <string.h>
#endif


#include "Mesquite.hpp"
#include "MsqError.hpp"
#include "Vector3D.hpp"
#include "Matrix3D.hpp"

#ifdef _MSC_VER
   typedef unsigned uint32_t;
#elif defined(HAVE_STDINT_H)
#  include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
#  include <inttypes.h>
#endif

namespace Mesquite
{
   
     /*! \class QualityMetric
       \brief Base class for concrete quality metrics.
     */
   class MsqVertex;
   class MsqMeshEntity;
   class PatchData;
   
   class QualityMetric
   {
   protected:
     /*!Constructor defaults concrete QualityMetric's to
       gradType=NUMERCIAL_GRADIENT and negateFlag=1.
       Concrete QualityMetric constructors over-write these defaults
       when appropriate.
       */
     MESQUITE_EXPORT QualityMetric() :
       mType(MT_UNDEFINED),
       gradType(NUMERICAL_GRADIENT),
       hessianType(NUMERICAL_HESSIAN),
       negateFlag(1)
     {}

   public:
       // This is defined in each concrete class.  It isn't virtual, so
       // it doesn't exist in the base class.
       //   static void create_new() = 0;
     
       // virtual destructor ensures use of polymorphism during destruction
     MESQUITE_EXPORT virtual ~QualityMetric()
        {};
     
     
       /*! \enum MetricType
       is a property of the metric. It should be set correctly in the constructor
       of the concrete QualityMetric.
       An example of a (mediocre) VERTEX_BASED metric is the smallest edge
       connected to a vertex.
       An example of a (mediocre) ELEMENT_BASED metric is the aspect ratio of an element.
       */
     enum MetricType
     {
        MT_UNDEFINED,
        VERTEX_BASED,
        ELEMENT_BASED,
        VERTEX_BASED_FREE_ONLY
     };
     
     MESQUITE_EXPORT MetricType get_metric_type() { return mType; }

       /*!AveragingMethod allows you to set how the quality metric values
         attained at each sample point will be averaged together to produce
         a single metric value for an element.
       */
     enum AveragingMethod
     {
        NONE,
        LINEAR,
        RMS,
        HMS,
        MINIMUM,
        MAXIMUM,
        HARMONIC,
        GEOMETRIC,
        SUM,
        SUM_SQUARED,
        GENERALIZED_MEAN,
        STANDARD_DEVIATION,
        MAX_OVER_MIN,
        MAX_MINUS_MIN,
        SUM_OF_RATIOS_SQUARED
     };
     
       /*!Set the averaging method for the quality metric. Current
         options are
         NONE: the values are not averaged,
         GEOMETRIC:  the geometric average,
         HARMONIC:  the harmonic average,
         LINEAR:  the linear average,
         MAXIMUM:  the maximum value,
         MINIMUM:  the minimum value,
         RMS:  the root-mean-squared average,
         HMS:  the harmonic-mean-squared average,
         SUM:  the sum of the values,
         SUM_SQUARED:  the sum of the squares of the values,
         GENERALIZED_MEAN: self explainatory,
         STANDARD_DEVIATION:  the standard deviation squared of the values,
         MAX_MINUS_MIN:  the maximum value minus the minum value,
         MAX_OVER_MIN:  the maximum value divided by the minimum value,
         SUM_OF_RATIOS_SQUARED:  (1/(N^2))*(SUM (SUM (v_i/v_j)^2))
       */
     MESQUITE_EXPORT inline void set_averaging_method(AveragingMethod method, MsqError &err);
     
       /*! Set feasible flag (i.e., does this metric have a feasible region
         that the mesh must maintain.)
       */
     MESQUITE_EXPORT inline void set_feasible_constraint(int alpha)
        { feasible=alpha; }
     
       //!Returns the feasible flag for this metric
     MESQUITE_EXPORT inline int get_feasible_constraint()
        { return feasible; }
     
       //!Sets the name of this metric
     MESQUITE_EXPORT inline void set_name(msq_std::string st)
        { metricName=st; }
     
       //!Returns the name of this metric (as a string).
     MESQUITE_EXPORT inline msq_std::string get_name()
        { return metricName; }

       //!Escobar Barrier Function for Shape and Other Metrics
       // det = signed determinant of Jacobian Matrix at a Vertex
       // delta = scaling parameter
     MESQUITE_EXPORT inline double vertex_barrier_function(double det, double delta) 
            { return 0.5*(det+sqrt(det*det+4*delta*delta)); }
     
       //!Evaluate the metric for a vertex
     MESQUITE_EXPORT virtual bool evaluate_vertex(PatchData& /*pd*/, MsqVertex* /*vertex*/,
                                  double& /*value*/, MsqError &err);
     
       //!Evaluate the metric for an element
     MESQUITE_EXPORT virtual bool evaluate_element(PatchData& /*pd*/,
                                   MsqMeshEntity* /*element*/,
                                   double& /*value*/, MsqError &err);
     
       /*!\enum GRADIENT_TYPE Sets to either NUMERICAL_GRADIENT or
         ANALYTICAL_GRADIENT*/
     enum GRADIENT_TYPE
     {
        NUMERICAL_GRADIENT,
        ANALYTICAL_GRADIENT
     };
     
       //!Sets gradType for this metric.
     MESQUITE_EXPORT void set_gradient_type(GRADIENT_TYPE grad)
        { gradType=grad; }
     
       /*!\enum HESSIAN_TYPE Sets to either NUMERICAL_HESSIAN or
         ANALYTICAL_HESSIAN*/
     enum HESSIAN_TYPE
     {
        NUMERICAL_HESSIAN,
        ANALYTICAL_HESSIAN
     };
     
       //!Sets hessianType for this metric.
     MESQUITE_EXPORT void set_hessian_type(HESSIAN_TYPE ht)
        { hessianType=ht; }
     
       /*!For MetricType == VERTEX_BASED.
         Calls either compute_vertex_numerical_gradient or
         compute_vertex_analytical_gradient for gradType equal
         NUMERICAL_GRADIENT or ANALYTICAL_GRADIENT, respectively.

         \return true if the element is valid, false otherwise. 
       */
     MESQUITE_EXPORT bool compute_vertex_gradient(PatchData &pd,MsqVertex &vertex,
                                  MsqVertex* vertices[],Vector3D grad_vec[],
                                  int num_vtx, double &metric_value,
                                  MsqError &err);
     
       /*! \brief For MetricType == ELEMENT_BASED.
         Calls either compute_element_numerical_gradient() or
         compute_element_analytical_gradient() for gradType equal
         NUMERICAL_GRADIENT or ANALYTICAL_GRADIENT, respectively.
       */
     MESQUITE_EXPORT bool compute_element_gradient(PatchData &pd, MsqMeshEntity* element,
                                   MsqVertex* free_vtces[], Vector3D grad_vec[],
                                   int num_free_vtx, double &metric_value, MsqError &err);

     /*! same as compute_element_gradient(), but fills fixed vertices spots with
       zeros instead of not returning values for fixed vertices. Also, the vertices
       are now ordered according to the element vertices array.
       */
     MESQUITE_EXPORT bool compute_element_gradient_expanded(PatchData &pd, MsqMeshEntity* element,
                                   MsqVertex* free_vtces[], Vector3D grad_vec[],
                                   int num_free_vtx, double &metric_value, MsqError &err);
     
       /*! \brief For MetricType == ELEMENT_BASED.
         Calls either compute_element_numerical_hessian() or
         compute_element_analytical_hessian() for hessianType equal
         NUMERICAL_HESSIAN or ANALYTICAL_HESSIAN, respectively.
       */
     MESQUITE_EXPORT bool compute_element_hessian(PatchData &pd, MsqMeshEntity* element,
                                  MsqVertex* free_vtces[], Vector3D grad_vec[],
                                  Matrix3D hessian[],
                                  int num_free_vtx, double &metric_value, MsqError &err);
     
       /*! Set the value of QualityMetric's negateFlag.  Concrete
         QualityMetrics should set this flag to -1 if the QualityMetric
         needs to be maximized.
       */
     MESQUITE_EXPORT void set_negate_flag(int neg)
        { negateFlag=neg; }
     
       //!Returns negateFlag.
     MESQUITE_EXPORT int get_negate_flag()
        { return negateFlag; }
       /*! This function is user accessible and virtual.  The base
         class implementation sets an error, because many metrics
         will only be defined as Element_based or Vertex_based, and
         this function will not be needed.  Some concrete metrics
         will have both Element_based and Vertex_based definintions,
         and those metrics will re-implement this function to the
         MetricType to be changed to either QualityMetric::VERTEX_BASED
         or QualityMetric::ELEMENT_BASED.*/
     MESQUITE_EXPORT virtual void change_metric_type(MetricType t, MsqError &err);
     
     
  protected:
     
     //! This function should be used in the constructor of every concrete
     //! quality metric. Errors will result if type is left to MT_UNDEFINED.
     void set_metric_type(MetricType t) { mType = t; }
     
     //! average_metrics takes an array of length num_values and averages the
     //! contents using averaging method data member avgMethod .
     double average_metrics(const double metric_values[], const int& num_values,
                            MsqError &err);
                            
     //! Given a list of metric values, calculate the average metric
     //! valude according to the current avgMethod and write into
     //! the passed metric_values array the the value weight/count to
     //! use when averaging gradient vectors for the metric.
     //!\param metric_values : As input, a set of quality metric values
     //!                       to average.  As output, the fraction of
     //!                       the corresponding gradient vector that
     //!                       contributes to the average gradient.
     //!\param num_metric_values The number of values in the passed array.
     double average_metric_and_weights( double metric_values[],
                                        int num_metric_values,
                                        MsqError& err );
     
     /** \brief Average metric values and gradients for per-corner evaluation
      *
      *\param element_type   The element type
      *\param num_corners    The number of corners (e.g. pass 4 for a pyramid
      *                      if the metric couldn't be evaluated for the apex)
      *\param corner_values  An array of metric values, one per element corner
      *\param corner_grads   The corner gradients, 4 for each corner
      *\param vertex_grads   Output.  Gradient at each vertex.
      *\return average metric value for element
      */
     double average_corner_gradients( EntityTopology element_type,
                                  uint32_t fixed_vertices,
                                  unsigned num_corners,
                                  double corner_values[],
                                  const Vector3D corner_grads[],
                                  Vector3D vertex_grads[],
                                  MsqError& err );
     
     /** \brief Average metric values, gradients, an hessians for per-corner evaluation
      *
      *\param element_type   The element type
      *\param num_corners    The number of corners (e.g. pass 4 for a pyramid
      *                      if the metric couldn't be evaluated for the apex)
      *\param corner_values  An array of metric values, one per element corner
      *\param corner_grads   The corner gradients, 4 for each corner
      *\param corner_hessians The hessians, 10 for each corner
      *\param vertex_grads   Output.  Gradient at each vertex.
      *\param vertex_hessians Output.  Hessians.  Length must be (n*(n+1))/2,
      *                       where n is the number of vertices in the element.
      *\return average metric value for element
      */
      double average_corner_hessians( EntityTopology element_type,
                                     uint32_t fixed_vertices,
                                     unsigned num_corners,
                                     const double corner_values[],
                                     const Vector3D corner_grads[],
                                     const Matrix3D corner_hessians[],
                                     Vector3D vertex_grads[],
                                     Matrix3D vertex_hessians[],
                                     MsqError& err );

      /** \brief Set gradient values to zero for fixed vertices.
       *
       * Zero gradients for fixed vertices.
       *\param type            Element type
       *\param fixed_vertices  Bit flags, one per vertex, 1 if
       *                       vertex is fixed.
       *\param gradients       Array of gradients
       */
      static void zero_fixed_gradients( EntityTopology type, 
                                       uint32_t fixed_vertices, 
                                       Vector3D* gradients );
                                       
      /**\brief Copy only gradients for free vertices from one list
       *        to the other.
       *
       */
      static void copy_free_gradients( EntityTopology type,
                                      uint32_t fixed_vertices,
                                      const Vector3D* src_gradients,
                                      Vector3D* tgt_gradients ); 

      /** \brief Set Hessian values to zero for fixed vertices.
       *
       * Zero Hessians for fixed vertices.
       *\param type            Element type
       *\param fixed_vertices  Bit flags, one per vertex, 1 if
       *                       vertex is fixed.
       *\param gradients       Array of gradients
       */
      static void zero_fixed_hessians ( EntityTopology type, 
                                       uint32_t fixed_vertices, 
                                       Matrix3D* hessians );
     
     /** \brief Convert fixed vertex format from list to bit flags
      *
      * Given list of pointers to fixed vertices as passed to
      * evaluation functions, convert to bit flag format used
      * for many utility functions in this class.  Bits correspond
      * to vertices in the canonical vertex order, beginning with
      * the least-significant bit.  The bit is cleared for free
      * vertices and set (1) for fixed vertices.
      */
      static uint32_t fixed_vertex_bitmap( PatchData& pd, 
                                          MsqMeshEntity* elem,
                                          MsqVertex* free_list[],
                                          unsigned num_free );
      
     
     //! takes an array of coefficients and an array of metrics (both of length num_value)
     //! and averages the contents using averaging method 'method'.
      double weighted_average_metrics(const double coef[],
                                    const double metric_values[],
                                    const int& num_values, MsqError &err);
     
       /*!Non-virtual function which numerically computes the gradient
         of a QualityMetric of a given free vertex. This is used by metric
         which mType is VERTEX_BASED. 
         \return true if the element is valid, false otherwise. */
      bool compute_vertex_numerical_gradient(PatchData &pd,
                                            MsqVertex &vertex,
                                            MsqVertex* vertices[],
                                            Vector3D grad_vec[],
                                            int num_vtx,
                                            double &metric_value,
                                            MsqError &err);
     
     
       /*!\brief Non-virtual function which numerically computes the gradient
         of a QualityMetric of a given element for a given set of free vertices
         on that element.
         This is used by metric which mType is ELEMENT_BASED.
         For parameters, see compute_element_gradient() . */
      bool compute_element_numerical_gradient(PatchData &pd, MsqMeshEntity* element,
                                             MsqVertex* free_vtces[], Vector3D grad_vec[],
                                             int num_free_vtx, double &metric_value,
                                             MsqError &err);

     /*! \brief Virtual function that computes the gradient of the QualityMetric
         analytically.  The base class implementation of this function
         simply prints a warning and calls compute_numerical_gradient
         to calculate the gradient. This is used by metric
         which mType is VERTEX_BASED. */
      virtual bool compute_vertex_analytical_gradient(PatchData &pd,
                                                     MsqVertex &vertex,
                                                     MsqVertex* vertices[],
                                                     Vector3D grad_vec[],
                                                     int num_vtx,
                                                     double &metric_value,
                                                     MsqError &err);
     
     
     /*! \brief Virtual function that computes the gradient of the QualityMetric
         analytically.  The base class implementation of this function
         simply prints a warning and calls compute_numerical_gradient
         to calculate the gradient. This is used by metric
         which mType is ELEMENT_BASED.
         For parameters, see compute_element_gradient() . */
      virtual bool compute_element_analytical_gradient(PatchData &pd,
                                                      MsqMeshEntity* element,
                                                      MsqVertex* free_vtces[],
                                                      Vector3D grad_vec[],
                                                      int num_free_vtx,
                                                      double &metric_value,
                                                      MsqError &err);


      bool compute_element_numerical_hessian(PatchData &pd,
                                            MsqMeshEntity* element,
                                            MsqVertex* free_vtces[],
                                            Vector3D grad_vec[],
                                            Matrix3D hessian[],
                                            int num_free_vtx,
                                            double &metric_value,
                                            MsqError &err);

      virtual bool compute_element_analytical_hessian(PatchData &pd,
                                            MsqMeshEntity* element,
                                            MsqVertex* free_vtces[],
                                            Vector3D grad_vec[],
                                            Matrix3D hessian[],
                                            int num_free_vtx,
                                            double &metric_value,
                                            MsqError &err);

     friend class MsqMeshEntity;

     // TODO : pass this private and write protected access fucntions.
     AveragingMethod avgMethod;
     int feasible;
     msq_std::string metricName;
  private:
     MetricType mType;
     GRADIENT_TYPE gradType;
     HESSIAN_TYPE hessianType;
     int negateFlag;
   };

  
  inline void  QualityMetric::set_averaging_method(AveragingMethod method, MsqError &err)
  {
    switch(method)
    {
      case(NONE):
      case(GEOMETRIC):
      case(HARMONIC):
      case(LINEAR):
      case(MAXIMUM):
      case(MINIMUM):
      case(RMS):
      case(HMS):
      case(STANDARD_DEVIATION):
      case(SUM):
      case(SUM_SQUARED):
      case(MAX_OVER_MIN):
      case(MAX_MINUS_MIN):
      case(SUM_OF_RATIOS_SQUARED):
        avgMethod=method;
        break;
      default:
       MSQ_SETERR(err)("Requested Averaging Method Not Implemented", MsqError::NOT_IMPLEMENTED);
      };
    return;
  }
  

/*! 
  \brief Calls compute_vertex_numerical_gradient if gradType equals
  NUMERCIAL_GRADIENT.  Calls compute_vertex_analytical_gradient if 
  gradType equals ANALYTICAL_GRADIENT;
*/
   inline bool QualityMetric::compute_vertex_gradient(PatchData &pd,
                                                      MsqVertex &vertex,
                                                      MsqVertex* vertices[],
                                                      Vector3D grad_vec[],
                                                      int num_vtx,
                                                      double &metric_value,
                                                      MsqError &err)
   {
     bool ret=false;;
     switch(gradType)
     {
       case NUMERICAL_GRADIENT:
          ret = compute_vertex_numerical_gradient(pd, vertex, vertices,
                                                  grad_vec, num_vtx,
                                                  metric_value, err);
          MSQ_CHKERR(err);
          break;
       case ANALYTICAL_GRADIENT:
          ret = compute_vertex_analytical_gradient(pd, vertex, vertices,
                                                   grad_vec,num_vtx,
                                                   metric_value, err);
          MSQ_CHKERR(err);
          break;
     }
     return ret;
   }
   

/*! 
    \param free_vtces base address of an array of pointers to the element vertices which
    are considered free for purposes of computing the gradient. The quality metric
    gradient relative to each of those vertices is computed and stored in grad_vec.
    \param grad_vec base address of an array of Vector3D where the gradient is stored,
    in the order specified by the free_vtces array.
    \param num_free_vtx This is the size of the vertices and gradient arrays. 
    \param metric_value Since the metric is computed, we return it. 
    \return true if the element is valid, false otherwise.
*/
   inline bool QualityMetric::compute_element_gradient(PatchData &pd,
                                                       MsqMeshEntity* el,
                                                       MsqVertex* free_vtces[],
                                                       Vector3D grad_vec[],
                                                       int num_free_vtx,
                                                       double &metric_value,
                                                       MsqError &err)
   {
     bool ret=false;
     switch(gradType)
     {
       case NUMERICAL_GRADIENT:
          ret = compute_element_numerical_gradient(pd, el, free_vtces, grad_vec,
                                                  num_free_vtx, metric_value, err);
          MSQ_CHKERR(err);
          break;
       case ANALYTICAL_GRADIENT:
          ret = compute_element_analytical_gradient(pd, el, free_vtces, grad_vec,
                                                   num_free_vtx, metric_value, err);
          MSQ_CHKERR(err);
          break;
     }
     return ret;
   }
   
} //namespace


#endif // QualityMetric_hpp
