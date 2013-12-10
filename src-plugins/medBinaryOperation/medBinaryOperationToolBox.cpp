/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medBinaryOperationToolBox.h"

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
#include <medViewManager.h>
#include <medDataManager.h>
#include <medAbstractDbController.h>
#include <medDbControllerFactory.h>
#include <medMetaDataKeys.h>
#include <medDropSite.h>
#include <medImageFileLoader.h>


class medBinaryOperationToolBoxPrivate
{
public:
    
    dtkSmartPointer <dtkAbstractProcess> process;
    medProgressionStack * progression_stack;
    medDropSite *siteA;
    medDropSite *siteB;

    dtkSmartPointer<dtkAbstractView> view;
    dtkSmartPointer<dtkAbstractData> inputA;
    dtkSmartPointer<dtkAbstractData> inputB;

    QRadioButton *xorButton;
    QRadioButton *andButton;
    QRadioButton *orButton;
};

medBinaryOperationToolBox::medBinaryOperationToolBox(QWidget *parent) : medFilteringAbstractToolBox(parent), d(new medBinaryOperationToolBoxPrivate)
{
    QWidget *widget = new QWidget(this);
    
    d->siteA = new medDropSite(widget);
    d->siteA->setToolTip(tr("Drop a binary mask"));
    d->siteA->setText(tr("Drop a mask"));
    d->siteA->setCanAutomaticallyChangeAppereance(false);

    d->siteB = new medDropSite(widget);
    d->siteB->setToolTip(tr("Drop a binary mask"));
    d->siteB->setText(tr("Drop a mask"));
    d->siteB->setCanAutomaticallyChangeAppereance(true);

    QPushButton *clearRoiButton = new QPushButton("Clear", widget);
    clearRoiButton->setToolTip(tr("Clear previously loaded ROI."));
    QPushButton *clearInputButton = new QPushButton("Clear", widget);
    clearInputButton->setToolTip(tr("Clear previously loaded ROI."));

    d->xorButton  = new QRadioButton(tr("XOR"), widget);
    d->xorButton->setToolTip(tr("If \"XOR\" is selected fibers will need to overlap with this ROI to be displayed."));
    d->xorButton->setChecked(true);

    d->andButton  = new QRadioButton(tr("AND"), widget);
    d->andButton->setToolTip(tr("If \"AND\" is selected fibers will need to overlap with this ROI to be displayed."));

    d->orButton  = new QRadioButton(tr("OR"), widget);
    d->orButton->setToolTip(tr("If \"OR\" is selected fibers will need to overlap with this ROI to be displayed."));

    QVBoxLayout *roiButtonLayout = new QVBoxLayout;
    roiButtonLayout->addWidget(d->siteA);
    roiButtonLayout->addWidget (clearRoiButton);
    roiButtonLayout->addWidget(d->siteB);
    roiButtonLayout->addWidget (clearInputButton);
    roiButtonLayout->addWidget(d->xorButton);
    roiButtonLayout->addWidget(d->andButton);
    roiButtonLayout->addWidget(d->orButton);
    roiButtonLayout->setAlignment(Qt::AlignCenter);

    QVBoxLayout *bundlingLayout = new QVBoxLayout(widget);
    bundlingLayout->addLayout(roiButtonLayout);

    connect (d->siteA, SIGNAL(objectDropped(const medDataIndex&)), this, SLOT(onRoiImported(const medDataIndex&)));
    connect (d->siteA, SIGNAL(clicked()),                          this, SLOT(onDropSiteClicked()));
    connect (d->siteB, SIGNAL(objectDropped(const medDataIndex&)), this, SLOT(onImageImported(const medDataIndex&)));
    connect (d->siteB, SIGNAL(clicked()),                          this, SLOT(onDropSiteClicked()));
    connect (clearRoiButton,   SIGNAL(clicked()),                          this, SLOT(onClearRoiButtonClicked()));
    connect (clearInputButton,   SIGNAL(clicked()),                          this, SLOT(onClearInputButtonClicked()));
    connect (d->xorButton,     SIGNAL(toggled(bool)),                      this, SLOT(onXorButtonToggled(bool)));
    connect (d->andButton,     SIGNAL(toggled(bool)),                      this, SLOT(onAndButtonToggled(bool)));
    connect (d->orButton,     SIGNAL(toggled(bool)),                      this, SLOT(onOrButtonToggled(bool)));

    this->onXorButtonToggled(true);
    this->setTitle("Binary Operation");
    this->addWidget(widget);

    QPushButton *runButton = new QPushButton(tr("Run"), this);
    
    // progression stack
    d->progression_stack = new medProgressionStack(widget);
    QHBoxLayout *progressStackLayout = new QHBoxLayout;
    progressStackLayout->addWidget(d->progression_stack);
    
    this->addWidget(runButton);
    this->addWidget(d->progression_stack);
    
    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
}

medBinaryOperationToolBox::~medBinaryOperationToolBox()
{
    delete d;
    
    d = NULL;
}

