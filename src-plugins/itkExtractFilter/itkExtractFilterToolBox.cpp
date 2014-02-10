/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkExtractFilter.h"
#include "itkExtractFilterToolBox.h"

#include <QtGui>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractView.h>
#include <medRunnableProcess.h>
#include <medJobManager.h>

#include <medAbstractDataImage.h>

#include <medToolBoxFactory.h>
#include <medFilteringSelectorToolBox.h>
#include <medProgressionStack.h>
#include <medPluginManager.h>

class itkExtractFilterToolBoxPrivate
{
public:
    
    QSpinBox *frameNumberValue;
    dtkSmartPointer <dtkAbstractProcess> process;
    medProgressionStack * progression_stack;
};

itkExtractFilterToolBox::itkExtractFilterToolBox(QWidget *parent) : medFilteringAbstractToolBox(parent), d(new itkExtractFilterToolBoxPrivate)
{  
    QWidget *extractFrameWidget = new QWidget(this);
    QLabel *frameNumberLabel = new QLabel (tr("Frame to extract: "));
    d->frameNumberValue = new QSpinBox;
    d->frameNumberValue->setMinimum(0);
    d->frameNumberValue->setValue(0); //TODO: Maximum is the number of frames
    QHBoxLayout *extractFrameLayout = new QHBoxLayout;
    extractFrameLayout->addWidget(frameNumberLabel);
    extractFrameLayout->addWidget(d->frameNumberValue);
    extractFrameWidget->setLayout(extractFrameLayout);

    QPushButton *runButton = new QPushButton(tr("Run"));
    
    // Principal layout:
    QWidget *widget = new QWidget(this);

    // progression stack
    d->progression_stack = new medProgressionStack(widget);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(extractFrameWidget);
    layout->addWidget(runButton);
    layout->addWidget(d->progression_stack);
    widget->setLayout(layout);

    this->setTitle("itkExtractFilter");
    this->addWidget(widget);

    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
}

itkExtractFilterToolBox::~itkExtractFilterToolBox()
{
    delete d;
    
    d = NULL;
}

bool itkExtractFilterToolBox::registered()
{
    return medToolBoxFactory::instance()->
    registerToolBox<itkExtractFilterToolBox>("itkExtractFilterToolBox",
                               tr("Extract a slice"),
                               tr("short tooltip description"),
                               QStringList()<< "filtering");
}

dtkPlugin* itkExtractFilterToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "itkExtractFilterPlugin" );
    return plugin;
}

dtkAbstractData* itkExtractFilterToolBox::processOutput()
{
    if(!d->process)
        return NULL;
    
    return d->process->output();
}

void itkExtractFilterToolBox::run()
{
    if(!this->parentToolBox())
        return;
    
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("itkExtractFilter");
    
    if(!this->parentToolBox()->data())
        return;
    
    d->process->setInput(this->parentToolBox()->data());
    d->process->setParameter(d->frameNumberValue->value(), 0);
    
    
    medRunnableProcess *runProcess = new medRunnableProcess;
    runProcess->setProcess (d->process);
    
    d->progression_stack->addJobItem(runProcess, "Progress:");
    
    d->progression_stack->disableCancel(runProcess);
    
    connect (runProcess, SIGNAL (success  (QObject*)),  this, SIGNAL (success ()));
    connect (runProcess, SIGNAL (failure  (QObject*)),  this, SIGNAL (failure ()));
    connect (runProcess, SIGNAL (cancelled (QObject*)),  this, SIGNAL (failure ()));
    
    connect (runProcess, SIGNAL(activate(QObject*,bool)),
             d->progression_stack, SLOT(setActive(QObject*,bool)));
    
    medJobManager::instance()->registerJobItem(runProcess);
    QThreadPool::globalInstance()->start(dynamic_cast<QRunnable*>(runProcess));
}
