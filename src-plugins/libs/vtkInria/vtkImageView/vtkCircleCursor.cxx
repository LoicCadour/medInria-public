/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "vtkCircleCursor.h"

#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkUnsignedCharArray.h"

#include <vnl/vnl_cross.h>
#include <vnl/vnl_vector.h>
#include <algorithm>
#include <cmath>

vtkStandardNewMacro(vtkCircleCursor);

//-----------------------------------------------------------------------------
class vtkCircleCursor::vtkBasicOp
{
public:
  std::vector<vtkVector2i> points;

  void AddPoint(const vtkVector2i &point)
    {
    this->points.push_back(point);
    }

  void AddPoint(int x, int y)
    {
    this->AddPoint(vtkVector2i(x, y));
    }

  vtkVector2i GetPoint(vtkIdType index) const
    {
    return this->points[index];
    }

  vtkIdType GetNumberOfPoints() const
    {
    return this->points.size();
    }

  void Clear()
    {
    this->points.clear();
    }

  void ComputeCircle(const vtkVector2i * Center, double radius, double* Spacing,unsigned char *pixels,int *size,float * color)
  {
      double radius2 = radius*radius;
      const int Nx = std::max( 1, (int)std::ceil(radius/Spacing[0]));
      const int Ny = std::max( 1, (int)std::ceil(radius/Spacing[1]));
      int row,col;
      int xc = Center->GetX();
      int yc = Center->GetY();

      for ( int y(-Ny); y <= Ny; ++y )
      {
          double dy = y *Spacing[1];
          for ( int x(-Nx); x <= Nx; ++x ) 
          {
              double dx = x *Spacing[0];
              if (dx*dx + dy*dy > radius2) // add +- epsilon
                  continue;

              col =  xc + dx;
              row =  yc + dy;
              if (col<size[0] && row<size[1])
              {
                  double epsilon = radius2/10;
                  if (dx*dx +dy*dy >= radius2-epsilon)
                  {
                      // adds a contour to the cursor.
                      pixels[3*(row*size[0]+col)] = 255*color[0];
                      pixels[3*(row*size[0]+col)+1] = 255*color[1];
                      pixels[3*(row*size[0]+col)+2] = 255*color[2];
                  }
                  else
                  {
                      // if u want negative effect inside the cursor
                      /*pixels[3*(row*size[0]+col)] = 255 ^ pixels[3*(row*size[0]+col)]; 
                      pixels[3*(row*size[0]+col)+1] = 255 ^ pixels[3*(row*size[0]+col)+1];
                      pixels[3*(row*size[0]+col)+2] = 255 ^ pixels[3*(row*size[0]+col)+2];*/
                      
                      // if you want transparent effect
                      pixels[3*(row*size[0]+col)] = pixels[3*(row*size[0]+col)];
                      pixels[3*(row*size[0]+col)+1] =pixels[3*(row*size[0]+col)+1];
                      pixels[3*(row*size[0]+col)+2] =pixels[3*(row*size[0]+col)+2];
                  }
              }
          }
      }
  }
};

//----------------------------------------------------------------------------
vtkCircleCursor::vtkCircleCursor()
{
  this->Op = new vtkBasicOp();
  this->PixelArray = vtkUnsignedCharArray::New();
  this->Radius = 0;
  this->RenWin = 0;
  this->Initialize = true;
  this->CenterPoint = new vtkVector2i();
  this->Spacing = new double(2);
  this->Spacing[0]=1;
  this->Spacing[1]=1;
  this->Color = new float(3);
  this->Color[0] = 1;
  this->Color[1] = 1;
  this->Color[2] = 1;
}

//----------------------------------------------------------------------------
vtkCircleCursor::~vtkCircleCursor()
{
  this->PixelArray->Delete();
  delete this->Op;
}

//----------------------------------------------------------------------------

void vtkCircleCursor::DrawCirleInPosition(int x,int y)
{
    if (!RenWin || this->Radius==0)
        return;
    
    int *size = RenWin->GetSize();

    if (this->Initialize)
    {
        this->Initialize = false;
        this->PixelArray->Initialize();
        this->PixelArray->SetNumberOfComponents(3);
        this->PixelArray->SetNumberOfTuples(size[0]*size[1]);
        this->RenWin->GetPixelData(0, 0, size[0]-1, size[1]-1, 1, this->PixelArray);
    }

    if (x > (size[0]-Radius))
    {
        x = size[0]-Radius*2;
    }
    if (x < Radius)
    {
        x = Radius*2;
    }
    if (y > (size[1]-Radius))
    {
        y = size[1]-Radius*2;
    }
    if (y < Radius)
    {
        y = Radius*2;
    }

    if (x>=size[0]||y>=size[1]||y<=0||x<=0)
        return;

    CenterPoint->SetX(x);
    CenterPoint->SetY(y);

    vtkNew<vtkUnsignedCharArray> tmpPixelArray;
    tmpPixelArray->DeepCopy(this->PixelArray);
    unsigned char *pixels = tmpPixelArray->GetPointer(0);

    this->Op->ComputeCircle(CenterPoint,Radius,this->Spacing,pixels,size,this->Color);
    this->RenWin->SetPixelData(0, 0, size[0]-1, size[1]-1, pixels, 1);
}