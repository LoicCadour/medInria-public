#include <vtkLandmarkWidget.h>

#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkImageView2D.h>
#include <vtkPointHandleRepresentation2D.h>

vtkStandardNewMacro(vtkLandmarkWidget);
vtkCxxRevisionMacro(vtkLandmarkWidget, "$Revision: 1315 $");




vtkLandmarkWidget::vtkLandmarkWidget()
{
  this->Command = vtkLandmarkWidgetCommand::New();
  this->Command->SetLandmark (this);
  this->Value = 0.0;
  this->widget2D = vtkHandleWidget::New();
}

vtkLandmarkWidget::~vtkLandmarkWidget()
{
  this->Command->Delete();
}

void vtkLandmarkWidgetCommand::Execute(vtkObject *   caller, 
				       unsigned long event, 
				       void *        callData)
{
  vtkSphereWidget* l = vtkSphereWidget::SafeDownCast(caller);
  vtkHandleWidget * widget = vtkHandleWidget::SafeDownCast(caller);

  if (event == vtkCommand::InteractionEvent)
  {
    if (this->Landmark && l)
    {
      this->Landmark->SetCenter (l->GetCenter());
      if (this->Landmark->GetInteractor())
	this->Landmark->GetInteractor()->Render();
      if (this->Landmark==l)
          std::cout<<"i hate u dude!";
    }
    if (this->widget2D == widget)
    {
        if (this->Landmark)
            this->Landmark->SetCenter(dynamic_cast<vtkPointHandleRepresentation2D*>(this->widget2D->GetRepresentation())->GetWorldPosition());
    }
  }
  if ( (event == vtkCommand::EnableEvent) ||
       (event == vtkCommand::DisableEvent) )
  {
    if (this->Landmark && l)
      this->Landmark->SetEnabled (l->GetEnabled());
  }
}

void vtkLandmarkWidgetCommand::SetLandmark (vtkSphereWidget* l)
{
  this->Landmark = l;
}

void vtkLandmarkWidgetCommand::SetWidget2D (vtkHandleWidget* widget)
{
    this->widget2D = widget;
}

void vtkLandmarkWidget::SetEnabled( int val)
{
  Superclass::SetEnabled( val);
  
//  vtkRenderWindowInteractor *i = this->Interactor;
//  if (!i || !i->GetRenderWindow())
//      return;
//  vtkRendererCollection* renderers = i->GetRenderWindow()->GetRenderers();
//  renderers->InitTraversal();
//  std::cout<<"starting..."<<std::endl;
//  while(vtkRenderer* r = renderers->GetNextItem())
//  {
//      SphereActorCallback* cbk = SphereActorCallback::New();
//      vtkActor* actor = vtkActor::New();
//      cbk->SetActor (actor);
//      actor->SetMapper (this->SphereActor->GetMapper());
//      actor->SetVisibility(this->SphereActor->GetVisibility());
//      actor->SetProperty(this->SphereActor->GetProperty());
//      r->AddActor(actor);
//      std::cout<<"adding actor "<<actor<<" to renderer : "<<r<<std::endl;
//      actor->Delete();
//  }
}

vtkHandleWidget * vtkLandmarkWidget::GetWidget2D()
{
    return this->widget2D;
}