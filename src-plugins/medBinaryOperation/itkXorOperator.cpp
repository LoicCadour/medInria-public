/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkXorOperator.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include <medMetaDataKeys.h>
#include <medAbstractDataImage.h>
#include <medDataManager.h>

#include <itkCastImageFilter.h>

#include <itkXorImageFilter.h>

// /////////////////////////////////////////////////////////////////
// itkXorOperatorPrivate
// /////////////////////////////////////////////////////////////////

class itkXorOperatorPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> inputA;
    dtkSmartPointer <dtkAbstractData> inputB;
    dtkSmartPointer <dtkAbstractData> output;
};

// /////////////////////////////////////////////////////////////////
// itkXorOperator
// /////////////////////////////////////////////////////////////////

itkXorOperator::itkXorOperator() : dtkAbstractProcess(), d(new itkXorOperatorPrivate)
{
    d->inputA = NULL;
    d->inputB = NULL;
    d->output = NULL;   
}

itkXorOperator::~itkXorOperator()
{
    
}

bool itkXorOperator::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("itkXorOperator", createitkXorOperator);
}

QString itkXorOperator::description() const
{
    return "itkXorOperator";
}

void itkXorOperator::setInput ( dtkAbstractData *data, int channel)
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
        d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( "itkDataImageUChar3" );
        d->inputB = data;
    }
}    

void itkXorOperator::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int itkXorOperator::update()
{
    if ( !d->inputA ||!d->inputA->data() || !d->inputB ||!d->inputB->data())
        return -1;

    typedef itk::Image<unsigned char, 3> ImageType;
    ImageType::Pointer imageA;
    imageA = dynamic_cast< ImageType*>((itk::Object*)(d->inputA->data()));

    if (d->inputA->identifier() == "itkDataImageInt3")
    {
        typedef itk::Image<int, 3> InputImage;
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(dynamic_cast< InputImage*>((itk::Object*)(d->inputA->data())));
        caster->Update();
        imageA = caster->GetOutput();
    }

    ImageType::Pointer imageB;
    imageB = dynamic_cast< ImageType*>((itk::Object*)(d->inputB->data()));

    if (d->inputB->identifier() == "itkDataImageInt3")
    {
        typedef itk::Image<int, 3> InputImage;
        typedef itk::CastImageFilter< InputImage, ImageType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(dynamic_cast< InputImage*>((itk::Object*)(d->inputB->data())));
        caster->Update();
        imageB = caster->GetOutput();
    }



    typedef itk::XorImageFilter <ImageType, ImageType, ImageType> XorImageFilterType;
    XorImageFilterType::Pointer xorFilter = XorImageFilterType::New();
    xorFilter->SetInput1(imageA);
    xorFilter->SetInput2(imageB);
    xorFilter->Update();

    d->output->setData(xorFilter->GetOutput());

    setOutputMetadata(d->inputA, d->output);
    QString newSeriesDescription = d->inputA->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += " XOR " + d->inputB->metadata ( medMetaDataKeys::SeriesDescription.key() );
    d->output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    return EXIT_SUCCESS;
}        

void itkXorOperator::setOutputMetadata(const dtkAbstractData * inputData, dtkAbstractData * outputData)
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

dtkAbstractData * itkXorOperator::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createitkXorOperator()
{
    return new itkXorOperator;
}
