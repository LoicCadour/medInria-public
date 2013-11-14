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

#include "medBinaryOperationPluginExport.h"

#include "itkImage.h"

class itkAndOperatorPrivate;

class MEDBINARYOPERATIONPLUGIN_EXPORT itkAndOperator : public dtkAbstractProcess
{
    Q_OBJECT
    
public:
    itkAndOperator();
    virtual ~itkAndOperator();
    
    virtual QString description() const;
    
    static bool registered();

    void setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData);

    
public slots:
    
    //! Input data to the plugin is set through here
    void setInput(dtkAbstractData *data, int channel);
    
    //! Parameters are set through here, channel allows to handle multiple parameters
    void setParameter(double  data, int channel);
    
    //! Method to actually start the filter
    int update();
    
    //! The output will be available through here
    dtkAbstractData *output();
    
    
private:
    itkAndOperatorPrivate *d;
};

dtkAbstractProcess *createitkAndOperator();


