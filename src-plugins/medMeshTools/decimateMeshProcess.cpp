/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "decimateMeshProcess.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include <itkImage.h>
#include <vtkMetaDataSet.h>
#include <vtkMetaSurfaceMesh.h>

#include <vtkSmartPointer.h>
#include <vtkDecimatePro.h>
#include <vtkAlgorithmOutput.h>

#include <medDataManager.h>
#include <medMetaDataKeys.h>



// /////////////////////////////////////////////////////////////////
// decimateMeshProcessPrivate
// /////////////////////////////////////////////////////////////////

class decimateMeshProcessPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> input;
    dtkSmartPointer <dtkAbstractData> output;
    double targetReduction;
};



// /////////////////////////////////////////////////////////////////
// decimateMeshProcess
// /////////////////////////////////////////////////////////////////

decimateMeshProcess::decimateMeshProcess() : dtkAbstractProcess(), d(new decimateMeshProcessPrivate)
{
    
}

decimateMeshProcess::~decimateMeshProcess()
{
    
}

bool decimateMeshProcess::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("decimateMeshProcess", createDecimateMeshProcess);
}

QString decimateMeshProcess::description() const
{
    return "decimateMeshProcess";
}

void decimateMeshProcess::setInput ( dtkAbstractData *data )
{
    if ( !data )
        return;
    d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( "vtkDataMesh" );
    
    d->input = data;
}    

void decimateMeshProcess::setParameter ( double data, int channel )
{
    if (channel!=0)
        return;
    else
        d->targetReduction = data;
}

int decimateMeshProcess::update()
{
    if(!d->input->identifier().contains("vtkDataMesh"))
        return 0;
    vtkMetaDataSet * _dataset = static_cast<vtkMetaDataSet*>(d->input->data());
    vtkPolyData * polyData = static_cast<vtkPolyData*>(_dataset->GetDataSet());

    if (!polyData)
        return 0;

    vtkDecimatePro* contourDecimated = 0;
    contourDecimated = vtkDecimatePro::New();
    contourDecimated->SetInput(polyData);
    contourDecimated->SetTargetReduction(d->targetReduction);
    contourDecimated->SplittingOff();
    contourDecimated->PreserveTopologyOn();
    contourDecimated->Update();

    vtkMetaSurfaceMesh * smesh = vtkMetaSurfaceMesh::New();
    smesh->SetDataSet(contourDecimated->GetOutput());

    contourDecimated->Delete();

    d->output->setData(smesh);
    return EXIT_SUCCESS;
}


dtkAbstractData * decimateMeshProcess::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createDecimateMeshProcess()
{
    return new decimateMeshProcess;
}
