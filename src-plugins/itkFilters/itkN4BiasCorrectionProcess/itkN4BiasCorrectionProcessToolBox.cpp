/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkN4BiasCorrectionProcess.h"
#include "itkN4BiasCorrectionProcessToolBox.h"

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

class itkN4BiasCorrectionProcessToolBoxPrivate
{
public:
    
    dtkSmartPointer <dtkAbstractProcess> process;
    medProgressionStack * progression_stack;
};

itkN4BiasCorrectionProcessToolBox::itkN4BiasCorrectionProcessToolBox(QWidget *parent) : medFilteringAbstractToolBox(parent), d(new itkN4BiasCorrectionProcessToolBoxPrivate)
{
    QWidget *widget = new QWidget(this);
    
    QPushButton *runButton = new QPushButton(tr("Run"), this);
    
    this->setTitle("N4 Bias Correction Process");
    
    // progression stack
    d->progression_stack = new medProgressionStack(widget);
    QHBoxLayout *progressStackLayout = new QHBoxLayout;
    progressStackLayout->addWidget(d->progression_stack);
    
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(runButton);
    layout->addWidget(d->progression_stack);
    widget->setLayout(layout);
    this->addWidget(widget);
    
    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
}

itkN4BiasCorrectionProcessToolBox::~itkN4BiasCorrectionProcessToolBox()
{
    delete d;
    
    d = NULL;
}

bool itkN4BiasCorrectionProcessToolBox::registered()
{
    return medToolBoxFactory::instance()->
    registerToolBox<itkN4BiasCorrectionProcessToolBox>("itkN4BiasCorrectionProcessToolBox",
                               tr("N4 Bias Correction"),
                               tr("short tooltip description"),
                               QStringList()<< "filtering");
}

dtkPlugin* itkN4BiasCorrectionProcessToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "itkN4BiasCorrectionProcessPlugin" );
    return plugin;
}

dtkAbstractData* itkN4BiasCorrectionProcessToolBox::processOutput()
{
    if(!d->process)
        return NULL;
    
    return d->process->output();
}

void itkN4BiasCorrectionProcessToolBox::run()
{
    if(!this->parentToolBox())
        return;
    
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("itkN4BiasCorrectionProcess");
    
    if(!this->parentToolBox()->data())
        return;
    
    d->process->setInput(this->parentToolBox()->data());
    
    // Set your parameters here
    
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
