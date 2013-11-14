/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkOrOperator.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include <medMetaDataKeys.h>
#include <medAbstractDataImage.h>
#include <medDataManager.h>

#include <itkOrImageFilter.h>

// /////////////////////////////////////////////////////////////////
// itkOrOperatorPrivate
// /////////////////////////////////////////////////////////////////

class itkOrOperatorPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> inputA;
    dtkSmartPointer <dtkAbstractData> inputB;
    dtkSmartPointer <dtkAbstractData> output;
};

// /////////////////////////////////////////////////////////////////
// itkOrOperator
// /////////////////////////////////////////////////////////////////

itkOrOperator::itkOrOperator() : dtkAbstractProcess(), d(new itkOrOperatorPrivate)
{
    d->inputA = NULL;
    d->inputB = NULL;
    d->output = NULL;   
}

itkOrOperator::~itkOrOperator()
{
    
}

bool itkOrOperator::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("itkOrOperator", createitkOrOperator);
}

QString itkOrOperator::description() const
{
    return "itkOrOperator";
}

void itkOrOperator::setInput ( dtkAbstractData *data, int channel)
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

void itkOrOperator::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int itkOrOperator::update()
{
    if ( !d->inputA ||!d->inputA->data() || !d->inputB ||!d->inputB->data())
        return -1;

    typedef itk::Image<unsigned char, 3> ImageType;

    typedef itk::OrImageFilter <ImageType, ImageType, ImageType> OrImageFilterType;
    OrImageFilterType::Pointer orFilter = OrImageFilterType::New();
    orFilter->SetInput1(dynamic_cast<ImageType *> ( ( itk::Object* ) ( d->inputA->data() ) ));
    orFilter->SetInput2(dynamic_cast<ImageType *> ( ( itk::Object* ) ( d->inputB->data() ) ));
    orFilter->Update();

    d->output->setData(orFilter->GetOutput());
    QString newSeriesDescription = d->inputA->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += " OR " + d->inputB->metadata ( medMetaDataKeys::SeriesDescription.key() );

    d->output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    medDataManager::instance()->importNonPersistent(d->output);
    return EXIT_SUCCESS;
}        

dtkAbstractData * itkOrOperator::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createitkOrOperator()
{
    return new itkOrOperator;
}
