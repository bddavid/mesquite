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

#ifndef MSQ_MESH_WRITER_CPP
#define MSQ_MESH_WRITER_CPP

#include "MeshWriter.hpp"
#include "Mesquite.hpp"
#include "MeshInterface.hpp"
#include "MsqError.hpp"
#include "PatchData.hpp"
#include "PlanarDomain.hpp"
#include "VtkTypeInfo.hpp"

#ifdef MSQ_USE_OLD_STD_HEADERS
#include <memory.h>
#include <limits.h>
#include <vector.h>
#include <algorithm.h>
#else
#include <memory>
#include <limits>
#include <vector>
#include <algorithm>
#endif

#ifdef MSQ_USE_OLD_IO_HEADERS
#include <fstream.h>
#include <string.h>
#include <iomanip.h>
#else
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;
#endif

namespace Mesquite {

namespace MeshWriter {

/**\brief Transform from coordinates in the XY-plane 
 *        to graphics coordinates.
 */
class Transform2D
{
  public:
  
    Transform2D( PatchData* pd,
                 Projection& proj,
                 unsigned width, 
                 unsigned height,
                 bool flip_about_horizontal );
    
    void transform( const Vector3D& coords,
                    int& horizontal,
                    int& vertical ) const;
    
    int max_horizontal() const { return horizMax; }
    int max_vertical() const   { return vertMax; } 
    
  private:
  
