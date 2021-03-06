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
    pknupp@sandia.gov, tleurent@mcs.anl.gov, tmunson@mcs.anl.gov,
    kraftche@cae.wisc.edu
   
  ***************************************************************** */
//
//   SUMMARY: 
//     USAGE:
//
// ORIG-DATE: 16-May-02 at 10:26:21
//  LAST-MOD: 15-Nov-04 by kraftche@cae.wisc.edu
//
/*! \file MeshImpl.cpp

\brief This files contains a mesh database implementation that can be used
to run mesquite by default.
  
    \author Thomas Leurent
    \author Darryl Melander
    \author Jason Kraftcheck
    \date 2004-11-15
 */

#include "MeshImpl.hpp"
#include "FileTokenizer.hpp"
#include "Vector3D.hpp"
#include "MsqVertex.hpp"
#include "MeshImplData.hpp"
#include "MeshImplTags.hpp"
#include "MsqDebug.hpp"
#include "MsqError.hpp"
#include "VtkTypeInfo.hpp"

#ifdef MSQ_USE_OLD_STD_HEADERS
#  include <string.h>
#  include <vector.h>
#  include <algorithm.h>
#  include <functional.h>
#else
#  include <string>
#  include <vector>
#  include <algorithm>
#  include <functional>
   using std::string;
   using std::vector;
#endif

#ifdef MSQ_USE_OLD_IO_HEADERS
#  include <fstream.h>
#  include <iomanip.h>
#else
#  include <fstream>
#  include <iomanip>
   using std::ifstream;
   using std::ofstream;
   using std::endl;
#endif

#ifdef MSQ_USING_EXODUS
#include "exodusII.h"
#endif

