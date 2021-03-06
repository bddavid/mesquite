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

/*! \file InstructionQueue.cpp

Member functions of the Mesquite::InstructionQueue class

  \author Thomas Leurent
  \date   2002-05-01
 */

#ifdef MSQ_USE_OLD_STD_HEADERS
#  include <string.h>
#  include <list.h>
#  include <memory.h>
#else
#  include <string>
#  include <list>
#  include <memory>
#endif

#include "InstructionQueue.hpp"
#include "MsqInterrupt.hpp"
#include "QualityImprover.hpp"
#include "QualityAssessor.hpp"
#include "MsqError.hpp"
#include "MsqDebug.hpp"
#include "MeanMidNodeMover.hpp"
#include "MsqFPE.hpp"
#include "TargetCalculator.hpp"

using namespace Mesquite;

#ifdef MSQ_TRAP_FPE
const bool IQ_TRAP_FPE_DEFAULT = true;
#else
const bool IQ_TRAP_FPE_DEFAULT = false;
#endif


InstructionQueue::InstructionQueue() :
  autoQualAssess(true),
  autoAdjMidNodes(true),
  nbPreConditionners(0),
  isMasterSet(false),
  masterInstrIndex(0),
  trapFPE(IQ_TRAP_FPE_DEFAULT)
{
}


void InstructionQueue::add_target_calculator( TargetCalculator* tc, MsqError& )
{
  instructions.push_back( tc );
}

/*! \fn InstructionQueue::add_preconditioner(QualityImprover* instr, MsqError &err)
    \brief adds a QualityImprover at the end of the instruction list

    This function cannot be used once the set_master_quality_improver()
    function has been used.
    
    See also insert_preconditioner().
  */
void InstructionQueue::add_preconditioner(QualityImprover* instr,
                                        MsqError &err)
{
  if (isMasterSet) {
    MSQ_SETERR(err)("Cannot add preconditionners once the master "
                    "QualityImprover has been set.", MsqError::INVALID_STATE);
    return;
  }
  
  instructions.push_back(instr);
  nbPreConditionners++;
}


/*! \fn InstructionQueue::remove_preconditioner(size_t index, MsqError &err)
    \brief removes a QualityImprover* from the instruction queue

    \param index is 0-based. An error is set if the index does not correspond
           to a valid element in the queue.
*/
void InstructionQueue::remove_preconditioner(size_t index, MsqError &err)
{
  // checks index is valid
  if ( isMasterSet && index == masterInstrIndex ) {
    MSQ_SETERR(err)("cannot remove master QualityImprover.", MsqError::INVALID_ARG);
    return;
  } else if (index >= instructions.size() ) {
    MSQ_SETERR(err)("Index points beyond end of list.",MsqError::INVALID_ARG);
    return;
  }
  
  // position the instruction iterator over the preconditionner to delete
  msq_std::list<PatchDataUser*>::iterator pos;
  pos = instructions.begin();
  msq_std::advance(pos, index);

  if ( (*pos)->get_algorithm_type() != PatchDataUser::QUALITY_IMPROVER ) 
  {
    MSQ_SETERR(err)("Index does not point to a QualityImprover.",
                    MsqError::INVALID_ARG);
    return;
  }
  
  msq_std::string name = (*pos)->get_name();
  instructions.erase(pos);
  nbPreConditionners--;
}  


/*! \fn InstructionQueue::insert_preconditioner(QualityImprover* instr, size_t index, MsqError &err)
    \brief inserts a QualityImprover* into the instruction queue.

    Pre-conditionners can only be inserted before the master QualityImprover.

    \param index is 0-based. An error is set if the index does not correspond
           to a valid position in the queue.
*/
void InstructionQueue::insert_preconditioner(QualityImprover* instr,
                                           size_t index, MsqError &err)
{
  // checks index is valid
  if (isMasterSet==true && index > masterInstrIndex) {
    MSQ_SETERR(err)("Cannot add a preconditionner after the master "
                    "QualityImprover.", MsqError::INVALID_STATE);
    return;
  }
  if (index >= instructions.size() ) {
    MSQ_SETERR(err)("index", MsqError::INVALID_ARG);
    return;
  }

  // position the instruction iterator
  msq_std::list<PatchDataUser*>::iterator pos;
  pos = instructions.begin();
  msq_std::advance(pos, index);
  // adds the preconditioner
  instructions.insert(pos,instr);
  nbPreConditionners++;
}


/*! \fn InstructionQueue::add_quality_assessor(QualityAssessor* instr, MsqError &err)
    \brief adds a QualityAssessor to the instruction queue.

    QualityAssessor pointers can be added at any time to the instruction queue.
*/
void InstructionQueue::add_quality_assessor(QualityAssessor* instr,
                                            MsqError &/*err*/)
{
  instructions.push_back(instr);
}


