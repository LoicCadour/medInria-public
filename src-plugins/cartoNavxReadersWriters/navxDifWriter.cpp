#include <navxDifWriter.h>

#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractData.h>
#include <medAbstractDataFactory.h>
#include <medMetaDataKeys.h>
#include <viewerFilesToSaveDialog.h>

#include <vtkMetaSurfaceMesh.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkErrorCode.h>
#include <vtkCellArray.h>
#include <vtkActor.h>
#include <vtkProperty.h>

#include <QXmlStreamWriter>

// /////////////////////////////////////////////////////////////////
// navxDifWriter
// /////////////////////////////////////////////////////////////////

navxDifWriter::navxDifWriter() : dtkAbstractDataWriter()
{
}


navxDifWriter::~navxDifWriter()
{
}


QString navxDifWriter::identifier() const
{
    return QString("navxDifWriter");
}


QString navxDifWriter::description() const
{
    return QString("Saint-Jude NavX .xml mesh exporter");
}


QStringList navxDifWriter::handled() const
{
    return QStringList() << "vtkDataMesh";
}


QStringList navxDifWriter::supportedFileExtensions() const
{
    return QStringList() << ".xml";
}


bool navxDifWriter::registered()
{
    return medAbstractDataFactory::instance()->registerDataWriterType("navxDifWriter", QStringList() << "vtkDataMesh", create);
}


bool navxDifWriter::canWrite(const QString & path)
{
    return path.endsWith(".xml");
}


