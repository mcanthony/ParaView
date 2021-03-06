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
#include "vtkSQPlaneSourceCellGenerator.h"

#include "vtkObjectFactory.h"

#include "Numerics.hxx"
#include "Tuple.hxx"


//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSQPlaneSourceCellGenerator);

//-----------------------------------------------------------------------------
vtkSQPlaneSourceCellGenerator::vtkSQPlaneSourceCellGenerator()
{
  this->Resolution[0]=
  this->Resolution[1]=
  this->Resolution[2]=1;

  this->Origin[0]=
  this->Origin[1]=
  this->Origin[2]=0.0;

  this->Point1[0]=0.0;
  this->Point1[1]=1.0;
  this->Point1[2]=0.0;

  this->Point2[0]=1.0;
  this->Point2[1]=0.0;
  this->Point2[2]=0.0;

  this->Dx[0]=0.0;
  this->Dx[1]=1.0;
  this->Dx[2]=0.0;

  this->Dy[0]=1.0;
  this->Dy[1]=0.0;
  this->Dy[2]=0.0;
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::SetResolution(int *r)
{
  this->Resolution[0]=r[0];   // ncx
  this->Resolution[1]=r[1];   // ncy
  this->Resolution[2]=r[0]+1; // npx
  this->ComputeDeltas();
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::SetResolution(int r1, int r2)
{
  this->Resolution[0]=r1;    // ncx
  this->Resolution[1]=r2;    // ncy
  this->Resolution[2]=r1+1;  // npx
  this->ComputeDeltas();
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::SetOrigin(double *x)
{
  this->Origin[0]=x[0];
  this->Origin[1]=x[1];
  this->Origin[2]=x[2];
  this->ComputeDeltas();
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::SetOrigin(double x, double y, double z)
{
  this->Origin[0]=x;
  this->Origin[1]=y;
  this->Origin[1]=z;
  this->ComputeDeltas();
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::SetPoint1(double *x)
{
  this->Point1[0]=x[0];
  this->Point1[1]=x[1];
  this->Point1[2]=x[2];
  this->ComputeDeltas();
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::SetPoint1(double x, double y, double z)
{
  this->Point1[0]=x;
  this->Point1[1]=y;
  this->Point1[1]=z;
  this->ComputeDeltas();
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::SetPoint2(double *x)
{
  this->Point2[0]=x[0];
  this->Point2[1]=x[1];
  this->Point2[2]=x[2];
  this->ComputeDeltas();
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::SetPoint2(double x, double y, double z)
{
  this->Point2[0]=x;
  this->Point2[1]=y;
  this->Point2[1]=z;
  this->ComputeDeltas();
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::ComputeDeltas()
{
  if (this->Resolution[0]<1 || this->Resolution[1]<1 )
    {
    vtkErrorMacro(
      << "Invalid resolution " << Tuple<int>(this->Resolution,2) << ".");
    return;
    }

  this->Dx[0]=(this->Point1[0]-this->Origin[0])/this->Resolution[0];
  this->Dx[1]=(this->Point1[1]-this->Origin[1])/this->Resolution[0];
  this->Dx[2]=(this->Point1[2]-this->Origin[2])/this->Resolution[0];

  this->Dy[0]=(this->Point2[0]-this->Origin[0])/this->Resolution[1];
  this->Dy[1]=(this->Point2[1]-this->Origin[1])/this->Resolution[1];
  this->Dy[2]=(this->Point2[2]-this->Origin[2])/this->Resolution[1];
}

//-----------------------------------------------------------------------------
int vtkSQPlaneSourceCellGenerator::GetCellPointIndexes(
      vtkIdType cid,
      vtkIdType *idx)
{
  int i,j;
  indexToIJ((int)cid,this->Resolution[0],i,j);

  // vertex indices
  int I[12]={
      i  ,j  ,0,
      i+1,j  ,0,
      i+1,j+1,0,
      i  ,j+1,0
      };

  for (int q=0; q<4; ++q)
    {
    int qq=q*3;
    idx[q]=I[qq+1]*this->Resolution[2]+I[qq];
    }

  return 4;
}

//-----------------------------------------------------------------------------
int vtkSQPlaneSourceCellGenerator::GetCellPoints(vtkIdType cid, float *pts)
{
  // cell index (also lower left pt index)
  int i,j;
  indexToIJ((int)cid,this->Resolution[0],i,j);

  // vertex indices
  int I[12]={
      i  ,j  ,0,
      i+1,j  ,0,
      i+1,j+1,0,
      i  ,j+1,0
      };

  float dx[3]={
      ((float)this->Dx[0]),
      ((float)this->Dx[1]),
      ((float)this->Dx[2])};

  float dy[3]={
      ((float)this->Dy[0]),
      ((float)this->Dy[1]),
      ((float)this->Dy[2])};

  float O[3]={
      ((float)this->Origin[0]),
      ((float)this->Origin[1]),
      ((float)this->Origin[2])};

  for (int q=0; q<4; ++q)
    {
    int qq=q*3;
    pts[qq  ]=O[0]+((float)I[qq])*dx[0]+((float)I[qq+1])*dy[0];
    pts[qq+1]=O[1]+((float)I[qq])*dx[1]+((float)I[qq+1])*dy[1];
    pts[qq+2]=O[2]+((float)I[qq])*dx[2]+((float)I[qq+1])*dy[2];
    }

  return 4;
}

//-----------------------------------------------------------------------------
int vtkSQPlaneSourceCellGenerator::GetCellTextureCoordinates(
        vtkIdType cid,
        float *pts)
{
  // cell index (also lower left pt index)
  int i,j;
  indexToIJ((int)cid,this->Resolution[0],i,j);

  // vertex indices
  int I[8]={
      i  ,j  ,
      i+1,j  ,
      i+1,j+1,
      i  ,j+1
      };

  // cell to point
  float npx=this->Resolution[0];
  float npy=this->Resolution[1];

  for (int q=0; q<4; ++q)
    {
    int qq=q*2;
    pts[qq  ]=I[qq]/npx;
    pts[qq+1]=I[qq+1]/npy;
    }

  return 4;
}

//-----------------------------------------------------------------------------
void vtkSQPlaneSourceCellGenerator::PrintSelf(ostream& os, vtkIndent indent)
{
  (void)os;
  (void)indent;
}
