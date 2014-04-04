/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkN4BiasCorrectionProcess.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medMetaDataKeys.h>

#include "itkN4BiasCorrectionProcess_p.h"

//-------------------------------------------------------------------------------------------

itkN4BiasCorrectionProcess::itkN4BiasCorrectionProcess(itkN4BiasCorrectionProcess *parent) 
    : itkFiltersProcessBase(*new itkN4BiasCorrectionProcessPrivate(this), parent)
{
    DTK_D(itkN4BiasCorrectionProcess);
    
    d->filter = this;
    d->output = NULL;
    d->description = tr("N4 Bias Correction");
    d->saveBias = false;
}


itkN4BiasCorrectionProcess::itkN4BiasCorrectionProcess(const itkN4BiasCorrectionProcess& other) 
    : itkFiltersProcessBase(*new itkN4BiasCorrectionProcessPrivate(*other.d_func()), other)
{
}

//-------------------------------------------------------------------------------------------

itkN4BiasCorrectionProcess::~itkN4BiasCorrectionProcess( void )
{
}

//-------------------------------------------------------------------------------------------

bool itkN4BiasCorrectionProcess::registered( void )
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("itkN4BiasCorrectionProcess", createitkN4BiasCorrectionProcess);
}

//-------------------------------------------------------------------------------------------

void itkN4BiasCorrectionProcess::setParameter(double data, int channel)
{
   
    DTK_D(itkN4BiasCorrectionProcess);

    if (channel == 0)
        d->splineDistance = data;
    if (channel == 1)
        d->bfFWHM = (float)data;
    if (channel == 2)
        d->convergenceThreshold = data;
    if (channel == 3)
        d->bsplineOrder = data;
    if (channel == 4)
        d->shrinkFactor = data;
    if (channel == 5)
        d->nbHistogramBins = data;
    if (channel == 6)
        d->wienerFilterNoise = data;
    if (channel == 7)
        d->saveBias = data;
    if (channel>=10)
        d->numberOfIterations.push_back(data);
}

void itkN4BiasCorrectionProcess::setParameter(std::vector<int> data, int channel)
{

}

//-------------------------------------------------------------------------------------------

int itkN4BiasCorrectionProcess::update ( void )
{
    DTK_D(itkN4BiasCorrectionProcess);
    
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


// /////////////////////////////////////////////////////////////////
// Type instanciation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess * createitkN4BiasCorrectionProcess ( void )
{
    return new itkN4BiasCorrectionProcess;
}
