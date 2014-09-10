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
    //medToolBox* ICPToolBox = medToolBoxFactory::instance()->createToolBox("iterativeClosestPointToolBox");
    //if (ICPToolBox)
    //{
    //    ICPToolBox->setWorkspace(this);
    //    this->addToolBox(ICPToolBox);
    //}

    medToolBox* manualRegistrationToolBox = medToolBoxFactory::instance()->createToolBox("manualRegistrationToolBox");
    if (manualRegistrationToolBox)
    {
        manualRegistrationToolBox->setWorkspace(this);
        this->addToolBox(manualRegistrationToolBox);
    }
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
