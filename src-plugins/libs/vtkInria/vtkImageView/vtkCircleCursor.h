/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "vtkImageViewExport.h"
#include <vector>      // For returning Polygon Points
#include "vtkVector.h" // For Polygon Points
#include <vtkObject.h>
#include <vtkRenderWindow.h>

class vtkUnsignedCharArray;

class VTK_IMAGEVIEW_EXPORT vtkCircleCursor : public vtkObject 
{
public:
  static vtkCircleCursor *New();
  vtkTypeMacro(vtkCircleCursor, vtkObject);
  
  vtkSetMacro(Radius, unsigned int);
  vtkGetMacro(Radius, unsigned int);

  vtkSetMacro(RenWin, vtkRenderWindow*);
  vtkGetMacro(RenWin, vtkRenderWindow*);

  vtkSetMacro(Spacing, double*);
  vtkGetMacro(Spacing, double*);

  vtkSetMacro(CenterPoint, vtkVector2i*);
  vtkGetMacro(CenterPoint, vtkVector2i*);
  
  vtkSetMacro(Color,float*);
  vtkGetMacro(Color,float*);

  vtkSetMacro(Initialize,bool);
  vtkGetMacro(Initialize,bool);

  virtual void DrawCirleInPosition(int x,int y);
 
protected:
  vtkCircleCursor();
  ~vtkCircleCursor();
  
  unsigned int Radius;
  double* Spacing;
  
  vtkRenderWindow * RenWin;
  vtkUnsignedCharArray *PixelArray;

  float *Color; // [0;1]
  bool Initialize;

  vtkVector2i * CenterPoint;

  class vtkBasicOp;
  vtkBasicOp * Op;
    
private:
  vtkCircleCursor(const vtkCircleCursor&);  // Not implemented
  void operator=(const vtkCircleCursor&);  // Not implemented
};

