/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkFiltersDilateProcess.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medMetaDataKeys.h>

#include "itkFiltersDilateProcess_p.h"

//-------------------------------------------------------------------------------------------

itkFiltersDilateProcess::itkFiltersDilateProcess(itkFiltersDilateProcess *parent) 
    : itkFiltersProcessBase(*new itkFiltersDilateProcessPrivate(this), parent)
{
    DTK_D(itkFiltersDilateProcess);
    
    d->filter = this;
    d->output = NULL;
    d->radius[0] = 0;
    d->radius[1] = 0;
    d->radius[2] = 0;
    d->isRadiusInPixels = false;
    d->radiusInPixels = 0;
    d->description = tr("ITK Dilate filter");
}


itkFiltersDilateProcess::itkFiltersDilateProcess(const itkFiltersDilateProcess& other) 
    : itkFiltersProcessBase(*new itkFiltersDilateProcessPrivate(*other.d_func()), other)
{
}

//-------------------------------------------------------------------------------------------

itkFiltersDilateProcess::~itkFiltersDilateProcess( void )
{
}

//-------------------------------------------------------------------------------------------

bool itkFiltersDilateProcess::registered( void )
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("itkDilateProcess", createitkFiltersDilateProcess);
}

//-------------------------------------------------------------------------------------------

void itkFiltersDilateProcess::setParameter(int data, int channel)
{
    if (channel > 1)
        return;
    DTK_D(itkFiltersDilateProcess);
    d->radiusInPixels = data;
    if (channel == 1) // data is in pixels
    {
        d->radius[0] = data;
        d->radius[1] = data;
        d->radius[2] = data;
        d->isRadiusInPixels = true;
    }
    if (channel == 0) //data is in mm
    {
        itk::Image<unsigned char, 3> *image = dynamic_cast<itk::Image<unsigned char, 3> *> ( ( itk::Object* ) ( d->input->data() ) );
        double radius2[3];
        for (int i=0; i<image->GetSpacing().Size(); i++)
            radius2[i] = data/image->GetSpacing()[i];

        for (int i=0; i<image->GetSpacing().Size(); i++)
            d->radius[i] = floor((data/image->GetSpacing()[i])+0.5); //rounding

        qDebug()<<"RADIUS2   :  "<<radius2[0]<<" "<<radius2[1]<<" "<<radius2[2]<<endl;
        d->isRadiusInPixels = false;
    }
    qDebug()<<"RADIUS   :  "<<d->radius[0]<<" "<<d->radius[1]<<" "<<d->radius[2]<<endl;
}

//-------------------------------------------------------------------------------------------

int itkFiltersDilateProcess::update ( void )
{
    DTK_D(itkFiltersDilateProcess);
    
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
    QString unit;
    if (d->isRadiusInPixels)
        unit = "pixels";
    else
        unit = "mm";
    QString newSeriesDescription = d->input->metadata ( medMetaDataKeys::SeriesDescription.key() );
    newSeriesDescription += " Dilate filter ("+ QString::number(d->radiusInPixels) + unit + ")";

    d->output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );

    return EXIT_SUCCESS;
}


// /////////////////////////////////////////////////////////////////
// Type instanciation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess * createitkFiltersDilateProcess ( void )
{
    return new itkFiltersDilateProcess;
}
