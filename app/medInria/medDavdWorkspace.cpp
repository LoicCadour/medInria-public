/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medDavdWorkspace.h"

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
#include <dtkCore/dtkSmartPointer.h>
//#include <src-plugins/vtkImageView2D.h>
#include <dtkCore/dtkAbstractProcessFactory>
#include <dtkCore/dtkAbstractProcess.h>
#include <medMetaDataKeys.h>
#include <medDataManager.h>

class medDavdWorkspacePrivate
{
public:
    //medVisualizationLayoutToolBox *layoutToolBox;
    medViewPropertiesToolBox *viewPropertiesToolBox;
    medSegmentationSelectorToolBox *segmentationToolbox;
    medFilteringSelectorToolBox *morphoSelectorToolBox;
    medSegmentationAbstractToolBox *bezierSegToolBox;
    pipelineToolBox *pipelineToolbox;
    medToolBox *dilateToolBox, *thresholdToolBox;
    unsigned char step;
   
    QList<medToolBox*> toolboxes;
    QList<QString> stepDescriptions;

    dtkSmartPointer<dtkAbstractProcess> process;


    QSpinBox *thresholdFilterValue;
    QSpinBox *kernelSize;
    QRadioButton *mmButton, *pixelButton;

};

medDavdWorkspace::medDavdWorkspace(QWidget *parent) : medWorkspace(parent), d(new medDavdWorkspacePrivate)
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

    d->pipelineToolbox = new pipelineToolBox(parent);
    connect (d->pipelineToolbox, SIGNAL(previousClicked()),
             this,             SLOT(goToPreviousStep()));
    connect (d->pipelineToolbox, SIGNAL(nextClicked()),
             this,             SLOT(goToNextStep()));
    connect (d->pipelineToolbox, SIGNAL(nextClicked()),
             this,             SLOT(launchNextAlgo()));

    this->setCurrentViewContainer("Single");
    
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

    // initialization
    //d->layoutToolBox->switchMinimize();
    //d->viewPropertiesToolBox->switchMinimize();
    d->bezierSegToolBox->switchMinimize();
    //d->tb1->switchMinimize();
    paintSegToolBox->switchMinimize();
    d->dilateToolBox->switchMinimize();
    d->thresholdToolBox->switchMinimize();
    meshToolBox->switchMinimize();

    //this->addToolBox( d->layoutToolBox );
    this->addToolBox( d->viewPropertiesToolBox );
    this->addToolBox( d->pipelineToolbox );
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

    d->pipelineToolbox->setDescription(d->stepDescriptions[d->step]);
    d->pipelineToolbox->setLabel(d->toolboxes[d->step]->header()->title());
}

void medDavdWorkspace::setupViewContainerStack()
{
    if (!stackedViewContainers()->count())
    {
        const QString description = this->description();
        QString createdTab = addDefaultTypeContainer(description);
        this->connectToolboxesToCurrentContainer(createdTab);
    }
    this->stackedViewContainers()->unlockTabs();
}

void medDavdWorkspace::connectToolboxesToCurrentContainer(const QString &name)
{
    //connect(stackedViewContainers()->container(name),
    //        SIGNAL(viewAdded(dtkAbstractView*)),
    //        d->timeToolBox, SLOT(onViewAdded(dtkAbstractView*)));
    //connect(stackedViewContainers()->container(name),
    //        SIGNAL(viewRemoved(dtkAbstractView*)),
    //        d->timeToolBox, SLOT(onViewRemoved(dtkAbstractView*)));
    //connect(this->stackedViewContainers()->container(name),SIGNAL(droppedInput(medDataIndex)), d->morphoSelectorToolBox,SLOT(onInputSelected(medDataIndex)));
}

medDavdWorkspace::~medDavdWorkspace(void)
{
    delete d;
    d = NULL;
}

QString medDavdWorkspace::identifier() const {
    return "medDavd";
}

QString medDavdWorkspace::description() const {
    return tr("medDavd");
}

bool medDavdWorkspace::isUsable(){
    return true; // for the time being, no test is defined.
}

double medDavdWorkspace::regionGrowingThreshold()
{
    return 0;
}