/*! \fn InstructionQueue::remove_quality_assessor(size_t index, MsqError &err)
    \brief removes a QualityAssessor* from the instruction queue

    \param index is 0-based. An error is set if the index does not correspond
           to a valid element in the queue.
*/
void InstructionQueue::remove_quality_assessor(size_t index, MsqError &err)
{
  // checks index is valid
  if (index >= instructions.size() ) {
    MSQ_SETERR(err)("index", MsqError::INVALID_ARG);
    return;
  }
  
  // position the instruction iterator over the QualityAssessor to delete
  msq_std::list<PatchDataUser*>::iterator pos;
  pos = instructions.begin();
  msq_std::advance(pos, index);

  if ( (*pos)->get_algorithm_type() != PatchDataUser::QUALITY_ASSESSOR ) 
  {
    MSQ_SETERR(err)("Index does not point to a QualityImprover.",
                    MsqError::INVALID_ARG);
    return;
  }
  
  msq_std::string name = (*pos)->get_name();
  instructions.erase(pos);
}  


/*! \fn InstructionQueue::insert_quality_assessor(QualityAssessor* instr, size_t index, MsqError &err)
    \brief inserts a QualityAssessor* into the instruction queue.

    QualityAssessors can be inserted at any position in the instruction queue.

    \param index is 0-based. An error is set if the index is past the end of the queue.
*/
void InstructionQueue::insert_quality_assessor(QualityAssessor* instr,
                                           size_t index, MsqError &err)
{
  // checks index is valid
  if (index > instructions.size()) {
    MSQ_SETERR(err)("index points two positions beyond end of list.",
                    MsqError::INVALID_ARG);
    return;
  }

  // position the instruction iterator
  msq_std::list<PatchDataUser*>::iterator pos;
  pos = instructions.begin();
  msq_std::advance(pos, index);
  // adds the QualityAssessor
  instructions.insert(pos,instr);
}


void InstructionQueue::set_master_quality_improver(QualityImprover* instr,
                                                 MsqError &err)
{
  if (isMasterSet) {
    MSQ_DBGOUT(1) << "InstructionQueue::set_master_quality_improver():\n"
        << "\tOverwriting previously specified master quality improver.\n";
    // if master is already set, clears it and insert the new one at the same position.
    msq_std::list<PatchDataUser*>::iterator master_pos;
    master_pos = this->clear_master(err); MSQ_ERRRTN(err);
    instructions.insert(master_pos, instr);
    isMasterSet = true;
  } else {
    // if master is not set, add it at the end of the queue.
    instructions.push_back(instr);
    isMasterSet = true;
    masterInstrIndex = instructions.size()-1;
  }
}

void InstructionQueue::run_instructions( Mesh* mesh, 
                                         MeshDomain* domain,
                                         MsqError &err)
{ 
  MSQ_DBGOUT(1) << version_string(false) << "\n";

  if (nbPreConditionners != 0 && isMasterSet == false ) {
    MSQ_SETERR(err)("no pre-conditionners allowed if master QualityImprover "
                    "is not set.", MsqError::INVALID_STATE);
    return;
  }
  
#ifdef ENABLE_INTERRUPT
   // Register SIGINT handler
  MsqInterrupt msq_interrupt;
#endif

    // Generate SIGFPE on floating point errors
  MsqFPE( this->trapFPE );
  
  msq_std::list<PatchDataUser*>::const_iterator instr;
  
    // Create a global patch if anything in the instruction queue
    // requires it.
  PatchData global_patch;
  PatchData* global_patch_ptr = 0;
  global_patch.set_mesh( mesh );
  global_patch.set_domain( domain );
  
    // Run each instruction
  for (instr = instructions.begin(); instr != instructions.end(); ++instr) 
  {
    if (MsqInterrupt::interrupt())
    {
      MSQ_SETERR(err)(MsqError::INTERRUPTED);
      return;
    }
    
    if ((*instr)->get_patch_type() == PatchData::GLOBAL_PATCH)
    {
      if (!global_patch_ptr)
      {
        global_patch.fill_global_patch( err );
        MSQ_ERRRTN(err);
        global_patch_ptr = &global_patch;
      }
    }
    else if (global_patch_ptr)
    {
      global_patch_ptr = 0;
    }
    
    (*instr)->loop_over_mesh( mesh, domain, global_patch_ptr, err ); 
    MSQ_ERRRTN(err);
  }
  
  if (autoAdjMidNodes)
  {
    MeanMidNodeMover tool;
    tool.loop_over_mesh( mesh, domain, global_patch_ptr, err );
    MSQ_ERRRTN(err);
  }
}

  
void InstructionQueue::clear()
{
  instructions.clear();
  autoQualAssess = true;
  autoAdjMidNodes = false;
  isMasterSet = false;
  masterInstrIndex = 0;
}


msq_std::list<PatchDataUser*>::iterator InstructionQueue::clear_master(MsqError &err)
{
  msq_std::list<PatchDataUser*>::iterator instr_iter;
  msq_std::list<PatchDataUser*>::iterator master_pos;
  
  if (!isMasterSet) {
    MSQ_SETERR(err)("No master quality improver to clear.", MsqError::INVALID_STATE);
    return instr_iter;
  }
  
    // position the instruction iterator over the master quality improver
  master_pos = instructions.begin();
  msq_std::advance(master_pos, masterInstrIndex);
  
    // erases the master quality improver
  instr_iter = instructions.erase(master_pos);
  isMasterSet = false;
  
    // returns the position where the Master was
  return instr_iter;
}
