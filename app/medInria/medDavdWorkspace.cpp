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

class medDavdWorkspacePrivate
{
public:
    //medVisualizationLayoutToolBox *layoutToolBox;
    //medTimeLineToolBox *timeToolBox;
    medViewPropertiesToolBox *viewPropertiesToolBox;
    medSegmentationSelectorToolBox *segmentationToolbox;
    pipelineToolBox *pipelineToolbox;
    medToolBox *tb1, *tb2, *tb3;
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

    d->segmentationToolbox   = new medSegmentationSelectorToolBox(this, parent);
    d->segmentationToolbox->hide();
    //d->segmentationToolbox->setTitle("1. Healthy inter-ventricular septum definition");
    QString step1Description = "On short axis images, draw a 3 cm2 region of interest in healthy inter-ventricular septum";
    
    medSegmentationAbstractToolBox *paintSegToolbox = qobject_cast<medSegmentationAbstractToolBox*>(medToolBoxFactory::instance()->createToolBox("mseg::AlgorithmPaintToolbox", d->segmentationToolbox));
    paintSegToolbox->setTitle("1. Healthy inter-ventricular septum definition");

    d->tb1 = new medToolBox(parent);
    d->tb1->setTitle("2. Histogram threshold");
    d->tb1->setDisabled(true);
    QString step2Description = " Definition des droites + resultat region growing";

    d->tb2 = new medToolBox(parent);
    d->tb2->setTitle("3. Volum rendering");
    d->tb2->setDisabled(true);
    QString step3Description = " Resultat de la dilatation";

    d->tb3 = new medToolBox(parent);    
    d->tb3->setTitle("4. Bloup");
    d->tb3->setDisabled(true);
    QString step4Description = " Threshold sur l' histogramme";

    // initialization
    //d->layoutToolBox->switchMinimize();
    //d->viewPropertiesToolBox->switchMinimize();
    //d->segmentationToolbox->switchMinimize();
    //paintSegToolbox->switchMinimize();
    d->tb1->switchMinimize();
    d->tb2->switchMinimize();
    d->tb3->switchMinimize();


    //this->addToolBox( d->layoutToolBox );
    this->addToolBox( d->viewPropertiesToolBox );
    this->addToolBox( d->pipelineToolbox );
    //this->addToolBox( d->segmentationToolbox );
    this->addToolBox( paintSegToolbox );
    this->addToolBox( d->tb1 );
    this->addToolBox( d->tb2);
    this->addToolBox( d->tb3);

    d->toolboxes<<paintSegToolbox<<d->tb1<<d->tb2<<d->tb3;
    d->stepDescriptions<<step1Description<<step2Description<<step3Description<<step4Description;

    d->step =0;

    d->currentStepDesciption = d->stepDescriptions[d->step];
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
        d->toolboxes[d->step]->setDisabled(true);
        d->step++;
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->setDisabled(false);
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
        d->step--;
        d->toolboxes[d->step]->switchMinimize();
        d->toolboxes[d->step]->setDisabled(false);
    }
    if(d->step == 0)
        d->pipelineToolbox->getPreviousButton()->setDisabled(true);
}