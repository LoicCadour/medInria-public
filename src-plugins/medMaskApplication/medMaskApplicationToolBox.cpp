/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medMaskApplication.h"
#include "medMaskApplicationToolBox.h"

#include <QtGui>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewFactory.h>
#include <dtkCore/dtkAbstractViewInteractor.h>
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
#include <medAbstractViewFiberInteractor.h>
#include <medImageFileLoader.h>

class medMaskApplicationToolBoxPrivate
{
public:
    
    dtkSmartPointer <dtkAbstractProcess> process;
    medProgressionStack * progression_stack;
    QStandardItemModel *bundlingModel;
    medDropSite *dropOrOpenRoi;
    medDropSite *dropOrOpenRoi2;

    dtkSmartPointer<dtkAbstractView> view;
    dtkSmartPointer<dtkAbstractData> data;
    dtkSmartPointer<dtkAbstractData> mask;
};

medMaskApplicationToolBox::medMaskApplicationToolBox(QWidget *parent) : medFilteringAbstractToolBox(parent), d(new medMaskApplicationToolBoxPrivate)
{
    QWidget *widget = new QWidget(this);
    
        d->dropOrOpenRoi = new medDropSite(widget);
    d->dropOrOpenRoi->setToolTip(tr("MASK."));
    d->dropOrOpenRoi->setText(tr("MASK."));
    d->dropOrOpenRoi->setCanAutomaticallyChangeAppereance(false);

    d->dropOrOpenRoi2 = new medDropSite(widget);
    d->dropOrOpenRoi2->setToolTip(tr("INPUT 2"));
    d->dropOrOpenRoi2->setText(tr("INPUT 2"));
    d->dropOrOpenRoi2->setCanAutomaticallyChangeAppereance(true);

    QPushButton *clearRoiButton = new QPushButton("Clear ROI", widget);
    clearRoiButton->setToolTip(tr("Clear previously loaded ROIs."));
    QPushButton *clearInputButton = new QPushButton("Clear Input", widget);
    clearInputButton->setToolTip(tr("Clear previously loaded input."));

    d->bundlingModel = new QStandardItemModel(0, 1, widget);
    d->bundlingModel->setHeaderData(0, Qt::Horizontal, tr("Fiber bundles"));

    // QItemSelectionModel *selectionModel = new QItemSelectionModel(d->bundlingModel);

    QVBoxLayout *roiButtonLayout = new QVBoxLayout;
    roiButtonLayout->addWidget(d->dropOrOpenRoi);
    roiButtonLayout->addWidget (clearRoiButton);
    roiButtonLayout->addWidget(d->dropOrOpenRoi2);
    roiButtonLayout->addWidget (clearInputButton);
    roiButtonLayout->setAlignment(Qt::AlignCenter);

    QVBoxLayout *bundlingLayout = new QVBoxLayout(widget);
    bundlingLayout->addLayout(roiButtonLayout);

    connect (d->dropOrOpenRoi, SIGNAL(objectDropped(const medDataIndex&)), this, SLOT(onRoiImported(const medDataIndex&)));
    connect (d->dropOrOpenRoi, SIGNAL(clicked()),                          this, SLOT(onDropSiteClicked()));
    connect (d->dropOrOpenRoi2, SIGNAL(objectDropped(const medDataIndex&)), this, SLOT(onImageImported(const medDataIndex&)));
    connect (d->dropOrOpenRoi2, SIGNAL(clicked()),                          this, SLOT(onDropSiteClicked()));
    connect (clearRoiButton,   SIGNAL(clicked()),                          this, SLOT(onClearRoiButtonClicked()));
    connect (clearInputButton,   SIGNAL(clicked()),                          this, SLOT(onClearInputButtonClicked()));


    this->setTitle("MaskApplication");
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

medMaskApplicationToolBox::~medMaskApplicationToolBox()
{
    delete d;
    
    d = NULL;
}

bool medMaskApplicationToolBox::registered()
{
    return medToolBoxFactory::instance()->
    registerToolBox<medMaskApplicationToolBox>("medMaskApplicationToolBox",
                               tr("Friendly name"),
                               tr("short tooltip description"),
                               QStringList()<< "filtering");
}

dtkPlugin* medMaskApplicationToolBox::plugin()
{
    medPluginManager* pm = medPluginManager::instance();
    dtkPlugin* plugin = pm->plugin ( "medMaskApplicationPlugin" );
    return plugin;
}

dtkAbstractData* medMaskApplicationToolBox::processOutput()
{
    if(!d->process)
        return NULL;
    
    return d->process->output();
}

void medMaskApplicationToolBox::run()
{
    //if(!this->parentToolBox())
    //    return;
    
    if (!d->process)
        d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("medMaskApplication");
    
    //if(!this->parentToolBox()->data())
    //    return;
    //if(!d->process)
    //    return;
    //d->process->setInput(d->mask, 0);
    //d->process->setInput(d->data, 1);
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


void medMaskApplicationToolBox::setData(dtkAbstractData *data)
{
    if (!data)
        return;

    if (data->identifier()!="v3dDataFibers") {
        return;
    }

    if (d->data==data)
        return;

    d->data = data;
}

void medMaskApplicationToolBox::onRoiImported(const medDataIndex& index)
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
            d->dropOrOpenRoi->setPixmap(QPixmap::fromImage(thumbImage));
            shouldSkipLoading = true;
        }
    }
    if (!shouldSkipLoading) {
        medImageFileLoader *loader = new medImageFileLoader(thumbpath);

        connect(loader, SIGNAL(completed(const QImage&)), this, SLOT(setImage(const QImage&)));
        QThreadPool::globalInstance()->start(loader);
    }
    if(!d->process)
        d->process= dtkAbstractProcessFactory::instance()->create("medMaskApplication");

    d->mask = data;
    if (!d->process)
        return;

    d->process->setInput(data, 0);
    qDebug()<<"GET THE MASK ! "<<endl;
}

