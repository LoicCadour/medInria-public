/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "medGuiExport.h"
#include "medToolBox.h"

class pipelineToolBoxPrivate;

class MEDGUI_EXPORT pipelineToolBox : public medToolBox
{
    Q_OBJECT

public:
     pipelineToolBox(QWidget *parent = 0);
    ~pipelineToolBox();

    QPushButton *getNextButton();
    QPushButton *getPreviousButton();
    void setLabel(QString);

signals:
    void previousClicked();
    void nextClicked();

protected slots:
    void goToPreviousStep();
    void goToNextStep();

private:
    pipelineToolBoxPrivate *d;
};


