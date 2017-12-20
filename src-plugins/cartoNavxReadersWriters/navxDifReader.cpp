#include <navxDifReader.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractData.h>
#include <medAbstractDataFactory.h>

#include <vtkMetaSurfaceMesh.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkErrorCode.h>
#include <vtkCellArray.h>
#include <vtkActor.h>
#include <vtkProperty.h>

#include <QXmlStreamReader>

// /////////////////////////////////////////////////////////////////
// navxDifReader
// /////////////////////////////////////////////////////////////////

navxDifReader::navxDifReader() : dtkAbstractDataReader()
{
}


navxDifReader::~navxDifReader()
{
}


QString navxDifReader::identifier() const
{
    return QString("navxDifReader");
}


QString navxDifReader::description() const
{
    return QString("Saint-Jude NavX .dif mesh importer");
}


QStringList navxDifReader::handled() const
{
    return QStringList() << "vtkDataMesh";
}


QStringList navxDifReader::supportedFileExtensions() const
{
    return QStringList() << ".dif";
}


bool navxDifReader::registered()
{
    return medAbstractDataFactory::instance()->registerDataReaderType("navxDifReader", QStringList() << "vtkDataMesh", create);
}


bool navxDifReader::canRead(const QString & path)
{
    return path.endsWith(".dif");
}