void medMaskApplicationToolBox::onImageImported(const medDataIndex& index)
{
    dtkSmartPointer<dtkAbstractData> data = medDataManager::instance()->data(index);

    if (!data)
        return;
    
    if(!d->process)
        d->process= dtkAbstractProcessFactory::instance()->create("medMaskApplication");

    d->data = data;
    
    if (!d->process)
        return;

    d->process->setInput(data, 1);qDebug()<<"GET THE INPUT ! "<<endl;

}

void medMaskApplicationToolBox::onClearRoiButtonClicked(void)
{
    d->dropOrOpenRoi->clear();
    d->dropOrOpenRoi->setText(tr("MASK."));
}

void medMaskApplicationToolBox::onClearInputButtonClicked(void)
{
    d->dropOrOpenRoi2->clear();
    d->dropOrOpenRoi2->setText(tr("INPUT."));
}

void medMaskApplicationToolBox::clear(void)
{
    // clear ROIs and related GUI elements
    onClearRoiButtonClicked();

    d->bundlingModel->removeRows(0, d->bundlingModel->rowCount(QModelIndex()), QModelIndex());

    d->view = 0;
    d->data = 0;
}

void medMaskApplicationToolBox::update(dtkAbstractView *view)
{
    //if (d->view==view) {
    //    if (view)
    //        if (dtkAbstractViewInteractor *interactor = view->interactor ("v3dViewFiberInteractor"))
    //            this->setData (interactor->data()); // data may have changed
    //    return;
    //}

    //d->bundlingModel->removeRows(0, d->bundlingModel->rowCount(QModelIndex()), QModelIndex());

    //if (d->view) {
    //    if (dtkAbstractViewInteractor *interactor = d->view->interactor ("v3dViewFiberInteractor")) {
    //    }
    //}

    //if (!view) {
    //    d->view = 0;
    //    d->data = 0;
    //    return;
    //}

    ///*
    //if (view->property ("Orientation")!="3D") { // only interaction with 3D views is authorized
    //    d->view = 0;
    //    d->data = 0;
    //    return;
    //}
    //*/

    //d->view = view;

    //if (dtkAbstractViewInteractor *interactor = view->interactor ("v3dViewFiberInteractor")) {


    //    this->setData (interactor->data());
    //}
}

void medMaskApplicationToolBox::onDropSiteClicked()
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

void medMaskApplicationToolBox::setImage(const QImage& thumbnail)
{
    d->dropOrOpenRoi->setPixmap(QPixmap::fromImage(thumbnail));
}