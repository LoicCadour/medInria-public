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
    QPushButton  *bundlingButtonVdt;
    QPushButton  *bundlingButtonRst;
    QCheckBox    *bundlingShowCheckBox;
    QCheckBox    *bundleBoxCheckBox;
    QPushButton  *bundlingButtonAdd;
    QComboBox    *roiComboBox;
    medDropSite *dropOrOpenRoi;
    medDropSite *dropOrOpenRoi2;

    dtkSmartPointer<dtkAbstractView> view;
    dtkSmartPointer<dtkAbstractData> data;
};

medMaskApplicationToolBox::medMaskApplicationToolBox(QWidget *parent) : medFilteringAbstractToolBox(parent), d(new medMaskApplicationToolBoxPrivate)
{
    QWidget *widget = new QWidget(this);
    
        d->dropOrOpenRoi = new medDropSite(widget);
    d->dropOrOpenRoi->setToolTip(tr("Once you are viewing fibers, click here to open a ROI or drag-and-drop one from the database."));
    d->dropOrOpenRoi->setText(tr("Once you are viewing fibers\nclick here to open a ROI\nor drag-and-drop one\nfrom the database."));
    d->dropOrOpenRoi->setCanAutomaticallyChangeAppereance(false);

    d->dropOrOpenRoi2 = new medDropSite(widget);
    d->dropOrOpenRoi2->setToolTip(tr("INPUT 2"));
    d->dropOrOpenRoi2->setText(tr("INPUT 2"));
    d->dropOrOpenRoi2->setCanAutomaticallyChangeAppereance(true);

    QPushButton *clearRoiButton = new QPushButton("Clear ROI", widget);
    clearRoiButton->setToolTip(tr("Clear previously loaded ROIs."));
    d->roiComboBox = new QComboBox(widget);
    for (int i=0; i<255; i++)
        d->roiComboBox->addItem(tr("ROI ")+QString::number(i+1));
    d->roiComboBox->setCurrentIndex(0);
    d->roiComboBox->setToolTip(tr("Select a ROI to modify how its interaction with the fibers affects whether they are displayed."));


    QHBoxLayout *roiLayout = new QHBoxLayout;
    roiLayout->addWidget(d->roiComboBox);

    d->bundlingButtonAdd = new QPushButton("Add", widget);
    d->bundlingButtonAdd->setToolTip(tr("Select to either include the fibers that overlap with the bundling box, or to include the ones that do not overlap."));
    d->bundlingButtonRst = new QPushButton("Reset", widget);
    d->bundlingButtonRst->setToolTip(tr("Reset all previously tagged bundling boxes."));
    d->bundlingButtonVdt = new QPushButton("Validate", widget);
    d->bundlingButtonVdt->setToolTip(tr("Save the current shown bundle and show useful information about it."));
    d->bundlingButtonAdd->setCheckable(true);

    QHBoxLayout *bundlingButtonsLayout = new QHBoxLayout;
    bundlingButtonsLayout->addWidget(d->bundlingButtonAdd);
    bundlingButtonsLayout->addWidget(d->bundlingButtonRst);
    bundlingButtonsLayout->addWidget(d->bundlingButtonVdt);


    d->bundlingModel = new QStandardItemModel(0, 1, widget);
    d->bundlingModel->setHeaderData(0, Qt::Horizontal, tr("Fiber bundles"));

    // QItemSelectionModel *selectionModel = new QItemSelectionModel(d->bundlingModel);

    d->bundlingShowCheckBox = new QCheckBox("Show all bundles", widget);
    d->bundlingShowCheckBox->setChecked(true);
    d->bundlingShowCheckBox->setToolTip(tr("Uncheck if you do not want the previously validated bundles to be displayed."));
    d->bundleBoxCheckBox = new QCheckBox("Activate bundling box", widget);
    d->bundleBoxCheckBox->setToolTip(tr("Select to activate and show the fiber bundling box on the screen."));

    QVBoxLayout *roiButtonLayout = new QVBoxLayout;
    roiButtonLayout->addWidget(d->dropOrOpenRoi);
    roiButtonLayout->addWidget(d->dropOrOpenRoi2);
    roiButtonLayout->addWidget (clearRoiButton);
    roiButtonLayout->setAlignment(Qt::AlignCenter);

    QVBoxLayout *bundlingLayout = new QVBoxLayout(widget);
    bundlingLayout->addLayout(roiButtonLayout);
    bundlingLayout->addLayout(roiLayout);
    bundlingLayout->addWidget(d->bundleBoxCheckBox);
    bundlingLayout->addLayout(bundlingButtonsLayout);
    bundlingLayout->addWidget(d->bundlingShowCheckBox);

    connect (d->bundlingButtonVdt,     SIGNAL(clicked(void)),            this, SLOT (onBundlingButtonVdtClicked()));
    connect (d->bundleBoxCheckBox,     SIGNAL(toggled(bool)),            this, SLOT (onBundleBoxCheckBoxToggled (bool)));
    connect (d->bundlingButtonAdd,     SIGNAL(toggled(bool)),            this, SLOT (onBundlingButtonAndToggled(bool)));

    connect (d->bundlingShowCheckBox,  SIGNAL(toggled(bool)),            this, SLOT (onBundlingShowCheckBoxToggled (bool)));
    connect (d->bundlingButtonRst,     SIGNAL(clicked(void)),            this, SIGNAL (fiberSelectionReset(void)));

    connect (d->bundlingModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onBundlingItemChanged(QStandardItem*)));

    connect (d->dropOrOpenRoi, SIGNAL(objectDropped(const medDataIndex&)), this, SLOT(onRoiImported(const medDataIndex&)));
    connect (d->dropOrOpenRoi, SIGNAL(clicked()),                          this, SLOT(onDropSiteClicked()));
    connect (d->dropOrOpenRoi2, SIGNAL(objectDropped(const medDataIndex&)), this, SLOT(onImageImported(const medDataIndex&)));
    connect (d->dropOrOpenRoi2, SIGNAL(clicked()),                          this, SLOT(onDropSiteClicked()));
    connect (clearRoiButton,   SIGNAL(clicked()),                          this, SLOT(onClearRoiButtonClicked()));
    connect (d->roiComboBox,   SIGNAL(currentIndexChanged(int)),           this, SLOT(onRoiComboIndexChanged(int)));

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
    if(!this->parentToolBox())
        return;
    
    d->process = dtkAbstractProcessFactory::instance()->createSmartPointer("medMaskApplication");
    
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

    //d->bundlingModel->clear();
    d->bundlingModel->removeRows(0, d->bundlingModel->rowCount(QModelIndex()), QModelIndex());

    if (data->hasMetaData("BundleList") && data->hasMetaData("BundleColorList")) {

        QStringList bundleNames  = data->metaDataValues("BundleList");
        QStringList bundleColors = data->metaDataValues("BundleColorList");
        if (bundleNames.count()==bundleColors.count()) {
            for (int i=0; i<bundleNames.count(); i++)
                this->addBundle(bundleNames[i], QColor(bundleColors[i]));
        }
    }
}

