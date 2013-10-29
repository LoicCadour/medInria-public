/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medLayoutChooser.h"
#include "pipelineToolBox.h"

class pipelineToolBoxPrivate
{
public:
    QPushButton *nextButton, *previousButton;
    QLabel *stepDescription;
};

pipelineToolBox::pipelineToolBox(QWidget *parent) : medToolBox(parent), d(new pipelineToolBoxPrivate)
{
    QWidget *customPage = new QWidget(this);
    this->addWidget(customPage);
    this->setTitle(tr("Steps"));

    d->stepDescription = new QLabel();


    d->previousButton = new QPushButton("Previous", customPage);
    d->previousButton->setDisabled(true);
    connect(d->previousButton, SIGNAL(clicked()), this, SLOT(goToPreviousStep()));

    d->nextButton = new QPushButton("Next", customPage);
    connect(d->nextButton, SIGNAL(clicked()), this, SLOT(goToNextStep()));

    QHBoxLayout *layout = new QHBoxLayout(customPage);
    layout->addWidget(d->previousButton);
    layout->addWidget(d->nextButton);
}

pipelineToolBox::~pipelineToolBox(void)
{
    delete d;

    d = NULL;
}

void pipelineToolBox::goToPreviousStep(void)
{
    emit previousClicked();
}

void pipelineToolBox::goToNextStep(void)
{
    emit nextClicked();
}

QPushButton* pipelineToolBox::getNextButton()
{
    return d->nextButton;
}

QPushButton* pipelineToolBox::getPreviousButton()
{
    return d->previousButton;
}