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

class medBinaryOperationPrivate;

typedef itk::Image<unsigned short, 3>  ImageType;
typedef itk::Image<unsigned char, 3>  MaskType;

class MEDBINARYOPERATIONPLUGIN_EXPORT medBinaryOperation : public dtkAbstractProcess
{
    Q_OBJECT
    
public:
    medBinaryOperation();
    virtual ~medBinaryOperation();
    
    virtual QString description() const;
    
    static bool registered();
    
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
    medBinaryOperationPrivate *d;
};

dtkAbstractProcess *createMedBinaryOperation();