bool navxDifWriter::write(const QString & path)
{
    this->moveToThread(qApp->thread());
    connect(this, SIGNAL(needDataList()), 
        this, SLOT(selectDataToSave()), Qt::BlockingQueuedConnection);
    emit needDataList();

    if(indexList.isEmpty())
        return false;


    QFile difFile(path);
    if ( ! difFile.open(QIODevice::WriteOnly) ) {
        qDebug() << "navxDifWriter: Could not open file" << path;
        return false;
    }

    QXmlStreamWriter xmlWriter(&difFile);

    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("DIF");

    xmlWriter.writeStartElement("DIFHeader");
    xmlWriter.writeTextElement("Version", "SJM_DIF_3.0");
    xmlWriter.writeTextElement("VendorVersion", "NA");
    xmlWriter.writeTextElement("PatientName", "NA");
    xmlWriter.writeTextElement("PatientID", "NA");
    xmlWriter.writeEmptyElement("PatientBirthDate");
    xmlWriter.writeTextElement("PatientGender", "NA");
    xmlWriter.writeEmptyElement("StudyID");
    xmlWriter.writeEmptyElement("SeriesNumber");
    xmlWriter.writeEmptyElement("StudyDate");
    xmlWriter.writeEmptyElement("StudyTime");
    xmlWriter.writeEmptyElement("SeriesTime");
    xmlWriter.writeEmptyElement("Modality");
    xmlWriter.writeEmptyElement("RefPhysName");
    xmlWriter.writeEmptyElement("StudyDesc");
    xmlWriter.writeEmptyElement("SeriesDesc");
    xmlWriter.writeEmptyElement("OperatorName");
    xmlWriter.writeTextElement("OperatorComment", "NA");
    xmlWriter.writeEmptyElement("SegmentationDate");
    xmlWriter.writeTextElement("Inventor", "1");
    xmlWriter.writeEndElement(); // DIFHeader

    xmlWriter.writeStartElement("DIFBody");
    xmlWriter.writeStartElement("Volumes");
    xmlWriter.writeAttribute("number", QString::number(indexList.size()));


    vtkMetaSurfaceMesh * metaSurface = 0;
    vtkSmartPointer<vtkIdList> pointIds = vtkIdList::New();
    double point[3];
    double * color;
    int final_colors[3];

    foreach (medDataIndex index, indexList)
    {
        medAbstractData* data = medDataManager::instance()->retrieveData(index);
        if ( ! data || data->identifier() != "vtkDataMesh") 
        {
            qDebug() << "navxDifWriter: No data provided, or wrong type";
            return false;
        }

        vtkMetaSurfaceMesh * metadata = dynamic_cast<vtkMetaSurfaceMesh*>( (vtkObject*)(data->data()));
        if ( ! metadata) {
            qDebug() << "navxDifWriter: not a vtkMetaSurfaceMesh";
            return false;
        }

        xmlWriter.writeStartElement("Volume");
        QString seriesName = data->metaDataValues(medMetaDataKeys::SeriesDescription.key())[0];
        xmlWriter.writeAttribute("name", QString(seriesName));

        QString colorHex;
        vtkActor * dataSetActor = metadata->GetActor(0);
        if (dataSetActor) {
            color = dataSetActor->GetProperty()->GetColor();
            final_colors[0] = int(color[0]*255);
            final_colors[1] = int(color[1]*255);
            final_colors[2] = int(color[2]*255);

            colorHex = QString::number(final_colors[0], 16) +
                               QString::number(final_colors[1], 16) +
                               QString::number(final_colors[2], 16);
        } else {
            colorHex = "FFFFFF";
        }

        xmlWriter.writeAttribute("color", colorHex);

        xmlWriter.writeStartElement("Vertices");
        xmlWriter.writeAttribute("number", QString::number(metadata->GetPolyData()->GetNumberOfPoints()));
        for(vtkIdType i = 0; i < metadata->GetPolyData()->GetNumberOfPoints(); ++i)
        {
            metadata->GetPolyData()->GetPoint(i, point);
            xmlWriter.writeCharacters(QString::number(point[0]) + " " +
                                      QString::number(point[1]) + " " +
                                      QString::number(point[2]) + "\n");
        }
        xmlWriter.writeEndElement(); // Vertices

        xmlWriter.writeStartElement("Polygons");
        xmlWriter.writeAttribute("number", QString::number(metadata->GetPolyData()->GetPolys()->GetNumberOfCells()));
        for(unsigned int i = 0; i<metadata->GetPolyData()->GetPolys()->GetNumberOfCells(); ++i)
        {
            metadata->GetPolyData()->GetCellPoints(i, pointIds);
            if (pointIds->GetNumberOfIds() != 3)
            {
                qDebug() << "navxVtkWriter: wrong type of cells !";
                return false;
            }
            xmlWriter.writeCharacters(QString::number(pointIds->GetId(0)+1) + " " +
                                      QString::number(pointIds->GetId(1)+1) + " " +
                                      QString::number(pointIds->GetId(2)+1) + "\n");
        }
        xmlWriter.writeEndElement(); // Polygons

        xmlWriter.writeEndElement(); //Volume
    }

    xmlWriter.writeEndElement(); // Volumes

    xmlWriter.writeStartElement("Labels");
    xmlWriter.writeAttribute("number", QString::number(0));
    xmlWriter.writeCharacters("tag"); //need to have a readable file (without it, no </Labels>)
    xmlWriter.writeEndElement(); // Labels

    xmlWriter.writeStartElement("ObjectMap");
    xmlWriter.writeTextElement("Rotation", "0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000");
    xmlWriter.writeTextElement("Translation", "0 0 0");
    xmlWriter.writeTextElement("Scaling", "1 1 1");
    xmlWriter.writeTextElement("MD5Signature", "0");
    xmlWriter.writeEndElement(); // ObjectMap

    xmlWriter.writeEndElement(); // DIFBody

    xmlWriter.writeEndElement(); // DIF
    xmlWriter.writeEndDocument();

    difFile.close();

    return true;
}

void navxDifWriter::selectDataToSave()
{
    viewerFilesToSaveDialog *saveDialog = new viewerFilesToSaveDialog(NULL);
    saveDialog->show();

    int res = saveDialog->exec();

    if(res == QDialog::Accepted)
    {
        this->indexList = saveDialog->indexList();
    }
}


// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractDataWriter * navxDifWriter::create()
{
    return new navxDifWriter;
}