void medMaskApplicationToolBox::onBundlingButtonVdtClicked()
{
    if (!d->data)
        return;

    // bool ok;
    //QString text = QInputDialog::getText(this, tr("Enter bundle name"),
    //                                   tr(""), QLineEdit::Normal, tr(""), &ok);
    QString text = tr("Fiber bundle #") + QString::number(d->bundlingModel->rowCount()+1);

    //if (ok && !text.isEmpty()) {
    QColor color = QColor::fromHsv(qrand()%360, 255, 210);

    emit fiberSelectionValidated (text, color);
    this->addBundle (text, color);
    //}
}

void medMaskApplicationToolBox::onBundleBoxCheckBoxToggled (bool value)
{
    if (!d->view)
        return;

    if (dtkAbstractViewInteractor *interactor = d->view->interactor ("v3dViewFiberInteractor")) {
        if (value)
            interactor->setProperty ("BoxVisibility", "true");
        else
            interactor->setProperty ("BoxVisibility", "false");

        d->view->update();
    }
}

void medMaskApplicationToolBox::addBundle (const QString &name, const QColor &color)
{/*
    if(!d->data)
        return;
  */

    //d->bundlingModel->removeRows(0, d->bundlingModel->rowCount(QModelIndex()), QModelIndex());

    int row = d->bundlingModel->rowCount();
    QStandardItem *item = new QStandardItem (name);
    item->setCheckable(true);
    item->setTristate(false);
    item->setEditable(false); // for now

    double meanFA = 0.0;
    double minFA  = 0.0;
    double maxFA  = 0.0;
    double varFA  = 0.0;

    double meanADC = 0.0;
    double minADC  = 0.0;
    double maxADC  = 0.0;
    double varADC  = 0.0;

    double meanLength = 0.0;
    double minLength  = 0.0;
    double maxLength  = 0.0;
    double varLength  = 0.0;

    if (d->view) {
        if (medAbstractViewFiberInteractor *interactor = qobject_cast<medAbstractViewFiberInteractor*>(d->view->interactor ("v3dViewFiberInteractor"))) {
            interactor->bundleFAStatistics(name, meanFA, minFA, maxFA, varFA);
            interactor->bundleADCStatistics(name, meanADC, minADC, maxADC, varADC);
            interactor->bundleLengthStatistics(name, meanLength, minLength, maxLength, varLength);
        }
    }

    QStandardItem *childItem1 = new QStandardItem (tr("FA: ") + QString::number(meanFA));
    childItem1->setEditable(false);
    childItem1->appendRow(new QStandardItem (tr("mean: ")     + QString::number(meanFA)));
    childItem1->appendRow(new QStandardItem (tr("variance: ") + QString::number(varFA)));
    childItem1->appendRow(new QStandardItem (tr("min: ")      + QString::number(minFA)));
    childItem1->appendRow(new QStandardItem (tr("max: ")      + QString::number(maxFA)));

    QStandardItem *childItem2 = new QStandardItem (tr("ADC: ") + QString::number(meanADC));
    childItem2->setEditable(false);
    childItem2->appendRow(new QStandardItem (tr("mean: ")     + QString::number(meanADC)));
    childItem2->appendRow(new QStandardItem (tr("variance: ") + QString::number(varADC)));
    childItem2->appendRow(new QStandardItem (tr("min: ")      + QString::number(minADC)));
    childItem2->appendRow(new QStandardItem (tr("max: ")      + QString::number(maxADC)));

    QStandardItem *childItem3 = new QStandardItem (tr("Length: ") + QString::number(meanLength));
    childItem3->setEditable(false);
    childItem3->appendRow(new QStandardItem (tr("mean: ")     + QString::number(meanLength)));
    childItem3->appendRow(new QStandardItem (tr("variance: ") + QString::number(varLength)));
    childItem3->appendRow(new QStandardItem (tr("min: ")      + QString::number(minLength)));
    childItem3->appendRow(new QStandardItem (tr("max: ")      + QString::number(maxLength)));

    item->appendRow(childItem1);
    item->appendRow(childItem2);
    item->appendRow(childItem3);

    d->bundlingModel->setItem(row, item);

    //d->bundlingModel->addItem (name);

    //    d->bundlingModel->setData(d->bundlingModel->index(row, 0, QModelIndex()),
    //                            name);

    d->bundlingModel->setData(d->bundlingModel->index(row, 0, QModelIndex()),
                              true, Qt::CheckStateRole);

    d->bundlingModel->setData(d->bundlingModel->index(row, 0, QModelIndex()),
                              color, Qt::DecorationRole);


}