#include "MsqDebug.hpp"
namespace Mesquite
{


MeshImpl::MeshImpl() 
    : numCoords(0),
      myMesh( new MeshImplData ),
      myTags( new MeshImplTags )
{}

MeshImpl::~MeshImpl() 
{
  delete myMesh;
  delete myTags;
}

void MeshImpl::clear()
{
  myMesh->clear();
  myTags->clear();
}


void MeshImpl::write_vtk(const char* out_filename, MsqError &err)
{
  ofstream file(out_filename);
  if (!file)
  {
    MSQ_SETERR(err)( MsqError::FILE_ACCESS );
    return;
  }
  
    // Write a header
  file << "# vtk DataFile Version 2.0\n";
  file << "Mesquite Mesh\n";
  file << "ASCII\n";
  file << "DATASET UNSTRUCTURED_GRID\n";
  
    // Write vertex coordinates
  file << "POINTS " << myMesh->num_vertices() << " float\n";

  std::vector<size_t> vertex_indices( myMesh->max_vertex_index() );
  size_t i, count = 0;
  for (i = 0; i < myMesh->max_vertex_index(); ++i)
  {
    if (myMesh->is_vertex_valid(i))
    {
      Vector3D coords = myMesh->get_vertex_coords( i, err ); MSQ_ERRRTN(err);
      file << coords[0] << ' ' << coords[1] << ' ' << coords[2] << '\n';
      vertex_indices[i] = count++;
    }
    else
    {
      vertex_indices[i] = myMesh->max_vertex_index();
    }
  }
  
    // Write out the connectivity table
  size_t elem_idx;
  size_t connectivity_size = myMesh->num_elements() + myMesh->num_vertex_uses();
  file << "CELLS " << myMesh->num_elements() << ' ' << connectivity_size << '\n';
  for (elem_idx = 0; elem_idx < myMesh->max_element_index(); ++elem_idx)
  {
    if (!myMesh->is_element_valid(elem_idx))
      continue;
    
    msq_std::vector<size_t> conn = myMesh->element_connectivity( elem_idx, err ); MSQ_ERRRTN(err);
    EntityTopology topo = myMesh->element_topology( elem_idx, err ); MSQ_ERRRTN(err);
   
      // If necessary, convert from Exodus to VTK node-ordering.
    const VtkTypeInfo* info = VtkTypeInfo::find_type( topo, conn.size(), err ); MSQ_ERRRTN(err);
    info->mesquiteToVtkOrder( conn );
    
    file << conn.size();
    for (i = 0; i < conn.size(); ++i)
      file << ' ' << vertex_indices[(size_t)conn[i]];
    file << '\n';
  }
  
    // Write out the element types
  file << "CELL_TYPES " << myMesh->num_elements() << '\n';
  for (elem_idx = 0; elem_idx < myMesh->max_element_index(); ++elem_idx)
  {
    if (!myMesh->is_element_valid(elem_idx))
      continue;
    
    EntityTopology topo = myMesh->element_topology( elem_idx, err ); MSQ_ERRRTN(err);
    count = myMesh->element_connectivity( elem_idx, err ).size(); MSQ_ERRRTN(err);
    const VtkTypeInfo* info = VtkTypeInfo::find_type( topo, count, err ); MSQ_ERRRTN(err); 
    file << info->vtkType << '\n';
  }
  
    // Write out which points are fixed.
  file << "POINT_DATA " << myMesh->num_vertices()
       << "\nSCALARS fixed bit\nLOOKUP_TABLE default\n";
  for (i = 0; i < myMesh->max_vertex_index(); ++i)
    if (myMesh->is_vertex_valid( i ))
      file <<( myMesh->vertex_is_fixed( i, err ) ? "1" : "0") << "\n";
  
    // Write vertex tag data to vtk attributes
  MeshImplTags::TagIterator tagiter = myTags->tag_begin();
  for (tagiter = myTags->tag_begin(); tagiter != myTags->tag_end(); ++tagiter)
  {
    bool havevert = myTags->tag_has_vertex_data( *tagiter, err ); MSQ_ERRRTN(err);
    if (!havevert)
      continue;
    
    const TagDescription& desc = myTags->properties( *tagiter, err );
    MSQ_ERRRTN(err);
    
    std::vector<char> tagdata( myMesh->num_vertices() * desc.size );
    std::vector<char>::iterator iter = tagdata.begin();
    for (i = 0; i < myMesh->max_vertex_index(); ++i)
    {
      if (myMesh->is_vertex_valid(i))
      {
        myTags->get_vertex_data( *tagiter, 1, &i, &*iter, err );
        MSQ_ERRRTN(err);
        iter += desc.size;
      }
    }
    
    vtk_write_attrib_data( file, 
                           desc, 
                           &tagdata[0],
                           myMesh->num_vertices(),
                           err );
    MSQ_ERRRTN(err);
  }

    // If there are any element attributes, write them
  for (tagiter = myTags->tag_begin(); tagiter != myTags->tag_end(); ++tagiter)
  {
    bool haveelem = myTags->tag_has_element_data( *tagiter, err ); MSQ_ERRRTN(err);
    if (haveelem)
    {
      file << "\nCELL_DATA " << myMesh->num_elements() << "\n";
      break;
    }
  }
  for (tagiter = myTags->tag_begin(); tagiter != myTags->tag_end(); ++tagiter)
  {
    bool haveelem = myTags->tag_has_element_data( *tagiter, err ); MSQ_ERRRTN(err);
    if (!haveelem)
      continue;
    
    
    const TagDescription& desc = myTags->properties( *tagiter, err );
    MSQ_ERRRTN(err);
    
    std::vector<char> tagdata( myMesh->num_elements() * desc.size );
    std::vector<char>::iterator iter = tagdata.begin();
    for (i = 0; i < myMesh->max_element_index(); ++i)
    {
      if (myMesh->is_element_valid(i))
      {
        myTags->get_element_data( *tagiter, 1, &i, &*iter, err );
        MSQ_ERRRTN(err);
        iter += desc.size;
      }
    }
    
    vtk_write_attrib_data( file, 
                           desc, 
                           &tagdata[0],
                           myMesh->num_elements(),
                           err );
    MSQ_ERRRTN(err);
  }
    
  
    // Close the file
  file.close();
}

void MeshImpl::read_exodus(const char* in_filename , MsqError &err)
{
#ifndef MSQ_USING_EXODUS
  MSQ_SETERR(err)( MsqError::NOT_IMPLEMENTED );
  MSQ_DBGOUT(1) << "Cannot read ExodusII file: " << in_filename << "\n";
  return;
#else
  
  clear();
  
  int app_float_size = sizeof(double);
  int file_float_size = 0;
  float exo_version = 0;
  int exo_err = 0;
  
    // Open the file
  int file_id = ex_open(in_filename, EX_READ, &app_float_size,
                    &file_float_size, &exo_version);

    // Make sure we opened the file correctly
  if (file_id < 0)
  {
    MSQ_SETERR(err)( MsqError::FILE_ACCESS );
    return;
  }
  
    // make sure the file is saved as doubles
  if (file_float_size != sizeof(double))
  {
    MSQ_SETERR(err)("File saved with float-sized reals.  Can only read files "
                    "saved with doubles.", MsqError::NOT_IMPLEMENTED );
    return;
  }

  char title[MAX_LINE_LENGTH];
  int dim, vert_count, elem_count, block_count, ns_count, ss_count;
  
    // get info about the file
  exo_err = ex_get_init(file_id, title, &dim, &vert_count,
                        &elem_count, &block_count, &ns_count, &ss_count);
  if (exo_err < 0)
  {
    MSQ_SETERR(err)("Unable to get entity counts from file.", 
                    MsqError::PARSE_ERROR);
    return;
  }
  
  myMesh->allocate_vertices( vert_count, err ); MSQ_ERRRTN(err);
  myMesh->allocate_elements( elem_count, err ); MSQ_ERRRTN(err);
  
    // Now fill in the data
  
    // Get the vertex coordinates
  msq_std::vector<double> coords(vert_count * 3);
  double* x_iter = &coords[0];
  double* y_iter = &coords[vert_count];
  double* z_iter = &coords[2*vert_count];
  numCoords = dim;
  if (dim == 2)
  {
    exo_err = ex_get_coord( file_id, x_iter, y_iter, 0 );
    memset( z_iter, 0, sizeof(double)*vert_count );
  }
  else
  {
    exo_err = ex_get_coord( file_id, x_iter, y_iter, z_iter );
  }
    // Make sure it worked
  if (exo_err < 0)
  {
    MSQ_SETERR(err)("Unable to retrieve vertex coordinates from file.",
                    MsqError::PARSE_ERROR);
    return;
  }
  
    // Store vertex coordinates in vertex array
  int i;
  for (i = 0; i < vert_count; ++i)
    myMesh->reset_vertex( i, Vector3D(*(x_iter++), *(y_iter++), *(z_iter)++), false, err );
  coords.clear();
  
  
    // Get block list
  msq_std::vector<int> block_ids(block_count);
  exo_err = ex_get_elem_blk_ids(file_id, &block_ids[0]);
  if (exo_err < 0)
  {
    MSQ_SETERR(err)("Unable to read block IDs from file.", MsqError::PARSE_ERROR);
    return;
  }


  msq_std::vector<int> conn;
  size_t index = 0;
  for (i = 0; i < block_count; i++)
  {
      // Get info about this block's elements
    char elem_type_str[MAX_STR_LENGTH];
    int num_block_elems, verts_per_elem, num_atts;
    exo_err = ex_get_elem_block(file_id, block_ids[i], elem_type_str,
                                &num_block_elems, &verts_per_elem,
                                &num_atts);
    if (exo_err < 0)
    {
      MSQ_SETERR(err)("Unable to read parameters for block.",MsqError::PARSE_ERROR);
      return;
    }
    
      // Figure out which type of element we're working with
    EntityTopology elem_type;
    for (int j = 0; elem_type_str[j]; j++)
      elem_type_str[j] = toupper(elem_type_str[j]);
    if (!strncmp(elem_type_str, "TRI", 3))
    {
      elem_type = Mesquite::TRIANGLE;
    }
    else if (!strncmp(elem_type_str, "QUA", 3) ||
             !strncmp(elem_type_str, "SHE", 3))
    {
      elem_type = Mesquite::QUADRILATERAL;
    }
    else if (!strncmp(elem_type_str, "HEX", 3))
    {
      elem_type = Mesquite::HEXAHEDRON;
    }
    else if (!strncmp(elem_type_str, "TET", 3))
    {
      elem_type = Mesquite::TETRAHEDRON;
    }
    else if (!strncmp(elem_type_str, "PYRAMID", 7))
    {
      elem_type = Mesquite::PYRAMID;
    }
    else if (!strncmp(elem_type_str, "WEDGE", 5))
    {
      elem_type = Mesquite::PRISM;
    }
    else
    {
      MSQ_SETERR(err)("Unrecognized element type in block",
                      MsqError::UNSUPPORTED_ELEMENT);
      continue;
    }
    
    if (conn.size() < (unsigned)(num_block_elems*verts_per_elem))
      conn.resize( num_block_elems*verts_per_elem );
    exo_err = ex_get_elem_conn( file_id, block_ids[i], &conn[0] );
    if (exo_err < 0)
    {
      MSQ_SETERR(err)("Unable to read element block connectivity.",
                      MsqError::PARSE_ERROR);
      return;
    }
    
    msq_std::vector<size_t> vertices(verts_per_elem);
    msq_std::vector<int>::iterator conn_iter = conn.begin();
    for (const size_t end = index + num_block_elems; index < end; ++index)
    {
      for (msq_std::vector<size_t>::iterator iter = vertices.begin();
           iter != vertices.end(); ++iter, ++conn_iter)
        *iter = *conn_iter - 1;
        
      myMesh->reset_element( index, vertices, elem_type, err ); MSQ_CHKERR(err);
    }
  }
  
    // Finally, mark boundary nodes
  int num_fixed_nodes=0;
  int num_dist_in_set=0;
  if(ns_count>0){
    exo_err=ex_get_node_set_param(file_id,111,&num_fixed_nodes,
                                  &num_dist_in_set);
    if(exo_err<0){
      MSQ_PRINT(1)("\nError opening nodeset 111, no boundary nodes marked.");
      num_fixed_nodes=0;
    }
  }
  msq_std::vector<int> fixed_nodes(num_fixed_nodes);
  exo_err = ex_get_node_set(file_id, 111, &fixed_nodes[0]);
  if(exo_err<0){
    MSQ_SETERR(err)("Error retrieving fixed nodes.", MsqError::PARSE_ERROR);
  }
  
    // See if this vertex is marked as a boundary vertex
  for (i=0; i < num_fixed_nodes; ++i)
  {
    myMesh->fix_vertex( fixed_nodes[i]-1, true, err ); MSQ_CHKERR(err);
  }

    // Finish up
  exo_err=ex_close(file_id);
  if(exo_err<0)
    MSQ_SETERR(err)("Error closing Exodus file.", MsqError::IO_ERROR);
#endif
}
//!Writes an exodus file of the mesh.
void Mesquite::MeshImpl::write_exodus(const char* out_filename, 
                                      Mesquite::MsqError &err)
{
    //just return an error if we don't have access to exodus
#ifndef MSQ_USING_EXODUS
  MSQ_SETERR(err)("Exodus not enabled in this build of Mesquite",
                  MsqError::NOT_IMPLEMENTED);
  MSQ_DBGOUT(1) << "Cannot read ExodusII file: " << out_filename << "\n";
  return;
#else
  size_t i, j, k;
  if (!myMesh || !myMesh->num_vertices())
  {
    MSQ_SETERR(err)("No vertices in MeshImpl.  Nothing written to file.", 
                    MsqError::PARSE_ERROR);
    return;
  }
    //get some element info
    //We need to know how many of each element type we have.  We
    //are going to create an element block for each element type
    //that exists the mesh.  Block 1 will be tri3; block 2 will be
    //shell; block 3 will be tetra, and block 4 will be hex.
  const unsigned MAX_NODES = 27;
  const unsigned MIN_NODES = 2;
  int counts[MIXED][MAX_NODES+1];
  memset( counts, 0, sizeof(counts) );
  
  for (i = 0; i < myMesh->max_element_index(); ++i)
  {
    if (!myMesh->is_element_valid(i))
      continue;
    
    EntityTopology type = myMesh->element_topology(i, err); MSQ_ERRRTN(err);
    unsigned nodes = myMesh->element_connectivity(i, err).size(); MSQ_ERRRTN(err);
    if ((unsigned)type >= MIXED || nodes < MIN_NODES || nodes > MAX_NODES)
    {
      MSQ_SETERR(err)("Invalid element typology.", MsqError::INTERNAL_ERROR);
      return;
    }
    ++counts[type][nodes];
  }
  
    // Count number of required element blocks
  int block_count = 0;
  for (i = 0; i < MIXED; ++i)
    for (j = MIN_NODES; j < MAX_NODES; ++j)
      if (counts[i][j])
        ++block_count;

    //figure out if we have fixed nodes, if so, we need a nodeset
  int num_fixed_nodes=0;
  for (i = 0; i < myMesh->max_vertex_index(); ++i)
  {
    bool fixed = myMesh->is_vertex_valid(i) &&
                 myMesh->vertex_is_fixed(i, err); MSQ_ERRRTN(err);
    num_fixed_nodes += fixed;
  }
  
  
    //write doubles instead of floats
  int app_float_size = sizeof(double);
  int file_float_size = sizeof(double);
  int exo_err = 0;
  
    // Create the file.  If it exists, clobber it.  This could be dangerous.
  int file_id = ex_create(out_filename, EX_CLOBBER, &app_float_size,
                          &file_float_size);

    // Make sure we opened the file correctly
  if (file_id < 0)
  {
    MSQ_SETERR(err)(MsqError::FILE_ACCESS);
    return;
  }
  
  char title[MAX_LINE_LENGTH]="Mesquite Export";
  
  size_t vert_count = myMesh->num_vertices();
  size_t elem_count = myMesh->num_elements();
  
  int ns_count=0;
  if(num_fixed_nodes>0)
    ns_count=1;
  int ss_count=0;
  
    // put the initial info about the file
  exo_err = ex_put_init(file_id, title, numCoords, vert_count,
                        elem_count, block_count, ns_count, ss_count);
  if (exo_err < 0)
  {
    MSQ_SETERR(err)("Unable to initialize file data.", MsqError::IO_ERROR);
    return;
  }
  
  
    // Gather vertex coordinate data and write to file.
  msq_std::vector<double> coords(vert_count * 3);
  msq_std::vector<double>::iterator x, y, z;
  x = coords.begin();
  y = x + vert_count;
  z = y + vert_count;
  for (i = 0; i < myMesh->max_vertex_index(); ++i)
  {
    if (!myMesh->is_vertex_valid(i))
      continue;
    
    if (z == coords.end())
    {
      MSQ_SETERR(err)("Array overflow", MsqError::INTERNAL_ERROR);
      return;
    }
    
    Vector3D coords = myMesh->get_vertex_coords(i, err); MSQ_ERRRTN(err);
    *x = coords.x(); ++x;
    *y = coords.y(); ++y;
    *z = coords.z(); ++z;
  }
  if(z != coords.end())
  {
    MSQ_SETERR(err)("Counter at incorrect number.", MsqError::INTERNAL_ERROR);
    return;
  }
    //put the coords
  exo_err = ex_put_coord(file_id, &coords[0], &coords[vert_count], &coords[2*vert_count]);
  if (exo_err < 0)
  {
    MSQ_SETERR(err)("Unable to put vertex coordinates in file.",MsqError::IO_ERROR);
    return;
  } 

    //put the names of the coordinates
  char* coord_names[] = { "x", "y", "z" };
  exo_err = ex_put_coord_names(file_id, coord_names);
  
    // Create element-type arrays indexed by Mesquite::EntityTopology
  const char* tri_name = "TRI";
  const char* quad_name = "SHELL";
  const char* tet_name = "TETRA";
  const char* hex_name = "HEX";
  const char* wdg_name = "WEDGE";
  const char* pyr_name = "PYRAMID";
  const char* exo_names[MIXED];
  memset( exo_names, 0, sizeof(exo_names) );
  exo_names[TRIANGLE]      = tri_name;
  exo_names[QUADRILATERAL] = quad_name;
  exo_names[TETRAHEDRON]   = tet_name;
  exo_names[HEXAHEDRON]    = hex_name;
  exo_names[PRISM]         = wdg_name;
  exo_names[PYRAMID]       = pyr_name;
  unsigned min_nodes[MIXED];
  memset( min_nodes, 0, sizeof(min_nodes) );
  min_nodes[TRIANGLE]      = 3;
  min_nodes[QUADRILATERAL] = 4;
  min_nodes[TETRAHEDRON]   = 4;
  min_nodes[HEXAHEDRON]    = 8;
  min_nodes[PRISM]         = 6;
  min_nodes[PYRAMID]       = 5;
  
    // For each element type (topology and num nodes)
  int block_id = 0;
  char name_buf[16];
  int num_atts = 0;
  msq_std::vector<int> conn;
  for (i = 0; i < MIXED; ++i)
  {
    for (j = MIN_NODES; j < MAX_NODES; ++j)
    {
        // Have any of this topo & count combination?
      if (!counts[i][j])
        continue;
      
        // Is a supported ExodusII type?
      if (!exo_names[i])
      {
        MSQ_SETERR(err)(MsqError::INVALID_STATE,
          "Element topology %d not supported by ExodusII", (int)i );
        return;
      }
      
        // Construct ExodusII element name from topo & num nodes
      if (j == min_nodes[i])
        strcpy( name_buf, exo_names[i] );
      else
        sprintf( name_buf, "%s%d", exo_names[i], (int)j );
      
        // Create element block
      ++block_id;
      exo_err = ex_put_elem_block( file_id, block_id, name_buf, 
                                   counts[i][j], j, num_atts );
      if(exo_err<0)
      {
        MSQ_SETERR(err)("Error creating the tri block.", MsqError::IO_ERROR);
        return;
      }
      
        // For each element
      conn.resize( counts[i][j] * j );
      std::vector<int>::iterator iter = conn.begin();
      for (k = 0; k < myMesh->max_element_index(); ++k)
      {
          // If not correct topo, skip it.
        if (!myMesh->is_element_valid(k) ||
             (unsigned)(myMesh->element_topology(k, err)) != i)
          continue;
        MSQ_ERRRTN(err);
        
          // If not correct number nodes, skip it
        const msq_std::vector<size_t>& elem_conn = myMesh->element_connectivity(k, err);
        MSQ_ERRRTN(err);
        if (elem_conn.size() != j)
          continue;
        
          // Append element connectivity to list
        for (msq_std::vector<size_t>::const_iterator citer = elem_conn.begin();
             citer != elem_conn.end(); ++citer, ++iter)
        {
          assert(iter != conn.end());
          *iter = *citer + 1;
        }
      }

        // Make sure everything adds up
      if (iter != conn.end())
      {
        MSQ_SETERR(err)( MsqError::INTERNAL_ERROR );
        return;
      }

        // Write element block connectivity
      exo_err = ex_put_elem_conn( file_id, block_id, &conn[0] );
      if (exo_err<0)
      {
        MSQ_SETERR(err)("Error writing element connectivity.", MsqError::IO_ERROR );
        return;
      }
    }
  }  
 
    // Finally, mark boundary nodes
  
  if(num_fixed_nodes>0){
    exo_err=ex_put_node_set_param(file_id, 111, num_fixed_nodes, 0);
    if(exo_err<0) {
      MSQ_SETERR(err)("Error while initializing node set.", MsqError::IO_ERROR);
      return;
    }
    
    int node_id = 0;
    msq_std::vector<int> fixed_nodes( num_fixed_nodes );
    msq_std::vector<int>::iterator iter = fixed_nodes.begin();
    for (i = 0; i < myMesh->max_vertex_index(); ++i)
    {
      if (!myMesh->is_vertex_valid(i))
        continue;
      ++node_id;
      
      if (myMesh->vertex_is_fixed( i, err ))
      {
        if (iter == fixed_nodes.end())
        {
          MSQ_SETERR(err)(MsqError::INTERNAL_ERROR);
          return;
        }
        *iter = node_id;
        ++iter;
      }
    }
    
    if (iter != fixed_nodes.end())
    {
      MSQ_SETERR(err)(MsqError::INTERNAL_ERROR);
      return;
    }

    exo_err=ex_put_node_set(file_id, 111, &fixed_nodes[0]);
    if(exo_err<0) {
      MSQ_SETERR(err)("Error while writing node set.", MsqError::IO_ERROR);
      return;
    }
  }
  exo_err=ex_close(file_id);
  if(exo_err<0)
    MSQ_SETERR(err)("Error closing Exodus file.", MsqError::IO_ERROR);
  
#endif
}   
// Returns whether this mesh lies in a 2D or 3D coordinate system.
int MeshImpl::get_geometric_dimension(MsqError &/*err*/)
{
  return numCoords;
}


void MeshImpl::get_all_elements( msq_std::vector<ElementHandle>& elems,
                                 MsqError& err )
{
  assert( sizeof(ElementHandle) == sizeof(size_t) );
  msq_std::vector<size_t> temp;
  myMesh->all_elements( temp, err ); MSQ_ERRRTN(err);
  elems.resize( temp.size() );
  memcpy( &elems[0], &temp[0], sizeof(size_t)*temp.size() );
}

void MeshImpl::get_all_vertices( msq_std::vector<VertexHandle>& verts,
                                 MsqError& err )
{
  assert( sizeof(VertexHandle) == sizeof(size_t) );
  msq_std::vector<size_t> temp;
  myMesh->all_vertices( temp, err ); MSQ_ERRRTN(err);
  verts.resize( temp.size() );
  memcpy( &verts[0], &temp[0], sizeof(size_t)*temp.size() );
}

// Returns a pointer to an iterator that iterates over the
// set of all vertices in this mesh.  The calling code should
// delete the returned iterator when it is finished with it.
// If vertices are added or removed from the Mesh after obtaining
// an iterator, the behavior of that iterator is undefined.
VertexIterator* MeshImpl::vertex_iterator(MsqError &/*err*/)
{
  return new MeshImplVertIter( myMesh );
}
    
// Returns a pointer to an iterator that iterates over the
// set of all top-level elements in this mesh.  The calling code should
// delete the returned iterator when it is finished with it.
// If elements are added or removed from the Mesh after obtaining
// an iterator, the behavior of that iterator is undefined.
ElementIterator* MeshImpl::element_iterator(MsqError &/*err*/)
{
  return new MeshImplElemIter( myMesh );
}

//************ Vertex Properties ********************
// Returns true or false, indicating whether the vertex
// is allowed to be repositioned.  True indicates that the vertex
// is fixed and cannot be moved.  Note that this is a read-only
// property; this flag can't be modified by users of the
// Mesquite::Mesh interface.
void MeshImpl::vertices_get_fixed_flag(
 const VertexHandle vert_array[], bool on_bnd[],
 size_t num_vtx, MsqError& err)
{
  for (size_t i=0; i<num_vtx; ++i)
  {
    on_bnd[i] = myMesh->vertex_is_fixed( (size_t)vert_array[i], err ); 
    MSQ_ERRRTN(err);
  }
}

// Get/set location of a vertex
void MeshImpl::vertices_get_coordinates(
 const Mesh::VertexHandle vert_array[],
 MsqVertex* coordinates,
  size_t num_vtx,
 MsqError& err)
{
  for (size_t i=0; i<num_vtx; ++i) {
    coordinates[i] = myMesh->get_vertex_coords( (size_t)vert_array[i], err );
    MSQ_ERRRTN(err);
  }
}



void MeshImpl::vertex_set_coordinates(
  VertexHandle vertex,
  const Vector3D &coordinates,
  MsqError& err)
{
  myMesh->set_vertex_coords( (size_t)vertex, coordinates, err ); MSQ_CHKERR(err);
}

// Each vertex has a byte-sized flag that can be used to store
// flags.  This byte's value is neither set nor used by the mesh
// implementation.  It is intended to be used by Mesquite algorithms.
// Until a vertex's byte has been explicitly set, its value is 0.
void Mesquite::MeshImpl::vertex_set_byte ( VertexHandle vertex,
                                           unsigned char byte,
                                           MsqError& err)
{
  vertices_set_byte( &vertex, &byte, 1, err ); MSQ_CHKERR(err);
}

void MeshImpl::vertices_get_byte ( const VertexHandle *vert_array,
                                   unsigned char *byte_array,
                                   size_t array_size,
                                   MsqError& err)
{
  for (size_t i = 0; i < array_size; i++)
  {
    byte_array[i] = myMesh->get_vertex_byte( (size_t)vert_array[i], err ); 
    MSQ_ERRRTN(err);
  }
}

// Retrieve the byte value for the specified vertex or vertices.
// The byte value is 0 if it has not yet been set via one of the
// *_set_byte() functions.
void MeshImpl::vertex_get_byte( const VertexHandle vertex,
                                unsigned char *byte,
                                MsqError &err )
{
  vertices_get_byte( &vertex, byte, 1, err ); MSQ_CHKERR(err);
}

void MeshImpl::vertices_set_byte( const VertexHandle *vertex,
                                  const unsigned char *byte_array,
                                  size_t array_size,
                                  MsqError& err)
{
  for (size_t i = 0; i < array_size; i++)
  {
    myMesh->set_vertex_byte( (size_t)vertex[i], byte_array[i], err );
    MSQ_ERRRTN(err);
  }
}

template <typename T> struct cast_handle : public msq_std::unary_function<size_t, T>
{ T operator()( size_t idx ) const { return reinterpret_cast<T>(idx); } };

void MeshImpl::vertices_get_attached_elements( const VertexHandle* vertices,
                                               size_t num_vertices,
                                               msq_std::vector<ElementHandle>& elements,
                                               msq_std::vector<size_t>& offsets,
                                               MsqError& err )
{
  elements.clear();
  offsets.clear();
  size_t prev_offset = 0;
  offsets.reserve( num_vertices + 1 );
  offsets.push_back( prev_offset );
  const VertexHandle *const vtx_end = vertices + num_vertices;
  for (; vertices < vtx_end; ++vertices) {
    const msq_std::vector<size_t>& adj 
      = myMesh->vertex_adjacencies( (size_t)*vertices, err );
    MSQ_ERRRTN(err);
    
    prev_offset = prev_offset + adj.size();
    offsets.push_back( prev_offset );
    
    msq_std::transform( adj.begin(), adj.end(), msq_std::back_inserter( elements ), cast_handle<ElementHandle>() );
  }
}



void MeshImpl::elements_get_attached_vertices( const ElementHandle *elements,
                                               size_t num_elems,
                                               msq_std::vector<VertexHandle>& vertices,
                                               msq_std::vector<size_t>& offsets,
                                               MsqError &err) 
{
  vertices.clear();
  offsets.clear();
  size_t prev_offset = 0;
  offsets.reserve( num_elems + 1 );
  offsets.push_back( prev_offset );
  const ElementHandle *const elem_end = elements + num_elems;
  for (; elements < elem_end; ++elements) {
    const msq_std::vector<size_t>& conn 
      = myMesh->element_connectivity( (size_t)*elements, err );
    MSQ_ERRRTN(err);
    
    prev_offset = prev_offset + conn.size();
    offsets.push_back( prev_offset );
    
    msq_std::transform( conn.begin(), conn.end(), msq_std::back_inserter( vertices ), cast_handle<VertexHandle>() );
  }
}

// Returns the topologies of the given entities.  The "entity_topologies"
// array must be at least "num_elements" in size.
void MeshImpl::elements_get_topologies( const ElementHandle element_handle_array[],
                                        EntityTopology element_topologies[],
                                        size_t num_elements,
                                        MsqError& err)
{
  for (size_t i = 0; i < num_elements; i++)
  {
    element_topologies[i] = myMesh->
      element_topology( (size_t)element_handle_array[i], err );
    MSQ_CHKERR(err);
  }
}





//**************** Memory Management ****************
// Tells the mesh that the client is finished with a given
// entity handle.  
void Mesquite::MeshImpl::release_entity_handles(
  const Mesquite::Mesh::EntityHandle* /*handle_array*/,
  size_t /*num_handles*/,
  MsqError &/*err*/)
{
    // Do nothing
}

// Instead of deleting a Mesh when you think you are done,
// call release().  In simple cases, the implementation could
// just call the destructor.  More sophisticated implementations
// may want to keep the Mesh object to live longer than Mesquite
// is using it.
void Mesquite::MeshImpl::release()
{
  //delete this;
}



const char* const vtk_type_names[] = { "bit",
                                       "char",
                                       "unsigned_char",
                                       "short",
                                       "unsigned_short",
                                       "int",
                                       "unsigned_int",
                                       "long",
                                       "unsigned_long",
                                       "float",
                                       "double",
                                       0 };

void MeshImpl::read_vtk( const char* filename, MsqError &err )
{
  int major, minor;
  char vendor_string[257];
  size_t i;
  
  FILE* file = fopen( filename, "r" );
  if (!file)
  {
    MSQ_SETERR(err)(MsqError::FILE_ACCESS);
    return;
  }
  
    // Read file header
    
  if (!fgets( vendor_string, sizeof(vendor_string), file ))
  {
    MSQ_SETERR(err)( MsqError::IO_ERROR );
    fclose( file );
    return;
  }
  
  if (!strchr( vendor_string, '\n' ) ||
      2 != sscanf( vendor_string, "# vtk DataFile Version %d.%d", &major, &minor ))
  {
    MSQ_SETERR(err)( MsqError::FILE_FORMAT );
    fclose( file );
    return;
  }
  
  if (!fgets( vendor_string, sizeof(vendor_string), file )) 
  {
    MSQ_SETERR(err)( MsqError::IO_ERROR );
    fclose( file );
    return;
  }
  
    // VTK spec says this should not exceed 256 chars.
  if (!strchr( vendor_string, '\n' ))
  {
    MSQ_SETERR(err)( "Vendor string (line 2) exceeds 256 characters.",
                      MsqError::PARSE_ERROR);
    fclose( file );
    return;
  }
  
  
    // Check file type
  
  FileTokenizer tokens( file );
  const char* const file_type_names[] = { "ASCII", "BINARY", 0 };
  int filetype = tokens.match_token( file_type_names, err ); MSQ_ERRRTN(err);
  if (2 == filetype) {
    MSQ_SETERR(err)( "Cannot read BINARY VTK files -- use ASCII.",
                     MsqError::NOT_IMPLEMENTED );
    return;
  }

    // Clear any existing data
  this->clear();

    // Read the mesh
  tokens.match_token( "DATASET", err ); MSQ_ERRRTN(err);
  vtk_read_dataset( tokens, err ); MSQ_ERRRTN(err);
  
    // Make sure file actually contained some mesh
  if (myMesh->num_elements() == 0)
  {
    MSQ_SETERR(err)("File contained no mesh.", MsqError::PARSE_ERROR);
    return;
  }
  
    // Read attribute data until end of file.
  const char* const block_type_names[] = { "POINT_DATA", "CELL_DATA", 0 };
  int blocktype = 0;
  while (!tokens.eof())
  {
      // get POINT_DATA or CELL_DATA
    int new_block_type = tokens.match_token( block_type_names, err );
    if (tokens.eof())
    {
      err.clear();
      break;
    }
    if (err)
    {
        // If next token was neither POINT_DATA nor CELL_DATA,
        // then there's another attribute under the current one.
      if (blocktype)
      {
        tokens.unget_token();
        err.clear();
      }
      else
      {
        MSQ_ERRRTN(err);
      }
    }
    else
    {
      blocktype = new_block_type;
      long count;
      tokens.get_long_ints( 1, &count, err); MSQ_ERRRTN(err);
      
      if (blocktype == 1 && (unsigned long)count != myMesh->num_vertices())
      {
        MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                         "Count inconsistent with number of vertices" 
                         " at line %d.", tokens.line_number());
        return;
      }
      else if (blocktype == 2 && (unsigned long)count != myMesh->num_elements())
      {
         MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                         "Count inconsistent with number of elements" 
                         " at line %d.", tokens.line_number());
        return;
      }
    }
      
   
    if (blocktype == 1)
      vtk_read_point_data( tokens, err );
    else
      vtk_read_cell_data ( tokens, err );
    MSQ_ERRRTN(err);
  }
  
    // There is no option for a 2-D mesh in VTK files.  Always 3
  numCoords = 3;

    // Convert tag data for fixed nodes to internal bitmap
  TagHandle handle = tag_get( "fixed", err );
  if (!handle || MSQ_CHKERR(err)) return;
  
  const TagDescription& tag_desc = myTags->properties( (size_t)handle, err ); MSQ_ERRRTN(err);
  bool havedata = myTags->tag_has_vertex_data( (size_t)handle, err ); MSQ_ERRRTN(err);
  if (!havedata)
  {
    MSQ_SETERR(err)("'fixed' attribute on elements, not vertices", MsqError::FILE_FORMAT);
    return;
  }

  switch( tag_desc.type )
  {
    case BYTE:
    {
      char data;
      for (i = 0; i < myMesh->max_vertex_index(); ++i)
      {
        myTags->get_vertex_data( (size_t)handle, 1, &i, &data, err ); MSQ_ERRRTN(err);
        myMesh->fix_vertex( i, !!data, err ); MSQ_ERRRTN(err);
      }
      break;
    }
    case BOOL:
    {
      bool data;
      for (i = 0; i < myMesh->max_vertex_index(); ++i)
      {
        myTags->get_vertex_data( (size_t)handle, 1, &i, &data, err ); MSQ_ERRRTN(err);
        myMesh->fix_vertex( i, data, err ); MSQ_ERRRTN(err);
      }
      break;
    }
    case INT:
    {
      int data;
      for (i = 0; i < myMesh->max_vertex_index(); ++i)
      {
        myTags->get_vertex_data( (size_t)handle, 1, &i, &data, err ); MSQ_ERRRTN(err);
        myMesh->fix_vertex( i, !!data, err ); MSQ_ERRRTN(err);
      }
      break;
    }
    case DOUBLE:
    {
      double data;
      for (i = 0; i < myMesh->max_vertex_index(); ++i)
      {
        myTags->get_vertex_data( (size_t)handle, 1, &i, &data, err ); MSQ_ERRRTN(err);
        myMesh->fix_vertex( i, !!data, err ); MSQ_ERRRTN(err);
      }
      break;
    }
    default:
      MSQ_SETERR(err)("'fixed' attribute has invalid type", MsqError::PARSE_ERROR);
      return;
  }
  
  tag_delete( handle, err );
}

