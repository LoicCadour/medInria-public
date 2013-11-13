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

#include <medMetaDataKeys.h>
#include <medAbstractDataImage.h>
#include <medDataManager.h>

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

    template <class PixelType> int update()
    {
        typedef itk::Image<PixelType, 3> ImageType;
        if ( !input ||!input->data() || !mask ||!mask->data())
            return -1;
        //ImageType::Pointer mask = ImageType::New();
        typedef itk::MaskImageFilter< ImageType,  MaskType> MaskFilterType;
        MaskFilterType::Pointer maskFilter = MaskFilterType::New();

        maskFilter->SetInput(dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) ));
        maskFilter->SetMaskImage(dynamic_cast<MaskType *> ( ( itk::Object* ) ( mask->data() ) ));

        //Outside values set to the lowest reachable value
        maskFilter->SetOutsideValue(std::numeric_limits<PixelType>::min());
        maskFilter->Update();

        output->setData(maskFilter->GetOutput());
        QString newSeriesDescription = input->metadata ( medMetaDataKeys::SeriesDescription.key() );
        newSeriesDescription += " with mask :" + mask->metadata ( medMetaDataKeys::SeriesDescription.key() );

        output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
        medDataManager::instance()->importNonPersistent(output);
        return EXIT_SUCCESS;
    }
};

// /////////////////////////////////////////////////////////////////
// medMaskApplication
// /////////////////////////////////////////////////////////////////

medMaskApplication::medMaskApplication() : dtkAbstractProcess(), d(new medMaskApplicationPrivate)
{
    d->input = NULL;
    d->mask = NULL;
    d->output = NULL;
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

void medMaskApplication::setInput ( dtkAbstractData *data, int channel)
{
    if ( !data )
        return;
    if ( channel == 0)
    {
        QString identifier = data->identifier();
        d->mask = data;
        qDebug()<<"MASK SET !!"<<endl;
    }

    if ( channel == 1 )
    {
        QString identifier = data->identifier();qDebug()<<"identifier : "<<identifier<<endl;
        d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( identifier );
        d->input = data;
        qDebug()<<"INPUT SET !!IN id :"<<d->input->identifier()<<endl;
    }
}


void medMaskApplication::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int medMaskApplication::update()
{
    if ( !d->input )
        return -1;

    QString id = d->input->identifier();

    qDebug() << "itkFilters, update : " << id;

    if ( id == "itkDataImageChar3" )
    {
        d->update<char>();
    }
    else if ( id == "itkDataImageUChar3" )
    {
        d->update<unsigned char>();
    }
    else if ( id == "itkDataImageShort3" )
    {
        d->update<short>();
    }
    else if ( id == "itkDataImageUShort3" )
    {
        d->update<unsigned short>();
    }
    else if ( id == "itkDataImageInt3" )
    {
        d->update<int>();
    }
    else if ( id == "itkDataImageUInt3" )
    {
        d->update<unsigned int>();
    }
    else if ( id == "itkDataImageLong3" )
    {
        d->update<long>();
    }
    else if ( id== "itkDataImageULong3" )
    {
        d->update<unsigned long>();
    }
    else if ( id == "itkDataImageFloat3" )
    {
        d->update<float>();
    }
    else if ( id == "itkDataImageDouble3" )
    {
        d->update<double>();
    }
    else
    {
        qDebug() << "Error : pixel type not yet implemented ("
        << id
        << ")";
        return -1;
    }

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