bool navxDifReader::read(const QString & path)
{
//    dtkAbstractData * dtkData = this->data();

//    if ( ! dtkData || dtkData->identifier() != "vtkDataMesh") {
//        qDebug() << "navxDifReader: No data provided, or wrong type";
//        return false;
//    }

//    vtkMetaSurfaceMesh * dataSet = dynamic_cast<vtkMetaSurfaceMesh*>( (vtkObject*)(dtkData->data()));
//    if ( ! dataSet) {
//        qDebug() << "navxDifReader: not a vtkMetaSurfaceMesh";
//        return false;
//    }

//    vtkPolyData * polyData = dataSet->GetPolyData();

//    QList<vtkMetaDataSet*> datasetList;
//    datasetList.push_back(dataSet); //TODO = this->Input->GetMetaDataSetList();


//    QFile difFile(path);
//    if ( ! difFile.open(QIODevice::WriteOnly) ) {
//        qDebug() << "navxDifReader: Could not open file" << path;
//        return false;
//    }

//    QXmlStreamWriter xmlWriter(&difFile);

//    xmlWriter.setAutoFormatting(true);

//    xmlWriter.writeStartDocument();
//    xmlWriter.writeStartElement("DIF");

//    xmlWriter.writeStartElement("DIFHeader");
//    xmlWriter.writeTextElement("Version", "SJM_DIF_3.0");
//    xmlWriter.writeTextElement("VendorVersion", "NA");
//    xmlWriter.writeTextElement("PatientName", "NA");
//    xmlWriter.writeTextElement("PatientID", "NA");
//    xmlWriter.writeEmptyElement("PatientBirthDate");
//    xmlWriter.writeTextElement("PatientGender", "NA");
//    xmlWriter.writeEmptyElement("StudyID");
//    xmlWriter.writeEmptyElement("SeriesNumber");
//    xmlWriter.writeEmptyElement("StudyDate");
//    xmlWriter.writeEmptyElement("StudyTime");
//    xmlWriter.writeEmptyElement("SeriesTime");
//    xmlWriter.writeEmptyElement("Modality");
//    xmlWriter.writeEmptyElement("RefPhysName");
//    xmlWriter.writeEmptyElement("StudyDesc");
//    xmlWriter.writeEmptyElement("SeriesDesc");
//    xmlWriter.writeEmptyElement("OperatorName");
//    xmlWriter.writeTextElement("OperatorComment", "NA");
//    xmlWriter.writeEmptyElement("SegmentationDate");
//    xmlWriter.writeTextElement("Inventor", "1");
//    xmlWriter.writeEndElement(); // DIFHeader

//    xmlWriter.writeStartElement("DIFBody");
//    xmlWriter.writeStartElement("Volumes");
//    xmlWriter.writeAttribute("number", QString::number(datasetList.size()));

//    vtkMetaSurfaceMesh * metaSurface = 0;
//    vtkSmartPointer<vtkIdList> pointIds = vtkIdList::New();
//    double point[3];
//    double * color;
//    int final_colors[3];

//    for (unsigned int j = 0; j<datasetList.size(); ++j)
//    {
//        vtkMetaDataSet* metadata = datasetList.at(j);

//        if (metadata->GetType() != vtkMetaDataSet::VTK_META_SURFACE_MESH)
//            continue;

//        metaSurface = vtkMetaSurfaceMesh::SafeDownCast(metadata);

//        xmlWriter.writeStartElement("Volume");
//        xmlWriter.writeAttribute("name", QString(metadata->GetName()));

//        QString colorHex;
//        vtkActor * dataSetActor = metadata->GetActor(0);
//        if (dataSetActor) {
//            color = dataSetActor->GetProperty()->GetColor();
//            final_colors[0] = int(color[0]*255);
//            final_colors[1] = int(color[1]*255);
//            final_colors[2] = int(color[2]*255);

//            colorHex = QString::number(final_colors[0], 16) +
//                               QString::number(final_colors[1], 16) +
//                               QString::number(final_colors[2], 16);
//        } else {
//            colorHex = "FFFFFF";
//        }

//        xmlWriter.writeAttribute("color", colorHex);

//        xmlWriter.writeStartElement("Vertices");
//        xmlWriter.writeAttribute("number", QString::number(metaSurface->GetPolyData()->GetNumberOfPoints()));
//        for(vtkIdType i = 0; i < metaSurface->GetPolyData()->GetNumberOfPoints(); ++i)
//        {
//            metaSurface->GetPolyData()->GetPoint(i, point);
//            xmlWriter.writeCharacters(QString::number(point[0]) + " " +
//                                      QString::number(point[1]) + " " +
//                                      QString::number(point[2]) + "\n");
//        }
//        xmlWriter.writeEndElement(); // Vertices

//        xmlWriter.writeStartElement("Polygons");
//        xmlWriter.writeAttribute("number", QString::number(metaSurface->GetPolyData()->GetPolys()->GetNumberOfCells()));
//        for(unsigned int i = 0; i<metaSurface->GetPolyData()->GetPolys()->GetNumberOfCells(); ++i)
//        {
//            metaSurface->GetPolyData()->GetCellPoints(i, pointIds);
//            if (pointIds->GetNumberOfIds() != 3)
//            {
//                qDebug() << "navxVtkWriter: wrong type of cells !";
//                return false;
//            }
//            xmlWriter.writeCharacters(QString::number(pointIds->GetId(0)+1) + " " +
//                                      QString::number(pointIds->GetId(1)+1) + " " +
//                                      QString::number(pointIds->GetId(2)+1) + "\n");
//        }
//        xmlWriter.writeEndElement(); // Polygons

//        xmlWriter.writeEndElement(); //Volume
//    }

//    xmlWriter.writeEndElement(); // Volumes

//    xmlWriter.writeStartElement("Labels");
//    xmlWriter.writeAttribute("number", 0);
//    xmlWriter.writeEndElement(); // Labels

//    xmlWriter.writeStartElement("ObjectMap");
//    xmlWriter.writeTextElement("Rotation", "0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000");
//    xmlWriter.writeTextElement("Translation", "0 0 0");
//    xmlWriter.writeTextElement("Scaling", "1 1 1");
//    xmlWriter.writeTextElement("MD5Signature", "0");
//    xmlWriter.writeEndElement(); // ObjectMap

//    xmlWriter.writeEndElement(); // DIFBody

//    xmlWriter.writeEndElement(); // DIF
//    xmlWriter.writeEndDocument();

//    difFile.close();

    return true;
}


// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractDataReader * navxDifReader::create()
{
    return new navxDifReader;
}
