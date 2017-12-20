/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medDatabaseNonPersistentController.h>
#include <medDatabaseNonPersistentItem.h>
#include <medDataIndex.h>

#include <QtGui>

#include <viewerFilesToSaveDialog.h>

class viewerFilesToSaveDialogPrivate
{
public:

    QTreeWidget *treeWidget;
    QPushButton *saveButton;
    QPushButton *cancelButton;
    
    bool quitRequested;

    unsigned int counter;
    QList<medDataIndex> dataToSaveIndexList;
    QMutex mutex;
};

/*********************************/

class viewerFilesToSaveDialogCheckListItem : public QTreeWidgetItem
{
    public :
        viewerFilesToSaveDialogCheckListItem(QTreeWidgetItem *root, const medDataIndex &index, const QString &seriesName, const QString &studyName, const QString &name, const QString &file)
        : QTreeWidgetItem(), medIndex(index)
        {
            root->addChild(this);

            setFlags(flags() | Qt::ItemIsUserCheckable);
            setText(1, seriesName);
            setText(2, studyName);
            setText(3, name);
            setText(5, file);
            setCheckState(0, Qt::Checked);
        }

        const medDataIndex& getIndex() const
        {
            return medIndex;
        }

        ~viewerFilesToSaveDialogCheckListItem()
        {}

    private:
        medDataIndex medIndex;
};

/***********************************************/

viewerFilesToSaveDialog::viewerFilesToSaveDialog(QWidget *parent) : QDialog(parent), d (new viewerFilesToSaveDialogPrivate)
{
    QLabel *label = new QLabel(this);
    label->setText(tr("Select the data you want to include in the NavX file"));

    d->saveButton = new QPushButton(tr("Save"),this);
    d->cancelButton = new QPushButton(tr("Cancel"),this);

    d->treeWidget = new QTreeWidget(this);
    d->treeWidget->setColumnCount(6);
    
    d->quitRequested = false;

    QStringList headers;
    headers << tr("Select") << tr("Series") << tr("Study") << tr("Name") << tr("File");

    d->treeWidget->setHeaderLabels(headers);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(d->saveButton);
    hlayout->addWidget(d->cancelButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(d->treeWidget);
    layout->addLayout(hlayout);

    //TODO a little bit ugly the way viewerFilesToSaveDialogCheckListItem are parented no ?
    foreach(medDatabaseNonPersistentItem *item, medDatabaseNonPersistentController::instance()->items())
    {
        if ((item->studyName() != "") && (item->seriesName() != ""))
            new viewerFilesToSaveDialogCheckListItem(d->treeWidget->invisibleRootItem(), item->index(), item->seriesName(), item->studyName(), item->name(), item->file());
    }
    
    d->treeWidget->resizeColumnToContents(0);

    connect (d->saveButton, SIGNAL(clicked()), this, SLOT(save()));
    connect (d->cancelButton,SIGNAL(clicked()), this, SLOT(reject()));

    connect (medDataManager::instance(), SIGNAL(dataImported(medDataIndex,QUuid)),this, SLOT(updateCounter()));

    setModal(true);
}

viewerFilesToSaveDialog::~viewerFilesToSaveDialog()
{
    delete d;
    d = NULL;
}

void viewerFilesToSaveDialog::save()
{
    d->counter = 0;

    for (int i = 0; i < d->treeWidget->topLevelItemCount(); ++i)
    {
        viewerFilesToSaveDialogCheckListItem * checkListItem = dynamic_cast<viewerFilesToSaveDialogCheckListItem*> (d->treeWidget->topLevelItem(i));

        if (checkListItem->checkState(0) == Qt::Checked)
        {
            d->counter++;
            d->dataToSaveIndexList.append(checkListItem->getIndex());
        }
    }
    this->accept();
}

void viewerFilesToSaveDialog::updateCounter()
{
    d->mutex.lock();
    d->counter--;
    d->mutex.unlock();
    
    if(d->counter == 0)
    {
        if (d->quitRequested)
            this->accept();
        else
            this->onUpdateTree();
    }
}

void viewerFilesToSaveDialog::onUpdateTree()
{
    d->treeWidget->clear();
    
    foreach(medDatabaseNonPersistentItem *item, medDatabaseNonPersistentController::instance()->items())
    d->treeWidget->insertTopLevelItem(0,new viewerFilesToSaveDialogCheckListItem(d->treeWidget->invisibleRootItem(), item->index(), item->seriesName(), item->studyName(), item->name(), item->file()));
    
    d->treeWidget->update();
    d->treeWidget->resizeColumnToContents(0);
    d->treeWidget->showMaximized();
}

QList<medDataIndex> viewerFilesToSaveDialog::indexList()
{
    return d->dataToSaveIndexList;
}