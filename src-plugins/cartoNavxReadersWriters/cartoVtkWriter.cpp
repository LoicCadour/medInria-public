#include <cartoPatientInfoDialog.h>
#include <cartoVtkWriter.h>

#include <medAbstractDataFactory.h>
#include <medMetaDataKeys.h>

#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkErrorCode.h>
#include <vtkLookupTableManager.h>
#include <vtkMetaSurfaceMesh.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

// /////////////////////////////////////////////////////////////////
// cartoVtkWriter
// /////////////////////////////////////////////////////////////////

cartoVtkWriter::cartoVtkWriter() : medAbstractDataWriter()
{
    patientFirstName = "John";
    patientLastName = "Doe";
    patientID = "06111989";

    ptAttributes << "First name";
    ptAttributes << "Last name";
    ptAttributes << "Patient ID";

    ptValues << patientFirstName;
    ptValues << patientLastName;
    ptValues << patientID;

    isDialogOK = false;
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
    if(!getDataList().isEmpty())
    {
        // In case of multiple exports, we don't want to rewrite a same file on each loop
        QString baseFilePath;
        if (getDataList().size() > 1)
        {
            baseFilePath = QFileInfo(path).absolutePath();
        }

        // Display a pop-up window asking for patient lastname, firstname and identifier
        this->moveToThread(qApp->thread());
        connect(this, SIGNAL(needMoreParameters()),
                this, SLOT(showPatientInfoDialog()), Qt::BlockingQueuedConnection);
        emit needMoreParameters();

        foreach(medAbstractData* dtkData, getDataList())
        {
            QString completeFileName;
            if (getDataList().size() > 1) // multiple exports in a directory
            {
                // Put this file description, and finish with ".vtk"
                QString currentDescription = dtkData->metadata(medMetaDataKeys::SeriesDescription.key());
                completeFileName = baseFilePath + "/" + currentDescription + ".vtk";
            }
            else
            {
                completeFileName = path;
            }

            if ( dtkData && dtkData->identifier() == "vtkDataMesh")
            {
                vtkMetaSurfaceMesh * dataSet = dynamic_cast<vtkMetaSurfaceMesh*>( (vtkObject*)(dtkData->data()));
                if (dataSet)
                {
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

                        double rgb[3];
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
                    writer->SetFileName(completeFileName.toUtf8().constData());

                    try
                    {
                        writer->SetInput (tempMesh);

                        if (isDialogOK)
                        {
                            QString header = QString("PatientData %1 %2 %3").arg(patientFirstName, patientLastName, patientID);
                            writer->SetHeader(header.toAscii().constData());
                            writer->Write();
                        }
                    }
                    catch (vtkErrorCode::ErrorIds error)
                    {
                        qDebug() << "cartoVtkWriter: " << vtkErrorCode::GetStringFromErrorCode(error);
                        return false;
                    }
                }
                else
                {
                    qDebug() << "cartoVtkWriter: not a vtkMetaSurfaceMesh";
                    return false;
                }
            }
            else
            {
                qDebug() << "cartoVtkWriter: no data provided, or wrong type";
                return false;
            }
        }
    }

    return true;
}

void cartoVtkWriter::showPatientInfoDialog()
{
    cartoPatientInfoDialog editDialog(ptAttributes, ptValues, NULL);

    if(editDialog.exec() == QDialog::Accepted)
    {
        patientFirstName = editDialog.value("First name");
        patientLastName = editDialog.value("Last name");
        patientID = editDialog.value("Patient ID");

        isDialogOK = true;
    }
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