    Projection& myProj;
    float myScale;
    int horizOffset, vertOffset;
    int horizMax, vertMax;
    int vertSign;
};
                             

/**\brief Iterate over all edges in a patch*/
class EdgeIterator
{
public: 
  EdgeIterator( PatchData* patch, MsqError& err );
  bool is_at_end() const;
  const Vector3D& start() const;
  const Vector3D& end() const;
  void step( MsqError& err );
private:
  PatchData* patchPtr;
  size_t vertIdx;
  msq_std::vector<size_t> adjList;
  msq_std::vector<size_t>::iterator adjIter;
  void get_adjacent_vertices( MsqError& err );
};

EdgeIterator::EdgeIterator( PatchData* p, MsqError& err )
  : patchPtr(p),
    vertIdx(0)
{
  p->generate_vertex_to_element_data();
  get_adjacent_vertices( err );
}

bool EdgeIterator::is_at_end() const
{
  return vertIdx >= patchPtr->num_vertices() || 
         adjIter == adjList.end();
}

const Vector3D& EdgeIterator::start() const
  { return patchPtr->vertex_by_index( vertIdx ); }

const Vector3D& EdgeIterator::end() const
  { return patchPtr->vertex_by_index( *adjIter ); }

void EdgeIterator::step( MsqError& err )
{
  if (adjIter != adjList.end())
  {
    ++adjIter;
  }
  
  while (adjIter == adjList.end() && ++vertIdx < patchPtr->num_vertices())
  {
    get_adjacent_vertices( err );  MSQ_ERRRTN(err);
  }
}

void EdgeIterator::get_adjacent_vertices( MsqError& err )
{
  adjList.clear();
  
    // Get all adjacent elements
  size_t num_elem;
  size_t* elems = patchPtr->get_vertex_element_adjacencies( vertIdx, num_elem, err );
  MSQ_ERRRTN(err);
  
    // Get all adjacent vertices from elements
  msq_std::vector<size_t> elem_verts;
  for (size_t e = 0; e < num_elem; ++e)
  {
    MsqMeshEntity& elem = patchPtr->element_by_index(elems[e]);
    EntityTopology type = elem.get_element_type();
    size_t num_edges = TopologyInfo::edges( type );
    
      // For each edge
    for (size_t d = 0; d < num_edges; ++d)
    {
      const unsigned* edge = TopologyInfo::edge_vertices( type, d, err );
      MSQ_ERRRTN(err);
      size_t vert1 = elem.get_vertex_index( edge[0] );
      size_t vert2 = elem.get_vertex_index( edge[1] );

        // If this edge contains the input vertex (vert_idx)
        // AND the input vertex index is less than the 
        // other vertex (avoids iterating over this edge twice)
        // add it to the list.
      if (vert1 > vert2)
      {
        if (vert2 == vertIdx)
          adjList.push_back( vert1 );
      } 
      else 
      {
        if (vert1 == vertIdx)
          adjList.push_back( vert2 );
      }
    }
  }
  
    // Remove duplicates
  msq_std::sort( adjList.begin(), adjList.end() );
  adjIter = msq_std::unique( adjList.begin(), adjList.end() );
  adjList.resize( adjIter - adjList.begin() );
  adjIter = adjList.begin();
}


/**\brief Write VTK file
 *
 * Copied from src/Mesh/MeshSet.cpp and adapted for removal of 
 * MeshSet class by J.Kraftcheck on 2005-7-28.
 *
 * This code is provided mainly for debugging.  A more efficient
 * and complete writer implementation is provided in the MeshImpl
 * class for saving meshes that were read from a file initially.
 */
void write_vtk( Mesh* mesh, const char* out_filename, MsqError &err)
{
    // Open the file
  msq_stdio::ofstream file(out_filename);
  if (!file)
  {
    MSQ_SETERR(err)(MsqError::FILE_ACCESS);
    return;
  }

    // loads a global patch
  PatchData pd;
  pd.set_mesh( mesh );
  pd.fill_global_patch( err ); MSQ_ERRRTN(err);
    
    // Write a header
  file << "# vtk DataFile Version 2.0\n";
  file << "Mesquite Mesh " << out_filename << " .\n";
  file << "ASCII\n";
  file << "DATASET UNSTRUCTURED_GRID\n";
  
    // Write vertex coordinates
  file << "POINTS " << pd.num_nodes() << " float\n";
  size_t i;
  for (i = 0; i < pd.num_nodes(); i++)
  {
    file << pd.vertex_by_index(i)[0] << ' '
         << pd.vertex_by_index(i)[1] << ' '
         << pd.vertex_by_index(i)[2] << '\n';
  }
  
    // Write out the connectivity table
  size_t connectivity_size = 0;
  for (i = 0; i < pd.num_elements(); ++i)
    connectivity_size += pd.element_by_index(i).node_count()+1;
    
  file << "CELLS " << pd.num_elements() << ' ' << connectivity_size << '\n';
  for (i = 0; i < pd.num_elements(); i++)
  {
    msq_std::vector<size_t> vtx_indices;
    pd.element_by_index(i).get_node_indices(vtx_indices);
    
      // Convert native to VTK node order, if not the same
    const VtkTypeInfo* info = VtkTypeInfo::find_type( pd.element_by_index(i).get_element_type(),
                                                      vtx_indices.size(),
                                                      err ); MSQ_ERRRTN(err);
    info->mesquiteToVtkOrder( vtx_indices );
     
    file << vtx_indices.size();
    for (msq_stdc::size_t j = 0; j < vtx_indices.size(); ++j)
    {
      file << ' ' << vtx_indices[j];
    }
    file << '\n';
  }
  
    // Write out the element types
  file << "CELL_TYPES " << pd.num_elements() << '\n';
  for (i = 0; i < pd.num_elements(); i++)
  {
    const VtkTypeInfo* info = VtkTypeInfo::find_type( 
                               pd.element_by_index(i).get_element_type(),
                               pd.element_by_index(i).node_count(),
                               err ); MSQ_ERRRTN(err);
    file << info->vtkType << '\n';
  }
  
    // Write out which points are fixed.
  file << "POINT_DATA " << pd.num_nodes()
       << "\nSCALARS fixed float\nLOOKUP_TABLE default\n";
  for (i = 0; i < pd.num_nodes(); ++i)
  {
    if (pd.vertex_by_index(i).is_free_vertex())
      file << "0\n";
    else
      file << "1\n";
  }
  
    // Close the file
  file.close();
}



/** Writes a gnuplot file directly from the MeshSet.
 *  This means that any mesh imported successfully into Mesquite
 *  can be outputed in gnuplot format.
 *
 *  Within gnuplot, use \b plot 'file1.gpt' w l, 'file2.gpt' w l  
 *   
 *  This is not geared for performance, since it has to load a global Patch from
 *  the mesh to write a mesh file. 
 *
 * Copied from src/Mesh/MeshSet.cpp and adapted for removal of 
 * MeshSet class by J.Kraftcheck on 2005-7-28.
*/
void write_gnuplot( Mesh* mesh, const char* out_filebase, MsqError &err)
{
    // Open the file
  msq_std::string out_filename = out_filebase;
  out_filename += ".gpt";
  msq_stdio::ofstream file(out_filename.c_str());
  if (!file)
  {
    MSQ_SETERR(err)(MsqError::FILE_ACCESS);
    return;
  }

    // loads a global patch
  PatchData pd;
  pd.set_mesh( mesh );
  pd.fill_global_patch( err ); MSQ_ERRRTN(err);
    
    // Write a header
  file << "\n";
  
  for (size_t i=0; i<pd.num_elements(); ++i)
  {
    msq_std::vector<size_t> vtx_indices;
    pd.element_by_index(i).get_node_indices(vtx_indices);
    for (size_t j = 0; j < vtx_indices.size(); ++j)
    {
      file << pd.vertex_by_index(vtx_indices[j])[0] << ' '
           << pd.vertex_by_index(vtx_indices[j])[1] << ' '
           << pd.vertex_by_index(vtx_indices[j])[2] << '\n';
    }
      file << pd.vertex_by_index(vtx_indices[0])[0] << ' '
           << pd.vertex_by_index(vtx_indices[0])[1] << ' '
           << pd.vertex_by_index(vtx_indices[0])[2] << '\n';
    file << '\n';
  }
  
    // Close the file
  file.close();
}

void write_stl( Mesh* mesh, const char* filename, MsqError& err )
{
    // Open the file
  ofstream file(filename);
  if (!file)
  {
    MSQ_SETERR(err)(MsqError::FILE_ACCESS);
    return;
  }
  
    // Write header
  char header[70];
  sprintf( header, "Mesquite%d", rand() );
  file << "solid " << header << endl;
  
  MsqVertex coords[3];
  std::vector<Mesh::VertexHandle> verts(3);
  std::vector<size_t> offsets(2);
  
    // Iterate over all elements
  size_t count = 0;
  ElementIterator* iter = mesh->element_iterator( err ); MSQ_ERRRTN(err);
  msq_std::auto_ptr<ElementIterator> deleter( iter );
  for (; !iter->is_at_end(); iter->operator++())
  {
      // Skip non-triangles
    Mesh::ElementHandle elem = iter->operator*();
    EntityTopology type;
    mesh->elements_get_topologies( &elem, &type, 1, err ); MSQ_ERRRTN(err);
    if (type != TRIANGLE) 
      continue;
    ++count;
    
      // Get vertex coordinates
    mesh->elements_get_attached_vertices( &elem, 1, verts, offsets, err ); MSQ_ERRRTN(err);
    mesh->vertices_get_coordinates( &verts[0], coords, 3, err ); MSQ_ERRRTN(err);
    
      // Get triagnle normal
    Vector3D n = (coords[0] - coords[1]) * (coords[0] - coords[2]);
    n.normalize();
    
      // Write triangle
    file << "facet normal " << n.x() << " " << n.y() << " " << n.z() << endl;
    file << "outer loop" << endl;
    for (unsigned i = 0; i < 3; ++i)
      file << "vertex " << coords[i].x() << " " 
                        << coords[i].y() << " " 
                        << coords[i].z() << endl;
    file << "endloop" << endl;
    file << "endfacet" << endl;
  }
  
  file << "endsolid " << header << endl;
  
  file.close();
  if (count == 0)
  {
    msq_stdc::remove(filename);
    MSQ_SETERR(err)("Mesh contains no triangles", MsqError::INVALID_STATE);
  }
}
  
  

Projection::Projection( PlanarDomain* domain )
{
  Vector3D normal;
  domain->normal_at( 0, normal );
  init( normal );
}

Projection::Projection( const Vector3D& n )
{ init( n ); }

Projection::Projection( const Vector3D& n, const Vector3D& up )
{ init( n, up ); }

Projection::Projection( Axis h, Axis v )
{
  Vector3D horiz(0,0,0), vert(0,0,0);
  horiz[h] = 1.0;
  vert[v] = 1.0;
  init( horiz * vert, vert );
}

void Projection::init( const Vector3D& n )
{
    // Choose an "up" direction

  Axis max = X;
  for (Axis i = Y; i <= Z; i = (Axis)(i+1))
    if (fabs(n[i]) > fabs(n[max]))
      max = i;
  
  Axis up;
  if (max == Y)
    up = Z;
  else 
    up = Y;
  
    // Initialize rotation matrix
  
  Vector3D up_vect(0,0,0);
  up_vect[up] = 1.0;
  init( n, up_vect );
}  

void Projection::init(  const Vector3D& n1, const Vector3D& up1 )
{
  MsqError err;
  const Vector3D n = n1/n1.length();
  const Vector3D u = up1/up1.length();
  
  // Rotate for projection
  const Vector3D z( 0., 0., 1. );
  Vector3D r = n * z;
  double angle = r.interior_angle( n, z, err );
  Matrix3D rot1 = rotation( r, angle );
   
  // In-plane rotation for up vector
  Vector3D pu = u - n * (n % u);
  Vector3D y( 0., 1., 0. );
  angle = z.interior_angle( pu, y, err );
  Matrix3D rot2 = rotation( z, angle );
  
  this->myTransform = rot1 * rot2;
}

Matrix3D Projection::rotation( const Vector3D& axis, double angle )
{
  const double c = cos( angle );
  const double s = sin( angle );
  const double x = axis.x();
  const double y = axis.y();
  const double z = axis.z(); 
  
  const double xform[9] = 
    {    c + x*x*(1-c),  -z*s + x*y*(1-c),  y*s + x*z*(1-c),
       z*s + x*y*(1-c),     c + y*y*(1-c), -x*s + y*z*(1-c),
      -y*s + x*z*(1-c),   x*s + y*z*(1-c),    c + z*z*(1-c) };
  return Matrix3D( xform );
}

void Projection::project( const Vector3D& p, float& h, float& v )
{
  Vector3D pr = myTransform * p;
  h = (float)pr.x();
  v = (float)pr.y();
}

Transform2D::Transform2D( PatchData* pd,
                          Projection& projection,
                          unsigned width, 
                          unsigned height,
                          bool flip )
  : myProj(projection),
    vertSign(flip ? -1 : 1)
{
    // Get the bounding box of the projected points
  float w_max, w_min, h_max, h_min;
  w_max = h_max = -msq_std::numeric_limits<float>::max();
  w_min = h_min =  msq_std::numeric_limits<float>::max();
  MsqError err;
  MsqVertex* verts = pd->get_vertex_array( err );
  const size_t num_vert = pd->num_nodes();
  for (unsigned i = 0; i < num_vert; ++i)
  {
    float w, h;
    myProj.project( verts[i], w, h );
    if (w > w_max) w_max = w;
    if (w < w_min) w_min = w;
    if (h > h_max) h_max = h;
    if (h < h_min) h_min = h;
  }
  
    // Determine the scale factor
  const float w_scale = (float)width  / (w_max - w_min);
  const float h_scale = (float)height / (h_max - h_min);
  myScale = w_scale > h_scale ? h_scale : w_scale;
  
    // Determine offset
  horizOffset = -(int)(myScale * w_min);
  vertOffset  = -(int)(myScale * (flip ? -h_max : h_min));
  
    // Determine bounding box
  horizMax = (int)(                 w_max  * myScale) + horizOffset;
  vertMax  = (int)((flip ? -h_min : h_max) * myScale) +  vertOffset; 
    
}
    
void Transform2D::transform( const Vector3D& coords,
                             int& horizontal,
                             int& vertical ) const
{
    float horiz, vert;
    myProj.project( coords, horiz, vert );
    horizontal =            (int)(myScale * horiz) + horizOffset;
    vertical   = vertSign * (int)(myScale *  vert) +  vertOffset;
}
 
void write_eps( Mesh* mesh, 
                const char* filename, 
                Projection proj, 
                MsqError& err,
                int width, int height )
{
    // Get a global patch
  PatchData pd;
  pd.set_mesh( mesh );
  pd.fill_global_patch( err ); MSQ_ERRRTN(err);
  
  Transform2D transf( &pd, proj, width, height, false );
    
    // Open the file
  ofstream s(filename);
  if (!s)
  {
    MSQ_SETERR(err)(MsqError::FILE_ACCESS);
    return;
  }

    // Write header
  s << "%!PS-Adobe-2.0 EPSF-2.0"                      << endl;
  s << "%%Creator: Mesquite"                          << endl;
  s << "%%Title: Mesquite "                           << endl;
  s << "%%DocumentData: Clean7Bit"                    << endl;
  s << "%%Origin: 0 0"                                << endl;
  s << "%%BoundingBox: 0 0 " 
    << transf.max_horizontal() <<  ' ' 
    << transf.max_vertical()                          << endl;
  s << "%%Pages: 1"                                   << endl;
  
  s << "%%BeginProlog"                                << endl;
  s << "save"                                         << endl;
  s << "countdictstack"                               << endl;
  s << "mark"                                         << endl;
  s << "newpath"                                      << endl;
  s << "/showpage {} def"                             << endl;
  s << "/setpagedevice {pop} def"                     << endl;
  s << "%%EndProlog"                                  << endl;
  
  s << "%%Page: 1 1"                                  << endl;
  s << "1 setlinewidth"                               << endl;
  s << "0.0 setgray"                                  << endl;
  
    // Write mesh edges
  EdgeIterator iter( &pd, err );  MSQ_ERRRTN(err);
  while( !iter.is_at_end() )
  {
    int s_w, s_h, e_w, e_h;
    transf.transform( iter.start(), s_w, s_h );
    transf.transform( iter.end  (), e_w, e_h );
    
    s << "newpath"                                    << endl;
    s << s_w << ' ' << s_h << " moveto"               << endl;
    s << e_w << ' ' << e_h << " lineto"               << endl;
    s << "stroke"                                     << endl;
    
    iter.step(err); MSQ_ERRRTN(err);
  }
  
    // Write footer
  s << "%%Trailer"                                    << endl;
  s << "cleartomark"                                  << endl;
  s << "countdictstack"                               << endl;
  s << "exch sub { end } repeat"                      << endl;
  s << "restore"                                      << endl;  
  s << "%%EOF"                                        << endl;
}

void write_svg( Mesh* mesh, 
                const char* filename, 
                Projection proj, 
                MsqError& err )
{
    // Get a global patch
  PatchData pd;
  pd.set_mesh( mesh );
  pd.fill_global_patch( err ); MSQ_ERRRTN(err);
  
  Transform2D transf( &pd, proj, 400, 400, true );
    
    // Open the file
  ofstream file(filename);
  if (!file)
  {
    MSQ_SETERR(err)(MsqError::FILE_ACCESS);
    return;
  }

    // Write header
  file << "<?xml version=\"1.0\" standalone=\"no\"?>"                << endl;
  file << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" " 
       << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">"    << endl;
  file <<                                                               endl;
  file << "<svg width=\"" << transf.max_horizontal() 
       << "\" height=\"" << transf.max_vertical()
       << "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">" << endl;
  
    // Write mesh edges
  EdgeIterator iter( &pd, err );  MSQ_ERRRTN(err);
  while( !iter.is_at_end() )
  {
    int s_w, s_h, e_w, e_h;
    transf.transform( iter.start(), s_w, s_h );
    transf.transform( iter.end  (), e_w, e_h );
    
    file << "<line "
         << "x1=\"" << s_w << "\" "
         << "y1=\"" << s_h << "\" "
         << "x2=\"" << e_w << "\" "
         << "y2=\"" << e_h << "\" "
         << " style=\"stroke:rgb(99,99,99);stroke-width:2\""
         << "/>" << endl;
    
    iter.step( err ); MSQ_ERRRTN(err);
  }
  
    // Write footer
  file << "</svg>" << endl;
}

} // namespace MeshWriter

} // namespace Mesquite

#endif
