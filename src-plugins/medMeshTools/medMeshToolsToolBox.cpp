/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medMeshTools.h"
#include "medMeshToolsToolBox.h"

#include <QtGui>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractLayeredView.h>
#include <medRunnableProcess.h>
#include <medJobManager.h>
#include <medDataManager.h>
#include <medMetaDataKeys.h>
#include <medViewContainer.h>
#include <medTabbedViewContainers.h>

#include <medAbstractImageData.h>

#include <medToolBoxFactory.h>
#include <medFilteringSelectorToolBox.h>
#include <medProgressionStack.h>
#include <medPluginManager.h>

#include <medSliderSpinboxPair.h>


class medMeshToolsToolBoxPrivate
{
public:

    medAbstractWorkspace *workspace;
    medMeshTools* process;
    medProgressionStack * progression_stack;
    medAbstractLayeredView * view;
    medSliderSpinboxPair * thresholdSlider;
    QDoubleSpinBox * reductionSpinBox;
    QCheckBox * decimateCheckbox;
    QCheckBox * smoothCheckbox;
    QSpinBox * iterationsSpinBox;
    QDoubleSpinBox * relaxationSpinBox;
    QLabel * trianglesLabel;
};

medMeshToolsToolBox::medMeshToolsToolBox(QWidget *parent)
 : medToolBox(parent)
 , d(new medMeshToolsToolBoxPrivate)
{
    this->setTitle("Create Mesh from Mask");

    d->thresholdSlider = new medSliderSpinboxPair;
    d->thresholdSlider->setMinimum(0);
    d->thresholdSlider->setMaximum(2000);
    d->thresholdSlider->setValue(1);

    QLabel * thresholdLabel = new QLabel("Threshold : ");
    QHBoxLayout *thresholdLayout = new QHBoxLayout;
    thresholdLayout->addWidget(thresholdLabel);
    thresholdLayout->addWidget(d->thresholdSlider);

    QCheckBox* refineCheckBox = new QCheckBox("Refine your mesh in real time");
    connect (refineCheckBox, SIGNAL(toggled(bool)), this, SLOT(refine()));

    d->decimateCheckbox = new QCheckBox("Decimate mesh");
    d->decimateCheckbox->setChecked(true);

    d->reductionSpinBox = new QDoubleSpinBox;
    d->reductionSpinBox->setRange(0.0, 1.0);
    d->reductionSpinBox->setSingleStep(0.01);
    d->reductionSpinBox->setDecimals(2);
    d->reductionSpinBox->setValue(0.8);
    connect(d->decimateCheckbox, SIGNAL(toggled(bool)), d->reductionSpinBox, SLOT(setEnabled(bool)));

    QLabel * reductionLabel = new QLabel("Reduction target : ");
    QHBoxLayout *reductionLayout = new QHBoxLayout;
    reductionLayout->addWidget(reductionLabel);
    reductionLayout->addWidget(d->reductionSpinBox);

    d->smoothCheckbox = new QCheckBox("Smooth mesh");
    d->smoothCheckbox->setChecked(true);

    d->iterationsSpinBox = new QSpinBox;
    d->iterationsSpinBox->setRange(0, 100);
    d->iterationsSpinBox->setSingleStep(1);
    d->iterationsSpinBox->setValue(30);
    connect(d->smoothCheckbox, SIGNAL(toggled(bool)), d->iterationsSpinBox, SLOT(setEnabled(bool)));

    QLabel * iterationsLabel = new QLabel("Iterations : ");
    QHBoxLayout * iterationsLayout = new QHBoxLayout;
    iterationsLayout->addWidget(iterationsLabel);
    iterationsLayout->addWidget(d->iterationsSpinBox);

    d->relaxationSpinBox = new QDoubleSpinBox;
    d->relaxationSpinBox->setRange(0.0, 1.0);
    d->relaxationSpinBox->setSingleStep(0.01);
    d->relaxationSpinBox->setDecimals(2);
    d->relaxationSpinBox->setValue(0.2);
    connect(d->smoothCheckbox, SIGNAL(toggled(bool)), d->relaxationSpinBox, SLOT(setEnabled(bool)));

    QLabel * relaxationLabel = new QLabel("Relaxation factor : ");
    QHBoxLayout * relaxationLayout = new QHBoxLayout;
    relaxationLayout->addWidget(relaxationLabel);
    relaxationLayout->addWidget(d->relaxationSpinBox);

    QPushButton * runButton = new QPushButton(tr("Run"));

    d->trianglesLabel = new QLabel;
    d->trianglesLabel->hide();
    
    // progression stack
    QWidget *widget = new QWidget(this);
    d->progression_stack = new medProgressionStack(widget);
    QHBoxLayout *progressStackLayout = new QHBoxLayout;
    progressStackLayout->addWidget(d->progression_stack);
    
    QVBoxLayout *displayLayout = new QVBoxLayout();

    displayLayout->addLayout(thresholdLayout);
    displayLayout->addWidget(d->decimateCheckbox);
    displayLayout->addLayout(reductionLayout);
    displayLayout->addWidget(d->smoothCheckbox);
    displayLayout->addLayout(iterationsLayout);
    displayLayout->addLayout(relaxationLayout);
    displayLayout->addWidget(runButton);
    displayLayout->addWidget(d->trianglesLabel);
    displayLayout->addLayout(progressStackLayout);
    widget->setLayout(displayLayout);

    this->addWidget(widget);
    this->switchMinimize();

    d->view = 0;

    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
}