void MeshImpl::vtk_read_dataset( FileTokenizer& tokens, MsqError& err )
{
  const char* const data_type_names[] = { "STRUCTURED_POINTS",
                                          "STRUCTURED_GRID",
                                          "UNSTRUCTURED_GRID",
                                          "POLYDATA",
                                          "RECTILINEAR_GRID",
                                          "FIELD",
                                          0 };
  int datatype = tokens.match_token( data_type_names, err ); MSQ_ERRRTN(err);
  switch( datatype )
  {
    case 1: vtk_read_structured_points( tokens, err ); break;
    case 2: vtk_read_structured_grid  ( tokens, err ); break;
    case 3: vtk_read_unstructured_grid( tokens, err ); break;
    case 4: vtk_read_polydata         ( tokens, err ); break;
    case 5: vtk_read_rectilinear_grid ( tokens, err ); break;
    case 6: vtk_read_field            ( tokens, err ); break;
  }
}


void MeshImpl::vtk_read_structured_points( FileTokenizer& tokens, MsqError& err )
{
  long i, j, k;
  long dims[3];
  double origin[3], space[3];
 
  tokens.match_token( "DIMENSIONS", err ); MSQ_ERRRTN(err);
  tokens.get_long_ints( 3, dims, err );    MSQ_ERRRTN(err);
  tokens.get_newline( err );               MSQ_ERRRTN(err);
  
  if (dims[0] < 1 || dims[1] < 1 || dims[2] < 1)
  {
    MSQ_SETERR(err)(MsqError::PARSE_ERROR,
                   "Invalid dimension at line %d", 
                   tokens.line_number());
    return;
  }
  
  tokens.match_token( "ORIGIN", err );     MSQ_ERRRTN(err);
  tokens.get_doubles( 3, origin, err );    MSQ_ERRRTN(err);
  tokens.get_newline( err );               MSQ_ERRRTN(err);
  
  const char* const spacing_names[] = { "SPACING", "ASPECT_RATIO", 0 };
  tokens.match_token( spacing_names, err );MSQ_ERRRTN(err);
  tokens.get_doubles( 3, space, err );     MSQ_ERRRTN(err);
  tokens.get_newline( err );               MSQ_ERRRTN(err);
  
  myMesh->allocate_vertices( dims[0]*dims[1]*dims[2], err ); MSQ_ERRRTN(err);
  size_t vtx = 0;
  Vector3D off( origin[0], origin[1], origin[2] );
  for (k = 0; k < dims[2]; ++k)
    for (j = 0; j < dims[1]; ++j)
      for (i = 0; i < dims[0]; ++i)
      {
        myMesh->reset_vertex( vtx++, off + Vector3D( i*space[0], j*space[1], k*space[2] ), false, err ); 
        MSQ_ERRRTN(err);
      }

  vtk_create_structured_elems( dims, err ); MSQ_ERRRTN(err);
}

