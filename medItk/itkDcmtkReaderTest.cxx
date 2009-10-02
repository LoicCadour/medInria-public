/* itkDcmtkReaderTest.cxx --- 
 * 
 * Author: Pierre Fillard
 * Copyright (C) 2008 - Julien Wintz, Inria.
 * Created: Fri Oct  2 21:46:08 2009 (+0200)
 * Version: $Id$
 * Last-Updated: Fri Oct  2 21:47:00 2009 (+0200)
 *           By: Julien Wintz
 *     Update #: 3
 */

/* Commentary: 
 * 
 */

/* Change log:
 * 
 */

#include <iostream>

#include <itkCommand.h>
#include <itkDCMTKImageIO.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include <vtkViewImage2D.h>
#include <vtkViewImage3D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageMapToColors.h>
#include <vtkImageActor.h>
#include <vtkCallbackCommand.h>

#include <time.h>

namespace itk
{

    class SliceReadCommand : public Command
    {
        
    public:
        typedef SliceReadCommand               Self;
        typedef Command                        Superclass;
        typedef itk::SmartPointer<Self>        Pointer;
        typedef itk::SmartPointer<const Self>  ConstPointer;

        itkTypeMacro( SliceReadCommand, Command );
        itkNewMacro(Self);
        
        typedef itk::Image<unsigned short, 3> ImageType;
        
        void Execute(Object *caller, const EventObject &event);
        void Execute(const Object *caller, const EventObject &event);
        
        itkSetObjectMacro (Image, ImageType);
        itkGetObjectMacro (Image, ImageType);

        void SetView (vtkViewImage2D* view)
        { m_View = view; }
    
    protected:
        SliceReadCommand(){ this->ImageNotSet = 1; };
        virtual ~SliceReadCommand(){};

    private:
        ImageType::Pointer m_Image;
        vtkViewImage2D*    m_View;
        int ImageNotSet;
    
    };


    void SliceReadCommand::Execute (Object *caller, const EventObject &event)
    {
        DCMTKImageIO *po = dynamic_cast<DCMTKImageIO *>(caller);
        if (! po) return;
     
        if( typeid(event) == typeid ( itk::SliceReadEvent )  )
        {
            std::cout << "Slice read" << std::endl;
            //qDebug() << "Slice read";
            if( this->ImageNotSet ){
                m_View->SetITKImage ( m_Image );
                m_View->ResetCurrentPoint();
                m_View->ResetZoom();
                m_View->SetWindow (2320);
                m_View->SetLevel (1138);
                this->ImageNotSet = 0;
            }
            //m_View->GetImageReslice()->Modified();
            m_View->GetWindowLevel()->Modified();
            m_View->GetRenderWindow()->InvokeEvent ( m_View->GetRenderWindow()->GetEventPending() );
            m_View->GetRenderWindowInteractor()->Render();
       
            //qDebug() << "Rendered!";
        }
    }


    void SliceReadCommand::Execute (const Object *caller, const EventObject &event)
    {
        DCMTKImageIO *po = dynamic_cast<DCMTKImageIO *>(const_cast<Object *>(caller) );
        if (! po) return;
     
        if( typeid(event) == typeid ( SliceReadEvent  )  )
        {
            //qDebug() << "Slice read";
            if( this->ImageNotSet ){
                m_View->SetITKImage ( m_Image );
                m_View->ResetCurrentPoint();
                m_View->ResetZoom();
                m_View->SetWindow (2320);
                m_View->SetLevel (1138);
                this->ImageNotSet = 0;
            }
            m_View->GetWindowLevel()->Modified();
            m_View->GetRenderWindow()->InvokeEvent ( m_View->GetRenderWindow()->GetEventPending() );
            m_View->GetRenderWindow()->Render();
  
        }
    }
  
}


typedef itk::Image<unsigned short, 3>   ImageType;
typedef itk::ImageFileReader<ImageType> ImageReaderType;
static ImageReaderType::Pointer reader    = 0;
static int                      readImage = 0;


void handle_vtk(vtkObject* caller, unsigned long id, void* clientdata, void* calldata)
{
    std::cout << "handle_vtk()" << std::endl;
    //vtkRenderWindowInteractor* iren = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast( caller );
    if( !readImage ){
        readImage = 1;
        std::cout << "Reading the image" << std::endl;
        try{
            reader->Update();
            reader->GetOutput()->DisconnectPipeline();
        }
        catch (itk::ExceptionObject &e){
            std::cerr <<e;
            exit (-1);
        }
        std::cout << "Reading the image Done." << std::endl;
    }
}




