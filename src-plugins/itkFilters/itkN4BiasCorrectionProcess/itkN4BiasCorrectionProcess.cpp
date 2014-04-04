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
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

// /////////////////////////////////////////////////////////////////
// itkN4BiasCorrectionProcessPrivate
// /////////////////////////////////////////////////////////////////

class itkN4BiasCorrectionProcessPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> input;
    dtkSmartPointer <dtkAbstractData> output;
};

// /////////////////////////////////////////////////////////////////
// itkN4BiasCorrectionProcess
// /////////////////////////////////////////////////////////////////

itkN4BiasCorrectionProcess::itkN4BiasCorrectionProcess() : dtkAbstractProcess(), d(new itkN4BiasCorrectionProcessPrivate)
{
    
}

itkN4BiasCorrectionProcess::~itkN4BiasCorrectionProcess()
{
    
}

bool itkN4BiasCorrectionProcess::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("itkN4BiasCorrectionProcess", createItkN4BiasCorrectionProcess);
}

QString itkN4BiasCorrectionProcess::description() const
{
    return "itkN4BiasCorrectionProcess";
}

void itkN4BiasCorrectionProcess::setInput ( dtkAbstractData *data )
{
    if ( !data )
        return;
    
    QString identifier = data->identifier();
    
    d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( identifier );
    
    d->input = data;
}    

void itkN4BiasCorrectionProcess::setParameter ( double  data, int channel )
{
    // Here comes a switch over channel to handle parameters
}

int itkN4BiasCorrectionProcess::update()
{
    if ( !d->input )
        return -1;
    
    // Your update code comes in here
    
    return EXIT_SUCCESS;
}        

dtkAbstractData * itkN4BiasCorrectionProcess::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createItkN4BiasCorrectionProcess()
{
    return new itkN4BiasCorrectionProcess;
}