void MeshImpl::vtk_read_structured_grid( FileTokenizer& tokens, MsqError& err )
{
  long num_verts, dims[3];
 
  tokens.match_token( "DIMENSIONS", err );   MSQ_ERRRTN(err);
  tokens.get_long_ints( 3, dims, err );      MSQ_ERRRTN(err);
  tokens.get_newline( err );                 MSQ_ERRRTN(err); 
  
  if (dims[0] < 1 || dims[1] < 1 || dims[2] < 1)
  {
    MSQ_SETERR(err)(MsqError::PARSE_ERROR,
                   "Invalid dimension at line %d", 
                   tokens.line_number());
    return;
  }
  
  tokens.match_token( "POINTS", err );        MSQ_ERRRTN(err); 
  tokens.get_long_ints( 1, &num_verts, err ); MSQ_ERRRTN(err); 
  tokens.match_token( vtk_type_names, err );  MSQ_ERRRTN(err); 
  tokens.get_newline( err );                  MSQ_ERRRTN(err); 
  
  if (num_verts != (dims[0] * dims[1] * dims[2]))
  {
    MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                    "Point count not consistent with dimensions "
                    "at line %d", tokens.line_number() );
    return;
  }
  
  myMesh->allocate_vertices( num_verts, err ); MSQ_ERRRTN(err);
  for (size_t vtx = 0; vtx < (size_t)num_verts; ++vtx)
  {
    Vector3D pos;
    tokens.get_doubles( 3, const_cast<double*>(pos.to_array()), err ); MSQ_ERRRTN(err);
    myMesh->reset_vertex( vtx, pos, false, err );  MSQ_ERRRTN(err);
  }
 
  vtk_create_structured_elems( dims, err ); MSQ_ERRRTN(err);
}

