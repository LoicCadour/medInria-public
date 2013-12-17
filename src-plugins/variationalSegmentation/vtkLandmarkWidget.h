#ifndef _vtk_LandmarkWidget_h_
#define _vtk_LandmarkWidget_h_

#include <vtkSphereWidget.h>
#include <vtkHandleWidget.h>
#include <vtkLandmark.h>
#include <vtkCommand.h>
#include <vtkSetGet.h>
#include <vtkImageView2D.h>
#include <vtkImageView3D.h>
#include <qlist.h>

class vtkLandmarkWidget;

//BTX
class VTK_EXPORT vtkLandmarkWidgetCommand : public vtkCommand
{    
 public:  
  static  vtkLandmarkWidgetCommand* New() { return new vtkLandmarkWidgetCommand; }
  void Execute(vtkObject *   caller, 
               unsigned long event, 
               void *        callData);
  void SetLandmark (vtkLandmarkWidget* l);

  void SetWidget2D (vtkHandleWidget * widget);

 protected:
  vtkLandmarkWidgetCommand()
  {
    this->Landmark = NULL;
  }
  ~vtkLandmarkWidgetCommand(){}  
  
 private:
  vtkLandmarkWidget* Landmark;
  vtkHandleWidget * Widget2D;
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
  
  int * GetIndices(){return indices;};
  void SetIndices(int ind[3]){indices[0]=ind[0];indices[1]=ind[1];indices[2]=ind[2];};

  QList<vtkImageView2D*> * GetViews2D();
  void SetViews2D(QList<vtkImageView2D*> * Views,int Ind);
  vtkGetObjectMacro(View3D,vtkImageView3D);
  vtkSetObjectMacro(View3D,vtkImageView3D);

  vtkGetObjectMacro(Widget2D,vtkHandleWidget);
  
  vtkGetMacro(ToDelete,bool);
  vtkSetMacro(ToDelete,bool);
  vtkGetMacro(IndView2D,int);

  void showOrHide2DWidget();
  void updateLandmarksPosFromWidget2D();

  protected:
  vtkLandmarkWidget();
  ~vtkLandmarkWidget();
  
 private:
  vtkLandmarkWidgetCommand* Command;
  vtkHandleWidget * Widget2D;
  int IndView2D;
  QList<vtkImageView2D*> * Views2D;
  vtkImageView3D * View3D;
  double Value;
  int indices[3]; // indices in image
  bool ToDelete;



};

//ETX

#endif //_vtk_LandmarkWidget_h_
