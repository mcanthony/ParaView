/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCompleteArrays.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCompleteArrays.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkDataArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkMultiProcessController.h"
#include "vtkIdTypeArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkShortArray.h"
#include "vtkLongArray.h"
#include "vtkCharArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkUnsignedCharArray.h"

#include "vtkPVDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkPVArrayInformation.h"

vtkCxxRevisionMacro(vtkCompleteArrays, "1.2");
vtkStandardNewMacro(vtkCompleteArrays);


vtkCxxSetObjectMacro(vtkCompleteArrays,Controller,vtkMultiProcessController);

//-----------------------------------------------------------------------------
vtkCompleteArrays::vtkCompleteArrays()
{
  this->Controller = vtkMultiProcessController::GetGlobalController();
  if (this->Controller)
    {
    this->Controller->Register(this);
    }
}

//-----------------------------------------------------------------------------
vtkCompleteArrays::~vtkCompleteArrays()
{
  if (this->Controller)
    {
    this->Controller->UnRegister(this);
    this->Controller = NULL;
    }

}

//-----------------------------------------------------------------------------
void vtkCompleteArrays::Execute()
{
  vtkDataSet *input = this->GetInput();
  vtkDataSet *output = this->GetOutput();
  int noNeed = 0;
  int myProcId;
  int numProcs;
  int idx;
  int length;
  unsigned char* msg;

  // Initialize
  //
  vtkDebugMacro(<<"Generating elevation scalars!");

  this->GetOutput()->CopyStructure( input );
  this->GetOutput()->GetPointData()->PassData(input->GetPointData());
  this->GetOutput()->GetCellData()->PassData(input->GetCellData());

  myProcId = this->Controller->GetLocalProcessId();
  numProcs = this->Controller->GetNumberOfProcesses();

  if (myProcId == 0)
    {
    if (input->GetNumberOfPoints() > 0 && input->GetNumberOfCells() > 0)
      {
      noNeed = 1;
      }
    for (idx = 1; idx < numProcs; ++idx)
      {
      this->Controller->Send(&noNeed, 1, idx, 3389001);
      }
    if (noNeed)
      {
      return;
      }
    // Receive and collected information from the remote processes.
    vtkPVDataInformation* dataInfo = vtkPVDataInformation::New();
    vtkPVDataInformation* tmpInfo = vtkPVDataInformation::New();
    for (idx = 1; idx < numProcs; ++idx)
      {
      this->Controller->Receive(&length, 1, idx, 3389002);
      msg = new unsigned char[length];
      this->Controller->Receive(msg, length, idx, 3389003);
      tmpInfo->CopyFromMessage(msg);
      dataInfo->AddInformation(tmpInfo);
      }
    this->FillArrays(output->GetPointData(), dataInfo->GetPointDataInformation());
    this->FillArrays(output->GetCellData(), dataInfo->GetCellDataInformation());
    dataInfo->Delete();
    tmpInfo->Delete();
    }
  else
    { // remote processes
    this->Controller->Receive(&noNeed, 1, 0, 3389001);
    if (noNeed)
      {
      return;
      }    
    vtkPVDataInformation* dataInfo = vtkPVDataInformation::New();
    dataInfo->CopyFromObject(output);
    length = dataInfo->GetMessageLength();
    msg = new unsigned char[length];
    dataInfo->WriteMessage(msg);
    this->Controller->Send(&length, 1, 0, 3389002);
    this->Controller->Send(msg, length, 0, 3389003);
    dataInfo->Delete();
    delete [] msg;
    }
}


//-----------------------------------------------------------------------------
void vtkCompleteArrays::FillArrays(vtkDataSetAttributes* da, 
                                   vtkPVDataSetAttributesInformation* attrInfo)
{
  int num, idx;
  vtkPVArrayInformation* arrayInfo;
  vtkDataArray* array;

  da->Initialize();
  num = attrInfo->GetNumberOfArrays();
  for (idx = 0; idx < num; ++idx)
    {
    arrayInfo = attrInfo->GetArrayInformation(idx);
    switch (arrayInfo->GetDataType())
      {
      case VTK_FLOAT:
        array = vtkFloatArray::New();
        break;
      case VTK_DOUBLE:
        array = vtkDoubleArray::New();
        break;
      case VTK_INT:
        array = vtkIntArray::New();
        break;
      case VTK_CHAR:
        array = vtkCharArray::New();
        break;
      case VTK_ID_TYPE:
        array = vtkIdTypeArray::New();
        break;
      case VTK_LONG:
        array = vtkLongArray::New();
        break;
      case VTK_SHORT:
        array = vtkShortArray::New();
        break;
      case VTK_UNSIGNED_CHAR:
        array = vtkUnsignedCharArray::New();
        break;
      case VTK_UNSIGNED_INT:
        array = vtkUnsignedIntArray::New();
        break;
      case VTK_UNSIGNED_LONG:
        array = vtkUnsignedLongArray::New();
        break;
      case VTK_UNSIGNED_SHORT:
        array = vtkUnsignedShortArray::New();
        break;
      default:
        array = NULL;
      }
    if (array)
      {
      array->SetNumberOfComponents(arrayInfo->GetNumberOfComponents());
      array->SetName(arrayInfo->GetName());
      switch (attrInfo->IsArrayAnAttribute(idx))
        {
        case vtkDataSetAttributes::SCALARS:
          da->SetScalars(array);
          break;
        case vtkDataSetAttributes::VECTORS:
          da->SetVectors(array);
          break;
        case vtkDataSetAttributes::NORMALS:
          da->SetNormals(array);
          break;
        case vtkDataSetAttributes::TENSORS:
          da->SetTensors(array);
          break;
        case vtkDataSetAttributes::TCOORDS:
          da->SetTCoords(array);
          break;
        default:
          da->AddArray(array);
        }
      array->Delete();
      array = NULL;
      }
    }
}


//-----------------------------------------------------------------------------
void vtkCompleteArrays::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->Controller)
    {
    os << indent << "Controller: " << this->Controller << endl;
    }
}
