/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medFilteringAbstractToolBox.h>
#include "itkN4BiasCorrectionProcessPluginExport.h"

class itkN4BiasCorrectionProcessToolBoxPrivate;

class ITKN4BIASCORRECTIONPROCESSPLUGIN_EXPORT itkN4BiasCorrectionProcessToolBox : public medFilteringAbstractToolBox
{
    Q_OBJECT
    
public:
    itkN4BiasCorrectionProcessToolBox(QWidget *parent = 0);
    ~itkN4BiasCorrectionProcessToolBox();
    
    dtkAbstractData *processOutput();
    
    static bool registered();
    dtkPlugin * plugin();
    
signals:
    void success();
    void failure();
    
    public slots:
    void run();
    
private:
    itkN4BiasCorrectionProcessToolBoxPrivate *d;
};


