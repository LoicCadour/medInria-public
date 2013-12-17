/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "davdWorkspace.h"

#include <dtkCore/dtkSmartPointer.h>
#include <dtkCore/dtkAbstractProcessFactory>
#include <dtkCore/dtkAbstractDataFactory>
#include <dtkCore/dtkAbstractProcess.h>

#include <medWorkspaceFactory.h>

#include <medViewPropertiesToolBox.h>
#include <medViewContainer.h>
#include <medTabbedViewContainers.h>
#include <medTimeLineToolBox.h>
#include <medVisualizationLayoutToolBox.h>
#include <medSegmentationSelectorToolBox.h>
#include <medSegmentationAbstractToolbox.h>
#include <medFilteringSelectorToolBox.h>
#include <medFilteringAbstractToolBox.h>
#include <medToolboxFactory.h>
#include <pipelineToolBox.h>
#include <medSettingsManager.h>
#include <medToolBoxHeader.h>
#include <medAbstractView.h>
#include <medMetaDataKeys.h>
#include <medDataManager.h>

#include "itkImage.h"
#include "itkCommand.h"
#include "itkStatisticsImageFilter.h"
#include "itkCastImageFilter.h"


static QString s_identifier()
{
   return "davdWorkspace";
}

static QString s_description()
{
   return "MyDescription";
}

// /////////////////////////////////////////////////////////////////
// davdWorkspacePrivate
// /////////////////////////////////////////////////////////////////

class davdWorkspacePrivate
{
public:
    //medVisualizationLayoutToolBox *layoutToolBox;
    medViewPropertiesToolBox *viewPropertiesToolBox;
    medSegmentationSelectorToolBox *segmentationToolbox;
    medFilteringSelectorToolBox *morphoSelectorToolBox;
    medSegmentationAbstractToolBox *bezierSegToolBox;
    pipelineToolBox *pipelineToolBox;
    medToolBox *dilateToolBox, *thresholdToolBox, *resultsToolBox;
    unsigned char step;
   
    QList<medToolBox*> toolboxes;
    QList<QString> stepDescriptions;

    dtkSmartPointer<dtkAbstractProcess> process;


    QSpinBox *thresholdFilterValue;
    QSpinBox *kernelSize;
    QRadioButton *mmButton, *pixelButton;

    unsigned int nb_voxels_freeWall, nb_voxels_fat;
    float fat, RV_volume;
    QLabel *fatValue, *RV_VolumeValue;
};

// /////////////////////////////////////////////////////////////////
// davd
// /////////////////////////////////////////////////////////////////