void medMaskApplicationToolBox::onRoiImported(const medDataIndex& index)
{
    dtkSmartPointer<dtkAbstractData> data = medDataManager::instance()->data(index);
    qDebug()<<" data->identifier() : "<<data->identifier()<<endl;
    // we accept only ROIs (itkDataImageUChar3)
    if (!data /*|| !d->view */|| data->identifier() != "itkDataImageUChar3")
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

    //if (medAbstractViewFiberInteractor *interactor = qobject_cast<medAbstractViewFiberInteractor*>(d->view->interactor ("v3dViewFiberInteractor")))
    //{
    //    interactor->setROI(data);
    //    d->view->update();
    //}
}

void medMaskApplicationToolBox::onImageImported(const medDataIndex& index)
{
    //dtkSmartPointer<dtkAbstractData> data = medDataManager::instance()->data(index);
    //qDebug()<<" data->identifier() : "<<data->identifier()<<endl;
    //if (!data)
    //{
    //    return;
    //}
    //// put the thumbnail in the medDropSite as well
    //// (code copied from @medDatabasePreviewItem)
    //medAbstractDbController* dbc = medDataManager::instance()->controllerForDataSource(index.dataSourceId());
    //QString thumbpath = dbc->metaData(index, medMetaDataKeys::ThumbnailPath);

    //bool shouldSkipLoading = false;
    //if ( thumbpath.isEmpty() )
    //{
    //    // first try to get it from controller
    //    QImage thumbImage = dbc->thumbnail(index);
    //    if (!thumbImage.isNull())
    //    {
    //        d->dropOrOpenRoi2->setPixmap(QPixmap::fromImage(thumbImage));
    //        shouldSkipLoading = true;
    //    }
    //}
    //if (!shouldSkipLoading) {
    //    medImageFileLoader *loader = new medImageFileLoader(thumbpath);

    //    connect(loader, SIGNAL(completed(const QImage&)), this, SLOT(setImage(const QImage&))); SLOT A MODIFIER
    //    QThreadPool::globalInstance()->start(loader);
    //}
}

void medMaskApplicationToolBox::onClearRoiButtonClicked(void)
{
    //if (!d->view)
    //    return;

    //// create dummy mask image
    //dtkAbstractData *data = dtkAbstractDataFactory::instance()->create("itkDataImageUChar3");
    //if (medAbstractViewFiberInteractor *interactor = qobject_cast<medAbstractViewFiberInteractor*>(d->view->interactor ("v3dViewFiberInteractor"))) {
    //    interactor->setROI(data);
    //    d->view->update();
    //}
    //data->deleteLater();

    //// clear medDropSite and put text again
    //d->dropOrOpenRoi->clear();
    //d->dropOrOpenRoi->setText(tr("Click to open a ROI\nfrom your hard drive\nor drag-and-drop one\nfrom the database."));
}

