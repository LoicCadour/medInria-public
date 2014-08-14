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
#include <medAbstractData.h>
#include <medAbstractWorkspace.h>
#include "medMeshToolsPluginExport.h"

class medMeshToolsToolBoxPrivate;

class MEDMESHTOOLSPLUGIN_EXPORT medMeshToolsToolBox : public medToolBox
{
    Q_OBJECT
    MED_TOOLBOX_INTERFACE("medMeshToolsToolBox","Mesh ToolBox",<<"mesh")
    
public:
    medMeshToolsToolBox(QWidget *parent = 0);
    ~medMeshToolsToolBox();
    
    medAbstractData *processOutput();
    
    static bool registered();
    dtkPlugin * plugin();
    
    
    void clear(void);
    void setWorkspace(medAbstractWorkspace* workspace);
    medAbstractWorkspace* getWorkspace();

signals:
    void success();
    void failure();
    
public slots:
    void run();
    void displayNumberOfTriangles();
    void refine();
void updateView();
    
protected slots :
    void addData(medAbstractData* data, int layer);
    void removeData(medAbstractData* data, int layer);

    void addMeshToView();

private:
    medMeshToolsToolBoxPrivate *d;
};


