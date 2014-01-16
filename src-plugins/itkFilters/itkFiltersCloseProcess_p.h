/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkSmartPointer.h>

#include "itkFiltersProcessBase_p.h"

#include <medMetaDataKeys.h>

#include "itkImage.h"
#include "itkCommand.h"
#include <itkGrayscaleMorphologicalClosingImageFilter.h>
#include "itkFlatStructuringElement.h"

class itkFiltersCloseProcess;

class itkFiltersCloseProcessPrivate : public itkFiltersProcessBasePrivate
{
public:
    itkFiltersCloseProcessPrivate(itkFiltersCloseProcess *q = 0) : itkFiltersProcessBasePrivate(q) {}
    itkFiltersCloseProcessPrivate(const itkFiltersCloseProcessPrivate& other) : itkFiltersProcessBasePrivate(other) {}

    virtual ~itkFiltersCloseProcessPrivate(void) {}
    
    int radius[3];
    double radiusMm[3];
    bool isRadiusInPixels;
    int radiusInPixels;

    template <class PixelType> void update ( void )
    {
        typedef itk::Image< PixelType, 3 > ImageType;
        if(!isRadiusInPixels)
            convertMmInPixels<ImageType>();

        typedef itk::FlatStructuringElement < 3> StructuringElementType;
        StructuringElementType::RadiusType elementRadius;
        elementRadius[0] = radius[0];
        elementRadius[1] = radius[1];
        elementRadius[2] = radius[2];

        typedef itk::GrayscaleMorphologicalClosingImageFilter< ImageType, ImageType, StructuringElementType >  CloseType;
        typename CloseType::Pointer closeFilter = CloseType::New();

        StructuringElementType ball = StructuringElementType::Ball(elementRadius);

        closeFilter->SetInput ( dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) ) );
        closeFilter->SetKernel ( ball );
        
        callback = itk::CStyleCommand::New();
        callback->SetClientData ( ( void * ) this );
        callback->SetCallback ( itkFiltersProcessBasePrivate::eventCallback );
        
        closeFilter->AddObserver ( itk::ProgressEvent(), callback );
        
        closeFilter->Update();
        output->setData ( closeFilter->GetOutput() );
        
        QString newSeriesDescription = input->metadata ( medMetaDataKeys::SeriesDescription.key() );
        
        if (isRadiusInPixels)
            newSeriesDescription += " Close filter\n("+ QString::number(radius[0])+", "+ 
            QString::number(radius[1])+", "+ QString::number(radius[2])+" pixels)";
        else
            newSeriesDescription += " Close filter\n("+ QString::number(radiusMm[0])+", "+ 
            QString::number(radiusMm[1])+", "+ QString::number(radiusMm[2])+" mm)";
        
        output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    }

    template <class ImageType> void convertMmInPixels ( void )
    {
        ImageType *image = dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) );
        for (int i=0; i<image->GetSpacing().Size(); i++)
        {
            radius[i] = floor((radius[i]/image->GetSpacing()[i])+0.5); //rounding
            radiusMm[i] = radius[i] * image->GetSpacing()[i];
        }
    }
};

DTK_IMPLEMENT_PRIVATE(itkFiltersCloseProcess, itkFiltersProcessBase)