void MeshImpl::vtk_read_rectilinear_grid( FileTokenizer& tokens, MsqError& err )
{
  int i, j, k;
  long dims[3];
  const char* labels[] = { "X_COORDINATES", "Y_COORDINATES", "Z_COORDINATES" };
  vector<double> coords[3];
  
  tokens.match_token( "DIMENSIONS", err );                   MSQ_ERRRTN(err);
  tokens.get_long_ints( 3, dims, err );                     MSQ_ERRRTN(err);
  tokens.get_newline( err );                                MSQ_ERRRTN(err);
  
  if (dims[0] < 1 || dims[1] < 1 || dims[2] < 1)
  {
     MSQ_SETERR(err)(MsqError::PARSE_ERROR,
                   "Invalid dimension at line %d", 
                   tokens.line_number());
    return;
  }

  for (i = 0; i < 3; i++)
  {
    long count;
    tokens.match_token( labels[i], err );                   MSQ_ERRRTN(err);
    tokens.get_long_ints( 1, &count, err );                 MSQ_ERRRTN(err);
    tokens.match_token( vtk_type_names, err );              MSQ_ERRRTN(err);
    tokens.get_newline( err );                              MSQ_ERRRTN(err);
    
    if (count != dims[i])
    {
      MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                       "Coordinate count inconsistent with dimensions"
                       " at line %d", tokens.line_number());
      return;
    }
    
    coords[i].resize(count);
    tokens.get_doubles( count, &coords[i][0], err );   MSQ_ERRRTN(err);
  }
  
  myMesh->allocate_vertices( dims[0]*dims[1]*dims[2], err ); MSQ_ERRRTN(err);
  size_t vtx = 0;
  for (k = 0; k < dims[2]; ++k)
    for (j = 0; j < dims[1]; ++j)
      for (i = 0; i < dims[0]; ++i)
      {
        myMesh->reset_vertex( vtx++, Vector3D( coords[0][i], coords[1][j], coords[2][k] ), false, err );
        MSQ_ERRRTN(err);
      }
  
  
  vtk_create_structured_elems( dims, err );                 MSQ_ERRRTN(err);
}

