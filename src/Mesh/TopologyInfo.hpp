/* ***************************************************************** 
    MESQUITE -- The Mesh Quality Improvement Toolkit

    Copyright 2004 Lawrence Livermore National Laboratory.  Under 
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

#ifndef MESQUITE_TOPOLOGY_INFO_HPP
#define MESQUITE_TOPOLOGY_INFO_HPP

#include "Mesquite.hpp"

namespace Mesquite
{

class MsqError;

/** \brief Information about different element topologies */
class MESQUITE_EXPORT TopologyInfo
{
  public:
    
      /** \brief Dimension of element topology */
    static unsigned dimension( EntityTopology topo )
      { return topo >= MIXED ? 0: instance.dimMap[topo]; }
    
      /** \brief Number of adjacent entities of a specified dimension 
       *
       * For a given element topology, get the number of adjacent entities
       * of the specified dimension.  
       */
    static unsigned adjacent( EntityTopology topo, unsigned dimension )
      { return (topo >= MIXED  || dimension > 2) ? 0 : instance.adjMap[topo][dimension]; }
    
      /** \brief Get number of sides a given topology type has 
       *
       * Get the number of sides for a given element topology.  Returns
       * the number of adjacent entities of one less dimension.  The number
       * of faces for a volume element and the number of edges for a face
       * element 
       */
    static unsigned sides( EntityTopology topo )
      { return (topo >= MIXED || instance.dimMap[topo] < 1) ? 0 : 
          instance.adjMap[topo][instance.dimMap[topo]-1]; }
    
      /** \brief Get the number of defining vertices for a given element topology 
       *
       * Get the number of corner vertices necessary to define an element
       * of the specified topology.  This is the number of nodes a linear
       * element of the specified topology will have.
       */
    static unsigned corners( EntityTopology topo )
      { return topo >= MIXED ? 0 : instance.adjMap[topo][0]; }
    
      /** \brief Get the number of edges in a given topology */
    static unsigned edges( EntityTopology topo )
      { return adjacent(topo, 1); }
      
      /** \brief Get the number of faces in a given topology */
    static unsigned faces( EntityTopology topo )
      { return adjacent(topo, 2 ); }
      
      /** \brief Check which mid-nodes a higher-order element has.
       *
       * Assuming at most one mid-node per sub-entity per dimension
       * (i.e. at most one mid-node per edge, at most one mid-node per face, etc.)
       * determine which mid-nodes are present given the topology
       * and total number of nodes.
       */
    static void higher_order( EntityTopology topo, unsigned num_nodes,
                              bool& midedge, bool& midface, bool& midvol,
                              MsqError &err );
    
      /**\brief Get indices of edge ends in element connectivity array 
       *
       * Given an edge number in (0,edges(type)], return which positions
       * in the connectivity list for the element type correspond to the
       * end vertices of that edge.
       */
    static const unsigned* edge_vertices( EntityTopology topo,
                                          unsigned edge_number,
                                          MsqError& err );
    
     /**\brief Get face corner indices in element connectivity array 
       *
       * Given an face number in (0,faces(type)], return which positions
       * in the connectivity list for the element type correspond to the
       * vertices of that face, ordered in a counter-clockwise cycle
       * around a vector pointing out of the element for an ideal element.
       */
    static const unsigned* face_vertices( EntityTopology topo,
                                          unsigned face_number,
                                          unsigned& num_vertices_out,
                                          MsqError& err );

    /**\brief Get corner indices of side 
     *
     * Get the indices into element connectivity list for the 
     * corners/ends of the specified side of the element.  
     * \ref edge_vertices and \ref face_vertices are special cases
     * of this method.  
     *
     * If the passed dimension equals that of the specified topology,
     * the side number is ignored and all the corners of the 
     * element are returned.  Fails if side dimension
     * greater than the dimension of the specified topology type.
     */
    static const unsigned* side_vertices( EntityTopology topo,
                                          unsigned side_dimension, 
                                          unsigned side_number,
                                          unsigned& num_verts_out,
                                          MsqError& err );

     
      