bool medBinaryOperationToolBox::registered()
{
    return medToolBoxFactory::instance()->
    registerToolBox<medBinaryOperationToolBox>("medBinaryOperationToolBox",
                               tr("Binary Operation"),
                               tr("Allow binary operations between masks (OR, AND, XOR, etc.)"),
                               QStringList()<< "filtering");
}

dtkPlugin* medBinaryOperationToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "medBinaryOperationPlugin" );
    return plugin;
}

dtkAbstractData* medBinaryOperationToolBox::processOutput()
{
    if(!d->process)
        return NULL;
    
    return d->process->output();
}

void medBinaryOperationToolBox::onXorButtonToggled(bool value)
{
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("itkXorOperator");
}

void medBinaryOperationToolBox::onAndButtonToggled(bool value)
{
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("itkAndOperator");
}

void medBinaryOperationToolBox::onOrButtonToggled(bool value)
{
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("itkOrOperator");
}

void medBinaryOperationToolBox::run()
{
    //if(!this->parentToolBox())
    //    return;
    
    if (!d->process)
        d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("itkXorOperator");
    
    d->process->setInput ( d->inputA, 0 );
    d->process->setInput ( d->inputB, 1 );

    //if(!this->parentToolBox()->data())
    //    return;
    
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

void medBinaryOperationToolBox::onRoiImported(const medDataIndex& index)
{
    dtkSmartPointer<dtkAbstractData> data = medDataManager::instance()->data(index);
    // we accept only ROIs (itkDataImageUChar3)
    if (!data || data->identifier() != "itkDataImageUChar3")
    {
        return;
    }
    // put the thumbnail in the medDropSite as well
    // (code copied from @medDatabasePreviewItem)
    medAbstractDbController* dbc = medDataManager::instance()->controllerForDataSource(index.dataSourceId());
    QString thumbpath = dbc->metaData(index, medMetaDataKeys::ThumbnailPath);

    bool shouldSkipLoading = false;
    if ( thumbpath.isEmpty() )
    {
        // first try to get it from controller
        QImage thumbImage = dbc->thumbnail(index);
        if (!thumbImage.isNull())
        {
            d->siteA->setPixmap(QPixmap::fromImage(thumbImage));
            shouldSkipLoading = true;
        }
    }
    if (!shouldSkipLoading) {
        medImageFileLoader *loader = new medImageFileLoader(thumbpath);

        connect(loader, SIGNAL(completed(const QImage&)), this, SLOT(setImage(const QImage&)));
        QThreadPool::globalInstance()->start(loader);
    }
    d->inputA = data;
}

void medBinaryOperationToolBox::onImageImported(const medDataIndex& index)
{
    dtkSmartPointer<dtkAbstractData> data = medDataManager::instance()->data(index);
    // we accept only ROIs (itkDataImageUChar3)
    if (!data || data->identifier() != "itkDataImageUChar3")
    {
        return;
    }

    d->inputB = data;
}

void medBinaryOperationToolBox::onClearRoiButtonClicked(void)
{
    d->siteA->clear();
    d->siteA->setText(tr("Drop a mask"));
}

void medBinaryOperationToolBox::onClearInputButtonClicked(void)
{
    d->siteB->clear();
    d->siteB->setText(tr("Drop a mask"));
}

void medBinaryOperationToolBox::clear(void)
{
    // clear ROIs and related GUI elements
    onClearRoiButtonClicked();

    d->view = 0;
    d->inputA = 0;
    d->inputB = 0;
}

void medBinaryOperationToolBox::update(dtkAbstractView *view)
{
    //if (d->view==view) {
    //    if (view)
    //        if (dtkAbstractViewInteractor *interactor = view->interactor ("v3dViewFiberInteractor"))
    //            this->setData (interactor->data()); // data may have changed
    //    return;
    //}

    //if (d->view) {
    //    if (dtkAbstractViewInteractor *interactor = d->view->interactor ("v3dViewFiberInteractor")) {
    //    }
    //}

    //if (!view) {
    //    d->view = 0;
    //    d->inputA = 0;
    //    return;
    //}

    ///*
    //if (view->property ("Orientation")!="3D") { // only interaction with 3D views is authorized
    //    d->view = 0;
    //    d->inputA = 0;
    //    return;
    //}
    //*/

    //d->view = view;

    //if (dtkAbstractViewInteractor *interactor = view->interactor ("v3dViewFiberInteractor")) {


    //    this->setData (interactor->data());
    //}
}

void medBinaryOperationToolBox::onDropSiteClicked()
{
    if (!d->view)
        return;

    QString roiFileName = QFileDialog::getOpenFileName(this, tr("Open ROI"), "", tr("Image file (*.*)"));

    if (roiFileName.isEmpty())
        return;

    medDataManager* mdm = medDataManager::instance();
    connect(mdm, SIGNAL(dataAdded(const medDataIndex &)), this, SLOT(onRoiImported(const medDataIndex &)));
    mdm->importNonPersistent(roiFileName);
}

void medBinaryOperationToolBox::setImage(const QImage& thumbnail)
{
    d->siteA->setPixmap(QPixmap::fromImage(thumbnail));
}