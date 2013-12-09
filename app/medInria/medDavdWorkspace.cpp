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

class medDavdWorkspacePrivate
{
public:
    //medVisualizationLayoutToolBox *layoutToolBox;
    medViewPropertiesToolBox *viewPropertiesToolBox;
    medSegmentationSelectorToolBox *segmentationToolbox;
    medFilteringSelectorToolBox *morphoSelectorToolBox;
    pipelineToolBox *pipelineToolbox;
    medToolBox *tb1, *tb2, *tb3, *tb4, *tb5;
    unsigned char step;
   
    QList<medToolBox*> toolboxes;
    QList<QString> stepDescriptions;

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




    d->segmentationToolbox   = new medSegmentationSelectorToolBox(this, parent);
    d->segmentationToolbox->hide();

    d->tb1 = new medToolBox(parent);
    d->tb1->setTitle("1. Definition of the Volume of Interest");
    d->tb1->header()->blockSignals(true); //prevent the user from minimizing the toolbox
    QString step1Description = "Define the tricuspid and pulmonary planes";
    
    medSegmentationAbstractToolBox *bezierSegToolBox = 
        qobject_cast<medSegmentationAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("bezierCurveToolBox", d->segmentationToolbox));
    bezierSegToolBox->setTitle("2. Healthy inter-ventricular septum definition");
    QString step2Description = "On short axis images, draw a 3 cm2 region of interest in healthy \n inter-ventricular septum";
    
    medSegmentationAbstractToolBox *paintSegToolBox = 
        qobject_cast<medSegmentationAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("mseg::AlgorithmPaintToolbox", d->segmentationToolbox));
    paintSegToolBox->setTitle("3. RV endocardium segmentation");
    QString step3Description = " You can change the automatically calculated thresholds \n(3SD above mean healthy myocardial density) for the region growing.";

    d->tb3 = new medToolBox(parent);
    /*d->morphoSelectorToolBox   = new medFilteringSelectorToolBox(parent);
    d->morphoSelectorToolBox->hide();
    medFilteringAbstractToolBox *morphoToolBox = 
        qobject_cast<medFilteringAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("itkMorphologicalFilters", d->morphoSelectorToolBox));*/
    d->tb3->setTitle("4. RV free wall layer segmentation");
    QString step4Description = " A 2-mm thick RV free wall is derived from \nthe last segmentation using a dilation operator.";

    d->tb4 = new medToolBox(parent);
    d->tb4->setTitle("5. Fat density pixels segmentation");
    QString step5Description = " Myocardial fat is segmented on the histogram as \npixels with density < 10 Hounsfield Units.";

    medToolBox *meshToolBox =medToolBoxFactory::instance()->createToolBox("medMeshToolsToolBox", parent);
    meshToolBox->setTitle("6. 3D fat distribution model");
    QString step6Description = " Segmented images are then used to compute patient-specific \n3D models displaying fat distribution. ";

    // initialization
    //d->layoutToolBox->switchMinimize();
    //d->viewPropertiesToolBox->switchMinimize();
    bezierSegToolBox->switchMinimize();
    //d->tb1->switchMinimize();
    paintSegToolBox->switchMinimize();
    d->tb3->switchMinimize();
    d->tb4->switchMinimize();
    meshToolBox->switchMinimize();

    //this->addToolBox( d->layoutToolBox );
    this->addToolBox( d->viewPropertiesToolBox );
    this->addToolBox( d->pipelineToolbox );
    this->addToolBox( d->tb1 );
    this->addToolBox( bezierSegToolBox );
    this->addToolBox( paintSegToolBox);
    this->addToolBox( d->tb3);
    this->addToolBox( d->tb4);
    this->addToolBox( meshToolBox);

    d->toolboxes<<d->tb1<<bezierSegToolBox<<paintSegToolBox<<d->tb3<<d->tb4<<meshToolBox;
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