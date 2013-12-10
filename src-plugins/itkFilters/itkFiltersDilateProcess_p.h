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
#include "itkGrayscaleDilateImageFilter.h"
#include "itkFlatStructuringElement.h"

class itkFiltersDilateProcess;

class itkFiltersDilateProcessPrivate : public itkFiltersProcessBasePrivate
{
public:
    itkFiltersDilateProcessPrivate(itkFiltersDilateProcess *q = 0) : itkFiltersProcessBasePrivate(q) {}
    itkFiltersDilateProcessPrivate(const itkFiltersDilateProcessPrivate& other) : itkFiltersProcessBasePrivate(other) {}

    virtual ~itkFiltersDilateProcessPrivate(void) {}
    
    int radius[3];
    double radiusMm[3];
    bool isRadiusInPixels;
    int radiusInPixels;

    template <class PixelType> void update ( void )
    {
        typedef itk::Image< PixelType, 3 > ImageType;

        typedef itk::FlatStructuringElement < 3> StructuringElementType;
        StructuringElementType::RadiusType elementRadius;
        //elementRadius.Fill(radius[0]);
        elementRadius[0] = radius[0];
        elementRadius[1] = radius[1];
        elementRadius[2] = radius[2];

        typedef itk::GrayscaleDilateImageFilter< ImageType, ImageType,StructuringElementType >  DilateType;
        typename DilateType::Pointer dilateFilter = DilateType::New();
        
        StructuringElementType ball = StructuringElementType::Ball(elementRadius);
        //ball.CreateStructuringElement();

        dilateFilter->SetInput ( dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) ) );
        dilateFilter->SetKernel ( ball );
        
        callback = itk::CStyleCommand::New();
        callback->SetClientData ( ( void * ) this );
        callback->SetCallback ( itkFiltersProcessBasePrivate::eventCallback );
        
        dilateFilter->AddObserver ( itk::ProgressEvent(), callback );
        
        dilateFilter->Update();
        output->setData ( dilateFilter->GetOutput() );
    }
};

DTK_IMPLEMENT_PRIVATE(itkFiltersDilateProcess, itkFiltersProcessBase)