medMeshToolsToolBox::~medMeshToolsToolBox()
{
    delete d;
    d = NULL;
}

bool medMeshToolsToolBox::registered()
{
    return medToolBoxFactory::instance()->
    registerToolBox<medMeshToolsToolBox>();
}

dtkPlugin* medMeshToolsToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "medMeshToolsPlugin" );
    return plugin;
}

medAbstractData* medMeshToolsToolBox::processOutput()
{
    if(!d->process)
        return NULL;
    
    return d->process->output();
}

medAbstractWorkspace* medMeshToolsToolBox::getWorkspace()
{
	return d->workspace;
}

void medMeshToolsToolBox::updateView()
{
    medTabbedViewContainers * containers = this->getWorkspace()->stackedViewContainers();
    QList<medViewContainer*> containersInTabSelected = containers->containersInTab(containers->currentIndex());
    medAbstractView *view=NULL;
    for(int i=0;i<containersInTabSelected.length();i++)
    {
        if (containersInTabSelected[i]->isSelected())
        {
            view = containersInTabSelected[i]->view();
            break;
        }
    }
    //medToolBox::update(view);
    if(!view)
    {
        clear();
        return;
    }

    medAbstractLayeredView * medView = dynamic_cast<medAbstractLayeredView *> (view);
    if ( !medView )
        return;

    if ((d->view) && (d->view != medView) )
    {
        //d->view->disconnect(this,0);
        clear();
    }
    d->view = medView;

    QObject::connect(d->view, SIGNAL(dataAdded(medAbstractData*, int)),
                     this, SLOT(addData(medAbstractData*, int)),
                     Qt::UniqueConnection);

    QObject::connect(d->view, SIGNAL(dataRemoved(medAbstractData*,int)),
                     this, SLOT(removeData(medAbstractData*, int)),
                     Qt::UniqueConnection);
}


void medMeshToolsToolBox::setWorkspace(medAbstractWorkspace* workspace)
{
    d->workspace = workspace;
    medTabbedViewContainers * containers = workspace->stackedViewContainers();

    QObject::connect(containers,SIGNAL(containersSelectedChanged()),this,SLOT(updateView()));
    updateView();
}

