/* ***************************************************************** 
    MESQUITE -- The Mesh Quality Improvement Toolkit

    Copyright 2005 Lawrence Livermore National Laboratory.  Under 
    the terms of Contract B545069 with the University of Wisconsin -- 
    Madison, Lawrence Livermore National Laboratory retains certain
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

    kraftche@cae.wisc.edu    

  ***************************************************************** */

#ifndef MSQ_MESH_WRITER_HPP
#define MSQ_MESH_WRITER_HPP

#include "Matrix3D.hpp"

namespace Mesquite {

class Mesh;
class MsqError;
class PlanarDomain;
class Vector3D;

namespace MeshWriter {

/** \brief Enumeration of principal coordinate axes */
enum Axis {
  X = 0, Y = 1, Z = 2
};

/**\brief Specify a projection to use for output 
 * 
 * This class defines a projection used to transform 
 * R^3 vertex positions to 2D positions to use in graphics
 * file formats.
 */
class Projection {
  public:
      /** Project into specified plane - choice of up direction is arbitrary */
    Projection( PlanarDomain* domain );
      /** Project into plane with specified normal - choice of up direction is arbitrary */
    Projection( const Vector3D& view );
      /** Project points into plane normal to \ref view vector.  Orient
       *  projection such that the projection of the \ref up vector into
       *  the plane is parallel with the vertical direction in the output.
       */
    Projection( const Vector3D& view, const Vector3D& up ); 
      /** Specify which principal axes should be aligned with the 
       *  horizontal and vertical in the output 
       */
    Projection( Axis horizontal, Axis vertical );
  
      /** Project a point into the plane */
    void project( const Vector3D& point, float& horiz, float& vert );
    
    static Matrix3D rotation( const Vector3D& axis, double angle );
  
  private:
    void init( const Vector3D& view );
    void init( const Vector3D& view, const Vector3D& up );
  
    Matrix3D myTransform;  
};

/** \brief Write mesh as gnuplot data
 *
 * Write a file that can be drawn in gnuplot with the command:
 * "plot 'filename' w 1"
 */
void write_gnuplot( Mesh* mesh, const char* filename, MsqError& err );

/** \brief Write mesh as a VTK file
 *
 * Write a simple VTK file for viewing.  The file written by this
 * function is intended for viewing.  It contains only a minimal
 * decription of the mesh.  It does not contain other data such as
 * tags/attributes.  If the Mesh is a MeshImpl, use the VTK writing
 * function provided in MeshImpl for a complete mesh export
 */
void write_vtk( Mesh* mesh, const char* filename, MsqError& err );

/** Convert inches to points */
inline int in2pt( float inches ) { return (int)(inches * 72.0f); }
/** Convert centimeters to points */
inline int cm2pt( float cm     ) { return (int)(cm * 72.0f / 2.54f); } 

/**\brief Write an Encapsulate PostScript file.
 *
 * Write encapsulated postscript.  
 *\param proj - PostScript is a 2-D drawing format.  This argument
 *              specifies which 2-D projection of the 3-D mesh to write.
 *\param width - The width of the output image, in points.
 *\param height - The height of the output image, in points.
 */
void write_eps( Mesh* mesh, 
                const char* filename, 
                Projection proj, 
                MsqError& err,
                int width = in2pt( 6.5 ),
                int height = in2pt( 9 ) );

/**\brief Write an SVG file.
 *
 * Write a 2-D projection of the mesh to a Scalable Vector Graphics
 * file. (W3C standard).
 *\param proj - SVG is a 2-D drawing format.  This argument
 *              specifies which 2-D projection of the 3-D mesh to write.
 */
void write_svg( Mesh* mesh, 
                const char* filename, 
                Projection proj,
                MsqError& err );

/**\brief Write STL
 *
 * Write the mesh as an ASCII STL (Stereo Lithography) file. 
 * The STL format only supports writing triangles.  
 * This writer will write only the triangles contained in the
 * passed mesh.  Any non-triangle elements will be ignored.
 */
void write_stl( Mesh* mesh, const char* filename, MsqError& err ); 

} // namespace MeshWriter

} // namespace Mesquite

#endif