davdWorkspace::davdWorkspace(QWidget *parent) : medWorkspace(parent), d(new davdWorkspacePrivate)
{
    //// -- Layout toolbox --
    //d->layoutToolBox = new medVisualizationLayoutToolBox(parent);

    //connect (d->layoutToolBox, SIGNAL(modeChanged(const QString&)),
    //         this,             SIGNAL(layoutModeChanged(const QString&)));
    //connect (d->layoutToolBox, SIGNAL(presetClicked(int)),
    //         this,             SIGNAL(layoutPresetClicked(int)));
    //connect (d->layoutToolBox, SIGNAL(split(int,int)),
    //         this,             SIGNAL(layoutSplit(int,int)));

    //connect(this,SIGNAL(setLayoutTab(const QString &)), d->layoutToolBox, SLOT(setTab(const QString &)));
    connect ( stackedViewContainers(), SIGNAL(currentChanged(const QString &)),
        this, SLOT(connectToolboxesToCurrentContainer(const QString &)));

    // -- View toolbox --
    d->viewPropertiesToolBox = new medViewPropertiesToolBox(parent);

    d->pipelineToolBox = new pipelineToolBox(parent);
    connect (d->pipelineToolBox, SIGNAL(previousClicked()),
             this,             SLOT(goToPreviousStep()));
    connect (d->pipelineToolBox, SIGNAL(nextClicked()),
             this,             SLOT(goToNextStep()));
    connect (d->pipelineToolBox, SIGNAL(nextClicked()),
             this,             SLOT(launchNextAlgo()));

    d->segmentationToolbox   = new medSegmentationSelectorToolBox(this, parent);
    d->segmentationToolbox->hide();

    medSegmentationAbstractToolBox *varSegToolBox = 
        qobject_cast<medSegmentationAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("mseg::VarSegToolBox", d->segmentationToolbox));
    varSegToolBox->setTitle("1. Definition of the Volume of Interest");
    varSegToolBox->header()->blockSignals(true); //prevent the user from minimizing the toolbox
    QString step1Description = "Define the tricuspid and pulmonary planes";
    
    d->bezierSegToolBox = 
        qobject_cast<medSegmentationAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("bezierCurveToolBox", d->segmentationToolbox));
    d->bezierSegToolBox->setTitle("2. Healthy inter-ventricular septum definition");
    QString step2Description = "On short axis images, draw a 3 cm2 region of interest in healthy \n inter-ventricular septum";
    
    medSegmentationAbstractToolBox *paintSegToolBox = 
        qobject_cast<medSegmentationAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("mseg::AlgorithmPaintToolbox", d->segmentationToolbox));
    paintSegToolBox->setTitle("3. RV endocardium segmentation");
    QString step3Description = " You can change the automatically calculated thresholds \n(3SD above mean healthy myocardial density) for the region growing.";


    d->dilateToolBox = new medToolBox(parent);
    d->dilateToolBox->setTitle("4. RV free wall layer segmentation");
    QString step4Description = " A 2-mm thick RV free wall is derived from \nthe last segmentation using a dilation operator.";
    QWidget *dilateWidget = new QWidget(d->dilateToolBox);
    d->kernelSize = new QSpinBox;
    d->kernelSize->setMaximum ( 10 );
    d->kernelSize->setValue ( 2 );
    QLabel * dilateFilterLabel = new QLabel ( tr ( "Kernel radius:" ) );
    QHBoxLayout * dilateFilterLayout = new QHBoxLayout;

    d->mmButton = new QRadioButton(tr("mm"), d->dilateToolBox);
    d->mmButton->setToolTip(tr("If \"mm\" is selected, the dimensions of the structuring element will be calculated in mm."));
    d->mmButton->setChecked(true);

    d->pixelButton = new QRadioButton(tr("pixels"), d->dilateToolBox);
    d->pixelButton->setToolTip(tr("If \"pixels\" is selected, the dimensions of the structuring element will be calculated in pixels."));

    dilateFilterLayout->addWidget ( dilateFilterLabel );
    dilateFilterLayout->addWidget ( d->kernelSize );
    dilateFilterLayout->addWidget ( d->mmButton );
    dilateFilterLayout->addWidget ( d->pixelButton );
    dilateFilterLayout->addStretch ( 1 );
    dilateWidget->setLayout ( dilateFilterLayout );
    d->dilateToolBox->addWidget(dilateWidget);


    d->thresholdToolBox = new medToolBox(parent);
    QWidget *thresholdFilterWidget = new QWidget(d->thresholdToolBox);
    d->thresholdFilterValue = new QSpinBox;
    d->thresholdFilterValue->setRange ( -10000, 10000 );
    d->thresholdFilterValue->setValue ( -10 );
    QLabel * thresholdFilterLabel = new QLabel ( tr ( "We ignore pixels with values lower than: " ) );
    QHBoxLayout * thresholdFilterLayout = new QHBoxLayout;
    thresholdFilterLayout->addWidget(thresholdFilterLabel);
    thresholdFilterLayout->addWidget(d->thresholdFilterValue);
    thresholdFilterWidget->setLayout(thresholdFilterLayout);
    d->thresholdToolBox->addWidget(thresholdFilterWidget);
    //d->thresholdToolBox->setLayout(thresholdFilterLayout);
    d->thresholdToolBox->setTitle("5. Fat density pixels segmentation");
    QString step5Description = " Myocardial fat is segmented on the histogram as \npixels with density < 10 Hounsfield Units.";


    medToolBox *meshToolBox =medToolBoxFactory::instance()->createToolBox("medMeshToolsToolBox", parent);
    meshToolBox->setTitle("6. 3D fat distribution model");
    QString step6Description = " Segmented images are then used to compute patient-specific \n3D models displaying fat distribution. ";

    d->resultsToolBox = new medToolBox(parent);
    d->resultsToolBox->setTitle("Results");
    QWidget *resultsWidget = new QWidget(d->resultsToolBox);

    QLabel *RV_VolumeLabel = new QLabel(tr("RV Volume (in mL): "));
    d->RV_VolumeValue = new QLabel("");
    QLabel *fatLabel = new QLabel(tr("Fat in %: "));
    d->fatValue = new QLabel("");
    QHBoxLayout * resultsLayout = new QHBoxLayout;
    resultsLayout->addWidget(RV_VolumeLabel);
    resultsLayout->addWidget(d->RV_VolumeValue);
    resultsLayout->addWidget(fatLabel);
    resultsLayout->addWidget(d->fatValue);
    resultsWidget->setLayout(resultsLayout);
    d->resultsToolBox->addWidget(resultsWidget);
    d->resultsToolBox->hide();

    // initialization
    //d->layoutToolBox->switchMinimize();
    //d->viewPropertiesToolBox->switchMinimize();
    d->resultsToolBox->switchMinimize();
    d->bezierSegToolBox->switchMinimize();
    //d->tb1->switchMinimize();
    paintSegToolBox->switchMinimize();
    d->dilateToolBox->switchMinimize();
    d->thresholdToolBox->switchMinimize();
    meshToolBox->switchMinimize();

    //this->addToolBox( d->layoutToolBox );
    this->addToolBox( d->viewPropertiesToolBox );
    this->addToolBox( d->pipelineToolBox );
    this->addToolBox( d->resultsToolBox );
    this->addToolBox( varSegToolBox );
    this->addToolBox( d->bezierSegToolBox );
    this->addToolBox( paintSegToolBox);
    this->addToolBox( d->dilateToolBox);
    this->addToolBox( d->thresholdToolBox);
    this->addToolBox( meshToolBox);

    d->toolboxes<<varSegToolBox<<d->bezierSegToolBox<<paintSegToolBox<<d->dilateToolBox<<d->thresholdToolBox<<meshToolBox;
    for(int i=1;i<d->toolboxes.size();i++)
    {
        d->toolboxes[i]->setDisabled(true);
        d->toolboxes[i]->header()->blockSignals(true);
    }
    d->stepDescriptions<<step1Description<<step2Description<<step3Description<<step4Description<<step5Description<<step6Description;

    d->step =0;

    d->pipelineToolBox->setDescription(d->stepDescriptions[d->step]);
    d->pipelineToolBox->setLabel(d->toolboxes[d->step]->header()->title());

}

