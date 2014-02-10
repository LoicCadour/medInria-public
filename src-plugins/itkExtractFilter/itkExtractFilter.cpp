/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkExtractFilter.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include "itkImage.h"
#include <itkExtractImageFilter.h>
#include <medMetaDataKeys.h>

// /////////////////////////////////////////////////////////////////
// itkExtractFilterPrivate
// /////////////////////////////////////////////////////////////////

class itkExtractFilterPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> input;
    dtkSmartPointer <dtkAbstractData> output;
    int frameNumber;

    template <class PixelType> void update ( void )
    {
        typedef itk::Image <PixelType, 4> Image4DType;
        typedef itk::Image <PixelType, 3> InputImageType;
        typename Image4DType::Pointer image4d = dynamic_cast<Image4DType *>((itk::Object*)(input->data()));


        typename Image4DType::RegionType region = image4d->GetLargestPossibleRegion();
        typename Image4DType::SizeType size = image4d->GetLargestPossibleRegion().GetSize();

        typename itk::ExtractImageFilter<Image4DType, InputImageType>::Pointer extractFilter = itk::ExtractImageFilter<Image4DType, InputImageType>::New();
        typename Image4DType::IndexType index = {{0,0,0,0}};
        size[3] = 0;
        index[3] = frameNumber;
        region.SetSize(size);
        region.SetIndex(index);
        extractFilter->SetExtractionRegion(region);
        extractFilter->SetDirectionCollapseToGuess();
        extractFilter->SetInput(image4d);

        try 
        {
            extractFilter->Update();
        } 
        catch (itk::ExceptionObject& e) 
        {
            qDebug() << e.GetDescription();
            return;
        }
        output = dtkAbstractDataFactory::instance()->createSmartPointer ("itkDataImageShort3");
    
        output->setData(extractFilter->GetOutput());
        qDebug()<<"d->output->identifier() : "<<output->identifier()<<endl;
    

        QString newSeriesDescription = input->metadata ( medMetaDataKeys::SeriesDescription.key() );
        newSeriesDescription = newSeriesDescription +" frame ("+ QString::number(frameNumber)+ ")";
    
       output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    }

};

// /////////////////////////////////////////////////////////////////
// itkExtractFilter
// /////////////////////////////////////////////////////////////////

itkExtractFilter::itkExtractFilter() : dtkAbstractProcess(), d(new itkExtractFilterPrivate)
{
    d->frameNumber = 1;
}

itkExtractFilter::~itkExtractFilter()
{
    
}

bool itkExtractFilter::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("itkExtractFilter", createItkExtractFilter);
}

QString itkExtractFilter::description() const
{
    return "itkExtractFilter";
}

void itkExtractFilter::setInput ( dtkAbstractData *data )
{
    if ( !data )
        return;
    
    QString identifier = data->identifier();
    
    d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( identifier );
    
    d->input = data;
}    

void itkExtractFilter::setParameter ( int  data, int channel )
{
    if(channel!=0)
        return;
    d->frameNumber = data;
}

int itkExtractFilter::update()
{
    if ( !d->input )
        return -1;

    QString id = d->input->identifier();

    qDebug() << "itkExtractFilter, update : " << id;

    if ( id == "itkDataImageChar4" )
    {
        d->update<char>();
    }
    else if ( id == "itkDataImageUChar4" )
    {
        d->update<unsigned char>();
    }
    else if ( id == "itkDataImageShort4" )
    {
        d->update<short>();
    }
    else if ( id == "itkDataImageUShort4" )
    {
        d->update<unsigned short>();
    }
    else if ( id == "itkDataImageInt4" )
    {
        d->update<int>();
    }
    else if ( id == "itkDataImageUInt4" )
    {
        d->update<unsigned int>();
    }
    else if ( id == "itkDataImageLong4" )
    {
        d->update<long>();
    }
    else if ( id== "itkDataImageULong4" )
    {
        d->update<unsigned long>();
    }
    else if ( id == "itkDataImageFloat4" )
    {
        d->update<float>();
    }
    else if ( id == "itkDataImageDouble4" )
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

dtkAbstractData * itkExtractFilter::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createItkExtractFilter()
{
    return new itkExtractFilter;
}