void medMaskApplicationToolBox::onRoiComboIndexChanged (int value)
{
    //if (!d->view)
    //    return;

    //if (medAbstractViewFiberInteractor *interactor = qobject_cast<medAbstractViewFiberInteractor*>(d->view->interactor ("v3dViewFiberInteractor"))) {
    //    int boolean = interactor->roiBoolean (value);
    //    switch (boolean) {
    //    case 2:
    //        d->andButton->blockSignals (true);
    //        d->andButton->setChecked   (true);
    //        d->andButton->blockSignals (false);
    //        break;

    //    case 1:
    //        d->notButton->blockSignals (true);
    //        d->notButton->setChecked   (true);
    //        d->notButton->blockSignals (false);
    //        break;

    //    case 0:
    //    default:
    //        d->nullButton->blockSignals (true);
    //        d->nullButton->setChecked   (true);
    //        d->nullButton->blockSignals (false);
    //        break;
    //    }
    //}

    //d->view->update();
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
    if (d->view==view) {
        if (view)
            if (dtkAbstractViewInteractor *interactor = view->interactor ("v3dViewFiberInteractor"))
                this->setData (interactor->data()); // data may have changed
        return;
    }

    d->bundlingModel->removeRows(0, d->bundlingModel->rowCount(QModelIndex()), QModelIndex());

    if (d->view) {
        if (dtkAbstractViewInteractor *interactor = d->view->interactor ("v3dViewFiberInteractor")) {
            disconnect (this, SIGNAL(fiberSelectionValidated(const QString&, const QColor&)),
                        interactor, SLOT(onSelectionValidated(const QString&, const QColor&)));
            disconnect (this, SIGNAL(fiberSelectionTagged()),    interactor, SLOT(onSelectionTagged()));
            disconnect (this, SIGNAL(fiberSelectionReset()),     interactor, SLOT(onSelectionReset()));
            disconnect (this, SIGNAL(bundlingBoxBooleanOperatorChanged(int)),
                        interactor, SLOT(onBundlingBoxBooleanOperatorChanged(int)));
        }
    }

    if (!view) {
        d->view = 0;
        d->data = 0;
        return;
    }

    /*
    if (view->property ("Orientation")!="3D") { // only interaction with 3D views is authorized
        d->view = 0;
        d->data = 0;
        return;
    }
    */

    d->view = view;

    if (dtkAbstractViewInteractor *interactor = view->interactor ("v3dViewFiberInteractor")) {
        connect (this, SIGNAL(fiberSelectionValidated(const QString&, const QColor&)),
                 interactor, SLOT(onSelectionValidated(const QString&, const QColor&)));
        connect (this, SIGNAL(fiberSelectionTagged()),           interactor, SLOT(onSelectionTagged()));
        connect (this, SIGNAL(fiberSelectionReset()),            interactor, SLOT(onSelectionReset()));
        connect (this, SIGNAL(bundlingBoxBooleanOperatorChanged(int)),
                 interactor, SLOT(onBundlingBoxBooleanOperatorChanged(int)));

        d->bundleBoxCheckBox->blockSignals (true);
        d->bundleBoxCheckBox->setChecked( interactor->property("BoxVisibility")=="true" );
        d->bundleBoxCheckBox->blockSignals (false);

        this->setData (interactor->data());
    }
}
void medMaskApplicationToolBox::onBundlingItemChanged(QStandardItem *item)
{
    //if (d->view) {
    //    if (medAbstractViewFiberInteractor *interactor = qobject_cast<medAbstractViewFiberInteractor*>(d->view->interactor ("v3dViewFiberInteractor")))
    //        interactor->setBundleVisibility(item->text(), item->checkState());
    //    d->view->update();
    //}
}

void medMaskApplicationToolBox::onBundlingShowCheckBoxToggled(bool value)
{
    //if (d->view) {
    //    if (medAbstractViewFiberInteractor *interactor = qobject_cast<medAbstractViewFiberInteractor*>(d->view->interactor ("v3dViewFiberInteractor")))
    //        interactor->setBundleVisibility(value);
    //    d->view->update();
    //}
}

void medMaskApplicationToolBox::onBundlingButtonAndToggled(bool value)
{
    if (value)
        emit bundlingBoxBooleanOperatorChanged (0);
    else
        emit bundlingBoxBooleanOperatorChanged (1);

    if (d->view)
        d->view->update();
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