void davdWorkspace::setupViewContainerStack()
{
    if (!stackedViewContainers()->count())
    {
        const QString description = this->description();
        QString createdTab = addDefaultTypeContainer(description);
        this->connectToolboxesToCurrentContainer(createdTab);
    }
    this->stackedViewContainers()->unlockTabs();
}

void davdWorkspace::connectToolboxesToCurrentContainer(const QString &name)
{
    //connect(stackedViewContainers()->container(name),
    //        SIGNAL(viewAdded(dtkAbstractView*)),
    //        d->timeToolBox, SLOT(onViewAdded(dtkAbstractView*)));
    //connect(stackedViewContainers()->container(name),
    //        SIGNAL(viewRemoved(dtkAbstractView*)),
    //        d->timeToolBox, SLOT(onViewRemoved(dtkAbstractView*)));
    //connect(this->stackedViewContainers()->container(name),SIGNAL(droppedInput(medDataIndex)), d->morphoSelectorToolBox,SLOT(onInputSelected(medDataIndex)));
}


davdWorkspace::~davdWorkspace()
{
    delete d;
    d = NULL;
}

QString davdWorkspace::identifier() const
{
    return s_identifier();
}

QString davdWorkspace::description() const
{
    return s_description();
}

bool davdWorkspace::isUsable()
{
    // TODO: you can add some conditions here to check if your workspace is ready to be used
    // (successfully initialized, ...)
    return true;
}

bool davdWorkspace::registered()
{
    return medWorkspaceFactory::instance()->registerWorkspace <davdWorkspace>
            (s_identifier(), "davdWorkspace", s_description());
}

void davdWorkspace::displayMask()
{
    if(this->currentViewContainer()->isEmpty())
        return;
    dtkAbstractView *view =this->currentViewContainer()->childContainers()[0]->view();
    if(!view)
        return;
    dtkSmartPointer<medAbstractView> currentView = dynamic_cast<medAbstractView *> (view);

    dtkAbstractData * data = currentView->dataInList(1);
    setOutputMetadata(currentView->dataInList(0), data); //copy info
    data->addMetaData ( medMetaDataKeys::SeriesDescription.key(), "RV endocardium" );
    this->currentViewContainer()->open(data);
    this->calculateVolume(data);
}

unsigned int davdWorkspace::getNumberOfOnes(dtkAbstractData* input)
{
    typedef itk::Image< unsigned char, 3 > ImageType;
    typedef itk::StatisticsImageFilter<ImageType> StatisticsImageFilterType;
    StatisticsImageFilterType::Pointer statisticsImageFilter = StatisticsImageFilterType::New();
    ImageType *image = dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) );
    statisticsImageFilter->SetInput (image);
    statisticsImageFilter->Update();
    return statisticsImageFilter->GetSum();
}

