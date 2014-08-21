/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medToolBox.h>
#include "medVtkViewPluginExport.h"
#include <medVtkView.h>
#include <medAbstractWorkspace.h>

#include <vtkSmartPointer.h>

class vtkBoxWidget;
class vtkMyCallback;
class vtkMetaDataSet;

class MEDVTKVIEWPLUGIN_EXPORT meshModifyToolBox : public medToolBox
{
    Q_OBJECT
    MED_TOOLBOX_INTERFACE("meshModifyToolBox","Toolbox to translate/rotate a mesh",<<"mesh")

    
public:
    meshModifyToolBox(QWidget * parent = 0);
    virtual ~meshModifyToolBox();
    
    static bool registered();

    void setWorkspace(medAbstractWorkspace*);

public slots:

    void toggleWidget();
    void cancel();
    void dataAdded(medAbstractData* data, int index);

    void exportTransform();
    void importTransform();
    void updateView();

private:
    medAbstractWorkspace* workspace;
    medVtkView * _view;
    QPushButton * _modifyButton;
    QPushButton * _cancelButton;
    QPushButton * _exportButton;
    QPushButton * _importButton;
    QSpinBox * _spinBox;
    vtkSmartPointer<vtkBoxWidget> _boxWidget;
    vtkSmartPointer<vtkMyCallback> _callback;
    vtkMetaDataSet * _dataset;
    bool _modifying;
};




