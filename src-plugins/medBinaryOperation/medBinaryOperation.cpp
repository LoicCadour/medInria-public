/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medBinaryOperation.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include <medMetaDataKeys.h>
#include <medAbstractDataImage.h>
#include <medDataManager.h>

#include <itkXorImageFilter.h>

// /////////////////////////////////////////////////////////////////
// medBinaryOperationPrivate
// /////////////////////////////////////////////////////////////////

class medBinaryOperationPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> inputA;
    dtkSmartPointer <dtkAbstractData> inputB;
    dtkSmartPointer <dtkAbstractData> output;
};

// /////////////////////////////////////////////////////////////////
// medBinaryOperation
// /////////////////////////////////////////////////////////////////

medBinaryOperation::medBinaryOperation() : dtkAbstractProcess(), d(new medBinaryOperationPrivate)
{
    d->inputA = NULL;
    d->inputB = NULL;
    d->output = NULL;   
}

medBinaryOperation::~medBinaryOperation()
{
    
}

bool medBinaryOperation::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("medBinaryOperation", createMedBinaryOperation);
}

QString medBinaryOperation::description() const
{
    return "medBinaryOperation";
}

void medBinaryOperation::setInput ( dtkAbstractData *data, int channel)
{
    if ( !data )
        return;
    if ( channel == 0)
    {
        d->inputA = data;
    }

    if ( channel == 1 )
    {
        QString identifier = data->identifier();
        d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( identifier );
        d->inputB = data;
    }
}    

void medBinaryOperation::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int medBinaryOperation::update()
{
    if ( !d->inputA ||!d->inputA->data() || !d->inputB ||!d->inputB->data())
        return -1;

    typedef itk::Image<unsigned char, 3> ImageType;

    typedef itk::XorImageFilter <ImageType, ImageType, ImageType> XorImageFilterType;
    XorImageFilterType::Pointer xorFilter = XorImageFilterType::New();
    xorFilter->SetInput1(dynamic_cast<ImageType *> ( ( itk::Object* ) ( d->inputA->data() ) ));
    xorFilter->SetInput2(dynamic_cast<ImageType *> ( ( itk::Object* ) ( d->inputB->data() ) ));
    xorFilter->Update();

    d->output->setData(xorFilter->GetOutput());
    QString newSeriesDescription = d->inputA->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += " XOR " + d->inputB->metadata ( medMetaDataKeys::SeriesDescription.key() );

    d->output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    medDataManager::instance()->importNonPersistent(d->output);
    return EXIT_SUCCESS;
}        

dtkAbstractData * medBinaryOperation::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createMedBinaryOperation()
{
    return new medBinaryOperation;
}