      /**\brief Return which side the specified mid-node lies on 
       *
       * Given an non-linear element type (specified by the
       * topology and length of the connectiivty array) and the 
       * index of a node in the element's connectivity array,
       * return the lower-dimension entity (side) of the element
       * the mid-node lies on.
       *
       *\param topo  Element topology
       *\param connectivity_length Number of nodes in element
       *\param node_index Which node of the element
       *\param side_dimension_out The dimension of the side containing the
       *             midnode (0 = vertex, 1 = edge, 2 = face, 3 = volume)
       *\param side_number_out The canonical number of the side 
       */
    static void side_number( EntityTopology topo, 
                             unsigned connectivity_length,
                             unsigned node_index,
                             unsigned& side_dimension_out,
                             unsigned& side_number_out,
                             MsqError& err );

    /**\brief  Get adjacent corner vertices
     *
     * Given the index of a vertex in an element, get the list of 
     * indices corresponding to the adjacent corner vertices.
     *
     * Adjcent corner vertex indices are returned in the proper
     * order for constructing the active matrix for the corner.
     *
     * Given the array v of all vertices in the patch, the array v_i
     * containing the connectivity list for an element as
     * indices into v, and adj as the result of this function for some
     * corner of the element, the corresponding active matrix A for
     * that corner can be constructed as:
     *  Matrix3D A;
     *  A.set_column( 0, v[v_i[adj[0]]] - v[v_i[0]] );
     *  A.set_column( 0, v[v_i[adj[1]]] - v[v_i[1]] );
     *  A.set_column( 0, v[v_i[adj[2]]] - v[v_i[2]] );
     *
     *\param topo  The element type
     *\param index The index of a corner vertex
     *\param num_adj_out The number of adjacent vertices (output)
     *\return The array of vertex indices
     */
    static const unsigned* adjacent_vertices( EntityTopology topo,
                                              unsigned index,
                                              unsigned& num_adj_out );
     
    /**\brief  Get reverse adjacency offsets
     *
     * Get reverse mapping of results from \ref adjacent_vertices.
     *
     * Let i be the input vertex index.  For each vertex index j
     * for which the result of \ref adjacent_vertices contains i, return
     * the offset into that result at which i would occur.  The
     * results are returned in the same order as each j is returned
     * in the results of adjacent_vertices(...,i,...).  Thus the
     * combination of the results of adjacent_vertices(...,i,...)
     * and this method provide a reverse mapping of the results of
     * adjacent_vertices(...,j,...) for i in all j.
     * 
     * Given:
     *   const unsigned *a, *b, *r;
     *   unsigned n, nn, c = corners(type);
     *   a = adjacent_vertices( type, i, n );            // for any i < c
     *   r = reverse_vertex_adjacency_offsets( type, i, n );
     *   b = adjacent_vertices( type, a[k], nn );        // for any k < n
     * Then:
     *   b[r[k]] == i
     */
    static const unsigned* reverse_vertex_adjacency_offsets( 
                                              EntityTopology topo,
                                              unsigned index,
                                              unsigned& num_idx_out );

  private:
 
    enum {
      MAX_CORNER = 8,
      MAX_EDGES = 12,
      MAX_FACES = 6,
      MAX_FACE_CONN = 5,
      MAX_VERT_ADJ = 4,
      FIRST_FACE = TRIANGLE,
      LAST_FACE = QUADRILATERAL,
      FIRST_VOL= TETRAHEDRON,
      LAST_VOL = PYRAMID
    };
    
    unsigned dimMap[MIXED];    /**< Get dimension of entity given topology */
    unsigned adjMap[MIXED][3]; /**< Get number of adj entities of dimension 0, 1 and dimension 2 */
    /** Vertex indices for element edges */
    unsigned edgeMap[LAST_VOL-FIRST_FACE+1][MAX_EDGES][2] ;
    /** Vertex indices for element faces */
    unsigned faceMap[LAST_VOL-FIRST_VOL+1][MAX_FACES][MAX_FACE_CONN];
    /** Vertex-Vertex adjacency map */
    unsigned vertAdjMap[LAST_VOL-FIRST_FACE+1][MAX_CORNER][MAX_VERT_ADJ+1];
    /** Reverse Vertex-Vertex adjacency index map */
    unsigned revVertAdjIdx[LAST_VOL-FIRST_FACE+1][MAX_CORNER][MAX_VERT_ADJ+1];

    TopologyInfo();
    
    static TopologyInfo instance;
  
};

} //namespace Mesquite

#endif
