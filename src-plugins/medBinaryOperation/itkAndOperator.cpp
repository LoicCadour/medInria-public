/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkAndOperator.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include <medMetaDataKeys.h>
#include <medAbstractDataImage.h>
#include <medDataManager.h>

#include <itkAndImageFilter.h>

// /////////////////////////////////////////////////////////////////
// itkAndOperatorPrivate
// /////////////////////////////////////////////////////////////////

class itkAndOperatorPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> inputA;
    dtkSmartPointer <dtkAbstractData> inputB;
    dtkSmartPointer <dtkAbstractData> output;
};

// /////////////////////////////////////////////////////////////////
// itkAndOperator
// /////////////////////////////////////////////////////////////////

itkAndOperator::itkAndOperator() : dtkAbstractProcess(), d(new itkAndOperatorPrivate)
{
    d->inputA = NULL;
    d->inputB = NULL;
    d->output = NULL;   
}

itkAndOperator::~itkAndOperator()
{
    
}

bool itkAndOperator::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("itkAndOperator", createitkAndOperator);
}

QString itkAndOperator::description() const
{
    return "itkAndOperator";
}

void itkAndOperator::setInput ( dtkAbstractData *data, int channel)
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

void itkAndOperator::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int itkAndOperator::update()
{
    if ( !d->inputA ||!d->inputA->data() || !d->inputB ||!d->inputB->data())
        return -1;

    typedef itk::Image<unsigned char, 3> ImageType;

    typedef itk::AndImageFilter <ImageType, ImageType, ImageType> AndImageFilterType;
    AndImageFilterType::Pointer andFilter = AndImageFilterType::New();
    andFilter->SetInput1(dynamic_cast<ImageType *> ( ( itk::Object* ) ( d->inputA->data() ) ));
    andFilter->SetInput2(dynamic_cast<ImageType *> ( ( itk::Object* ) ( d->inputB->data() ) ));
    andFilter->Update();

    d->output->setData(andFilter->GetOutput());

    setOutputMetadata(d->inputA, d->output);
    QString newSeriesDescription = d->inputA->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += " AND " + d->inputB->metadata ( medMetaDataKeys::SeriesDescription.key() );
    d->output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    medDataManager::instance()->importNonPersistent(d->output);
    return EXIT_SUCCESS;
}        

void itkAndOperator::setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData)
{
    Q_ASSERT(outputData && inputData);

    QStringList metaDataToCopy;
    metaDataToCopy 
        << medMetaDataKeys::PatientName.key()
        << medMetaDataKeys::StudyDescription.key();

    foreach( const QString & key, metaDataToCopy ) {
        outputData->setMetaData(key, inputData->metadatas(key));
    }
}

dtkAbstractData * itkAndOperator::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createitkAndOperator()
{
    return new itkAndOperator;
}