void medMeshToolsToolBox::addData(medAbstractData* data, int layer)
{
    if(!data)
        return;

    if (!d->view)
        return;

    if(data->identifier().contains("vtkDataMesh")) {
    }
}


void medMeshToolsToolBox::removeData(medAbstractData* data, int layer)
{
    if(!data)
        return;

    if (!d->view)
        return;

    if(data->identifier().contains("vtkDataMesh")) {
    }
}


void medMeshToolsToolBox::addMeshToView() {

    dtkSmartPointer<medAbstractData> ptr;
    medAbstractData * data = d->process->output();
    ptr.takePointer(data);

    //ptr->setMetaData(medMetaDataKeys::PatientName.key(), "John Doe");
    ptr->setMetaData(medMetaDataKeys::StudyDescription.key(), "generated");
    ptr->setMetaData(medMetaDataKeys::SeriesDescription.key(), "generated mesh");
    d->view->addLayer(ptr);

    medDataManager::instance()->importNonPersistent( ptr.data() );
}


void medMeshToolsToolBox::clear(void) {

}



void medMeshToolsToolBox::run()
{
    d->process = new medMeshTools();
    if( ! d->view->layerData(d->view->currentLayer()))
        return;
    d->process->setInput( static_cast<medAbstractData*>(d->view->layerData(d->view->currentLayer())));
    
    d->process->setParameter((double)(d->thresholdSlider->value()), 0); // iso value
    d->process->setParameter((double)(d->decimateCheckbox->isChecked()), 1); // decimation
    d->process->setParameter(d->reductionSpinBox->value(), 2); // reduction
    d->process->setParameter((double)(d->smoothCheckbox->isChecked()), 3); // smooth
    d->process->setParameter((double)(d->iterationsSpinBox->value()), 4); // iterations
    d->process->setParameter(d->relaxationSpinBox->value(), 5); // relaxation factor
    
    medRunnableProcess *runProcess = new medRunnableProcess;
    runProcess->setProcess (d->process);
    
    d->progression_stack->addJobItem(runProcess, "Progress:");
    
    d->progression_stack->disableCancel(runProcess);
    
    connect (runProcess, SIGNAL (success  (QObject*)),  this, SIGNAL (success ()));
    connect (runProcess, SIGNAL (success  (QObject*)),  this, SLOT(addMeshToView()));
    connect (runProcess, SIGNAL (success  (QObject*)),  this, SLOT(displayNumberOfTriangles()));
    connect (runProcess, SIGNAL (failure  (QObject*)),  this, SIGNAL (failure ()));
    connect (runProcess, SIGNAL (cancelled (QObject*)),  this, SIGNAL (failure ()));
    
    connect (runProcess, SIGNAL(activate(QObject*,bool)),
             d->progression_stack, SLOT(setActive(QObject*,bool)));
    
    medJobManager::instance()->registerJobItem(runProcess);
    QThreadPool::globalInstance()->start(dynamic_cast<QRunnable*>(runProcess));
}

void medMeshToolsToolBox:: displayNumberOfTriangles()
{
    d->trianglesLabel->setText("Number of triangles: " + QString::number(d->process->getNumberOfTriangles()));
    d->trianglesLabel->show();
}

void medMeshToolsToolBox::refine()
{
    if( ! d->view->layerData(d->view->currentLayer()))
        return;
    // Create a raw mesh
    d->process = new medMeshTools();
    d->process->setInput( static_cast<medAbstractData*>(d->view->layerData(d->view->currentLayer())));
    d->process->setParameter((double)(d->thresholdSlider->value()), 0); // iso value
    d->process->setParameter(0.0, 1); // decimation
    d->process->setParameter(d->reductionSpinBox->value(), 2); // reduction
    d->process->setParameter(0.0, 3); // smooth
    d->process->setParameter((double)(d->iterationsSpinBox->value()), 4); // iterations
    d->process->setParameter(d->relaxationSpinBox->value(), 5); // relaxation factor
    d->process->update();

}
