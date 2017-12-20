/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <QFileDialog>
#include <QTreeWidget>
#include <QTreeView>

#include <medDataIndex.h>
#include <medDataManager.h>

class QTreeWidget;
class QTreeWidgetItem;
class viewerFilesToSaveDialogPrivate;

/**
 * @brief Dialog window displaying non-persistent database items (i.e. unsaved data) when the user is about to close the application
 */
class viewerFilesToSaveDialog: public QDialog
{
    Q_OBJECT
    
public:
  
    viewerFilesToSaveDialog(QWidget *parent);
    virtual ~viewerFilesToSaveDialog();
    QList<medDataIndex> indexList();

public slots:

    /**
     * Method to store non-persistent data (according to the tree widget items that are checked) into the database.
     */
    void save();

    /**
     * Method to decrement a counter (i.e. the number of checked items that remain to be saved).
     */
    void updateCounter();

    /**
     * This method rebuilds the treewidget.
     */
    void onUpdateTree();

private:
  
    viewerFilesToSaveDialogPrivate *d;
};
