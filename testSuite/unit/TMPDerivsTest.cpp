/* ***************************************************************** 
    MESQUITE -- The Mesh Quality Improvement Toolkit

    Copyright 2009 Sandia Corporation and Argonne National
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
 
    jason@kraftcheck.com    
   
  ***************************************************************** */
/** \file TMPDerivsTest.cpp
 * \autor Jason Kraftcheck <jason@kraftcheck.com>
 */


#include "TMPDerivs.hpp"
#include "UnitUtil.hpp"

using namespace Mesquite;

class TMPDerivsTest : public CppUnit::TestFixture
{
private:
  CPPUNIT_TEST_SUITE(TMPDerivsTest);
  CPPUNIT_TEST (test_set_scaled_I);
  CPPUNIT_TEST (test_pluseq_scaled_I);
  CPPUNIT_TEST (test_pluseq_scaled_2nd_deriv_of_det);
  CPPUNIT_TEST (test_set_scaled_2nd_deriv_of_det);
  CPPUNIT_TEST (test_pluseq_scaled_outer_product);
  CPPUNIT_TEST (test_set_scaled_outer_product);
  CPPUNIT_TEST (test_pluseq_scaled_sum_outer_product);
  CPPUNIT_TEST (test_pluseq_scaled_sum_outer_product_I);
  CPPUNIT_TEST_SUITE_END();

public:
  void test_set_scaled_I();
  void test_pluseq_scaled_I();
  void test_pluseq_scaled_2nd_deriv_of_det();
  void test_set_scaled_2nd_deriv_of_det();
  void test_pluseq_scaled_outer_product();
  void test_set_scaled_outer_product();
  void test_pluseq_scaled_sum_outer_product();
  void test_pluseq_scaled_sum_outer_product_I();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TMPDerivsTest, "TMPDerivsTest");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TMPDerivsTest, "Unit");

void TMPDerivsTest::test_set_scaled_I()
{
  const double alpha = 3.14159, eps = 1e-12;
  MsqMatrix<3,3> R[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  set_scaled_I( R, alpha );
  
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(alpha)), R[0], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(0.0  )), R[1], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(0.0  )), R[2], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(alpha)), R[3], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(0.0  )), R[4], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(alpha)), R[5], eps );
}
  

void TMPDerivsTest::test_pluseq_scaled_I()
{
  const double s = 3.14159, eps = 1e-12, alpha = 0.222;
  MsqMatrix<3,3> R[6];
  for (int i = 0; i < 6; ++i)
    R[i] = MsqMatrix<3,3>( s * i );
  pluseq_scaled_I( R, alpha );

  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(0*s+alpha)), R[0], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(1*s      )), R[1], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(2*s      )), R[2], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(3*s+alpha)), R[3], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(4*s      )), R[4], eps );
  ASSERT_MATRICES_EQUAL( (MsqMatrix<3,3>(5*s+alpha)), R[5], eps );
}

