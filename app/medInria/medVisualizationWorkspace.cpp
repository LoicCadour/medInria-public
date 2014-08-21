/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medVisualizationWorkspace.h>

#include <medViewContainer.h>
#include <medTabbedViewContainers.h>
#include <medSettingsManager.h>
#include <medToolBoxFactory.h>
#include <medToolBox.h>

class medVisualizationWorkspacePrivate
{
public:

};

medVisualizationWorkspace::medVisualizationWorkspace(QWidget *parent) : medAbstractWorkspace(parent), d(new medVisualizationWorkspacePrivate)
{
	medToolBox* meshToolBox = medToolBoxFactory::instance()->createToolBox("medMeshToolsToolBox");
	if(meshToolBox){
		this->addToolBox(meshToolBox);
		meshToolBox->setWorkspace((this));
	}

    medToolBox* meshClutToolBox = medToolBoxFactory::instance()->createToolBox("medClutToolBox");
    if(meshClutToolBox){
        this->addToolBox(meshClutToolBox);
        meshClutToolBox->setWorkspace((this));
    }

    medToolBox* meshModifyToolBox = medToolBoxFactory::instance()->createToolBox("meshModifyToolBox");
    if(meshModifyToolBox){
        this->addToolBox(meshModifyToolBox);
        meshModifyToolBox->setWorkspace((this));
    }
    else 
        qDebug()<<"PAS DE MESHMODIFY TBX";
}

void medVisualizationWorkspace::setupViewContainerStack()
{
    if (!stackedViewContainers()->count()) {
        this->stackedViewContainers()->addContainerInTab(this->name());
    }
    this->stackedViewContainers()->unlockTabs();
}

medVisualizationWorkspace::~medVisualizationWorkspace(void)
{
    delete d;
    d = NULL;
}

bool medVisualizationWorkspace::isUsable(){
    return true; // for the time being, no test is defined.
}
