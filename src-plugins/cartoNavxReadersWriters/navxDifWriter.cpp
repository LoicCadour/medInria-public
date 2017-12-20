#include <navxDifWriter.h>

#include <medAbstractDataFactory.h>
#include <medMessageController.h>
#include <medMetaDataKeys.h>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkMetaSurfaceMesh.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>


// /////////////////////////////////////////////////////////////////
// navxDifWriter
// /////////////////////////////////////////////////////////////////

navxDifWriter::navxDifWriter() : medAbstractDataWriter()
{
    validated = true;
}

QString navxDifWriter::identifier() const
{
    return QString("navxDifWriter");
}

QString navxDifWriter::description() const
{
    return QString("Saint-Jude NavX/Siemens .xml mesh exporter");
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

QString navxDifWriter::getMetaData(medAbstractData* medData, QString key)
{
    if (medData->hasMetaData(key))
    {
        return medData->metadata(key);
    }
    return QString("NA");
}

bool navxDifWriter::write(const QString & path)
{
    if(!getDataList().isEmpty())
    {
        return writeFile(path);
    }
    else
    {
        displayWarning("There is no dataset to write.");
    }

    return false;
}

bool navxDifWriter::writeFile(const QString & path)
{
    //Later on, we'll write the metaData of this volume in the xml
    medAbstractData* medData = getDataList()[0];

    QFile difFile(path);
    if ( ! difFile.open(QIODevice::WriteOnly) )
    {
        displayWarning(QString("Could not open file "+path));
        return false;
    }

    QXmlStreamWriter xmlWriter(&difFile);

    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("DIF");

    xmlWriter.writeStartElement("DIFHeader");
    xmlWriter.writeTextElement("Version",          "SJM_DIF_3.0");
    xmlWriter.writeTextElement("VendorVersion",    "NA");
    xmlWriter.writeTextElement("PatientName",      getMetaData(medData, medMetaDataKeys::PatientName.key()));
    xmlWriter.writeTextElement("PatientID",        getMetaData(medData, medMetaDataKeys::PatientID.key()));
    xmlWriter.writeTextElement("PatientBirthDate", getMetaData(medData, medMetaDataKeys::BirthDate.key()));
    xmlWriter.writeTextElement("PatientGender",    getMetaData(medData, medMetaDataKeys::Gender.key()));
    xmlWriter.writeTextElement("StudyID",          getMetaData(medData, medMetaDataKeys::StudyID.key()));
    xmlWriter.writeTextElement("SeriesNumber",     getMetaData(medData, medMetaDataKeys::SeriesNumber.key()));
    xmlWriter.writeTextElement("StudyDate",        getMetaData(medData, medMetaDataKeys::StudyDate.key()));
    xmlWriter.writeTextElement("StudyTime",        getMetaData(medData, medMetaDataKeys::StudyTime.key()));
    xmlWriter.writeTextElement("SeriesTime",       getMetaData(medData, medMetaDataKeys::SeriesTime.key()));
    xmlWriter.writeTextElement("Modality",         getMetaData(medData, medMetaDataKeys::Modality.key()));
    xmlWriter.writeTextElement("RefPhyName",       getMetaData(medData, medMetaDataKeys::Referee.key()));
    xmlWriter.writeTextElement("StudyDesc",        getMetaData(medData, medMetaDataKeys::StudyDescription.key()));
    xmlWriter.writeTextElement("SeriesDesc",       getMetaData(medData, medMetaDataKeys::SeriesDescription.key()));
    xmlWriter.writeTextElement("OperatorName",     getMetaData(medData, medMetaDataKeys::Performer.key()));
    xmlWriter.writeTextElement("OperatorComments", getMetaData(medData, medMetaDataKeys::Comments.key()));
    xmlWriter.writeTextElement("SegmentationDate", "NA");
    xmlWriter.writeTextElement("Inventor",         "1");
    xmlWriter.writeEndElement(); // DIFHeader

    xmlWriter.writeStartElement("DIFBody");
    xmlWriter.writeStartElement("Volumes");
    xmlWriter.writeAttribute("number", QString::number(getDataList().size()));

    vtkSmartPointer<vtkIdList> pointIds = vtkIdList::New();
    double point[3];
    double * color;
    int final_colors[4];

    for (int j = 0; j < getDataList().size(); j++)
    {
        medAbstractData* data = getDataList().at(j);
        if ( !data || (data->identifier() != "vtkDataMesh"))
        {
            displayWarning("Only works with meshes");
            return false;
        }
        vtkMetaSurfaceMesh * metadata = dynamic_cast<vtkMetaSurfaceMesh*>( (vtkObject*)(data->data()));
        if ( ! metadata)
        {
            displayWarning("Only works with surface meshes");
            return false;
        }

        xmlWriter.writeStartElement("Volume");

        /* --- Name Tag --- */
        if (std::strcmp(metadata->GetName(), "") == 0)
        {
            xmlWriter.writeAttribute("name", QString("Vol")+QString::number(j));
        }
        else
        {
            xmlWriter.writeAttribute("name", QString(metadata->GetName()));
        }

        /* --- Color Tag --- RRGGBBAA */
        QString colorHex;
        vtkActor * dataSetActor = metadata->GetActor(0);
        if (dataSetActor)
        {
            // RGB channels
            color = dataSetActor->GetProperty()->GetColor();
            final_colors[0] = int(color[0]*255.0);
            final_colors[1] = int(color[1]*255.0);
            final_colors[2] = int(color[2]*255.0);

            // Alpha channel, AA: 00/0 opaque, FF/255 transparent
            final_colors[3] = int(dataSetActor->GetProperty()->GetOpacity()*255.0);

            // decimal to hex color
            for (int i=0; i<4; i++)
            {
                // Add a "0" to the hex number to follow "RRGGBBAA"
                if (final_colors[i] < 15)
                {
                    colorHex += "0";
                }
                colorHex += QString::number(final_colors[i], 16);
            }
        }
        else
        {
            colorHex = "FFFFFFFF";
        }

        xmlWriter.writeAttribute("color", colorHex);

        /* --- Vertices Tags --- */
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

        /* --- Normals Tags --- */
        xmlWriter.writeStartElement("Normals");
        xmlWriter.writeAttribute("number", QString::number(0));
        xmlWriter.writeEndElement(); // Normals

        /* --- Polygons Tags --- */
        xmlWriter.writeStartElement("Polygons");
        xmlWriter.writeAttribute("number", QString::number(metadata->GetPolyData()->GetPolys()->GetNumberOfCells()));
        for(unsigned int i = 0; i<metadata->GetPolyData()->GetPolys()->GetNumberOfCells(); ++i)
        {
            metadata->GetPolyData()->GetCellPoints(i, pointIds);
            if (pointIds->GetNumberOfIds() != 3)
            {
                qDebug() << "navxVtkWriter: wrong type of cells ! (number of Ids: "<<pointIds->GetNumberOfIds()<<")";
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

void navxDifWriter::showWarning(QString warning)
{
    qDebug() << name() + ": " + warning;
    medMessageController::instance()->showInfo(warning,6000);
}

void navxDifWriter::displayWarning(QString warning)
{
    this->moveToThread(qApp->thread());
    connect(this, SIGNAL(needWarning(QString)),
            this, SLOT(showWarning(QString)), Qt::BlockingQueuedConnection);
    emit needWarning(warning);
}

void navxDifWriter::showWarningPopUp(QString warning)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("NavX system error");
    msgBox.setText(warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes)
    {
        qDebug() << "navxDifWriter::showWarningPopUp - Yes was clicked";
        validated = true;

    }
    else
    {
        qDebug() << "navxDifWriter::showWarningPopUp - Yes was *not* clicked";
        validated = false;
    }
}

void navxDifWriter::displayWarningPopUp(QString warning)
{
    this->moveToThread(qApp->thread());
    connect(this, SIGNAL(needWarningPopUp(QString)),
            this, SLOT(showWarningPopUp(QString)), Qt::BlockingQueuedConnection);
    emit needWarningPopUp(warning);
}

void navxDifWriter::askForConfirmationIfNavXLimitsExceeded()
{
    // Compute number of vertices/triangles
    int numberVertices  = 0;
    int numberTriangles = 0;

    for (int j = 0; j < getDataList().size(); j++)
    {
        medAbstractData* data = getDataList().at(j);
        vtkMetaSurfaceMesh* metadata = dynamic_cast<vtkMetaSurfaceMesh*>( (vtkObject*)(data->data()));
        numberVertices += metadata->GetPolyData()->GetNumberOfPoints();
        numberTriangles += metadata->GetPolyData()->GetPolys()->GetNumberOfCells();
    }

    if ((numberVertices >= maxNumberVertices) || (numberTriangles >= maxNumberTriangles))
    {
        displayWarningPopUp("File exceeds NavX Feb 2016 limits: 100k vertices/200k triangles.\n\nExport it anyway?");
    }
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractDataWriter * navxDifWriter::create()
{
    return new navxDifWriter;
}
