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

#include "medMaskApplicationPluginExport.h"

#include "itkImage.h"
//#include "itkImageFileReader.h"
//#include "itkMaskImageFilter.h"
//#include "itkImageRegionIterator.h"
//#include "QuickView.h"

class medMaskApplicationPrivate;

typedef itk::Image<unsigned char, 2>  ImageType;

class MEDMASKAPPLICATIONPLUGIN_EXPORT medMaskApplication : public dtkAbstractProcess
{
    Q_OBJECT
    
public:
    medMaskApplication();
    virtual ~medMaskApplication();
    
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
    medMaskApplicationPrivate *d;
};

dtkAbstractProcess *createMedMaskApplication();


