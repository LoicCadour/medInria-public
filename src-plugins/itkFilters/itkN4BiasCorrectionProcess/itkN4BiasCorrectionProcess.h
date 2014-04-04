/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <dtkCore/dtkAbstractProcess.h>

#include "itkN4BiasCorrectionProcessPluginExport.h"

class itkN4BiasCorrectionProcessPrivate;

class ITKN4BIASCORRECTIONPROCESSPLUGIN_EXPORT itkN4BiasCorrectionProcess : public dtkAbstractProcess
{
    Q_OBJECT
    
public:
    itkN4BiasCorrectionProcess();
    virtual ~itkN4BiasCorrectionProcess();
    
    virtual QString description() const;
    
    static bool registered();
    
public slots:
    
    //! Input data to the plugin is set through here
    void setInput(dtkAbstractData *data);
    
    //! Parameters are set through here, channel allows to handle multiple parameters
    void setParameter(double  data, int channel);
    
    //! Method to actually start the filter
    int update();
    
    //! The output will be available through here
    dtkAbstractData *output();
    
    
private:
    itkN4BiasCorrectionProcessPrivate *d;
};

dtkAbstractProcess *createItkN4BiasCorrectionProcess();


