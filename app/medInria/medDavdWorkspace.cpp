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
#include <medToolboxFactory.h>
#include <pipelineToolBox.h>
#include <medSettingsManager.h>
#include <medToolBoxHeader.h>

class medDavdWorkspacePrivate
{
public:
    //medVisualizationLayoutToolBox *layoutToolBox;
    //medTimeLineToolBox *timeToolBox;
    medViewPropertiesToolBox *viewPropertiesToolBox;
    medSegmentationSelectorToolBox *segmentationToolbox;
    pipelineToolBox *pipelineToolbox;
    medToolBox *tb1, *tb2, *tb3, *tb4, *tb5;
    unsigned char step;
   
    QList<medToolBox*> toolboxes;
    QList<QString> stepDescriptions;
    QString currentStepDesciption;

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


    // -- View toolbox --
    d->viewPropertiesToolBox = new medViewPropertiesToolBox(parent);

    d->pipelineToolbox = new pipelineToolBox(parent);
    connect (d->pipelineToolbox, SIGNAL(previousClicked()),
             this,             SLOT(goToPreviousStep()));
    connect (d->pipelineToolbox, SIGNAL(nextClicked()),
             this,             SLOT(goToNextStep()));

    d->tb1 = new medToolBox(parent);
    d->tb1->setTitle("1. Definition of the Region of Interest");
    QString step1Description = " Trace de la droite";


    d->segmentationToolbox   = new medSegmentationSelectorToolBox(this, parent);
    d->segmentationToolbox->hide();
    
    medSegmentationAbstractToolBox *paintSegToolBox = 
        qobject_cast<medSegmentationAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("mseg::AlgorithmPaintToolbox", d->segmentationToolbox));
    paintSegToolBox->setTitle("2. Healthy inter-ventricular septum definition");
    QString step2Description = "On short axis images, draw a 3 cm2 region of interest in healthy \n inter-ventricular septum";

    d->tb2 = new medToolBox(parent);
    d->tb2->setTitle("3. RV endocardium segmentation");
    QString step3Description = " You can change the automatically calculated thresholds \n(3SD above mean healthy myocardial density) for the region growing.";

    d->tb3 = new medToolBox(parent);    
    d->tb3->setTitle("4. RV free wall layer segmentation");
    QString step4Description = " A 2-mm thick RV free wall is derived from \n the last segmentation using a dilation operator.";

    d->tb4 = new medToolBox(parent);
    d->tb4->setTitle("5. Fat density pixels segmentation");
    QString step5Description = " Myocardial fat is segmented on the histogram as \n pixels with density < 10 Hounsfield Units.";

    d->tb5 = new medToolBox(parent);
    d->tb5->setTitle("6. 3D fat distribution model");
    QString step6Description = " Segmented images are then used to compute patient-specific \n 3D models displaying fat distribution. ";


    // initialization
    //d->layoutToolBox->switchMinimize();
    //d->viewPropertiesToolBox->switchMinimize();
    //d->segmentationToolbox->switchMinimize();
    paintSegToolBox->switchMinimize();
    //d->tb1->switchMinimize();
    d->tb2->switchMinimize();
    d->tb3->switchMinimize();
    d->tb4->switchMinimize();
    d->tb5->switchMinimize();

    //this->addToolBox( d->layoutToolBox );
    this->addToolBox( d->viewPropertiesToolBox );
    this->addToolBox( d->pipelineToolbox );
    //this->addToolBox( d->segmentationToolbox );
    this->addToolBox( d->tb1 );
    this->addToolBox( paintSegToolBox );
    this->addToolBox( d->tb2);
    this->addToolBox( d->tb3);
    this->addToolBox( d->tb4);
    this->addToolBox( d->tb5);

    d->toolboxes<<d->tb1<<paintSegToolBox<<d->tb2<<d->tb3<<d->tb4<<d->tb5;
    for(int i=1;i<d->toolboxes.size();i++)d->toolboxes[i]->header()->blockSignals(true);
    d->stepDescriptions<<step1Description<<step2Description<<step3Description<<step4Description<<step5Description<<step6Description;

    d->step =0;

    d->currentStepDesciption = d->stepDescriptions[d->step];
    d->pipelineToolbox->setLabel(d->currentStepDesciption);
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
        //d->toolboxes[d->step]->setDisabled(true);
        d->toolboxes[d->step]->header()->blockSignals(true);
        d->step++;
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->header()->blockSignals(false);
        d->pipelineToolbox->setLabel(d->stepDescriptions[d->step]);
    }
    if (d->step == d->toolboxes.size()-1)
        d->pipelineToolbox->getNextButton()->setDisabled(true);
}
void medDavdWorkspace::goToPreviousStep(){
    if(d->step>0)
    {
        d->pipelineToolbox->getNextButton()->setDisabled(false);
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->header()->blockSignals(true);
        d->step--;
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->header()->blockSignals(false);
        d->pipelineToolbox->setLabel(d->stepDescriptions[d->step]);
    }
    if(d->step == 0)
        d->pipelineToolbox->getPreviousButton()->setDisabled(true);
}