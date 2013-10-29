/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <QtCore>

#include <medWorkspace.h>


class medDavdWorkspacePrivate;

class medDavdWorkspace : public medWorkspace
{
    Q_OBJECT

public:
     medDavdWorkspace(QWidget *parent = 0);
    ~medDavdWorkspace();

    virtual void setupViewContainerStack();

    virtual QString identifier()  const;
    virtual QString description() const;
    static bool isUsable();
    //QString updateDescription(){return d->currentStepDesciption;}

public slots:
    /**
     * @brief Connects toolboxes to the current container
     *
     * @param name the container name
    */
    virtual void connectToolboxesToCurrentContainer(const QString &name);
    void goToPreviousStep();
    void goToNextStep();

private:
    medDavdWorkspacePrivate *d;
};