void medDavdWorkspace::displayMask()
{
    if(this->currentViewContainer()->isEmpty())
        return;
    dtkAbstractView *view =this->currentViewContainer()->childContainers()[0]->view();
    if(!view)
        return;
    dtkSmartPointer<medAbstractView> currentView = dynamic_cast<medAbstractView *> (view);

    dtkAbstractData * data = currentView->dataInList(1);
    this->currentViewContainer()->open(data);
}

void medDavdWorkspace::dilateTheMask()
{
    if(this->currentViewContainer()->isEmpty())
        return;
    qDebug()<<"STEP 2 Dilate the mask";
    dtkAbstractView *view =this->currentViewContainer()->childContainers()[0]->view();
    if(!view)
        return;
    dtkSmartPointer<medAbstractView> currentView = dynamic_cast<medAbstractView *> (view);

    dtkAbstractData * data = currentView->dataInList(1);
    //this->currentViewContainer()->open(data);
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer ( "itkDilateProcess" );
    d->process->setInput(data);
    d->process->setParameter(d->kernelSize->value(),d->pixelButton->isChecked());//radius = 2mm
    d->process->update();
    QString newSeriesDescription;
    newSeriesDescription = " Dilate filter\n(2 mm)";
    dtkAbstractData * output = d->process->output();
    output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    this->currentViewContainer()->open(output);
    medDataManager::instance()->importNonPersistent(output);
}

void medDavdWorkspace::intersectMasks()
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

    QString newSeriesDescription;
    newSeriesDescription = " Intersection";
    dtkAbstractData * output = d->process->output();
    output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    setOutputMetadata(dilatedMask, output);
    medDataManager::instance()->importNonPersistent(output);
}

void medDavdWorkspace::setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData)
{
    Q_ASSERT(outputData && inputData);

    QStringList metaDataToCopy;
    metaDataToCopy 
        << medMetaDataKeys::PatientName.key()
        << medMetaDataKeys::StudyDescription.key();

    foreach( const QString & key, metaDataToCopy ) {
        outputData->setMetaData(key, inputData->metadatas(key));
    }

    //QString seriesDesc;
    //seriesDesc = tr("Segmented from ") + medMetaDataKeys::SeriesDescription.getFirstValue( inputData );

    //medMetaDataKeys::SeriesDescription.set(outputData,seriesDesc);
}

void medDavdWorkspace::applyMaskToImage()
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

    QString newSeriesDescription;
    newSeriesDescription = " mask applied";
    dtkAbstractData * output = d->process->output();
    output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    medDataManager::instance()->importNonPersistent(output);
}

void medDavdWorkspace::applyThresholdToImage()
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
    dtkAbstractData * output = d->process->output();
    output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    medDataManager::instance()->importNonPersistent(output);
    this->currentViewContainer()->open(output);
}

void medDavdWorkspace::goToNextStep(){

    if(d->step < d->toolboxes.size()-1)
    {
        d->pipelineToolbox->getPreviousButton()->setDisabled(false);
        d->toolboxes[d->step]->switchMinimize(); //minimize current
        d->toolboxes[d->step]->setDisabled(true);
        d->toolboxes[d->step]->header()->blockSignals(true); //prevent the user from minimizing the toolbox
        d->step++;
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->setDisabled(false);
        d->pipelineToolbox->setDescription(d->stepDescriptions[d->step]);
        d->pipelineToolbox->setLabel(d->toolboxes[d->step]->header()->title());
    }
    if (d->step == d->toolboxes.size()-1)
        d->pipelineToolbox->getNextButton()->setDisabled(true);

    if (d->step == 3)
    {
        double value = this->regionGrowingThreshold();
        qDebug()<<" VALUE : "<<value;
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

void medDavdWorkspace::goToPreviousStep(){
    if(d->step>0)
    {
        d->pipelineToolbox->getNextButton()->setDisabled(false);
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->setDisabled(true);
        d->toolboxes[d->step]->header()->blockSignals(true);
        d->step--;
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->setDisabled(false);
        d->pipelineToolbox->setDescription(d->stepDescriptions[d->step]);
        d->pipelineToolbox->setLabel(d->toolboxes[d->step]->header()->title());
    }
    if(d->step == 0)
        d->pipelineToolbox->getPreviousButton()->setDisabled(true);
}