/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medMaskApplication.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include <itkMaskImageFilter.h>

// /////////////////////////////////////////////////////////////////
// medMaskApplicationPrivate
// /////////////////////////////////////////////////////////////////

class medMaskApplicationPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> mask;
    dtkSmartPointer <dtkAbstractData> input;
    dtkSmartPointer <dtkAbstractData> output;
};

// /////////////////////////////////////////////////////////////////
// medMaskApplication
// /////////////////////////////////////////////////////////////////

medMaskApplication::medMaskApplication() : dtkAbstractProcess(), d(new medMaskApplicationPrivate)
{
    
}

medMaskApplication::~medMaskApplication()
{
    
}

bool medMaskApplication::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("medMaskApplication", createMedMaskApplication);
}

QString medMaskApplication::description() const
{
    return "medMaskApplication";
}

void medMaskApplication::setInput ( dtkAbstractData *data )
{
    if ( !data )
        return;
    
    QString identifier = data->identifier();
    
    d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( identifier );
    
    d->input = data;
}

void medMaskApplication::setMask ( dtkAbstractData *data )
{
    if ( !data )
        return;
    
    QString identifier = data->identifier();
      
    d->mask = data;
} 

void medMaskApplication::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int medMaskApplication::update()
{
    if ( !d->input )
        return -1;
    
    ImageType::Pointer mask = ImageType::New();
    
    typedef itk::MaskImageFilter< ImageType, ImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    maskFilter->SetInput(dynamic_cast<ImageType *> ( ( itk::Object* ) ( d->input->data() ) ));
    maskFilter->SetMaskImage(dynamic_cast<ImageType *> ( ( itk::Object* ) ( d->mask->data() ) ));
    
    return EXIT_SUCCESS;
}        

dtkAbstractData * medMaskApplication::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createMedMaskApplication()
{
    return new medMaskApplication;
}
