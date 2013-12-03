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

#include "vtkImageView2D.h"
#include "vtkImageView3D.h"

#include <vtkLandmarkSegmentationController.h>
#include <vtkLandmarkWidget.h>
#include <vtkHandleWidget.h>
#include <vtkWidgetRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>

#include <vtkCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <itkCastImageFilter.h>
#include <itkResampleImagefilter.h>
#include <medVtkViewBackend.h>
#include <algorithm> 

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

    QLabel * inside = new QLabel(QString("Inside VOI : Shift + right mouse button"),displayWidget);
    QLabel * outside = new QLabel(QString("On VOI : Shift + left mouse button"),displayWidget);
    QLabel * on = new QLabel(QString("Outside VOI : Shift + middle mouse button"),displayWidget);

    layout->addWidget(inside);
    layout->addWidget(outside);
    layout->addWidget(on);
    
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
    
    medAbstractView * v = qobject_cast<medAbstractView*>(this->sender());
    
    vtkRenderWindowInteractor * interactor = static_cast<medVtkViewBackend*>(v->backend())->renWin->GetInteractor();

    vtkCollection* landmarks = this->controller->GetTotalLandmarkCollection();
    landmarks->InitTraversal();
    vtkLandmarkWidget* l = vtkLandmarkWidget::SafeDownCast(landmarks->GetNextItemAsObject());
    
    while(l)
    {
        if ( (l->GetInteractor() == interactor))
            if (value=="3D")
                l->On();
            else if (this->controller->getMode3D()) // test if previous orientation was 3d
                if (l->GetCurrentRenderer())
                    l->Off();
        l = vtkLandmarkWidget::SafeDownCast(landmarks->GetNextItemAsObject());
    }
    
    if (value == "3D")
        this->controller->setMode3D(true);
    else
        this->controller->setMode3D(false);
    
    this->controller->showOrHide2DWidget();
}


void VarSegToolBox::update(dtkAbstractView * view)
{
    medAbstractView * v = qobject_cast<medAbstractView*>(view);

    if (this->controller->GetInteractorCollection())
        return;

    connect(view, SIGNAL(propertySet(QString,QString)), this, SLOT(updateLandmarksRenderer(QString,QString)));

    if (view->property("Orientation")=="3D")
        this->controller->setMode3D(true);
    else
        this->controller->setMode3D(false);

    vtkCollection* interactorcollection = vtkCollection::New();
    interactorcollection->AddItem(static_cast<medVtkViewBackend*>(v->backend())->renWin->GetInteractor());
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
        caster->Update(); // terribly expensive in term of memory look for alternative
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
    else if (data->identifier() == "itkDataImageDouble3")
    {
        typedef itk::Image<double, 3> InputImage;
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(dynamic_cast< InputImage*>((itk::Object*)(data->data())));
        caster->Update();
        image = caster->GetOutput();
    }
    else if (data->identifier() == "itkDataImageFloat3")
    {
        image = dynamic_cast< ImageType*>((itk::Object*)(data->data()));
    }
    else
    {
        qDebug() << "Failed : type " << data->identifier();
    }

    //itk::ChangeInformationImageFilter<itk::Image<float,3> > * infofilter = itk::ChangeInformationImageFilter<itk::Image<float,3> >::New();
    ImageType::Pointer imagetest;
    ImageType::SizeType imageSize = image->GetLargestPossibleRegion().GetSize(); ;
    ImageType::SpacingType imageSpacing  = image->GetSpacing();

    qDebug() << imageSize[0] << " " << imageSize[1] << " " << imageSize[2] ;
    double res = 25;
    ImageType::IndexType corner= {{0,0,0}};;
    double smallestSpacing = std::min(imageSpacing[0], std::min(imageSpacing[1], imageSpacing[2]));
    double mSpacing[3];
    for (unsigned int i = 0; i < 3; i++)
        mSpacing[i] = 100 * smallestSpacing / res;
        //mSpacing[i] = 100 * imageSpacing[i] / res;

    int mDim[3];
    for (unsigned int i = 0; i < 3; i++)
        mDim[i] = (int) (imageSize[i] * imageSpacing[i] / mSpacing[i]);


    ImageType::SpacingType NewSpacing;
    ImageType::SizeType NewSize;
    NewSpacing[0] = mSpacing[0];NewSpacing[1] = mSpacing[1];NewSpacing[2] = mSpacing[2];
    NewSize[0] = mDim[0];NewSize[1] = mDim[1];NewSize[2] = mDim[2];
    ImageType::RegionType region(corner,NewSize);
    imagetest = ImageType::New();
    imagetest->SetRegions(region);
    imagetest->Allocate();
    imagetest->SetDirection(image->GetDirection());
    imagetest->SetOrigin(image->GetOrigin());
    imagetest->SetSpacing(NewSpacing);
    
    this->controller->SetInput(imagetest);
    vtkImageView2D * view2d = static_cast<medVtkViewBackend*>(v->backend())->view2D;
    vtkImageView3D * view3d = static_cast<medVtkViewBackend*>(v->backend())->view3D;


    this->controller->setView2D(view2d);
    this->controller->setView3D(view3d);
    view2d->AddDataSet (controller->GetOutput());
    view3d->AddDataSet (controller->GetOutput());

  //   int returnValue = 0;
  //int res = this->GetResolution();

  //// Get image information
  //int    dim[3];
  //double spacing[3];
  //double origin[3];
  //this->Image->GetImageData()->GetDimensions(dim);
  //this->Image->GetImageData()->GetSpacing(spacing);
  //this->Image->GetImageData()->GetOrigin(origin);

  //double smallestSpacing = min(spacing[0], min(spacing[1], spacing[2]));
  //double mSpacing[3];
  //for (unsigned int i = 0; i < 3; i++)
  //  mSpacing[i] = 100 * smallestSpacing / res;

  //int mDim[3];
  //for (unsigned int i = 0; i < 3; i++)
  //  mDim[i] = (int) (dim[i] * spacing[i] / mSpacing[i]);

  //yav::VariationalFunction* vFun = new yav::VariationalFunction;
  //if ( !vFun->SetSampleDimensions(mDim[0], mDim[1], mDim[2]) )
  //{
  //  vtkKWPopupErrorMessage(this,"Invalid model dimensions. Please change the resolution\n");
  //  delete vFun;
  //  return -1;
  //}

  //if ( !vFun->SetModelBounds(0, dim[0] * spacing[0], 
		//	     0, dim[1] * spacing[1], 
		//	     0, dim[2] * spacing[2]) )
  //{
  //  vtkKWPopupErrorMessage(this,"Invalid model bounds. Please change the resolution\n");
  //  delete vFun;
  //  return -1;
  //}

  //// Create the implicit image
  //yav::Inrimage* iFun = 
  //  new yav::Inrimage(mDim[0], mDim[1], mDim[2], yav::Inrimage::WT_FLOAT,
		//      1, VM_INTERLACED,
		//      fabs(mSpacing[0]), fabs(mSpacing[1]), fabs(mSpacing[2]));

}




} // namespace mseg