int main (int narg, char* arg[])
{

    if( narg<2 )
    {
        std::cerr << "Usage: " << arg[0] << " dicom1 dicom2 ...\n";
        return -1;
    }
  
    std::vector<std::string> fileNames;
    for( int i=1; i<narg; i++){
        std::cout << "Inserting: " << arg[i] << std::endl;
        fileNames.push_back ( arg[i] );
    }
  
  
    itk::DCMTKImageIO::Pointer io = itk::DCMTKImageIO::New();
    //  if( narg>2 )
    io->SetFileNames ( fileNames );
    //  else
    //    io->SetFileName ( arg[1] );
    //io->SetFileName ( arg[1] );
    //io->SetNumberOfThreads (1);
  
    /*
      typedef itk::Image<unsigned short, 3> ImageType;
      typedef itk::ImageFileReader<ImageType> ImageReaderType;
    */
    /*ImageReaderType::Pointer */
    reader = ImageReaderType::New();
    reader->SetImageIO ( io );
    reader->SetFileName ( arg[1] );
  
    itk::SliceReadCommand::Pointer command = itk::SliceReadCommand::New();
    command->SetImage ( reader->GetOutput() );
  
    io->AddObserver (itk::SliceReadEvent(), command);
    std::cout << "Threads: " << io->GetNumberOfThreads() /* << " Elapsed time:" << (double)(t2-t1)/(double)(CLOCKS_PER_SEC)*/ << std::endl;
  
    vtkViewImage2D* view = vtkViewImage2D::New();
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
    vtkRenderWindow* rwin = vtkRenderWindow::New();
    vtkRenderer* renderer = vtkRenderer::New();
    iren->SetRenderWindow (rwin);
    rwin->AddRenderer (renderer);
    view->SetRenderWindow ( rwin );
    view->SetRenderer ( renderer );
    view->SetBackgroundColor (0.0, 0.0, 0.0);
    view->Show2DAxisOff();
    view->CursorFollowMouseOn();
    view->SetInterpolationMode(0);

    view->SetLeftButtonInteractionStyle   (vtkViewImage2D::ZOOM_INTERACTION);
    view->SetMiddleButtonInteractionStyle (vtkViewImage2D::SELECT_INTERACTION);
    view->SetWheelInteractionStyle        (vtkViewImage2D::SELECT_INTERACTION);
    view->SetRightButtonInteractionStyle  (vtkViewImage2D::WINDOW_LEVEL_INTERACTION);

    command->SetView ( view );
  
    ImageType::Pointer image = reader->GetOutput();
    if( image.IsNull() )
    {
        std::cout << "Image is null" << std::endl;
        return -1;
    }


    iren->Initialize();

    std::cout << "Reading the image" << std::endl;
    try{
        reader->Update();
        reader->GetOutput()->DisconnectPipeline();
    }
    catch (itk::ExceptionObject &e){
        std::cerr <<e;
        view->Delete();
        iren->Delete();
        renderer->Delete();
        rwin->Delete();
        return -1;
    }
    std::cout << image << std::endl;
  
    iren->Start();

    /*
      clock_t t1 = clock();
      try
      {
      reader->Update();
      }
      catch (itk::ExceptionObject &e)
      {
      std::cerr << e;
      view->Delete();
      iren->Delete();
      renderer->Delete();
      rwin->Delete();
      return -1;
      }
      clock_t t2 = clock();
  
  
      std::cout << "Threads: " << io->GetNumberOfThreads() << " Elapsed time:" << (double)(t2-t1)/(double)(CLOCKS_PER_SEC) << std::endl;

      image->DisconnectPipeline();
      std::cout << image << std::endl;
    */

    //iren->Start();


    view->Delete();
    iren->Delete();
    renderer->Delete();
    rwin->Delete();
  
    /*  
 
        typedef itk::ImageFileWriter<ImageType> WriterType;
        WriterType::Pointer writer = WriterType::New();
        writer->SetFileName ("test.nii.gz");
        writer->SetInput ( image );
        writer->Update();

    */
  
    return 0;  
}