void TMPDerivsTest::test_pluseq_scaled_2nd_deriv_of_det()
{
  const double a = 1.33;
  const double e = 1e-12;
  MsqMatrix<3,3> T;
  T(0,0) = 11;
  T(0,1) = 12;
  T(0,2) = 13;
  T(1,0) = 21;
  T(1,1) = 22;
  T(1,2) = 23;
  T(2,0) = 31;
  T(2,1) = 32;
  T(2,2) = 33;
  MsqMatrix<3,3> R[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  pluseq_scaled_2nd_deriv_of_det( R, a, T );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[1](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  33 * a, R[1](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -32 * a, R[1](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -33 * a, R[1](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[1](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  31 * a, R[1](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  32 * a, R[1](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -31 * a, R[1](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[1](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[2](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -23 * a, R[2](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  22 * a, R[2](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  23 * a, R[2](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[2](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -21 * a, R[2](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -22 * a, R[2](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  21 * a, R[2](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[2](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[4](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  13 * a, R[4](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -12 * a, R[4](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -13 * a, R[4](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[4](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  11 * a, R[4](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  12 * a, R[4](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -11 * a, R[4](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[4](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](2,2), e );
  }


void TMPDerivsTest::test_set_scaled_2nd_deriv_of_det()
{
  const double a = 0.7;
  const double e = 1e-12;
  MsqMatrix<3,3> T;
  T(0,0) = 11;
  T(0,1) = 12;
  T(0,2) = 13;
  T(1,0) = 21;
  T(1,1) = 22;
  T(1,2) = 23;
  T(2,0) = 31;
  T(2,1) = 32;
  T(2,2) = 33;
  MsqMatrix<3,3> R[6];
  for (int i = 0; i < 6; ++i)
    for (int r = 0; r < 3; ++r)
      for (int c = 0; c < 3; ++c)
        R[i](r,c) = 9*i+3*r+c;
        
  set_scaled_2nd_deriv_of_det( R, a, T );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[0](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[1](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  33 * a, R[1](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -32 * a, R[1](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -33 * a, R[1](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[1](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  31 * a, R[1](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  32 * a, R[1](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -31 * a, R[1](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[1](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[2](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -23 * a, R[2](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  22 * a, R[2](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  23 * a, R[2](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[2](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -21 * a, R[2](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -22 * a, R[2](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  21 * a, R[2](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[2](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[3](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[4](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  13 * a, R[4](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -12 * a, R[4](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -13 * a, R[4](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[4](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  11 * a, R[4](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(  12 * a, R[4](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -11 * a, R[4](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL(     0.0, R[4](2,2), e );

  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](0,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](0,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](0,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](1,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](1,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](1,2), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](2,0), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](2,1), e );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, R[5](2,2), e );
}

void TMPDerivsTest::test_pluseq_scaled_outer_product()
{
  const double vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  const MsqMatrix<3,3> M(vals);
  const double a = 0.1212;
  const double e = 1e-12;
  
  MsqMatrix<3,3> R[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  pluseq_scaled_outer_product( R, a, M );
  
#ifdef MSQ_ROW_BASED_OUTER_PRODUCT
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(0)) * M.row(0), R[0], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(0)) * M.row(1), R[1], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(0)) * M.row(2), R[2], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(1)) * M.row(1), R[3], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(1)) * M.row(2), R[4], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(2)) * M.row(2), R[5], e );
#else
  ASSERT_MATRICES_EQUAL( a * M.column(0) * transpose(M.column(0)), R[0], e );
  ASSERT_MATRICES_EQUAL( a * M.column(0) * transpose(M.column(1)), R[1], e );
  ASSERT_MATRICES_EQUAL( a * M.column(0) * transpose(M.column(2)), R[2], e );
  ASSERT_MATRICES_EQUAL( a * M.column(1) * transpose(M.column(1)), R[3], e );
  ASSERT_MATRICES_EQUAL( a * M.column(1) * transpose(M.column(2)), R[4], e );
  ASSERT_MATRICES_EQUAL( a * M.column(2) * transpose(M.column(2)), R[5], e );
#endif
}

void TMPDerivsTest::test_set_scaled_outer_product()
{
  const double vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  const MsqMatrix<3,3> M(vals);
  const double a = 0.1212;
  const double e = 1e-12;
  
  MsqMatrix<3,3> R[6];
  for (int i = 0; i < 6; ++i)
    for (int r = 0; r < 3; ++r)
      for (int c = 0; c < 3; ++c)
        R[i](r,c) = 9*i+3*r+c;
  set_scaled_outer_product( R, a, M );
  
#ifdef MSQ_ROW_BASED_OUTER_PRODUCT
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(0)) * M.row(0), R[0], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(0)) * M.row(1), R[1], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(0)) * M.row(2), R[2], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(1)) * M.row(1), R[3], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(1)) * M.row(2), R[4], e );
  ASSERT_MATRICES_EQUAL( a * transpose(M.row(2)) * M.row(2), R[5], e );
#else
  ASSERT_MATRICES_EQUAL( a * M.column(0) * transpose(M.column(0)), R[0], e );
  ASSERT_MATRICES_EQUAL( a * M.column(0) * transpose(M.column(1)), R[1], e );
  ASSERT_MATRICES_EQUAL( a * M.column(0) * transpose(M.column(2)), R[2], e );
  ASSERT_MATRICES_EQUAL( a * M.column(1) * transpose(M.column(1)), R[3], e );
  ASSERT_MATRICES_EQUAL( a * M.column(1) * transpose(M.column(2)), R[4], e );
  ASSERT_MATRICES_EQUAL( a * M.column(2) * transpose(M.column(2)), R[5], e );
#endif
}

void TMPDerivsTest::test_pluseq_scaled_sum_outer_product()
{
  const double vals1[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  const double vals2[] = { -1, -2, -3, -4, -5, -6, -7, -8, -9 };
  const MsqMatrix<3,3> A(vals1), B(vals2);
  const double a = 2.3;
  const double e = 1e-12;
  
  MsqMatrix<3,3> R[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  pluseq_scaled_sum_outer_product( R, a, A, B );
  
#ifdef MSQ_ROW_BASED_OUTER_PRODUCT
  const MsqMatrix<3,3> E[6] = {
    transpose(A.row(0)) * B.row(0) + transpose(B.row(0)) * A.row(0),
    transpose(A.row(0)) * B.row(1) + transpose(B.row(0)) * A.row(1),
    transpose(A.row(0)) * B.row(2) + transpose(B.row(0)) * A.row(2),
    transpose(A.row(1)) * B.row(1) + transpose(B.row(1)) * A.row(1),
    transpose(A.row(1)) * B.row(2) + transpose(B.row(1)) * A.row(2),
    transpose(A.row(2)) * B.row(2) + transpose(B.row(2)) * A.row(2) 
  };
#else
  const MsqMatrix<3,3> E[6] = {
    A.column(0) * transpose(B.column(0)) + B.column(0) * transpose(A.column(0)),
    A.column(0) * transpose(B.column(1)) + B.column(0) * transpose(A.column(1)),
    A.column(0) * transpose(B.column(2)) + B.column(0) * transpose(A.column(2)),
    A.column(1) * transpose(B.column(1)) + B.column(1) * transpose(A.column(1)),
    A.column(1) * transpose(B.column(2)) + B.column(1) * transpose(A.column(2)),
    A.column(2) * transpose(B.column(2)) + B.column(2) * transpose(A.column(2)) 
  };
#endif

  ASSERT_MATRICES_EQUAL( a * E[0], R[0], e );
  ASSERT_MATRICES_EQUAL( a * E[1], R[1], e );
  ASSERT_MATRICES_EQUAL( a * E[2], R[2], e );
  ASSERT_MATRICES_EQUAL( a * E[3], R[3], e );
  ASSERT_MATRICES_EQUAL( a * E[4], R[4], e );
  ASSERT_MATRICES_EQUAL( a * E[5], R[5], e );
}

void TMPDerivsTest::test_pluseq_scaled_sum_outer_product_I()
{
  const double e = 1e-12;
  const double vals[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  const MsqMatrix<3,3> A(vals), I(1.0);
  
  MsqMatrix<3,3> R[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  MsqMatrix<3,3> S[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  pluseq_scaled_sum_outer_product( R, 1.0, I, A );
  pluseq_scaled_sum_outer_product_I( S, 1.0, A );
  ASSERT_MATRICES_EQUAL( R[0], S[0], e );
  ASSERT_MATRICES_EQUAL( R[1], S[1], e );
  ASSERT_MATRICES_EQUAL( R[2], S[2], e );
  ASSERT_MATRICES_EQUAL( R[3], S[3], e );
  ASSERT_MATRICES_EQUAL( R[4], S[4], e );
  ASSERT_MATRICES_EQUAL( R[5], S[5], e );
  
  MsqMatrix<3,3> T[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  MsqMatrix<3,3> U[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  pluseq_scaled_sum_outer_product( R, -0.333, I, A );
  pluseq_scaled_sum_outer_product_I( S, -0.333, A );
  ASSERT_MATRICES_EQUAL( T[0], U[0], e );
  ASSERT_MATRICES_EQUAL( T[1], U[1], e );
  ASSERT_MATRICES_EQUAL( T[2], U[2], e );
  ASSERT_MATRICES_EQUAL( T[3], U[3], e );
  ASSERT_MATRICES_EQUAL( T[4], U[4], e );
  ASSERT_MATRICES_EQUAL( T[5], U[5], e );
}