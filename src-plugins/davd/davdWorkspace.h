/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medWorkspace.h>

#include "davdPluginExport.h"

class davdWorkspacePrivate;
    
class DAVDPLUGIN_EXPORT davdWorkspace : public medWorkspace
{
    Q_OBJECT
    
public:
    davdWorkspace(QWidget *parent = 0);
    virtual ~davdWorkspace();
    
    virtual QString description() const;

    virtual QString identifier() const;

    virtual void setupViewContainerStack();

    static bool isUsable();

    static bool registered();

    void displayMask();
    void dilateTheMask();
    void intersectMasks();
    void applyMaskToImage();
    void applyThresholdToImage();
    void setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData);
    void calculateVolume(dtkAbstractData* input);
    unsigned int getNumberOfOnes(dtkAbstractData* input);

public slots:
    virtual void connectToolboxesToCurrentContainer(const QString &name);
    void goToPreviousStep();
    void goToNextStep();


    
private:
    davdWorkspacePrivate *d;
};
