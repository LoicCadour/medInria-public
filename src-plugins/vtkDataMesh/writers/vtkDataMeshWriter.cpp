/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <vtkDataMeshWriter.h>

#include <medAbstractDataFactory.h>
#include <medAbstractData.h>
#include <medMetaDataKeys.h>
#include <vtkErrorCode.h>
#include <dtkLog/dtkLog.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkMetaSurfaceMesh.h>
#include <vtkSmartPointer.h>



#include <vtkMetaDataSet.h>


const char vtkDataMeshWriter::ID[] = "vtkDataMeshWriter";

vtkDataMeshWriter::vtkDataMeshWriter() : medAbstractDataWriter()
{
}

vtkDataMeshWriter::~vtkDataMeshWriter()
{
}

QStringList vtkDataMeshWriter::handled() const
{
    return QStringList() << "vtkDataMesh";
}

QStringList vtkDataMeshWriter::s_handled()
{
    return QStringList() << "vtkDataMesh";
}

bool vtkDataMeshWriter::canWrite(const QString& path)
{
    return path.endsWith(".vtk");
}

bool vtkDataMeshWriter::write(const QString& path)
{   
	if(!getDataList().isEmpty())
    {
        qDebug()<<"not empty";
        // In case of multiple exports, we don't want to rewrite a same file on each loop
        QString baseFilePath;
        if (getDataList().size() > 1)
        {
            qDebug()<<"size > 1";
            baseFilePath = QFileInfo(path).absolutePath();
        }
        
        foreach(medAbstractData* dtkData, getDataList())
        {
            qDebug()<<"dans for each";
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
            qDebug()<<"completeFileName : "<<completeFileName;
			if ( dtkData && dtkData->identifier() == "vtkDataMesh")
            {
                vtkMetaSurfaceMesh * dataSet = dynamic_cast<vtkMetaSurfaceMesh*>( (vtkObject*)(dtkData->data()));
                if (dataSet)
                {
                    vtkPolyData * polyData = dataSet->GetPolyData();
                    // done this to not touch the actual data loaded
                    vtkSmartPointer<vtkPolyData> tempMesh = vtkPolyData::New();
                    tempMesh->DeepCopy(polyData);
                    vtkSmartPointer<vtkPolyDataWriter> writer = vtkPolyDataWriter::New();
                    writer->SetFileName(completeFileName.toUtf8().constData());
                    try
                    {
                        writer->SetInput (tempMesh);
                        writer->Write();
                    }
                    catch (vtkErrorCode::ErrorIds error)
                    {
                        qDebug() << "vtkDataMeshWriter: " << vtkErrorCode::GetStringFromErrorCode(error);
                        return false;
                    }
                }
            }
            else
            {
                qDebug() << "cartoVtkWriter: not a vtkMetaSurfaceMesh";
                return false;
            }
        }
    }
    else
    {
        qDebug() << "vtkDataMeshWriter: no data provided, or wrong type";
        return false;
    }
    
        

//   qDebug() << "Can write with: " << this->identifier();
// 
//   medAbstractData * medData = dynamic_cast<medAbstractData*>(this->data());
// 
//   if(medData->identifier() != "vtkDataMesh")
//   {
//     return false;
//   }
// 
//   vtkMetaDataSet * mesh = dynamic_cast< vtkMetaDataSet*>( (vtkObject*)(this->data()->data()));
//   if (!mesh)
//     return false;

  //mesh->Write(path.toLocal8Bit().constData());

  return true;
}

QString vtkDataMeshWriter::description() const
{
    return tr( "VTK Mesh Writer" );
}

QString vtkDataMeshWriter::identifier() const
{
    return ID;
}

QStringList vtkDataMeshWriter::supportedFileExtensions() const
{
    return QStringList() << ".vtk" << ".vtp";
}

bool vtkDataMeshWriter::registered()
{
  return medAbstractDataFactory::instance()->registerDataWriterType("vtkDataMeshWriter", vtkDataMeshWriter::s_handled(), createVtkDataMeshWriter);
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractDataWriter *createVtkDataMeshWriter()
{
  return new vtkDataMeshWriter;
}


