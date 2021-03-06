/*
 * Copyright 2012 SciberQuest Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of SciberQuest Inc. nor the names of any contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __BOVReader_h
#define __BOVReader_h

#ifdef SQTK_WITHOUT_MPI
typedef void *MPI_Comm;
typedef void *MPI_Info;
#else
#include "SQMPICHWarningSupression.h" // for suppressing MPI warnings
#include <mpi.h> // for MPI_Comm and MPI_Info
#endif

#include "RefCountedPointer.h" // for RefCountedPointer
#include "BOVMetaData.h" // for BOVMetaData

class vtkDataSet;
class vtkAlgorithm;
class BOVScalarImageIterator;
class BOVArrayImageIterator;
class BOVTimeStepImage;
class CartesianDataBlockIODescriptor;

/// Low level reader for BOV files with domain decomposition capability.
/**
Given a domain and a set of files reads subsets of the files into
vtkImageData objects point data.

Calls that return an int generally return 0 to indicate an error.
*/
class BOVReader : public RefCountedPointer
{
public:
  static BOVReader *New(){ return new BOVReader; }

  /**
  Safely copying the reader.
  */
  const BOVReader &operator=(const BOVReader &other);

  /**
  Set the controller that will be used during IO and
  communication operations. Typically it's COMM_WORLD.
  */
  void SetCommunicator(MPI_Comm comm);
  MPI_Comm GetCommunicator(){ return this->Comm; }

  /**
  Set the info object conatining the file hints.
  Optional. If not set INFO_NULL is used.
  */
  void SetHints(MPI_Info hints);

  /**
  Set the metadata object that will interpret the metadata file,
  a deep copy of the passed in object is made prior to returning.
  See BOVMetaData for interface details.
  */
  void SetMetaData(const BOVMetaData *metaData);

  /**
  Get the active metadata object. Use this to querry the open dataset.
  See BOVMetaData.
  */
  BOVMetaData *GetMetaData() const { return this->MetaData; }

  /**
  Open a dataset. During open meta data is parsed but no
  heavy data is read.
  */
  int Open(const char *fileName);

  /**
  Return's true if the dataset has been successfully opened.
  */
  bool IsOpen();

  /**
  Close the dataset.
  */
  int Close();

  /**
  Set number of ghost cells to use with each sub-domain default
  is 1.
  */
  int GetNumberOfGhostCells(){ return this->NGhost; }
  void SetNumberOfGhostCells(int nGhost){ this->NGhost=nGhost; }


  /**
  Open a specific time step. This is done indepedently of the
  read so that if running out of core only a single open is
  requried.
  */
  BOVTimeStepImage *OpenTimeStep(int stepNo);
  void CloseTimeStep(BOVTimeStepImage *handle);

  /**
  Read the named set of arrays from disk, use "Add" methods to add arrays
  to be read.
  */
  int ReadTimeStep(
        const BOVTimeStepImage *handle,
        vtkDataSet *idds,
        vtkAlgorithm *exec=0);

  int ReadTimeStep(
        const BOVTimeStepImage *hanlde,
        const CartesianDataBlockIODescriptor *descr,
        vtkDataSet *grid,
        vtkAlgorithm *exec=0);

  int ReadMetaTimeStep(int stepNo, vtkDataSet *idds, vtkAlgorithm *exec=0);


  /**
  Get an instance of the appropriate dataset type needed to hold the
  data. May be one of vtkImageData, vtkRectilinearGrid, or vtkStructuredGrid.
  */
  vtkDataSet *GetDataSet();

  /**
  Return the string naming the dataset type, will be one of vtkImageData
  vtkRectilinearGrid, or vtkStructuredData.
  */
  const char *GetDataSetType() const
    {
    return this->MetaData->GetDataSetType();
    }

  /**
  Test for the named dataset type.
  */
  bool DataSetTypeIsImage() const
    {
    return this->MetaData->DataSetTypeIsImage();
    }
  bool DataSetTypeIsRectilinear()
    {
    return this->MetaData->DataSetTypeIsRectilinear();
    }
  bool DataSetTypeIsStructured()
    {
    return this->MetaData->DataSetTypeIsStructured();
    }

  /**
  Project a 3D vector field into axis aligned 2D space.
  */
  enum {
    VECTOR_PROJECT_NONE=0x000,
    VECTOR_PROJECT_2D_XY=0x100,
    VECTOR_PROJECT_2D_XZ=0x010,
    VECTOR_PROJECT_2D_YZ=0x001
    };
  void SetVectorProjection(int mode){ this->VectorProjection=mode; }
  int GetVectorProjection(){ return this->VectorProjection; }

  /**
  Print internal state.
  */
  void PrintSelf(std::ostream &os);

protected:
  BOVReader();
  BOVReader(const BOVReader &other);
  ~BOVReader();

private:
  /**
  Read the array from the specified file into point data in a single
  pass.
  */
  int ReadScalarArray(const BOVScalarImageIterator &it, vtkDataSet *grid);
  int ReadVectorArray(const BOVArrayImageIterator &it, vtkDataSet *grid);
  int ReadSymetricTensorArray(const BOVArrayImageIterator &it, vtkDataSet *grid);

  /**
  Read the array from the specified file into point data in multiple
  passes described by the IO descriptor.
  */
  int ReadScalarArray(
        const BOVScalarImageIterator &fhit,
        const CartesianDataBlockIODescriptor *descr,
        vtkDataSet *grid);

  int ReadVectorArray(
        const BOVArrayImageIterator &fhit,
        const CartesianDataBlockIODescriptor *descr,
        vtkDataSet *grid);

  int ReadSymetricTensorArray(
        const BOVArrayImageIterator &fhit,
        const CartesianDataBlockIODescriptor *descr,
        vtkDataSet *grid);

private:
  BOVMetaData *MetaData;     // Object that knows how to interpret dataset.
  int NGhost;                // Number of ghost nodes, default is 1.
  int ProcId;                // My process id.
  int NProcs;                // Number of processes.
  MPI_Comm Comm;             // Communicator handle
  MPI_Info Hints;            // MPI-IO file hints.
  int VectorProjection;      // Option to project onto axis aligned plane.
};

#endif

// VTK-HeaderTest-Exclude: BOVReader.h
