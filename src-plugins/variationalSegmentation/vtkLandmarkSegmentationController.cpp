#include <vtkLandmarkSegmentationController.h>

#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkContourFilter.h>
#include <vtkLandmarkWidget.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkContourFilter.h>
#include <vtkPropPicker.h>
#include <vtkCellPicker.h>
#include <vtkPicker.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkInformationVector.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRendererCollection.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkLandmarkManager.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkProperty2D.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>

vtkCxxRevisionMacro(vtkLandmarkSegmentationController, "$Revision: 1315 $");
vtkStandardNewMacro(vtkLandmarkSegmentationController);

//----------------------------------------------------------------------------
vtkLandmarkSegmentationControllerCommand::vtkLandmarkSegmentationControllerCommand()
{
  this->Controller = NULL;
}
//----------------------------------------------------------------------------
vtkLandmarkSegmentationControllerCommand::~vtkLandmarkSegmentationControllerCommand()
{
  if (this->Controller)
    this->Controller->UnRegister(this);
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationControllerCommand::SetController (vtkLandmarkSegmentationController* arg)
{
  if (arg == this->Controller)
    return;
  if (this->Controller)
    this->Controller->UnRegister(this);
  this->Controller = arg;
  if (this->Controller)
    this->Controller->Register(this);
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationControllerCommand::Execute ( vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{

    if (!this->Controller) // if we are in 3D mode we block all interactions, IMO it is too difficult to add or manipulate sphere widgets in 3d.
    return;
  if (!this->Controller->GetInteractorCollection())
    return;
  
  if ( ( (event == vtkCommand::RightButtonPressEvent)  ||
	 (event == vtkCommand::MiddleButtonPressEvent) ||
	 (event == vtkCommand::LeftButtonPressEvent) ) )
  {
    /**************  Landmark addition  **************/

    /*************************************************/
    /**  First we find the 3D position of the click  */
    /*************************************************/
    vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::SafeDownCast(caller);
    if (! rwi->GetShiftKey() )
      return;
    int X, Y;
    X = rwi->GetEventPosition()[0];
    Y = rwi->GetEventPosition()[1];
    vtkRenderer* renderer = rwi->FindPokedRenderer(X,Y);
    if (!renderer)
      return;
    vtkCellPicker* pointpicker = vtkCellPicker::New();
    pointpicker->Pick(X,Y,0.0, renderer);
    vtkDataSet* dataset = pointpicker->GetDataSet();
    if (!dataset)
    {
      pointpicker->Delete();
      return;
    }
    vtkPoints* points = pointpicker->GetPickedPositions(); 
    if (!points || !points->GetNumberOfPoints())
    {
      pointpicker->Delete();
      return;
    }
    double* position = points->GetPoint ((vtkIdType)0);
    
    /*************************************************/
    /**  Second we check the type of landmark to add */
    /*************************************************/
    int type = 0;
    switch(event)
    {
    case vtkCommand::RightButtonPressEvent:
        if (rwi->GetControlKey())
            type = 1;
        else
            type = -1;
        break;
	case vtkCommand::LeftButtonPressEvent:
	  type = 0;
	  break;
	case vtkCommand::MiddleButtonPressEvent:
	default:
	  type = 1;
	  break;
    }
    
    /*************************************************/
    /**  Third we add the landmark and invoke event  */
    /*************************************************/
    if (!this->Controller->getMode3D())
    {
        vtkLandmarkWidget* initial_landmark = this->Controller->AddConstraint(position, type);
        initial_landmark->InvokeEvent (vtkCommand::EndInteractionEvent);
        return;
    }
    
  }
  
  if ( event == vtkCommand::EndInteractionEvent )
  {
    vtkLandmarkWidget* landmark = vtkLandmarkWidget::SafeDownCast (caller);
    if (!landmark)
    {
        vtkHandleWidget * widget2d = vtkHandleWidget::SafeDownCast(caller);
        this->Controller->updateLandmarksPosFromWidget2D();
    }
    else 
        if (landmark->GetInteractor()->GetControlKey()) /**************  Landmark deletion  **************/
        {
            this->Controller->RemoveConstraint (landmark);
        }
    
    this->Controller->RefreshConstraints();
    this->Controller->Update();
    
    this->Controller->GetInteractorCollection()->InitTraversal();
    vtkRenderWindowInteractor* interactor = vtkRenderWindowInteractor::SafeDownCast (this->Controller->GetInteractorCollection()->GetNextItemAsObject());
    while(interactor)
    {
      interactor->Render();
      interactor = vtkRenderWindowInteractor::SafeDownCast (this->Controller->GetInteractorCollection()->GetNextItemAsObject());
    }
  }
  if ( event == vtkImageView2D::SliceChangedEvent )
  {
      vtkImageView2D * view2d= vtkImageView2D::SafeDownCast(caller);
      this->Controller->showOrHide2DWidget();                                              
  }
}

//----------------------------------------------------------------------------
vtkLandmarkSegmentationController::vtkLandmarkSegmentationController()
{
  this->SetNumberOfInputPorts(0);
  m_Input                    = NULL;
  implicitFunction           = NULL;
  this->InteractorCollection = NULL;
  this->Enabled              = 0;
  m_Filter                 = FilterType::New();
  m_Converter              = ConverterType::New();
  this->SurfaceExtractor   = vtkContourFilter::New();
  this->Transformer        = vtkMatrixToLinearTransform::New();
  this->LandmarkCollection = vtkCollection::New();
  this->Command            = vtkLandmarkSegmentationControllerCommand::New();
  this->Mapper             = vtkPolyDataMapper::New();
  this->Actor              = vtkActor::New();
  this->TotalLandmarkCollection = vtkCollection::New();
  this->Command->SetController (this);
  m_Converter->SetInput (m_Filter->GetOutput());
  vtkImageData* tmp = vtkImageData::New();
  this->SurfaceExtractor->SetInput (tmp);
  this->SurfaceExtractor->SetValue (0, 0.0);
  this->SurfaceExtractor->Update();
  this->SetOutput (this->SurfaceExtractor->GetOutput());
  this->Mapper->SetInput (this->SurfaceExtractor->GetOutput());
  this->Actor->SetMapper (this->Mapper);
  
  this->view2d = 0;
  this->view3d = 0;

  this->LandmarkRadius = 1.5;
  tmp->Delete();
}

//----------------------------------------------------------------------------
vtkLandmarkSegmentationController::~vtkLandmarkSegmentationController()
{
  this->Transformer->Delete();
  this->SurfaceExtractor->Delete();
  this->LandmarkCollection->Delete();
  if (this->view2d)
      this->view2d->RemoveObserver(this->Command);

  this->Command->Delete();
  if (this->InteractorCollection)
    this->InteractorCollection->UnRegister(this);
  this->Mapper->Delete();
  this->Actor->Delete();
  this->TotalLandmarkCollection->Delete();
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationController::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationController::SetInteractorCollection (vtkCollection* arg)
{
  if (arg == this->InteractorCollection)
    return;
  if (this->InteractorCollection)
    this->InteractorCollection->UnRegister(this);
  this->InteractorCollection = arg;
  if (this->InteractorCollection)
    this->InteractorCollection->Register(this);
  this->SetEnabled (this->GetEnabled());
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationController::SetInput(ImageType::Pointer input)
{
  m_Input = input;
  m_Filter->SetInput (m_Input);
  
  ImageType::DirectionType directions = input->GetDirection();
  ImageType::PointType origin = input->GetOrigin();
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  matrix->Identity();
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      matrix->SetElement (i, j, directions (i,j));
  double v_origin[4], v_origin2[4];
  for (int i=0; i<3; i++)
    v_origin[i] = origin[i];
  v_origin[3] = 1.0;
  matrix->MultiplyPoint (v_origin, v_origin2);
  for (int i=0; i<3; i++)
    matrix->SetElement (i, 3, v_origin[i]-v_origin2[i]);
  this->Transformer->SetInput (matrix);
  matrix->Delete();
  this->SurfaceExtractor->SetInput (m_Converter->GetOutput());
  this->Modified();
  double minspacing = std::min(input->GetSpacing()[0], std::min(input->GetSpacing()[1], input->GetSpacing()[2]));
  this->LandmarkRadius = minspacing/2;
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationController::SetConstraints (ConstraintListType arg)
{
  this->Constraints = arg;
  m_Filter->SetConstraints (this->Constraints);
  if (this->GetInteractorCollection())
  {
    for (unsigned int i=0; i<this->Constraints.size(); i++)
    {
      ConstraintType c = this->Constraints[i];      
      this->AddConstraint (c.first.GetDataPointer(), c.second);
    }
    this->RefreshConstraints();
  }
  this->Modified();
}

void vtkLandmarkSegmentationController::SetConstraints (vtkPointSet* arg)
{
  if (!arg->GetPoints()->GetNumberOfPoints() || !arg->GetPointData()->GetScalars())
  {
    vtkErrorMacro (<<"vtkLandmarkSegmentationController::SetConstraints() : argument empty");
    return;
  }
  ConstraintListType constraints;
  for (unsigned int i=0; i<arg->GetPoints()->GetNumberOfPoints(); i++)
  {
    ConstraintType c;
    c.first = arg->GetPoint (i);
    c.second = arg->GetPointData()->GetScalars()->GetComponent (i,0);
    constraints.push_back (c);
  }
  this->SetConstraints (constraints);
}

//----------------------------------------------------------------------------
vtkLandmarkWidget* vtkLandmarkSegmentationController::AddConstraint (double* pos, int type)
{
  if (!this->InteractorCollection)
    return NULL;
  this->GetInteractorCollection()->InitTraversal();
  vtkRenderWindowInteractor* item = vtkRenderWindowInteractor::SafeDownCast (this->GetInteractorCollection()->GetNextItemAsObject());
  vtkLandmarkWidget* initial_landmark = NULL;
  while(item)
  {
    vtkLandmarkWidget* l = vtkLandmarkWidget::New();
    vtkPointHandleRepresentation2D * pointRep = vtkPointHandleRepresentation2D::New();
    //pointRep->SetRenderer(view2d->GetRenderer());
    pointRep->SetWorldPosition(pos);
    pointRep->BuildRepresentation();
    l->ScaleOff();
    l->SetCenter (pos);
    l->SetRadius (this->LandmarkRadius);
    l->SetValue (type);
    if (type == 1) {l->GetSphereProperty()->SetColor (1,0,0);pointRep->GetProperty()->SetColor(1,0,0); }
    if (type == 0) {l->GetSphereProperty()->SetColor (1,1,0);pointRep->GetProperty()->SetColor(1,1,0); }
    if (type ==-1) {l->GetSphereProperty()->SetColor (0,1,0);pointRep->GetProperty()->SetColor(0,1,0); }
    l->GetWidget2D()->SetRepresentation(pointRep);
    if (!mode3D)
        l->GetWidget2D()->SetInteractor(item);
    int indices[3];
    view2d->GetImageCoordinatesFromWorldCoordinates(pos,indices);
    l->SetIndices(indices);
    l->GetWidget2D()->On();
    l->SetRepresentationToSurface();
    l->SetInteractor (item);
    if (l->GetCurrentRenderer())
        l->Off();
    this->GetTotalLandmarkCollection()->AddItem (l);
    if (!initial_landmark)
      initial_landmark = l;
    l->Delete();
    item = vtkRenderWindowInteractor::SafeDownCast (this->GetInteractorCollection()->GetNextItemAsObject());
  }
  this->LandmarkCollection->AddItem(initial_landmark);
  this->LinkInteractions();
  return initial_landmark;
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationController::RemoveConstraint (vtkLandmarkWidget* arg)
{
  unsigned int                  id = this->TotalLandmarkCollection->IsItemPresent (arg) - 1.0;
  unsigned int numberofinteractors = this->InteractorCollection->GetNumberOfItems();
  unsigned int     firstlandmarkid = (unsigned int)(std::floor ((double)id / (double)numberofinteractors)) * numberofinteractors;
  vtkLandmarkWidget* firstlandmark = vtkLandmarkWidget::SafeDownCast (this->TotalLandmarkCollection->GetItemAsObject (firstlandmarkid));
  unsigned int          idtoremove = this->LandmarkCollection->IsItemPresent (firstlandmark) - 1.0;
  vtkLandmarkWidget*      toremove = vtkLandmarkWidget::SafeDownCast (this->LandmarkCollection->GetItemAsObject (idtoremove));
  toremove->Off();
  toremove->RemoveAllObservers();
  this->LandmarkCollection->RemoveItem(idtoremove);
  for (unsigned int dump = 0; dump < numberofinteractors; dump++)
  {
    toremove = vtkLandmarkWidget::SafeDownCast (this->TotalLandmarkCollection->GetItemAsObject (idtoremove * numberofinteractors));
    toremove->RemoveAllObservers();
    // We cannot actually remove the object as the landmark still has some
    // invoked events to process. So we just let the TotalLandmarkCollection
    // grow without any consequence.
    //this->TotalLandmarkCollection->RemoveItem (toremove);
  }
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationController::RefreshConstraints (void)
{
  this->Constraints.clear();
  for (int i=0; i<this->LandmarkCollection->GetNumberOfItems(); i++)
  {
    vtkLandmarkWidget* landmark = vtkLandmarkWidget::SafeDownCast (this->LandmarkCollection->GetItemAsObject(i));
    FilterType::ConstraintType c;
    FilterType::PointType p (landmark->GetCenter());
    c.first = p;
    c.second = landmark->GetValue();
    this->Constraints.push_back (c);
  }
  m_Filter->SetConstraints (this->Constraints);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationController::GetLandmarkSet(vtkPolyData* arg)
{
  vtkPoints* pts = vtkPoints::New();
  vtkFloatArray* values = vtkFloatArray::New();
  this->LandmarkCollection->InitTraversal();
  vtkLandmarkWidget* landmark = vtkLandmarkWidget::SafeDownCast (this->LandmarkCollection->GetNextItemAsObject());
  while(landmark)
  {
    pts->InsertNextPoint (landmark->GetCenter());
    values->InsertNextValue (landmark->GetValue());
    landmark = vtkLandmarkWidget::SafeDownCast (this->LandmarkCollection->GetNextItemAsObject());
  }
  arg->SetPoints (pts);
  arg->GetPointData()->SetScalars (values);
  pts->Delete();
  values->Delete();
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationController::SetEnabled (unsigned int arg)
{
  this->Enabled = arg;

  if (this->InteractorCollection)
  {
    this->InteractorCollection->InitTraversal();
    vtkRenderWindowInteractor* interactor = vtkRenderWindowInteractor::SafeDownCast (this->InteractorCollection->GetNextItemAsObject());
    while(interactor)
    {
      if (arg)
      {
	if (!interactor->HasObserver (vtkCommand::CharEvent, this->Command) )
	  interactor->AddObserver(vtkCommand::CharEvent, this->Command, -1);
	if (!interactor->HasObserver (vtkCommand::LeftButtonPressEvent, this->Command) )
	  interactor->AddObserver(vtkCommand::LeftButtonPressEvent, this->Command, -1);
	if (!interactor->HasObserver (vtkCommand::MiddleButtonPressEvent, this->Command) )
	  interactor->AddObserver(vtkCommand::MiddleButtonPressEvent, this->Command, -1);
	if (!interactor->HasObserver (vtkCommand::RightButtonPressEvent, this->Command) )
	  interactor->AddObserver(vtkCommand::RightButtonPressEvent, this->Command, -1);
      }
      else
	interactor->RemoveObserver(this->Command);
      interactor = vtkRenderWindowInteractor::SafeDownCast (this->InteractorCollection->GetNextItemAsObject());
    }
  }
  this->LandmarkCollection->InitTraversal();
  vtkLandmarkWidget* landmark = vtkLandmarkWidget::SafeDownCast (this->LandmarkCollection->GetNextItemAsObject());
  while(landmark)
  {
    landmark->SetEnabled(arg);
    landmark = vtkLandmarkWidget::SafeDownCast (this->LandmarkCollection->GetNextItemAsObject());
  }
}

//----------------------------------------------------------------------------
void vtkLandmarkSegmentationController::LinkInteractions ( void)
{ 
  vtkLandmarkWidget* l1 = NULL;
  vtkLandmarkWidget* l2 = NULL;
  unsigned int numberofinteractors = this->GetInteractorCollection()->GetNumberOfItems();
  vtkCollection* collection        = this->GetTotalLandmarkCollection();
  unsigned int numberoflandmarks   = collection->GetNumberOfItems() / numberofinteractors;
  for (unsigned int n = 0; n < numberoflandmarks; n++)
  {
    for (unsigned int i=0; i<numberofinteractors; i++)
    {
      int id1 = numberofinteractors * n + i;
      l1 = vtkLandmarkWidget::SafeDownCast (collection->GetItemAsObject(id1));
      if (!l1->HasObserver(vtkCommand::EndInteractionEvent, this->Command))
	    l1->AddObserver(vtkCommand::EndInteractionEvent, this->Command, -1);
      if (!l1->GetWidget2D()->HasObserver(vtkCommand::EndInteractionEvent, this->Command))
        l1->GetWidget2D()->AddObserver(vtkCommand::EndInteractionEvent, this->Command, -1);
      
      for (unsigned int j=0; j<numberofinteractors; j++)
      {
	int id2 = numberofinteractors * n + j;
	l2 = vtkLandmarkWidget::SafeDownCast (collection->GetItemAsObject(id2));
	
	if (l1 != l2)
	{
	  if (!l1->HasObserver(vtkCommand::InteractionEvent, l2->GetCommand()))
	    l1->AddObserver(vtkCommand::InteractionEvent, l2->GetCommand());
	  if (!l1->HasObserver(vtkCommand::EnableEvent, l2->GetCommand()))
	    l1->AddObserver(vtkCommand::EnableEvent,      l2->GetCommand());
	  if (!l1->HasObserver(vtkCommand::DisableEvent, l2->GetCommand()))
	    l1->AddObserver(vtkCommand::DisableEvent,     l2->GetCommand());
	  if (!l2->HasObserver(vtkCommand::InteractionEvent, l1->GetCommand()))
	    l2->AddObserver(vtkCommand::InteractionEvent, l1->GetCommand());
	  if (!l2->HasObserver(vtkCommand::EnableEvent, l1->GetCommand()))
	    l2->AddObserver(vtkCommand::EnableEvent,      l1->GetCommand());
	  if (!l2->HasObserver(vtkCommand::DisableEvent, l1->GetCommand()))
	    l2->AddObserver(vtkCommand::DisableEvent,     l1->GetCommand());
	}  
      }    
    }
  }
}

void vtkLandmarkSegmentationController::setView2D(vtkImageView2D *view)
{
    if (this->view2d)
        this->view2d->RemoveObserver(this->Command);
    this->view2d = view;
    this->view2d->AddObserver(vtkImageView2D::SliceChangedEvent,this->Command);
}

void vtkLandmarkSegmentationController::setView3D(vtkImageView3D *view)
{
    this->view3d = view;
}

void vtkLandmarkSegmentationController::showOrHide2DWidget()
{
  for (int i=0; i<this->LandmarkCollection->GetNumberOfItems(); i++)
  {
    vtkLandmarkWidget* landmark = vtkLandmarkWidget::SafeDownCast (this->LandmarkCollection->GetItemAsObject(i));
    if (landmark->GetWidget2D()->GetInteractor() && landmark->GetWidget2D()->GetRepresentation()->GetRenderer())
    {
        vtkHandleWidget * widget  = landmark->GetWidget2D();
        if (widget->GetInteractor()->GetRenderWindow())
            if (landmark->GetIndices()[view2d->GetViewOrientation()]!=view2d->GetSlice())
                widget->Off();
            else
                widget->On();
    }
  }
}                                              

void vtkLandmarkSegmentationController::updateLandmarksPosFromWidget2D()
{
    for (int i=0; i<this->LandmarkCollection->GetNumberOfItems(); i++)
    {
        vtkLandmarkWidget* landmark = vtkLandmarkWidget::SafeDownCast (this->LandmarkCollection->GetItemAsObject(i));
        vtkPointHandleRepresentation2D * pointRep = dynamic_cast<vtkPointHandleRepresentation2D*> (landmark->GetWidget2D()->GetRepresentation());
        landmark->SetCenter(pointRep->GetWorldPosition());
        int * previous_indices = landmark->GetIndices();
        int indices[3];
        view2d->GetImageCoordinatesFromWorldCoordinates(pointRep->GetWorldPosition(),indices);
        int orientation = view2d->GetViewOrientation();
        indices[orientation] = previous_indices[orientation]; // the slice id of the current Orientation cannot change does not make sense. This line is here to prevent that.
        landmark->SetIndices(indices);
    }
}

//----------------------------------------------------------------------------
int vtkLandmarkSegmentationController::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  //try
  //{
  m_Filter->Update();
  implicitFunction = m_Filter->GetOutput();
  m_Converter->Update();
  //} catch (itk::ExceptionObject &e)
  /*{
    std::cerr << e << std::endl;
    return 0;
  }*/
  
  this->SurfaceExtractor->Update();

  vtkPolyData* data = this->SurfaceExtractor->GetOutput();
  vtkPoints* newpoints = vtkPoints::New();
  if (data->GetPoints()) this->Transformer->TransformPoints (data->GetPoints(), newpoints);
  data->SetPoints (newpoints);
  newpoints->Delete();
  output->SetPoints (data->GetPoints());
  output->SetPolys (data->GetPolys());
  return 1;
}

vtkLandmarkSegmentationController::binaryType::Pointer vtkLandmarkSegmentationController::GetBinaryImage()
{
    if (!implicitFunction)
        return NULL;
    
    itk::ResampleImageFilter<ImageType , ImageType>::Pointer filter = itk::ResampleImageFilter<ImageType , ImageType>::New();
    ImageType::SizeType size;
    size[0]=outputSize[0];
    size[1]=outputSize[1];
    size[2]=outputSize[2];
    ImageType::SpacingType outputSpacing;
    ImageType::SizeType filterInputSize = implicitFunction->GetLargestPossibleRegion().GetSize();
    outputSpacing[0] = implicitFunction->GetSpacing()[0] * (static_cast<double>(filterInputSize[0]) / static_cast<double>(outputSize[0]));
    outputSpacing[1] = implicitFunction->GetSpacing()[1] * (static_cast<double>(filterInputSize[1]) / static_cast<double>(outputSize[1]));
    outputSpacing[2] = implicitFunction->GetSpacing()[2] * (static_cast<double>(filterInputSize[2]) / static_cast<double>(outputSize[2]));
    filter->SetInput(implicitFunction);
    filter->SetSize(size);
    filter->SetOutputOrigin(m_Input->GetOrigin());
    filter->SetOutputDirection(m_Input->GetDirection());
    filter->SetOutputSpacing(outputSpacing);
    filter->UpdateLargestPossibleRegion();

    itk::BinaryThresholdImageFilter<ImageType,binaryType>::Pointer thresholdFilter = itk::BinaryThresholdImageFilter<ImageType,binaryType>::New();
    thresholdFilter->SetInput(filter->GetOutput());
    thresholdFilter->SetUpperThreshold(0);
    thresholdFilter->SetInsideValue(1);
    thresholdFilter->Update();
    binaryType::Pointer img = thresholdFilter->GetOutput();
    
    return img;
}