void MeshImpl::vtk_read_polydata( FileTokenizer& tokens, MsqError& err )
{
  long num_verts;
  vector<int> connectivity;
  const char* const poly_data_names[] = { "VERTICES",
                                          "LINES",
                                          "POLYGONS",
                                          "TRIANGLE_STRIPS", 
                                          0 };
  
  tokens.match_token( "POINTS", err );                      MSQ_ERRRTN(err);
  tokens.get_long_ints( 1, &num_verts, err );               MSQ_ERRRTN(err);
  tokens.match_token( vtk_type_names, err );                MSQ_ERRRTN(err);
  tokens.get_newline( err );                                MSQ_ERRRTN(err);
  
  if (num_verts < 1)
  {
    MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                     "Invalid point count at line %d", tokens.line_number());
    return;
  }
  
  myMesh->allocate_vertices( num_verts, err );              MSQ_ERRRTN(err);
  for (size_t vtx = 0; vtx < (size_t)num_verts; ++vtx)
  {
    Vector3D pos;
    tokens.get_doubles( 3, const_cast<double*>(pos.to_array()), err ); MSQ_ERRRTN(err);
    myMesh->reset_vertex( vtx, pos, false, err );           MSQ_ERRRTN(err);
  }

  int poly_type = tokens.match_token( poly_data_names, err );MSQ_ERRRTN(err);
  switch (poly_type)
  {
    case 3:
      vtk_read_polygons( tokens, err );                     MSQ_ERRRTN(err);
      break;
    case 4:
      MSQ_SETERR(err)( MsqError::NOT_IMPLEMENTED, 
                       "Unsupported type: triangle strips at line %d",
                       tokens.line_number() );
      return;
    case 1:
    case 2:
      MSQ_SETERR(err)( MsqError::NOT_IMPLEMENTED, 
                       "Entities of dimension < 2 at line %d",
                       tokens.line_number() );
      return;
  }
}

void MeshImpl::vtk_read_polygons( FileTokenizer& tokens, MsqError& err )
{
  long size[2];
  
  tokens.get_long_ints( 2, size, err );                     MSQ_ERRRTN(err);
  tokens.get_newline( err );                                MSQ_ERRRTN(err);
  myMesh->allocate_elements( size[0], err );                MSQ_ERRRTN(err);
  msq_std::vector<size_t> conn;
  assert(sizeof(long) == sizeof(size_t));

  for (int i = 0; i < size[0]; ++i)
  {
    long count;
    tokens.get_long_ints( 1, &count, err );                 MSQ_ERRRTN(err);
    conn.resize( count );
    tokens.get_long_ints( count, (long*)&conn[0], err );    MSQ_ERRRTN(err);
    myMesh->reset_element( i, conn, POLYGON, err );         MSQ_ERRRTN(err);
  } 
}



void MeshImpl::vtk_read_unstructured_grid( FileTokenizer& tokens, MsqError& err )
{
  long i, num_verts, num_elems[2];
  
  tokens.match_token( "POINTS", err );                      MSQ_ERRRTN(err);
  tokens.get_long_ints( 1, &num_verts, err );               MSQ_ERRRTN(err);
  tokens.match_token( vtk_type_names, err );                MSQ_ERRRTN(err);
  tokens.get_newline( err );                                MSQ_ERRRTN(err);
  
  if (num_verts < 1)
  {
    MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                     "Invalid point count at line %d", tokens.line_number());
    return;
  }
  
  myMesh->allocate_vertices( num_verts, err );              MSQ_ERRRTN(err);
  for (size_t vtx = 0; vtx < (size_t)num_verts; ++vtx)
  {
    Vector3D pos;
    tokens.get_doubles( 3, const_cast<double*>(pos.to_array()), err ); MSQ_ERRRTN(err);
    myMesh->reset_vertex( vtx, pos, false, err );           MSQ_ERRRTN(err);
  }
  
  tokens.match_token( "CELLS", err );                       MSQ_ERRRTN(err);
  tokens.get_long_ints( 2, num_elems, err );                MSQ_ERRRTN(err);
  tokens.get_newline( err );                                MSQ_ERRRTN(err);

  myMesh->allocate_elements( num_elems[0], err );           MSQ_ERRRTN(err);
  msq_std::vector<size_t> conn;
  assert(sizeof(long) == sizeof(size_t));
  for (i = 0; i < num_elems[0]; ++i)
  {
    long count;
    tokens.get_long_ints( 1, &count, err);                  MSQ_ERRRTN(err);
    conn.resize( count );
    tokens.get_long_ints( count, (long*)&conn[0], err );    MSQ_ERRRTN(err);
    myMesh->reset_element( i, conn, MIXED, err );           MSQ_ERRRTN(err);
  }
 
  tokens.match_token( "CELL_TYPES", err );                  MSQ_ERRRTN(err);
  tokens.get_long_ints( 1, &num_elems[1], err );            MSQ_ERRRTN(err);
  tokens.get_newline( err );                                MSQ_ERRRTN(err);
  
  if (num_elems[0] != num_elems[1])
  {
    MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                    "Number of element types does not match number of elements"
                    "at line %d", tokens.line_number() );
    return;
  }
  
  msq_std::vector<size_t> tconn;
  for (i = 0; i < num_elems[0]; ++i)
  {
    long type;
    size_t size;
    tokens.get_long_ints( 1, &type, err );                      MSQ_ERRRTN(err);

      // Check if type is a valid value
    const VtkTypeInfo* info = VtkTypeInfo::find_type( type, err );   
    if (err || !info || !info->numNodes)
    {
      MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                       "Invalid cell type %ld at line %d.",
                       type, tokens.line_number() );
      return;
    }
      // Check if Mesquite supports the type
    if (info->msqType == MIXED)
    {
      MSQ_SETERR(err)( MsqError::UNSUPPORTED_ELEMENT,
                       "Unsupported cell type %ld (%s) at line %d.",
                       type, info->name, tokens.line_number() );
      return;
    }
    
      // If node-ordering is not the same as exodus...
    if (info->vtkConnOrder)
    {
      size = myMesh->element_connectivity( i, err ).size();     MSQ_ERRRTN(err);
      if (info->numNodes != size)
      {
        MSQ_SETERR(err)(MsqError::UNSUPPORTED_ELEMENT,
          "Cell type %ld (%s) for element with %d nodes at Line %d",
          type, info->name, (int)size, tokens.line_number() ); 
        return;
      }
      
      tconn.resize( size );
      const std::vector<size_t>& conn = myMesh->element_connectivity( i, err ); MSQ_ERRRTN(err);
      for (size_t j = 0; j < size; ++j)
      {
        tconn[j] = conn[info->vtkConnOrder[j]];
      }
        
      myMesh->reset_element( i, tconn, info->msqType, err );MSQ_ERRRTN(err);
    }
      // Othewise (if node ordering is the same), just set the type.
    else
    {
      myMesh->element_topology( i, info->msqType, err );MSQ_ERRRTN(err);
    }
  } // for(i)
}

void MeshImpl::vtk_create_structured_elems( const long* dims, 
                                            MsqError& err )
{
    //NOTE: this should be work fine for edges also if 
    //      Mesquite ever supports them.  Just add the
    //      type for dimension 1 to the switch statement.
    
  int non_zero[3] = {0,0,0};  // True if dim > 0 for x, y, z respectively
  long elem_dim = 0;          // Element dimension (2->quad, 3->hex)
  long num_elems = 1;         // Total number of elements
  long vert_per_elem;         // Element connectivity length
  long edims[3] = { 1, 1, 1 };// Number of elements in each grid direction
  
    // Populate above data
  for (int d = 0; d < 3; d++) 
    if (dims[d] > 1)
    {
      non_zero[elem_dim++] = d;
      edims[d] = dims[d] - 1;
      num_elems *= edims[d];
    }
  vert_per_elem = 1 << elem_dim;
  
    // Get element type from element dimension
  EntityTopology type;
  switch( elem_dim )
  {
  //case 1: type = EDGE;          break;
    case 2: type = QUADRILATERAL; break;
    case 3: type = HEXAHEDRON;    break;
    default:
      MSQ_SETERR(err)( "Cannot create structured mesh with elements "
                       "of dimension < 2 or > 3.",
                       MsqError::NOT_IMPLEMENTED );
      return;
  }

    // Allocate storage for elements
  myMesh->allocate_elements( num_elems, err ); MSQ_ERRRTN(err);
  
    // Offsets of element vertices in grid relative to corner closest to origin 
  long k = dims[0]*dims[1];
  const long corners[8] = { 0, 1, 1+dims[0], dims[0], k, k+1, k+1+dims[0], k+dims[0] };
                             
    // Populate element list
  msq_std::vector<size_t> conn( vert_per_elem );
  size_t elem_idx = 0;
  for (long z = 0; z < edims[2]; ++z)
    for (long y = 0; y < edims[1]; ++y)
      for (long x = 0; x < edims[0]; ++x)
      {
        const long index = x + y*dims[0] + z*(dims[0]*dims[1]);
        for (long j = 0; j < vert_per_elem; ++j)
          conn[j] = index + corners[j];
        myMesh->reset_element( elem_idx++, conn, type, err ); MSQ_ERRRTN(err);
      }
}

