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
{qDebug()<<"APPLYING MASK !!   IN id :"<<d->input->identifier()<<endl;
    if ( !d->input ||!d->input->data() || !d->mask ||!d->mask->data())
        return -1;
    ImageType::Pointer mask = ImageType::New();
    //typedef itk::Image<unsigned char, 3>  MaskType;
    typedef itk::MaskImageFilter< ImageType,  MaskType> MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    

    ImageType::Pointer image1 = dynamic_cast<ImageType*>(static_cast<itk::Object*>(d->input->data()));
    if (image1.IsNull())
        qDebug()<<"image1 is null"<<endl;
    MaskType::Pointer image2 = dynamic_cast<MaskType*>(static_cast<itk::Object*>(d->mask->data()));
    if (image2.IsNull())
        qDebug()<<"image2 is null"<<endl;
    maskFilter->SetInput(dynamic_cast<ImageType *> ( ( itk::Object* ) ( d->input->data() ) ));
    maskFilter->SetMaskImage(dynamic_cast<MaskType *> ( ( itk::Object* ) ( d->mask->data() ) ));
    //d->output =dynamic_cast<medAbstractDataImage *>(dtkAbstractDataFactory::instance()->create ("itkDataImageUShort3"));
    maskFilter->Update();
    if(maskFilter.IsNull())
        qDebug()<<"maskFilter est nul "<<endl;
    d->output->setData(maskFilter->GetOutput());
    QString newSeriesDescription = d->input->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += " mask application ()";
        
    d->output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    medDataManager::instance()->importNonPersistent(d->output);
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
