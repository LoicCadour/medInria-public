#include <cartoVtkWriter.h>
#include "cartoPatientInfoDialog.h"
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractData.h>
#include <medAbstractDataFactory.h>
#include <medDataManager.h>
#include <medMetaDataKeys.h>
#include <viewerFilesToSaveDialog.h>

#include <vtkMetaSurfaceMesh.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkProperty.h>
#include <vtkErrorCode.h>

#include <vtkLookupTableManager.h>

// /////////////////////////////////////////////////////////////////
// cartoVtkWriter
// /////////////////////////////////////////////////////////////////

cartoVtkWriter::cartoVtkWriter() : dtkAbstractDataWriter()
{
}


cartoVtkWriter::~cartoVtkWriter()
{
}


QString cartoVtkWriter::identifier() const
{
    return QString("cartoVtkWriter");
}


QString cartoVtkWriter::description() const
{
    return QString("BioSense CARTO .vtk mesh exporter");
}


QStringList cartoVtkWriter::handled() const
{
    return QStringList() << "vtkDataMesh";
}


QStringList cartoVtkWriter::supportedFileExtensions() const
{
    return QStringList() << ".vtk";
}


bool cartoVtkWriter::registered()
{
    return medAbstractDataFactory::instance()->registerDataWriterType("cartoVtkWriter", QStringList() << "vtkDataMesh", create);
}


bool cartoVtkWriter::canWrite(const QString & path)
{
    return path.endsWith(".vtk");
}


bool cartoVtkWriter::write(const QString & path)
{
    dtkAbstractData * dtkData = this->data();

    if ( ! dtkData || dtkData->identifier() != "vtkDataMesh") {
        qDebug() << "cartoVtkWriter: No data provided, or wrong type";
        return false;
    }

    vtkMetaSurfaceMesh * dataSet = dynamic_cast<vtkMetaSurfaceMesh*>( (vtkObject*)(dtkData->data()));
    if ( ! dataSet) {
        qDebug() << "cartoVtkWriter: not a vtkMetaSurfaceMesh";
        return false;
    }

    QString patientName = dtkData->metadata(medMetaDataKeys::PatientName.key());
    QString patientID = dtkData->metaDataValues(medMetaDataKeys::PatientID.key())[0];

    vtkPolyData * polyData = dataSet->GetPolyData();

    // done this to not touch the actual data loaded
    vtkSmartPointer<vtkPolyData> tempMesh = vtkPolyData::New();
    tempMesh->DeepCopy(polyData);

    // done this to create a LUT and save the changes made in the application, in the written vtk file
    if(!(dataSet->GetCurrentScalarArray()))
    {
        for(int i=0;i<tempMesh->GetPointData()->GetNumberOfArrays();i++)
            tempMesh->GetPointData()->RemoveArray(tempMesh->GetPointData()->GetArrayName(i));

        for(int i=0;i<tempMesh->GetCellData()->GetNumberOfArrays();i++)
            tempMesh->GetCellData()->RemoveArray(tempMesh->GetCellData()->GetArrayName(i));

        vtkDoubleArray* scalarArray=vtkDoubleArray::New();
        for(int i=0;i<tempMesh->GetNumberOfPoints();i++)
            scalarArray->InsertValue(i, 1.0);
        tempMesh->GetPointData()->SetScalars(scalarArray);

        double rgb[3];//,alpha;
        vtkProperty::SafeDownCast(dataSet->GetProperty())->GetColor(rgb);
    }
    else
    {
        tempMesh->GetPointData()->GetScalars()->SetLookupTable(polyData->GetPointData()->GetScalars()->GetLookupTable());

        for(int i=0;i<tempMesh->GetPointData()->GetNumberOfArrays();i++)
            if(((std::string)(tempMesh->GetPointData()->GetArrayName(i))) != ((std::string)(dataSet->GetCurrentScalarArray()->GetName())))
                tempMesh->GetPointData()->RemoveArray(tempMesh->GetPointData()->GetArrayName(i));

        for(int i=0;i<tempMesh->GetCellData()->GetNumberOfArrays();i++)
            if(((std::string)(tempMesh->GetPointData()->GetArrayName(i))) != ((std::string)(dataSet->GetCurrentScalarArray()->GetName())))
                tempMesh->GetPointData()->RemoveArray(tempMesh->GetPointData()->GetArrayName(i));

        // done this to rescale the scalar values to LUT range, required by CARTO Machine
        rescaleScalarsToLUTRange(tempMesh);

    }

    vtkSmartPointer<vtkPolyDataWriter> writer = vtkPolyDataWriter::New();
    writer->SetFileName(path.toUtf8().constData());

    try
    {
        writer->SetInput (tempMesh);

        QString header = QString("PatientData %1 %2").arg(patientName, patientID);
        writer->SetHeader(header.toAscii().constData());
        writer->Write();
    }
    catch (vtkErrorCode::ErrorIds error)
    {
        qDebug() << "cartoVtkWriter: " << vtkErrorCode::GetStringFromErrorCode(error);
        return false;
    }

    return true;
}

void cartoVtkWriter::rescaleScalarsToLUTRange(vtkPolyData *mesh)
{
    if(!(mesh->GetPointData()->GetScalars()))
        return;

    if(!(mesh->GetPointData()->GetScalars())->GetLookupTable())
        return;

    double lutRange[2];
    mesh->GetPointData()->GetScalars()->GetLookupTable()->GetTableRange(lutRange);

    vtkDoubleArray *scalarArray = vtkDoubleArray::New();
    scalarArray->DeepCopy(mesh->GetPointData()->GetScalars());
    scalarArray->SetLookupTable(mesh->GetPointData()->GetScalars()->GetLookupTable());

    // check if scalar array has values
    if(scalarArray)
    {
        for(int i=0;i<scalarArray->GetSize();i++)
        {
            if(scalarArray->GetValue(i)<lutRange[0])
                scalarArray->SetValue(i,lutRange[0]);
            if(scalarArray->GetValue(i)>lutRange[1])
                scalarArray->SetValue(i,lutRange[1]);

            scalarArray->SetValue(i, ((scalarArray->GetValue(i)) - lutRange[0])/(lutRange[1]-lutRange[0]));
        }

        // to do check what happens to mesh->GetPointData()->GetScalars()
        mesh->GetPointData()->SetScalars(scalarArray);
    }
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractDataWriter * cartoVtkWriter::create()
{
    return new cartoVtkWriter;
}