void MeshImpl::vtk_read_field( FileTokenizer& tokens, MsqError& err )
{
    // This is not supported yet.
    // Parse the data but throw it away because
    // Mesquite has no internal representation for it.
  
    // Could save this in tags, but the only useful thing that
    // could be done with the data is to write it back out
    // with the modified mesh.  As there's no way to save the
    // type of a tag in Mesquite, it cannot be written back
    // out correctly either.
    // FIXME: Don't know what to do with this data.
    // For now, read it and throw it out.

  long num_arrays;
  tokens.get_long_ints( 1, &num_arrays, err );              MSQ_ERRRTN(err);
  
  for (long i = 0; i < num_arrays; ++i)
  {
    /*const char* name =*/ tokens.get_string( err );        MSQ_ERRRTN(err);
    
    long dims[2];
    tokens.get_long_ints( 2, dims, err );                   MSQ_ERRRTN(err);
    tokens.match_token( vtk_type_names, err );              MSQ_ERRRTN(err);
    
    long num_vals = dims[0] * dims[1];
    
    for (long j = 0; j < num_vals; j++)
    {
      double junk;
      tokens.get_doubles( 1, &junk, err );                  MSQ_ERRRTN(err);
    }
  }
}

void* MeshImpl::vtk_read_attrib_data( FileTokenizer& tokens, 
                                      long count,
                                      TagDescription& tag,
                                      MsqError& err )
{
  const char* const type_names[] = { "SCALARS",
                                     "COLOR_SCALARS",
                                     "VECTORS",
                                     "NORMALS",
                                     "TEXTURE_COORDINATES",
                                     "TENSORS",
                                     "FIELD",
                                     0 };

  int type = tokens.match_token( type_names, err );
  const char* name = tokens.get_string( err ); MSQ_ERRZERO(err);
  tag.name = name;
  
  void* data = 0;
  switch( type )
  {
    case 1: data = vtk_read_scalar_attrib ( tokens, count, tag, err ); 
            tag.vtkType = TagDescription::SCALAR; 
            break;
    case 2: data = vtk_read_color_attrib  ( tokens, count, tag, err ); 
            tag.vtkType = TagDescription::COLOR;
            break;
    case 3: data = vtk_read_vector_attrib ( tokens, count, tag, err );
            tag.vtkType = TagDescription::VECTOR;
            break;
    case 4: data = vtk_read_vector_attrib ( tokens, count, tag, err ); 
            tag.vtkType = TagDescription::NORMAL;
            break;
    case 5: data = vtk_read_texture_attrib( tokens, count, tag, err ); 
            tag.vtkType = TagDescription::TEXTURE;
            break;
    case 6: data = vtk_read_tensor_attrib ( tokens, count, tag, err ); 
            tag.vtkType = TagDescription::TENSOR;
            break;
    case 7: // Can't handle field data yet.
      MSQ_SETERR(err)( MsqError::NOT_IMPLEMENTED,
                       "Cannot read field data (line %d).",
                       tokens.line_number());
  }

  return data;
}

void MeshImpl::vtk_read_point_data( FileTokenizer& tokens, 
                                    MsqError& err )
{
  TagDescription tag;
  void* data = vtk_read_attrib_data( tokens, myMesh->num_vertices(), tag, err );
  MSQ_ERRRTN(err);
  
  size_t tag_handle = myTags->handle( tag.name, err ); 
  if (MSQ_CHKERR(err)) {
    free( data );
    return;
  }
  if (!tag_handle)
  {
    tag_handle = myTags->create( tag, err ); 
    if (MSQ_CHKERR(err)) {
      free( data );
      return;
    }
  }
  else 
  {
    const TagDescription& desc = myTags->properties( tag_handle, err ); 
    if (MSQ_CHKERR(err)) {
      free( data );
      return;
    }
    if (desc != tag)
    {
      MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                       "Inconsistent types between element "
                       "and vertex attributes of same name "
                       "at line %d", tokens.line_number() );
      free( data );
      return;
    }
  }
  
  msq_std::vector<size_t> vertex_handles;
  myMesh->all_vertices( vertex_handles, err );
  if (MSQ_CHKERR(err)) {
    free( data );
    return;
  }
  myTags->set_vertex_data( tag_handle, 
                           vertex_handles.size(),
                           &vertex_handles[0],
                           data,
                           err ); MSQ_CHKERR(err);
  free( data );
  
}


void MeshImpl::vtk_read_cell_data( FileTokenizer& tokens, 
                                   MsqError& err )
{
  TagDescription tag;
  void* data = vtk_read_attrib_data( tokens, myMesh->num_elements(), tag, err );
  MSQ_ERRRTN(err);
  
  size_t tag_handle = myTags->handle( tag.name, err );  
  if (MSQ_CHKERR(err)) {
    free( data );
    return;
  }
  if (!tag_handle)
  {
    tag_handle = myTags->create( tag, err ); 
    if (MSQ_CHKERR(err)) {
      free( data );
      return;
    }
  }
  else 
  {
    const TagDescription& desc = myTags->properties( tag_handle, err ); 
    if (MSQ_CHKERR(err)) {
      free( data );
      return;
    }
    if (desc != tag)
    {
      MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                       "Inconsistent types between element "
                       "and vertex attributes of same name "
                       "at line %d", tokens.line_number() );
      free( data );
      return;
    }
  }
  
  msq_std::vector<size_t> element_handles;
  myMesh->all_elements( element_handles, err );
  if (MSQ_CHKERR(err)) {
    free( data );
    return;
  }
  myTags->set_element_data( tag_handle, 
                            element_handles.size(),
                            &element_handles[0],
                            data,
                            err ); MSQ_CHKERR(err);
  free( data );
  
}

void* MeshImpl::vtk_read_typed_data( FileTokenizer& tokens, 
                                     int type, 
                                     size_t per_elem, 
                                     size_t num_elem,
                                     TagDescription& tag,
                                     MsqError &err )
{
  void* data_ptr;
  size_t count = per_elem * num_elem;
  switch ( type )
  {
    case 1:
      tag.size = per_elem*sizeof(bool);
      tag.type = BOOL;
      data_ptr = malloc( num_elem*tag.size );
      tokens.get_booleans( count, (bool*)data_ptr, err );
      break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      tag.size = per_elem*sizeof(int);
      tag.type = INT;
      data_ptr = malloc( num_elem*tag.size );
      tokens.get_integers( count, (int*)data_ptr, err );
      break;
    case 10:
    case 11:
      tag.size = per_elem*sizeof(double);
      tag.type = DOUBLE;
      data_ptr = malloc( num_elem*tag.size );
      tokens.get_doubles( count, (double*)data_ptr, err );
      break;
    default:
      MSQ_SETERR(err)( "Invalid data type", MsqError::INVALID_ARG );
      return 0;
  }
  
  if (MSQ_CHKERR(err))
  {
    free( data_ptr );
    return 0;
  }
  
  return data_ptr;
}
  
      
      

void* MeshImpl::vtk_read_scalar_attrib( FileTokenizer& tokens,
                                        long count,
                                        TagDescription& desc,
                                        MsqError& err )
{
  int type = tokens.match_token( vtk_type_names, err );      MSQ_ERRZERO(err);
    
  long size;
  const char* tok = tokens.get_string( err );                MSQ_ERRZERO(err);
  const char* end = 0;
  size = strtol( tok, (char**)&end, 0 );
  if (*end)
  {
    size = 1;
    tokens.unget_token();
  }
  
    // VTK spec says cannot be greater than 4--do we care?
  if (size < 1 || size > 4)
  {
    MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                    "Scalar count out of range [1,4]" 
                    " at line %d", tokens.line_number());
    return 0;
  }
  
  tokens.match_token("LOOKUP_TABLE",err);                     MSQ_ERRZERO(err);
  tok = tokens.get_string(err);                               MSQ_ERRZERO(err);
  
    // If no lookup table, just read and return the data
  if (!strcmp( tok, "default" ))
  {
    void* ptr = vtk_read_typed_data( tokens, type, size, count, desc, err );
    MSQ_ERRZERO(err);
    return ptr;
  }
  
    // If we got this far, then the data has a lookup
    // table.  First read the lookup table and convert
    // to integers.
  string name = tok;
  vector<long> table( size*count );
  if (type > 0 && type < 10)  // Is an integer type
  {
    tokens.get_long_ints( table.size(), &table[0], err );   
    MSQ_ERRZERO(err);
  }
  else // Is a real-number type
  {
    for (msq_std::vector<long>::iterator iter = table.begin(); iter != table.end(); ++iter)
    {
      double data;
      tokens.get_doubles( 1, &data, err );
      MSQ_ERRZERO(err);

      *iter = (long)data;
      if ((double)*iter != data)
      {
        MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                         "Invalid lookup index (%.0f) at line %d",
                         data, tokens.line_number() );
        return 0;
      }
    }
  }
  
    // Now read the data - must be float RGBA color triples
  
  long table_count;
  tokens.match_token( "LOOKUP_TABLE", err );                  MSQ_ERRZERO(err);
  tokens.match_token( name.c_str(), err );                    MSQ_ERRZERO(err);
  tokens.get_long_ints( 1, &table_count, err );               MSQ_ERRZERO(err);
 
  vector<float> table_data(table_count*4);
  tokens.get_floats( table_data.size(), &table_data[0], err );MSQ_ERRZERO(err);
  
    // Create list from indexed data
  
  float* data = (float*)malloc( sizeof(float)*count*size*4 );
  float* data_iter = data;
  for (std::vector<long>::iterator idx = table.begin(); idx != table.end(); ++idx)
  {
    if (*idx < 0 || *idx >= table_count)
    {
      MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                       "LOOKUP_TABLE index %ld out of range.",
                       *idx );
      free( data );
      return  0;
    }
    
    for (int i = 0; i < 4; i++)
    {
      *data_iter = table_data[4 * *idx + i];
      ++data_iter;
    }
  }
  
  desc.size = size * 4 * sizeof(float);
  desc.type = DOUBLE;
  return data;
}