void davdWorkspace::calculateVolume(dtkAbstractData* input)
{
    d->RV_volume = 0;
    typedef itk::Image< unsigned char, 3 > ImageType;
    ImageType *image = dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) );
    unsigned int nb_voxels = getNumberOfOnes(input);
    float volumeInMm3 = nb_voxels*image->GetSpacing()[0]*image->GetSpacing()[1]*image->GetSpacing()[2];
    d->RV_volume = volumeInMm3/1000; //volume in mL
    qDebug()<<"NOMBRE DE PIXELS DANS LE RV : "<<nb_voxels;
    qDebug()<<"Volume en mm3 : "<<volumeInMm3;
    qDebug()<<"Volume en mL : "<<d->RV_volume;
    d->resultsToolBox->switchMinimize();
    d->RV_VolumeValue->setText(QString::number(d->RV_volume));
}

void davdWorkspace::dilateTheMask()
{
    if(this->currentViewContainer()->isEmpty())
        return;
    qDebug()<<"STEP 2 Dilate the mask";
    dtkAbstractView *view =this->currentViewContainer()->childContainers()[0]->view();
    if(!view)
        return;
    dtkSmartPointer<medAbstractView> currentView = dynamic_cast<medAbstractView *> (view);

    dtkAbstractData * data = currentView->dataInList(1);

    //d->process = dtkAbstractProcessFactory::instance()->createSmartPointer ( "medMeshTools" );
    //d->process->setInput(data);
    ////Can we set mesh generation parameters ???
    //d->process->setParameter(1, 0);
    //d->process->setParameter(1, 1);
    //d->process->setParameter(0.8, 2);
    //d->process->setParameter(1, 3);
    //d->process->setParameter(30, 4);
    //d->process->setParameter(0.2, 5);
    //d->process->update();

    //dtkAbstractData * RVendoMesh = d->process->output();
    //setOutputMetadata(data, RVendoMesh); //copy info
    //RVendoMesh->addMetaData ( medMetaDataKeys::SeriesDescription.key(), "Mesh endocardium" );
    //medDataManager::instance()->importNonPersistent(RVendoMesh);
    //this->currentViewContainer()->open(RVendoMesh);

    //this->currentViewContainer()->open(data);
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer ( "itkDilateProcess" );
    d->process->setInput(data);
    d->process->setParameter(d->kernelSize->value(),d->pixelButton->isChecked());//radius = 2mm
    d->process->update();
    dtkAbstractData * output = d->process->output();
    setOutputMetadata(data, output); //copy info
    this->currentViewContainer()->open(output);
    medDataManager::instance()->importNonPersistent(output);
}

void davdWorkspace::intersectMasks()
{
    if(this->currentViewContainer()->isEmpty())
        return;
    qDebug()<<"STEP 3 Intersect masks";
    dtkAbstractData * dilatedMask = d->process->output();

    dtkAbstractView *view =this->currentViewContainer()->childContainers()[1]->view();
    if(!view)
        return;
    dtkSmartPointer<medAbstractView> currentView = dynamic_cast<medAbstractView *> (view);
    dtkAbstractData * mask = static_cast<dtkAbstractData*>(currentView->data());
    //dtkAbstractData * mask = currentView->dataInList(1);

    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer ( "itkXorOperator" );
    d->process->setInput(mask, 0);
    d->process->setInput(dilatedMask, 1);
    d->process->update();

    dtkAbstractData * output = d->process->output();
    setOutputMetadata(dilatedMask, output); //copy info
    medDataManager::instance()->importNonPersistent(output);
    d->nb_voxels_freeWall = this->getNumberOfOnes(output);
}

void davdWorkspace::setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData)
{
    Q_ASSERT(outputData && inputData);

    QStringList metaDataToCopy;
    metaDataToCopy 
        << medMetaDataKeys::PatientName.key()
        << medMetaDataKeys::StudyDescription.key();

    foreach( const QString & key, metaDataToCopy ) {
        outputData->setMetaData(key, inputData->metadatas(key));
    }
}

