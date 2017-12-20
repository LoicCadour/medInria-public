#include <navxDifReader.h>

#include <medAbstractData.h>
#include <medAbstractDataFactory.h>
#include <medMetaDataKeys.h>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkMetaSurfaceMesh.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkTriangle.h>

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
    return QString("Saint-Jude NavX/Siemens mesh importer");
}

QStringList navxDifReader::handled() const
{
    return QStringList() << "vtkDataMesh";
}

QStringList navxDifReader::supportedFileExtensions() const
{
    return QStringList() << ".xml" << ".dif";
}

bool navxDifReader::registered()
{
    return medAbstractDataFactory::instance()->registerDataReaderType("navxDifReader", QStringList() << "vtkDataMesh", create);
}

bool navxDifReader::canRead(const QString & path)
{
    return path.endsWith(".xml") || path.endsWith(".dif");
}

bool navxDifReader::readInformation(const QString& path)
{
    medAbstractData *medData = medAbstractDataFactory::instance()->create("vtkDataMesh");
    this->setData(medData);

    return true;
}

bool navxDifReader::read(const QString & path)
{
    // create data
    if (!data())
    {
        readInformation(path);
    }

    vtkPolyData *polyData = vtkPolyData::New();
    vtkMetaSurfaceMesh * smesh = vtkMetaSurfaceMesh::New();
    smesh->SetDataSet(polyData);
    this->data()->setData(smesh);

    QString strHeader;

    // Open the file
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QXmlStreamReader xmlReader;
        xmlReader.setDevice(&file);

        while(!xmlReader.atEnd())
        {
            xmlReader.readNext();
            if(xmlReader.isStartElement())
            {
                // Read the tag name.
                QString sec(xmlReader.name().toString());

                /* --- Color Tag --- */
                if (sec == "Volume")
                {
                    QString color = xmlReader.attributes().value("color").toString();

                    double final_colors[4];
                    final_colors[0] = double(QString(color.at(0)).append(color.at(1)).toUInt(NULL, 16))/255.0; //rr
                    final_colors[1] = double(QString(color.at(2)).append(color.at(3)).toUInt(NULL, 16))/255.0; //gg
                    final_colors[2] = double(QString(color.at(4)).append(color.at(5)).toUInt(NULL, 16))/255.0; //bb
                    final_colors[3] = double(QString(color.at(6)).append(color.at(7)).toUInt(NULL, 16))/255.0; //aa

                    vtkMetaDataSet * dataSet = dynamic_cast<vtkMetaDataSet*>( (vtkObject*)(this->data()->data()));

                    // Cf. vtkDataMeshInteractor::setupParameters(), add color and opacity in the view
                    vtkActor* actor = vtkActor::New();
                    dataSet->AddActor(actor);
                    dataSet->GetActor(0)->GetProperty()->SetColor(final_colors[0], final_colors[1], final_colors[2]);
                    dataSet->GetActor(0)->GetProperty()->SetOpacity(final_colors[3]);
                }

                /* --- Header --- */
                else if ((sec == "PatientName")  ||
                         (sec == "PatientID")    ||
                         (sec == "StudyID")      ||
                         (sec == "SeriesNumber") ||
                         (sec == "StudyDate")    ||
                         (sec == "StudyTime")    ||
                         (sec == "SeriesTime")   ||
                         (sec == "Modality")     ||
                         (sec == "SeriesDesc")
                         )
                {
                    strHeader = xmlReader.readElementText();
                    if (strHeader != "NA") //if valid data
                    {
                        this->data()->setMetaData(sec, strHeader);
                    }
                }
                else if (sec == "PatientBirthDate")
                {
                    strHeader = xmlReader.readElementText();
                    if (strHeader != "NA") //if valid data
                    {
                        this->data()->setMetaData(medMetaDataKeys::BirthDate.key(), strHeader);
                    }
                }
                else if (sec == "PatientGender")
                {
                    strHeader = xmlReader.readElementText();
                    if (strHeader != "NA") //if valid data
                    {
                        this->data()->setMetaData(medMetaDataKeys::Gender.key(), strHeader);
                    }
                }
                else if (sec == "RefPhyName")
                {
                    strHeader = xmlReader.readElementText();
                    if (strHeader != "NA") //if valid data
                    {
                        this->data()->setMetaData(medMetaDataKeys::Referee.key(), strHeader);
                    }
                }
                else if (sec == "StudyDesc")
                {
                    strHeader = xmlReader.readElementText();
                    if (strHeader != "NA") //if valid data
                    {
                        this->data()->setMetaData(medMetaDataKeys::StudyDescription.key(), strHeader);
                    }
                }
                else if (sec == "OperatorName")
                {
                    strHeader = xmlReader.readElementText();
                    if (strHeader != "NA") //if valid data
                    {
                        this->data()->setMetaData(medMetaDataKeys::Performer.key(), strHeader);
                    }
                }
                else if (sec == "OperatorComments")
                {
                    strHeader = xmlReader.readElementText();
                    if (strHeader != "NA") //if valid data
                    {
                        this->data()->setMetaData(medMetaDataKeys::Comments.key(), strHeader);
                    }
                }

                /* --- Vertices --- */
                else if (sec == "Vertices")
                {
                    // list of every points
                    QString strPoints = xmlReader.readElementText();
                    QStringList listPoints = strPoints.split("\n", QString::SkipEmptyParts);
                    vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();

                    // for each point, extract values
                    for (int i=0; i<listPoints.count(); i++)
                    {
                        QStringList listOnePoint = listPoints.at(i).split(" ");

                        double point[3];
                        point[0] = listOnePoint.at(0).toDouble();
                        point[1] = listOnePoint.at(1).toDouble();
                        point[2] = listOnePoint.at(2).toDouble();
                        newPoints->InsertNextPoint(point);
                    }

                    // add points to data
                    if (newPoints->GetNumberOfPoints() !=0)
                    {
                        static_cast<vtkMetaSurfaceMesh *>(this->data()->data())->GetPolyData()->SetPoints(newPoints);
                    }
                }

                /* --- Polygons Tags --- */
                else if (sec == "Polygons")
                {
                    // list of every cells
                    QString strPoints = xmlReader.readElementText();
                    QStringList listPoints = strPoints.split("\n", QString::SkipEmptyParts);
                    vtkCellArray* newCells = vtkCellArray::New();

                    // for each cells, extract values
                    for (int i=0; i<listPoints.count(); i++)
                    {
                        QStringList listOnePoint = listPoints.at(i).split(" ");

                        vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
                        triangle->GetPointIds()->SetId ( 0, listOnePoint.at(0).toInt()-1 );
                        triangle->GetPointIds()->SetId ( 1, listOnePoint.at(1).toInt()-1 );
                        triangle->GetPointIds()->SetId ( 2, listOnePoint.at(2).toInt()-1 );
                        newCells->InsertNextCell(triangle);
                    }

                    // add points to data
                    if (newCells->GetNumberOfCells() !=0)
                    {
                        static_cast<vtkMetaSurfaceMesh *>(this->data()->data())->GetPolyData()->SetPolys(newCells);
                    }
                }
            }
        }
        if (xmlReader.hasError())
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}


// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractDataReader * navxDifReader::create()
{
    return new navxDifReader;
}
