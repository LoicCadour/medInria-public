#ifndef _vtk_LandmarkWidget_h_
#define _vtk_LandmarkWidget_h_

#include <vtkSphereWidget.h>
#include <vtkHandleWidget.h>
#include <vtkLandmark.h>
#include <vtkCommand.h>
#include <vtkSetGet.h>

//BTX
class VTK_EXPORT vtkLandmarkWidgetCommand : public vtkCommand
{    
 public:  
  static  vtkLandmarkWidgetCommand* New() { return new vtkLandmarkWidgetCommand; }
  void Execute(vtkObject *   caller, 
               unsigned long event, 
               void *        callData);
  void SetLandmark (vtkSphereWidget* l);

  void SetWidget2D (vtkHandleWidget * widget);

 protected:
  vtkLandmarkWidgetCommand()
  {
    this->Landmark = NULL;
  }
  ~vtkLandmarkWidgetCommand(){}  
  
 private:
  vtkSphereWidget* Landmark;
  vtkHandleWidget * widget2D;
};

class VTK_EXPORT vtkLandmarkWidget : public vtkSphereWidget
{
 public:
  static vtkLandmarkWidget* New();
  vtkTypeRevisionMacro(vtkLandmarkWidget, vtkSphereWidget);
  vtkGetObjectMacro (Command, vtkLandmarkWidgetCommand);
  vtkGetMacro (Value, double);
  vtkSetMacro (Value, double);  

  vtkGetObjectMacro(SphereActor, vtkActor);
  vtkGetObjectMacro(HandleActor, vtkActor);

  virtual void SetEnabled(int);
  vtkHandleWidget * GetWidget2D();
  int * GetIndices(){return indices;};
  void SetIndices(int ind[3]){indices[0]=ind[0];indices[1]=ind[1];indices[2]=ind[2];};
  protected:
  vtkLandmarkWidget();
  ~vtkLandmarkWidget();
  
 private:
  vtkLandmarkWidgetCommand* Command;
  double Value;
  vtkHandleWidget * widget2D;
  int indices[3]; // indices in image
};

//ETX

#endif //_vtk_LandmarkWidget_h_