void davdWorkspace::applyMaskToImage()
{
    if(this->currentViewContainer()->isEmpty())
        return;
    
    qDebug()<<"STEP 4 Apply mask";
    dtkAbstractData * mask = d->process->output();

    dtkAbstractView *view =this->currentViewContainer()->childContainers()[0]->view();
    if(!view)
        return;
    dtkSmartPointer<medAbstractView> currentView = dynamic_cast<medAbstractView *> (view);
    dtkAbstractData * data = currentView->dataInList(0);

    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer ( "medMaskApplication" );
    d->process->setInput(mask, 0);
    d->process->setInput(data, 1);
    d->process->update();

    dtkAbstractData * output = d->process->output();
    setOutputMetadata(mask, output);
    medDataManager::instance()->importNonPersistent(output);
}

void davdWorkspace::applyThresholdToImage()
{
    dtkAbstractData * input = d->process->output();
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer ( "itkThresholdingProcess" );
    d->process->setInput(input);
    //Background pixels (< -1023) to 0
    d->process->setParameter(-1023, 0);
    d->process->setParameter(0, 1);
    d->process->setParameter(0, 2);
    d->process->update();


    //Pixels values > -10 ->0
    d->process->setInput(d->process->output());
    d->process->setParameter(d->thresholdFilterValue->value(), 0);
    d->process->setParameter(0, 1); //we should define a background value, use for applying masks, threshold, segmenting, etc.
    d->process->setParameter(1, 2);
    d->process->update();

    //Pixels values <= -10 (< -9) ->1
    d->process->setInput(d->process->output());
    d->process->setParameter(d->thresholdFilterValue->value()+1, 0);
    d->process->setParameter(1, 1);
    d->process->setParameter(0, 2);
    d->process->update();

    QString newSeriesDescription;
    newSeriesDescription = " thresholded";
    typedef itk::Image<int, 3> ImageType2;
    typedef itk::Image<unsigned char, 3> NewImageType;
    typedef itk::CastImageFilter <ImageType2, NewImageType> CastFilter;
    CastFilter::Pointer castFilter = CastFilter::New();

    castFilter->SetInput(dynamic_cast<ImageType2*>((itk::Object*)(d->process->output()->data())));
    dtkAbstractData* output = dtkAbstractDataFactory::instance()->createSmartPointer("itkDataImageUChar3");
    output->setData(castFilter->GetOutput());

    d->nb_voxels_fat = this->getNumberOfOnes(output);
    d->fat = (d->nb_voxels_fat*100/d->nb_voxels_freeWall);
    qDebug()<<"d->nb_voxels_fat : "<<d->nb_voxels_fat;
    qDebug()<<"d->nb_voxels_freeWall : "<<d->nb_voxels_freeWall;
    qDebug()<<"fat : "<<d->fat;
    qDebug()<<"d->nb_voxels_fat*100/d->nb_voxels_freeWall : "<<(d->nb_voxels_fat*100.0/d->nb_voxels_freeWall);

    medDataManager::instance()->importNonPersistent(d->process->output());
    this->currentViewContainer()->open(d->process->output());

    d->fatValue->setText(QString::number(d->fat));
}

void davdWorkspace::goToNextStep(){

    if(d->step < d->toolboxes.size()-1)
    {
        d->pipelineToolBox->getPreviousButton()->setDisabled(false);
        d->toolboxes[d->step]->switchMinimize(); //minimize current
        d->toolboxes[d->step]->setDisabled(true);
        d->toolboxes[d->step]->header()->blockSignals(true); //prevent the user from minimizing the toolbox
        d->step++;
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->setDisabled(false);
        d->pipelineToolBox->setDescription(d->stepDescriptions[d->step]);
        d->pipelineToolBox->setLabel(d->toolboxes[d->step]->header()->title());
    }
    if (d->step == d->toolboxes.size()-1)
        d->pipelineToolBox->getNextButton()->setDisabled(true);

    if (d->step == 3)
    {
        //double value = this->regionGrowingThreshold();
        //qDebug()<<" VALUE : "<<value;
        this->displayMask();
    }   

    if (d->step == 4)
    {
        this->dilateTheMask();
        this->intersectMasks();
        this->applyMaskToImage();
    }
    if (d->step == 5)
        this->applyThresholdToImage();
}

void davdWorkspace::goToPreviousStep(){
    if(d->step>0)
    {
        d->pipelineToolBox->getNextButton()->setDisabled(false);
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->setDisabled(true);
        d->toolboxes[d->step]->header()->blockSignals(true);
        d->step--;
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->setDisabled(false);
        d->pipelineToolBox->setDescription(d->stepDescriptions[d->step]);
        d->pipelineToolBox->setLabel(d->toolboxes[d->step]->header()->title());
    }
    if(d->step == 0)
        d->pipelineToolBox->getPreviousButton()->setDisabled(true);
}

