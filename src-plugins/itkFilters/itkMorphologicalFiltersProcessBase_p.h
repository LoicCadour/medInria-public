/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <itkFiltersProcessBase.h>
#include <itkFiltersProcessBase_p.h>
#include <medAbstractImageData.h>
#include <medDataManager.h>
#include <medMetaDataKeys.h>
#include <dtkLog/dtkLog.h>

#include <itkFiltersPluginExport.h>

#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>
#include <itkFlatStructuringElement.h>
#include <itkKernelImageFilter.h>
#include <itkCommand.h>
#include <itkImage.h>
#include <itkMinimumMaximumImageFilter.h>
#include <itkProcessObject.h>

class itkMorphologicalFiltersProcessBase;

class ITKFILTERSPLUGIN_EXPORT itkMorphologicalFiltersProcessBasePrivate : public itkFiltersProcessBasePrivate
{
public:
    itkMorphologicalFiltersProcessBasePrivate(itkMorphologicalFiltersProcessBase *q = 0) : itkFiltersProcessBasePrivate(q) {}
    itkMorphologicalFiltersProcessBasePrivate(const itkMorphologicalFiltersProcessBasePrivate& other) : itkFiltersProcessBasePrivate(other) {}

    virtual ~itkMorphologicalFiltersProcessBasePrivate(void) {}

    double radius[3];
    double radiusMm[3];
    bool isRadiusInPixels;
	
    template <class ImageType> void convertMmInPixels ( void )
    {
        ImageType *image = dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) );
        for (unsigned int i=0; i<image->GetSpacing().Size(); i++)
        {
            radius[i] = floor((radius[i]/image->GetSpacing()[i])+0.5); //rounding
            radiusMm[i] = radius[i] * image->GetSpacing()[i];
        }
    }


    template <class PixelType> void update ( void )
    {
        typedef itk::Image< PixelType, 3 > ImageType;

        if(!isRadiusInPixels)
        {
            convertMmInPixels<ImageType>();
        }

        typedef itk::FlatStructuringElement < 3> StructuringElementType;
        StructuringElementType::RadiusType elementRadius;
        elementRadius[0] = radius[0]; //radius (double) is truncated
        elementRadius[1] = radius[1];
        elementRadius[2] = radius[2];
        StructuringElementType ball = StructuringElementType::Ball(elementRadius);

        typedef itk::MinimumMaximumImageFilter <ImageType> ImageCalculatorFilterType;
        typename ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New();
        imageCalculatorFilter->SetInput( dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) ) );
        imageCalculatorFilter->Update();

        typedef itk::KernelImageFilter< ImageType, ImageType, StructuringElementType >  FilterType;
        typename FilterType::Pointer filter;

        if(description == "Dilate filter")
        {
            typedef itk::BinaryDilateImageFilter< ImageType, ImageType,StructuringElementType >  DilateFilterType;
            filter = DilateFilterType::New();
            dynamic_cast<DilateFilterType *>(filter.GetPointer())->SetForegroundValue(imageCalculatorFilter->GetMaximum());
            dynamic_cast<DilateFilterType *>(filter.GetPointer())->SetBackgroundValue(imageCalculatorFilter->GetMinimum());
        }

        else if(description == "Erode filter")
        {
            typedef itk::BinaryErodeImageFilter< ImageType, ImageType,StructuringElementType >  ErodeFilterType;
            filter = ErodeFilterType::New();
            dynamic_cast<ErodeFilterType *>(filter.GetPointer())->SetForegroundValue(imageCalculatorFilter->GetMaximum());
            dynamic_cast<ErodeFilterType *>(filter.GetPointer())->SetBackgroundValue(imageCalculatorFilter->GetMinimum());
        }

        else if(description == "Close filter")
        {
            typedef itk::BinaryMorphologicalClosingImageFilter< ImageType, ImageType, StructuringElementType >  CloseFilterType;
            filter = CloseFilterType::New();
            dynamic_cast<CloseFilterType *>(filter.GetPointer())->SetForegroundValue(imageCalculatorFilter->GetMaximum());
        }

        else if(description == "Open filter")
        {
            typedef itk::BinaryMorphologicalOpeningImageFilter< ImageType, ImageType, StructuringElementType >  OpenFilterType;
            filter = OpenFilterType::New();
            dynamic_cast<OpenFilterType *>(filter.GetPointer())->SetForegroundValue(imageCalculatorFilter->GetMaximum());
            dynamic_cast<OpenFilterType *>(filter.GetPointer())->SetBackgroundValue(imageCalculatorFilter->GetMinimum());
        }

        else
        {
            qDebug()<<"Wrong morphological filter";
            return;
        }

        filter->SetInput ( dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) ) );
        filter->SetKernel ( ball );

        callback = itk::CStyleCommand::New();
        callback->SetClientData ( ( void * ) this );
        callback->SetCallback ( itkMorphologicalFiltersProcessBasePrivate::eventCallback );

        filter->AddObserver ( itk::ProgressEvent(), callback );

        filter->Update();
        output->setData ( filter->GetOutput() );

        // Add description on output data
        QString newSeriesDescription = input->metadata ( medMetaDataKeys::SeriesDescription.key() );

        if (isRadiusInPixels)
            newSeriesDescription += description + "\n("+ QString::number(floor(radius[0]))+", "+
            QString::number(floor(radius[1]))+", "+ QString::number(floor(radius[2]))+" pixels)";
        else
            newSeriesDescription += description + "\n("+ QString::number(radiusMm[0])+", "+
            QString::number(radiusMm[1])+", "+ QString::number(radiusMm[2])+" mm)";

        output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    }
};

DTK_IMPLEMENT_PRIVATE(itkMorphologicalFiltersProcessBase, itkFiltersProcessBase)