void* MeshImpl::vtk_read_color_attrib( FileTokenizer& tokens, 
                                       long count, 
                                       TagDescription& tag,
                                       MsqError& err )
{
  long size;
  tokens.get_long_ints( 1, &size, err );                    MSQ_ERRZERO(err);
  
  if (size < 1)
  {
    MSQ_SETERR(err)( MsqError::PARSE_ERROR, 
                     "Invalid size (%ld) at line %d",
                     size, tokens.line_number() );
    return 0;
  }
  
  float* data = (float*)malloc( sizeof(float)*count*size );
  tokens.get_floats( count*size, data, err );
  if (MSQ_CHKERR(err))
  {
    free( data );
    return 0;
  }
  
  tag.size = size*sizeof(float);
  tag.type = DOUBLE;
  return data;
}

void* MeshImpl::vtk_read_vector_attrib( FileTokenizer& tokens, 
                                        long count, 
                                        TagDescription& tag,
                                        MsqError& err )
{
  int type = tokens.match_token( vtk_type_names, err );
  MSQ_ERRZERO(err);
    
  void* result = vtk_read_typed_data( tokens, type, 3, count, tag, err );
  MSQ_ERRZERO(err);
  return result;
}

void* MeshImpl::vtk_read_texture_attrib( FileTokenizer& tokens,
                                         long count,
                                         TagDescription& tag,
                                         MsqError& err )
{
  int type, dim;
  tokens.get_integers( 1, &dim, err );
  MSQ_ERRZERO(err);
  type = tokens.match_token( vtk_type_names, err );
  MSQ_ERRZERO(err);
    
  if (dim < 1 || dim > 3)
  {
    MSQ_SETERR(err)( MsqError::PARSE_ERROR,
                     "Invalid dimension (%d) at line %d.",
                     dim, tokens.line_number() );
    return 0;
  }
  
  void* result = vtk_read_typed_data( tokens, type, dim, count, tag, err );
  MSQ_ERRZERO(err);
  return result;
}

void* MeshImpl::vtk_read_tensor_attrib( FileTokenizer& tokens,
                                        long count, 
                                        TagDescription& tag,
                                        MsqError& err )
{
  int type = tokens.match_token( vtk_type_names, err );
  MSQ_ERRZERO(err);
    
  void* result = vtk_read_typed_data( tokens, type, 9, count, tag, err );
  MSQ_ERRZERO(err);
  return result;
}  

void MeshImpl::vtk_write_attrib_data( msq_stdio::ostream& file,
                                      const TagDescription& desc,
                                      const void* data, size_t count,
                                      MsqError& err ) const
{
  if (desc.type == HANDLE)
  {
    MSQ_SETERR(err)("Cannot write HANDLE tag data to VTK file.",
                    MsqError::FILE_FORMAT);
    return;
  }
    
  
  TagDescription::VtkType vtk_type = desc.vtkType;
  unsigned vlen = desc.size / MeshImplTags::size_from_tag_type(desc.type);
    // guess one from data length if not set
  if (vtk_type == TagDescription::NONE)
  {
    switch ( vlen )
    {
      default: vtk_type = TagDescription::SCALAR; break;
      case 3: vtk_type = TagDescription::VECTOR; break;
      case 9: vtk_type = TagDescription::TENSOR; break;
        return;
    }
  }
  
  const char* const typenames[] = { "unsigned_char", "bit", "int", "double" };
  
  int num_per_line;
  switch (vtk_type)
  {
    case TagDescription::SCALAR: 
      num_per_line = vlen; 
      file << "SCALARS " << desc.name << " " << typenames[desc.type] << " " << vlen << "\n";
      break;
    case TagDescription::COLOR : 
      num_per_line = vlen;
      file << "COLOR_SCALARS " << desc.name << " " << vlen << "\n";
      break;
    case TagDescription::VECTOR:
      num_per_line = 3;
      if (vlen != 3)
      {
        MSQ_SETERR(err)(MsqError::INTERNAL_ERROR,
         "Tag \"%s\" is labeled as a VTK vector attribute but has %u values.",
         desc.name.c_str(), vlen);
        return;
      }
      file << "VECTORS " << desc.name << " " << typenames[desc.type] << "\n";
      break;
    case TagDescription::NORMAL:
      num_per_line = 3;
      if (vlen != 3)
      {
        MSQ_SETERR(err)(MsqError::INTERNAL_ERROR,
         "Tag \"%s\" is labeled as a VTK normal attribute but has %u values.",
         desc.name.c_str(), vlen);
        return;
      }
      file << "NORMALS " << desc.name << " " << typenames[desc.type] << "\n";
      break;
    case TagDescription::TEXTURE:
      num_per_line = vlen;
      file << "TEXTURE_COORDINATES " << desc.name << " " << typenames[desc.type] << " " << vlen << "\n";
      break;
    case TagDescription::TENSOR:
      num_per_line = 3;
      if (vlen != 9)
      {
        MSQ_SETERR(err)(MsqError::INTERNAL_ERROR,
         "Tag \"%s\" is labeled as a VTK tensor attribute but has %u values.",
         desc.name.c_str(), vlen);
        return;
      }
      file << "TENSORS " << desc.name << " " << typenames[desc.type] << "\n";
      break;
    default:
      MSQ_SETERR(err)("Unknown VTK attribute type for tag.", MsqError::INTERNAL_ERROR );
      return;
  }
  
  size_t i = 0, total = count*vlen;
  char* space = new char[num_per_line];
  memset( space, ' ', num_per_line );
  space[0] = '\n';
  const unsigned char* odata = (const unsigned char*)data;
  const bool* bdata = (const bool*)data;
  const int* idata = (const int*)data;
  const double* ddata = (const double*)data;
  switch ( desc.type )
  {
    case BYTE:
      while (i < total)
        file << (unsigned int)odata[i++] << space[i%num_per_line];
      break;
    case BOOL:
      while (i < total)
        file << (bdata[i++] ? '1' : '0') << space[i%num_per_line];
      break;
    case INT:
      while (i < total)
        file << idata[i++] << space[i%num_per_line];
      break;
    case DOUBLE:
      while (i < total)
        file << ddata[i++] << space[i%num_per_line];
      break;
    default:
      MSQ_SETERR(err)("Unknown tag type.", MsqError::INTERNAL_ERROR);
  }
  delete [] space;
}
      
        
          
  


/**************************************************************************
 *                               TAGS
 **************************************************************************/


TagHandle MeshImpl::tag_create( const string& name,
                                TagType type,
                                unsigned length,
                                const void* defval,
                                MsqError& err )
{
  size_t index = myTags->create( name, type, length, defval, err ); MSQ_ERRZERO(err);
  return (TagHandle)index;
}

void MeshImpl::tag_delete( TagHandle handle, MsqError& err )
{
  myTags->destroy( (size_t)handle, err ); MSQ_CHKERR(err);
}

TagHandle MeshImpl::tag_get( const msq_std::string& name, MsqError& err )
{
  size_t index = myTags->handle( name, err ); MSQ_ERRZERO(err);
  return (TagHandle)index;
}

void MeshImpl::tag_properties( TagHandle handle,
                               msq_std::string& name,
                               TagType& type,
                               unsigned& length,
                               MsqError& err )
{
  const TagDescription& desc 
    = myTags->properties( (size_t)handle, err ); MSQ_ERRRTN(err);
  
  name = desc.name;
  type = desc.type;
  length = (unsigned)(desc.size / MeshImplTags::size_from_tag_type( desc.type ));
}


void MeshImpl::tag_set_element_data( TagHandle handle,
                                     size_t num_elems,
                                     const ElementHandle* elem_array,
                                     const void* values,
                                     MsqError& err )
{
  myTags->set_element_data( (size_t)handle, 
                            num_elems, 
                            (const size_t*)elem_array,
                            values,
                            err );  MSQ_CHKERR(err);
}

void MeshImpl::tag_get_element_data( TagHandle handle,
                                     size_t num_elems,
                                     const ElementHandle* elem_array,
                                     void* values,
                                     MsqError& err )
{
  myTags->get_element_data( (size_t)handle, 
                            num_elems, 
                            (const size_t*)elem_array,
                            values,
                            err );  MSQ_CHKERR(err);
}

void MeshImpl::tag_set_vertex_data(  TagHandle handle,
                                     size_t num_elems,
                                     const VertexHandle* elem_array,
                                     const void* values,
                                     MsqError& err )
{
  myTags->set_vertex_data( (size_t)handle, 
                            num_elems, 
                            (const size_t*)elem_array,
                            values,
                            err );  MSQ_CHKERR(err);
}

void MeshImpl::tag_get_vertex_data(  TagHandle handle,
                                     size_t num_elems,
                                     const VertexHandle* elem_array,
                                     void* values,
                                     MsqError& err )
{
  myTags->get_vertex_data( (size_t)handle, 
                            num_elems, 
                            (const size_t*)elem_array,
                            values,
                            err );  MSQ_CHKERR(err);
}




} // namespace Mesquite


