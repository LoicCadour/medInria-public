/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "seedPointRoi.h"
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSeedRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkHandleWidget.h>



class vtkSeedCallback : public vtkCommand
{
public:
    static vtkSeedCallback *New()
    { 
        return new vtkSeedCallback; 
    }

    vtkSeedCallback() {}

    void setView ( vtkImageView2D *view )
    {
        this->view = view;
        //view->AddObserver(vtkImageView2D::SliceChangedEvent,this);
    }

    virtual void Execute(vtkObject*, unsigned long event, void *calldata)
    {
        if(event == vtkCommand::StartInteractionEvent)
        {
            // when clicking update slice and orientation
            roi->setIdSlice(roi->getView()->GetSlice());
            roi->setOrientation(roi->getView()->GetViewOrientation());
        }

        if(event == vtkImageView2D::SliceChangedEvent)
        {
            roi->showOrHide(roi->getView()->GetViewOrientation(),roi->getView()->GetSlice());
        }
    }

    void setRoi(seedPointRoi *roi)
    {
        this->roi = roi;
    }

private:
    seedPointRoi* roi;
    vtkImageView2D *view;
};

class seedPointRoiPrivate
{
public : 
    seedPointRoiPrivate(){};
    ~seedPointRoiPrivate()
    {
        view->RemoveObserver(seedCallback);
    };
    vtkImageView2D *view;
    vtkSmartPointer<vtkSeedWidget> seedWidget;
    vtkSmartPointer<vtkSeedCallback> seedCallback;
};

seedPointRoi::seedPointRoi(vtkImageView2D * view, vtkSeedWidget * widget,medAbstractRoi *parent )
    : medAbstractRoi(parent)
    , d(new seedPointRoiPrivate)
{
    // Create the representation
    d->seedWidget = widget;
    d->seedWidget->CompleteInteraction();
    d->view = view;
    setOrientation(view->GetViewOrientation());
    setIdSlice(view->GetSlice());

    d->seedCallback = vtkSmartPointer<vtkSeedCallback>::New();
    d->seedCallback->setRoi(this);
    d->view->AddObserver(vtkImageView2D::SliceChangedEvent,d->seedCallback);
}

seedPointRoi::~seedPointRoi()
{
    delete d;
    d= NULL;
}

void seedPointRoi::Off()
{
    vtkHandleWidget * handleWidget = d->seedWidget->GetSeed(0); 
    vtkPointHandleRepresentation2D*  handleRep = dynamic_cast<vtkPointHandleRepresentation2D*>(handleWidget->GetRepresentation()); 
    handleWidget->Off();
}
void seedPointRoi::On()
{
    vtkHandleWidget * handleWidget = d->seedWidget->GetSeed(0); 
    vtkPointHandleRepresentation2D*  handleRep = dynamic_cast<vtkPointHandleRepresentation2D*>(handleWidget->GetRepresentation()); 
    handleWidget->On();
}

QString seedPointRoi::info(){return QString();}
QString seedPointRoi::type(){return "SeedPoint";}
void seedPointRoi::computeRoiStatistics(){}

vtkImageView2D * seedPointRoi::getView()
{
    return d->view;
}

void seedPointRoi::showOrHide(int orientation, int idSlice)
{
    if (!d->view->GetRenderWindow())
        return;
    if (getIdSlice()==idSlice && getOrientation()==orientation)
        On();
    else
        Off();
}

void seedPointRoi::select()
{
    vtkHandleWidget * handleWidget = d->seedWidget->GetSeed(0); 
    vtkPointHandleRepresentation2D*  handleRep = dynamic_cast<vtkPointHandleRepresentation2D*>(handleWidget->GetRepresentation()); 
    handleRep->GetProperty()->SetColor(0,1,0);
    medAbstractRoi::select();
}

void seedPointRoi::unselect()
{
    vtkHandleWidget * handleWidget = d->seedWidget->GetSeed(0);
    vtkPointHandleRepresentation2D*  handleRep = dynamic_cast<vtkPointHandleRepresentation2D*>(handleWidget->GetRepresentation()); 
    handleRep->GetProperty()->SetColor(1,0,0);
    medAbstractRoi::unselect();
}

vtkSmartPointer<vtkSeedWidget> seedPointRoi::getSeedWidget()
{
    return d->seedWidget;
}