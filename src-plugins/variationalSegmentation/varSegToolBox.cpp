#include "varSegToolBox.h"

#include <medAbstractData.h>
#include <medAbstractDataImage.h>
#include <medAbstractView.h>
#include <medAbstractViewCoordinates.h>
#include <medDataIndex.h>
#include <medImageMaskAnnotationData.h>
#include <medMetaDataKeys.h>
#include <medMessageController.h>
#include <medSegmentationSelectorToolBox.h>
#include <medMessageController.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkLog/dtkLog.h>
#include <dtkCore/dtkSmartPointer.h>
#include <dtkCore/dtkGlobal.h>

#include "../../v3dView/v3dView.h"
#include "vtkImageView2D.h"
#include "vtkImageView3D.h"

#include <vtkLandmarkSegmentationController.h>
#include <vtkLandmarkWidget.h>

#include <vtkCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <itkCastImageFilter.h>

namespace mseg {

//class SelectDataEventFilter : public medViewEventFilter
//{
//public:
//    SelectDataEventFilter(medToolBoxSegmentation * controller, VarSegToolBox *cb ) :
//        medViewEventFilter(),
//            m_cb(cb)
//        {}
//        virtual bool mousePressEvent( medAbstractView *view, QMouseEvent *mouseEvent )
//        {
//            mouseEvent->accept();

//            dtkAbstractData * viewData = medToolBoxSegmentation::viewData( view );
//            if (viewData) {
//                m_cb->setData( viewData );
//                this->removeFromAllViews();
//            }
//            return mouseEvent->isAccepted();
//        }

//private :
//    VarSegToolBox *m_cb;
//};

//    class ClickEventFilter : public medViewEventFilter
//    {
//    public:
//        ClickEventFilter(medToolBoxSegmentation * controller, VarSegToolBox *cb ) :
//        medViewEventFilter(),
//        m_cb(cb)
//        {}

//        virtual bool mousePressEvent( medAbstractView *view, QMouseEvent *mouseEvent )
//        {
//            medAbstractViewCoordinates * coords = view->coordinates();

//            mouseEvent->accept();

//            if (coords->is2D()) {
//                // Convert mouse click to a 3D point in the image.

//                QVector3D posImage = coords->displayToWorld( mouseEvent->posF() );

//                // handled after release
//                m_cb->updateWandRegion(view, posImage);
//            }
//            return mouseEvent->isAccepted();
//        }

//        private :
//        VarSegToolBox *m_cb;
//    };

//class ClickAndMoveEventFilter : public medViewEventFilter
//{
//public:
//    ClickAndMoveEventFilter(medToolBoxSegmentation * controller, VarSegToolBox *cb ) :
//        medViewEventFilter(),
//        m_cb(cb)
//        {}

//    virtual bool mousePressEvent( medAbstractView *view, QMouseEvent *mouseEvent )
//    {
//        medAbstractViewCoordinates * coords = view->coordinates();

//        mouseEvent->accept();

//        if (coords->is2D()) {
//            // Convert mouse click to a 3D point in the image.

//            QVector3D posImage = coords->displayToWorld( mouseEvent->posF() );
//            this->m_state = State::Painting;

//            //Project vector onto plane
////            dtkAbstractData * viewData = medToolBoxSegmentation::viewData( view );
//            this->m_points.push_back(posImage);

//            m_cb->updateStroke( this,view );
//        }
//        return mouseEvent->isAccepted();
//    }

//    virtual bool mouseMoveEvent( medAbstractView *view, QMouseEvent *mouseEvent )
//    {
//        if ( this->m_state != State::Painting )
//            return false;

//        medAbstractViewCoordinates * coords = view->coordinates();
//        mouseEvent->accept();

//        if (coords->is2D()) {
//            // Convert mouse click to a 3D point in the image.

//            QVector3D posImage = coords->displayToWorld( mouseEvent->posF() );
//            //Project vector onto plane
//            this->m_points.push_back(posImage);
//            m_cb->updateStroke( this,view );
//        }
//        return mouseEvent->isAccepted();
//    }

//    virtual bool mouseReleaseEvent( medAbstractView *view, QMouseEvent *mouseEvent )
//    {
//        if ( this->m_state == State::Painting )
//        {
//            this->m_state = State::Done;
//            m_cb->updateStroke(this,view);
//            this->m_points.clear();
//            return true;
//        }
//        return false;
//    }
//    struct State {
//        enum E { Start, Painting, Done };
//    };

//    State::E state() const { return m_state; }

//    const std::vector<QVector3D> & points() const { return m_points; }

//private :
//    VarSegToolBox *m_cb;
//    std::vector<QVector3D> m_points;
//    State::E m_state;
//};

VarSegToolBox::VarSegToolBox(QWidget * parent )
    : medSegmentationAbstractToolBox(parent)
{
    QWidget *displayWidget = new QWidget(this);
    this->addWidget(displayWidget);

    this->setTitle(this->s_name());

    QVBoxLayout * layout = new QVBoxLayout(displayWidget);

    controller = vtkLandmarkSegmentationController::New();
}

VarSegToolBox::~VarSegToolBox()
{
    controller->Delete();
}


//static
medSegmentationAbstractToolBox * VarSegToolBox::createInstance(QWidget *parent )
{
    return new VarSegToolBox( parent );
}

QString VarSegToolBox::s_description()
{
    static const QString desc = "Variational Segmentation Tool";
    return desc;
}

QString VarSegToolBox::s_identifier()
{
    static const QString id = "mseg::VarSegToolBox";
    return id;
}

QString VarSegToolBox::s_name()
{
    return "Variational Segmentation";
}


void VarSegToolBox::updateLandmarksRenderer(QString key, QString value)
{
    if (key != "Orientation")
        return;
    v3dView * v = qobject_cast<v3dView*>(this->sender());
    vtkRenderWindowInteractor * interactor = v->interactor();

    vtkCollection* landmarks = this->controller->GetTotalLandmarkCollection();
    landmarks->InitTraversal();
    vtkLandmarkWidget* l = vtkLandmarkWidget::SafeDownCast(landmarks->GetNextItemAsObject());
    vtkRenderer* newrenderer = interactor->FindPokedRenderer(interactor->GetLastEventPosition()[0],interactor->GetLastEventPosition()[1]);
    newrenderer->DrawOff();
    while(l)
    {
        if ( (l->GetInteractor() == interactor) && l->GetEnabled() )
        {
            if (l->GetCurrentRenderer())
            {
                l->GetCurrentRenderer()->RemoveActor(l->GetSphereActor());
                l->GetCurrentRenderer()->RemoveActor(l->GetHandleActor());
            }
            l->SetCurrentRenderer(newrenderer);
            newrenderer->AddActor(l->GetSphereActor());
            newrenderer->AddActor(l->GetHandleActor());
        }
        l = vtkLandmarkWidget::SafeDownCast(landmarks->GetNextItemAsObject());
    }
    newrenderer->DrawOn();
    newrenderer->Render();
}


void VarSegToolBox::update(dtkAbstractView * view)
{
    v3dView * v = qobject_cast<v3dView*>(view);

    if (this->controller->GetInteractorCollection())
        return;

    connect(view, SIGNAL(propertySet(QString,QString)), this, SLOT(updateLandmarksRenderer(QString,QString)));

    vtkCollection* interactorcollection = vtkCollection::New();
    interactorcollection->AddItem(v->interactor());
    this->controller->SetInteractorCollection(interactorcollection);
    interactorcollection->Delete();

    this->controller->EnabledOn();

    typedef vtkLandmarkSegmentationController::ImageType ImageType;
    ImageType::Pointer image;

    dtkAbstractData * data = reinterpret_cast<dtkAbstractData*>(v->data());
    if (!data) return;

    if (data->identifier() == "itkDataImageShort3")
    {
        typedef itk::Image<short, 3> InputImage;
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(dynamic_cast< InputImage*>((itk::Object*)(data->data())));
        caster->Update();
        image = caster->GetOutput();
    }
    else if (data->identifier() == "itkDataImageUShort3")
    {
        typedef itk::Image<unsigned short, 3> InputImage;
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(dynamic_cast< InputImage*>((itk::Object*)(data->data())));
        caster->Update();
        image = caster->GetOutput();
    }
    else if (data->identifier() == "itkDataImageFloat3")
    {
        typedef itk::Image<float, 3> InputImage;
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(dynamic_cast< InputImage*>((itk::Object*)(data->data())));
        caster->Update();
        image = caster->GetOutput();
    }
    else if (data->identifier() == "itkDataImageDouble3")
    {
        image = dynamic_cast< ImageType*>((itk::Object*)(data->data()));
    }
    else
    {
        qDebug() << "Failed : type " << data->identifier();
    }

    this->controller->SetInput(image);
    v->view2d()->AddDataSet (controller->GetOutput());
    v->view3d()->AddDataSet (controller->GetOutput());
}




} // namespace mseg

