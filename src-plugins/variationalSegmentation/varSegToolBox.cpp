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
#include <medDataManager.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>


namespace mseg {

VarSegToolBox::VarSegToolBox(QWidget * parent )
    : medSegmentationAbstractToolBox(parent)
{
    QWidget *displayWidget = new QWidget(this);
    this->addWidget(displayWidget);

    this->setTitle(this->s_name());

    QVBoxLayout * layout = new QVBoxLayout(displayWidget);

    QLabel * inside = new QLabel(QString("Inside VOI : Shift + right mouse button"),displayWidget);
    QLabel * outside = new QLabel(QString("On VOI : Shift + left mouse button"),displayWidget);
    QLabel * on = new QLabel(QString("Outside VOI : Shift + Ctrl + right mouse button or Shift + middle mouse button"),displayWidget);


    segButton = new QPushButton(tr("Start Segmentation"),displayWidget);
    segButton->setCheckable(true);
    binaryImageButton = new QPushButton(tr("Generate binary image"),displayWidget);
    applyMaskButton = new QPushButton(tr("Apply segmentation"),displayWidget);
    clearChanges = new QPushButton(tr("Abandon all changes"),displayWidget);
    clearChanges->setEnabled(false);
    binaryImageButton->setEnabled(false);
    applyMaskButton->setEnabled(false);
    layout->addWidget(segButton);
    layout->addWidget(inside);
    layout->addWidget(outside);
    layout->addWidget(on);
    layout->addWidget(binaryImageButton);
    layout->addWidget(applyMaskButton);
    layout->addWidget(clearChanges);
    
    connect(segButton,SIGNAL(toggled(bool)),this,SLOT(segmentation(bool)));
    connect(binaryImageButton,SIGNAL(clicked()),this,SLOT(addBinaryImage()));
    connect(applyMaskButton,SIGNAL(clicked()),this,SLOT(applyMaskToImage()));
    connect(clearChanges,SIGNAL(clicked()),this,SLOT(bringBackOriginalImage()));
    
    controller = vtkLandmarkSegmentationController::New();
    output = output = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageUChar3");
    currentView=0;
    segOn = false;
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
    if (v!=currentView || !controller)
        return;
    
    vtkRenderWindowInteractor * interactor = static_cast<medVtkViewBackend*>(v->backend())->renWin->GetInteractor();

    vtkCollection* landmarks = this->controller->GetTotalLandmarkCollection();
    landmarks->InitTraversal();
    vtkLandmarkWidget* l = vtkLandmarkWidget::SafeDownCast(landmarks->GetNextItemAsObject());
    
    while(l)
    {
        if (!this->controller->RemoveConstraint (l))
        {
            if ( (l->GetInteractor() == interactor))
                if (value=="3D")
                    l->On();
                else if (this->controller->getMode3D()) // test if previous orientation was 3d
                    if (l->GetCurrentRenderer())
                        l->Off();
        }
        
        l = vtkLandmarkWidget::SafeDownCast(landmarks->GetNextItemAsObject());
    }
    
    if (value == "3D")
        this->controller->setMode3D(true);
    else
        this->controller->setMode3D(false);
    
    // This second loop is necessary especially in case of other orientation (not 3D). Not sure however since the slice event is called when the orientation is changed. 
    // TODO : check the orientation and slice id when the slice event is called when the orientation is changed.
    landmarks->InitTraversal(); 
    l = vtkLandmarkWidget::SafeDownCast(landmarks->GetNextItemAsObject());
    while(l)
    {
        l->showOrHide2DWidget();
        l = vtkLandmarkWidget::SafeDownCast(landmarks->GetNextItemAsObject());
    }
}

void VarSegToolBox::addBinaryImage()
{
    // Need to be tested a little more
    // TODO : Solve border problem
    typedef itk::Image<unsigned char,3> binaryType;
    binaryType::Pointer img = this->controller->GetBinaryImage();

    if (!img)
        return;

    output->setData(img);
    QString newSeriesDescription = reinterpret_cast<dtkAbstractData*>(this->currentView->data())->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += "varSeg";
    
    output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    medDataManager::instance()->importNonPersistent( output.data() );
}

void VarSegToolBox::applyMaskToImage()
{
    if (!currentView)
        return;
    
    dtkAbstractProcess * maskApplicationProcess = dtkAbstractProcessFactory::instance()->create("medMaskApplication");
    if (!maskApplicationProcess)
        return;

    typedef itk::Image<unsigned char,3> binaryType;
    binaryType::Pointer img = this->controller->GetBinaryImage();

    if (!img)
        return;

    dtkAbstractData * maskData = dtkAbstractDataFactory::instance()->createSmartPointer ( "itkDataImageUChar3" );
    maskData->setData(img);
    maskApplicationProcess->setInput(maskData,0);
    maskApplicationProcess->setInput(static_cast<dtkAbstractData*>(currentView->data()),1);
    maskApplicationProcess->update();
    currentView->removeOverlay(0);
    currentView->setData(maskApplicationProcess->output(),0);
    clearChanges->setEnabled(true);
}

void VarSegToolBox::update(dtkAbstractView * view)
{
    medAbstractView * v = qobject_cast<medAbstractView*>(view);

    if (!v)
        return;
        
    if (!segOn) // if the segmentation is activated we do not change the currentView. The currentView is supposed to be the view in which the segmentation is being done.
        currentView=v;
}

void VarSegToolBox::startSegmentation()
{
    if (!currentView)
    {
        segButton->setChecked(false);
        return;
    }

    connect(currentView, SIGNAL(propertySet(QString,QString)), this, SLOT(updateLandmarksRenderer(QString,QString)),Qt::UniqueConnection);

    if (currentView->property("Orientation")=="3D")
        this->controller->setMode3D(true);
    else
        this->controller->setMode3D(false);

    vtkCollection* interactorcollection = vtkCollection::New();
    interactorcollection->AddItem(static_cast<medVtkViewBackend*>(currentView->backend())->renWin->GetInteractor());
    this->controller->SetInteractorCollection(interactorcollection);
    interactorcollection->Delete();

    this->controller->EnabledOn();

    typedef vtkLandmarkSegmentationController::ImageType ImageType;
    ImageType::Pointer image;

    originalInput = reinterpret_cast<dtkAbstractData*>(currentView->data());
    
    if (!originalInput) return;

    if (originalInput->identifier() == "itkDataImageShort3")
    {
        typedef itk::Image<short, 3> InputImage;
        InputImage::Pointer imgView = dynamic_cast< InputImage*>((itk::Object*)(originalInput->data()));
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(imgView);
        caster->Update(); // terribly expensive in term of memory look for alternative
        image = caster->GetOutput();
    }
    else if (originalInput->identifier() == "itkDataImageUShort3")
    {
        typedef itk::Image<unsigned short, 3> InputImage;
        InputImage::Pointer imgView = dynamic_cast< InputImage*>((itk::Object*)(originalInput->data()));
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(imgView);
        caster->Update();
        image = caster->GetOutput();
    }
    else if (originalInput->identifier() == "itkDataImageDouble3")
    {
        typedef itk::Image<double, 3> InputImage;
        InputImage::Pointer imgView = dynamic_cast< InputImage*>((itk::Object*)(originalInput->data()));
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(imgView);
        caster->Update();
        image = caster->GetOutput();
    }
    else if (originalInput->identifier() == "itkDataImageFloat3")
    {
        typedef itk::Image<float, 3> InputImage;
        InputImage::Pointer imgView = dynamic_cast< InputImage*>((itk::Object*)(originalInput->data()));
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(imgView);
        caster->Update();
        image = caster->GetOutput();
    }
     else if (originalInput->identifier() == "itkDataImageInt3")
    {
        typedef itk::Image<int, 3> InputImage;
        InputImage::Pointer imgView = dynamic_cast< InputImage*>((itk::Object*)(originalInput->data()));
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(imgView);
        caster->Update();
        image = caster->GetOutput();
    }
    else
    {
        qDebug() << "Failed : type " << originalInput->identifier();
    }

    //itk::ChangeInformationImageFilter<itk::Image<float,3> > * infofilter = itk::ChangeInformationImageFilter<itk::Image<float,3> >::New();
    ImageType::Pointer smallerImage;

    // TODO : stash origin direction size and spacing in variables without having to make a freaking cast. TOO expensive in memory I hate that !@!@!!!!@!@!@!

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
    {
        mDim[i] = (int) (imageSize[i] * imageSpacing[i] / mSpacing[i]);
        inputSize[i]=imageSize[i];
    }
    this->controller->setOutputSize(inputSize[0],inputSize[1],inputSize[2]);

    ImageType::SpacingType NewSpacing;
    ImageType::SizeType NewSize;
    NewSpacing[0] = mSpacing[0];NewSpacing[1] = mSpacing[1];NewSpacing[2] = mSpacing[2];
    NewSize[0] = mDim[0];NewSize[1] = mDim[1];NewSize[2] = mDim[2];
    ImageType::RegionType region(corner,NewSize);
    smallerImage = ImageType::New();
    smallerImage->SetRegions(region);
    smallerImage->Allocate();
    smallerImage->SetDirection(image->GetDirection());
    smallerImage->SetOrigin(image->GetOrigin());
    smallerImage->SetSpacing(NewSpacing);
    
    this->controller->SetInput(smallerImage);
    vtkImageView2D * view2d = static_cast<medVtkViewBackend*>(currentView->backend())->view2D;
    vtkImageView3D * view3d = static_cast<medVtkViewBackend*>(currentView->backend())->view3D;

    this->controller->setView2D(view2d);
    this->controller->setView3D(view3d);
    
    view2d->AddDataSet (controller->GetOutput());
    view3d->AddDataSet (controller->GetOutput());
    
    binaryImageButton->setEnabled(true);
    applyMaskButton->setEnabled(true);
    currentView->widget()->setCursor(Qt::CrossCursor);
    segOn = true;
}

void VarSegToolBox::endSegmentation()
{
    segButton->setText("Start Segmentation");
    segOn = false;
    if (!controller)
        return;
    if (currentView)
    {
        currentView->widget()->unsetCursor();
        vtkImageView2D * view2d = static_cast<medVtkViewBackend*>(currentView->backend())->view2D;
        vtkImageView3D * view3d = static_cast<medVtkViewBackend*>(currentView->backend())->view3D;
        view2d->RemoveDataSet (controller->GetOutput());
        view3d->RemoveDataSet (controller->GetOutput());
    }
    qDebug() << " reference count for the controller before delete : " << this->controller->GetReferenceCount();
    this->controller->EnabledOff();
    this->controller->GetTotalLandmarkCollection()->RemoveAllItems();
    this->controller->GetLandmarkCollection()->RemoveAllItems();
}

void VarSegToolBox::segmentation(bool checked)
{
    if (checked)
    {
        segButton->setText("End Segmentation");
        startSegmentation();
    }
    else
        endSegmentation();
}

void VarSegToolBox::bringBackOriginalImage()
{
    currentView->removeOverlay(0);
    currentView->setData(originalInput,0);
}


} // namespace mseg

