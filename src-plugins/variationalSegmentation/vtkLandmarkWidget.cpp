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

void vtkLandmarkWidgetCommand::Execute(vtkObject *   caller, 
				       unsigned long event, 
				       void *        callData)
{
  vtkLandmarkWidget * l = vtkLandmarkWidget::SafeDownCast(caller);
  vtkHandleWidget * widget = vtkHandleWidget::SafeDownCast(caller);

  if (event == vtkCommand::InteractionEvent)
  {
    if (this->Landmark && l)
    {
      this->Landmark->SetCenter (l->GetCenter());
      if (this->Landmark->GetInteractor())
	this->Landmark->GetInteractor()->Render();
    }
    if (this->Widget2D == widget)
    {
        if (this->Landmark)
            this->Landmark->SetCenter(dynamic_cast<vtkPointHandleRepresentation2D*>(this->Widget2D->GetRepresentation())->GetWorldPosition());
    }
  }
  if ( (event == vtkCommand::EnableEvent) ||
       (event == vtkCommand::DisableEvent) )
  {
    if (this->Landmark && l)
      this->Landmark->SetEnabled (l->GetEnabled());
  }

  if ( event == vtkCommand::EndInteractionEvent)
  {
      if (widget)
      {
        if (widget->GetInteractor()->GetControlKey())
        {
            Landmark->InvokeEvent(vtkCommand::DeleteEvent);
            //widget->Off();
            //Landmark->Off();
            //Landmark->GetView2D()->RemoveObserver(this);
        }
        else
        {
            Landmark->updateLandmarksPosFromWidget2D();
            Landmark->InvokeEvent(vtkCommand::PlacePointEvent);
        }
      }
      else
          if (l && (l->GetInteractor()->GetControlKey()))
          {
              Landmark->InvokeEvent(vtkCommand::DeleteEvent);
              //this->Widget2D->Off();
              //Landmark->Off();
              //Landmark->GetView2D()->RemoveObserver(this);
              /*Landmark->RemoveAllObservers();
              widget->RemoveAllObservers();*/
          }
  }
  if ( event == vtkImageView2D::SliceChangedEvent )
  {
      vtkImageView2D * view2d= vtkImageView2D::SafeDownCast(caller);
      this->Landmark->showOrHide2DWidget();                                              
  }
}

void vtkLandmarkWidgetCommand::SetLandmark (vtkLandmarkWidget* l)
{
    this->Landmark = l;
}

void vtkLandmarkWidgetCommand::SetWidget2D (vtkHandleWidget* widget)
{
    this->Widget2D = widget;
}


vtkLandmarkWidget::vtkLandmarkWidget()
{
  this->Command = vtkLandmarkWidgetCommand::New();
  this->Command->SetLandmark (this);
  this->Value = 0.0;
  this->Widget2D = vtkHandleWidget::New();
  this->Command->SetWidget2D (this->Widget2D);
  this->AddObserver(vtkCommand::EndInteractionEvent,this->Command);
  this->Widget2D->AddObserver(vtkCommand::EndInteractionEvent,this->Command);
  this->View2D = 0;
  this->View3D = 0;
  this->ToDelete = false;
}

vtkLandmarkWidget::~vtkLandmarkWidget()
{
  this->Widget2D->RemoveAllObservers();
  this->RemoveAllObservers();
  if (View2D)
    this->View2D->RemoveObserver(this->Command);
  this->Command->Delete();
  this->Widget2D->Delete();
}

void vtkLandmarkWidget::SetEnabled( int val)
{
  Superclass::SetEnabled( val);
  this->Interactor->RemoveObservers(vtkCommand::MouseMoveEvent,reinterpret_cast<vtkCommand*>(this->EventCallbackCommand));// the sphere widget should not be movable. All movements should go through the 2d view -> widget2d.
}

void vtkLandmarkWidget::SetView2D(vtkImageView2D * view2d)
{
    if (this->View2D)
        this->View2D->RemoveObserver(this->Command);
    this->View2D = view2d;
    this->View2D->AddObserver(vtkImageView2D::SliceChangedEvent,this->Command);
}

void vtkLandmarkWidget::showOrHide2DWidget()
{
    if (Widget2D->GetInteractor() && Widget2D->GetRepresentation()->GetRenderer())
        if (Widget2D->GetInteractor()->GetRenderWindow())
            if (indices[View2D->GetSliceOrientation()]!=View2D->GetSlice() || ToDelete)
                Widget2D->Off();
            else
                Widget2D->On();
}        

void vtkLandmarkWidget::updateLandmarksPosFromWidget2D()
{
    vtkPointHandleRepresentation2D * pointRep = dynamic_cast<vtkPointHandleRepresentation2D*> (Widget2D->GetRepresentation());
    this->SetCenter(pointRep->GetWorldPosition());
    int new_indices[3];
    View2D->GetImageCoordinatesFromWorldCoordinates(pointRep->GetWorldPosition(),new_indices);
    int orientation = View2D->GetSliceOrientation();
    new_indices[orientation] = indices[orientation]; // the slice id of the current Orientation cannot change, it would not make sense. This line is here to prevent that.
    SetIndices(new_indices);
}