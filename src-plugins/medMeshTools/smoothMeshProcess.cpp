/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "smoothMeshProcess.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcess.h>

#include <itkImage.h>
#include <vtkMetaDataSet.h>
#include <vtkMetaSurfaceMesh.h>

#include <vtkSmartPointer.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkAlgorithmOutput.h>

#include <medDataManager.h>
#include <medMetaDataKeys.h>



// /////////////////////////////////////////////////////////////////
// smoothMeshProcessPrivate
// /////////////////////////////////////////////////////////////////

class smoothMeshProcessPrivate
{
public:
    dtkSmartPointer <dtkAbstractData> input;
    dtkSmartPointer <dtkAbstractData> output;
    int iterations;
    double relaxationFactor;
};



// /////////////////////////////////////////////////////////////////
// smoothMeshProcess
// /////////////////////////////////////////////////////////////////

smoothMeshProcess::smoothMeshProcess() : dtkAbstractProcess(), d(new smoothMeshProcessPrivate)
{
    
}

smoothMeshProcess::~smoothMeshProcess()
{
    
}

bool smoothMeshProcess::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("smoothMeshProcess", createSmoothMeshProcess);
}

QString smoothMeshProcess::description() const
{
    return "smoothMeshProcess";
}

void smoothMeshProcess::setInput ( dtkAbstractData *data )
{
    if ( !data )
        return;
    d->output = dtkAbstractDataFactory::instance()->createSmartPointer ( "vtkDataMesh" );
    
    d->input = data;
}    

void smoothMeshProcess::setParameter ( double data, int channel )
{
    if (channel>1)
        return;

    switch (channel) {
        case 0:
            d->iterations = data;
            break;
        case 1:
            d->relaxationFactor = data;
            break;
    }
}

int smoothMeshProcess::update()
{
    if(!d->input->identifier().contains("vtkDataMesh"))
        return 0;
    vtkMetaDataSet * _dataset = static_cast<vtkMetaDataSet*>(d->input->data());
    vtkPolyData * polyData = static_cast<vtkPolyData*>(_dataset->GetDataSet());

    if (!polyData)
        return 0;

    vtkSmoothPolyDataFilter* contourSmoothed = 0;
    contourSmoothed = vtkSmoothPolyDataFilter::New();
    contourSmoothed->SetInput(polyData);
    contourSmoothed->SetNumberOfIterations(d->iterations);
    contourSmoothed->SetRelaxationFactor(d->relaxationFactor);
    contourSmoothed->Update();

    vtkMetaSurfaceMesh * smesh = vtkMetaSurfaceMesh::New();
    smesh->SetDataSet(contourSmoothed->GetOutput());

    contourSmoothed->Delete();

    d->output->setData(smesh);
    return EXIT_SUCCESS;
}



dtkAbstractData * smoothMeshProcess::output()
{
    return ( d->output );
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createSmoothMeshProcess()
{
    return new smoothMeshProcess;
}
