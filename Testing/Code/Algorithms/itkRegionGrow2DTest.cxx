/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkRegionGrow2DTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
// Insight classes
#include "itkImage.h"
#include "itkVector.h"
#include "vnl/vnl_matrix_fixed.h"
#include "itkImageRegionIterator.h"
#include "itkTextOutput.h"
#include "itkRGBPixel.h"

#include "itkKLMRegionGrowImageFilter.h"
#include "itkScalarImageToHistogramGenerator.h"

#define   NUMBANDS1           1
#define   NUMBANDS2           2
#define   NUMBANDS3           3
#define   NUMDIM1D            1
#define   NUMDIM2D            2
#define   NUMDIM3D            3
#define   NUMDIM4D            4
#define   NUMDIM5D            5

static unsigned int test_RegionGrowKLMExceptionHandling();
static unsigned int test_regiongrowKLM1D();
static unsigned int test_regiongrowKLM2D();
static unsigned int test_regiongrowKLM3D();
static unsigned int test_regiongrowKLM4D();

//
// This tests KLM region growing segmentation
//

int itkRegionGrow2DTest(int, char* [] )
{
  int pass;

  // Exception test the KLM algorithm
  pass = test_RegionGrowKLMExceptionHandling();
  if ( pass == EXIT_FAILURE ) return pass;

  // Test the KLM algorithm applied to 1D data
  pass = test_regiongrowKLM1D();
  if ( pass == EXIT_FAILURE ) return pass;

  // Test the KLM algorithm applied to 2D data
  pass = test_regiongrowKLM2D();
  if ( pass == EXIT_FAILURE ) return pass;

  // Test the KLM algorithm applied to 3D data
  pass = test_regiongrowKLM3D();
  if ( pass == EXIT_FAILURE ) return pass;

  // Test the KLM algorithm applied to 4D data
  pass = test_regiongrowKLM4D();
  if ( pass == EXIT_FAILURE ) return pass;

  return EXIT_SUCCESS;
}


unsigned int test_RegionGrowKLMExceptionHandling()
{
  itk::OutputWindow::SetInstance(itk::TextOutput::New().GetPointer());

  std::cout << "Testing exception handling" << std::endl;

  // Perform the exception handling testing on a 5D image

  // Generate the image data

  int sizeLen  = 3;

  typedef itk::Image<itk::Vector<double,NUMBANDS2>,NUMDIM5D> ImageType5D;
  ImageType5D::Pointer image5D  = ImageType5D::New();

  ImageType5D::SizeType imageSize5D;
  imageSize5D.Fill( sizeLen );

  ImageType5D::IndexType index5D;
  index5D.Fill(0);

  ImageType5D::RegionType region5D;

  region5D.SetSize( imageSize5D );
  region5D.SetIndex( index5D );

  image5D->SetLargestPossibleRegion( region5D );
  image5D->SetBufferedRegion( region5D );
  image5D->Allocate();


  // Set the filter with valid inputs

  typedef itk::KLMRegionGrowImageFilter<ImageType5D,ImageType5D>
    KLMRegionGrowImageFilterType5D;

  KLMRegionGrowImageFilterType5D::Pointer
    exceptionTestingFilter5D = KLMRegionGrowImageFilterType5D::New();

  KLMRegionGrowImageFilterType5D::GridSizeType gridSize5D;
  gridSize5D.Fill(1);

  exceptionTestingFilter5D->SetInput(image5D);
  exceptionTestingFilter5D->SetGridSize(gridSize5D);
  exceptionTestingFilter5D->SetMaximumNumberOfRegions(2);
  exceptionTestingFilter5D->SetMaximumLambda(1000);

  std::cout << "Test error handling" << std::endl;

  bool passed;

#undef LOCAL_TEST_EXCEPTION_MACRO
#define LOCAL_TEST_EXCEPTION_MACRO( MSG, FILTER ) \
  passed = false; \
  try \
    { \
    std::cout << MSG << std::endl; \
    FILTER->Update(); \
    } \
  catch( itk::ExceptionObject& err ) \
    { \
    std::cout << "Caught expected error." << std::endl; \
    std::cout << err << std::endl; \
    FILTER->ResetPipeline(); \
    passed = true; \
    } \
  if ( !passed ) \
    { \
    std::cout << "Test FAILED" << std::endl; \
    return EXIT_FAILURE; \
    }


  // maximum number of regions must be greater than 1

  exceptionTestingFilter5D->SetMaximumNumberOfRegions(0);
  LOCAL_TEST_EXCEPTION_MACRO( "Maximum number of user specified region is 0",
    exceptionTestingFilter5D );

  exceptionTestingFilter5D->SetMaximumNumberOfRegions(1);
  LOCAL_TEST_EXCEPTION_MACRO( "Maximum number of user specified region is 1",
    exceptionTestingFilter5D );

  exceptionTestingFilter5D->SetMaximumNumberOfRegions(2);

  // size lengths must be divisible by the grid size along each dimension

  for( int idim = 0; idim < NUMDIM5D; idim++ )
    {
    gridSize5D[idim]++;
    exceptionTestingFilter5D->SetGridSize( gridSize5D );
    LOCAL_TEST_EXCEPTION_MACRO( "Invalid grid size, dimension " << idim+1 <<
                                " of " << NUMDIM5D, exceptionTestingFilter5D );
    gridSize5D[idim]--;
    }
  exceptionTestingFilter5D->SetGridSize( gridSize5D );

  // gridSize can't be 0

  gridSize5D[2] = 0;
  exceptionTestingFilter5D->SetGridSize( gridSize5D );
  LOCAL_TEST_EXCEPTION_MACRO( "Invalid grid size = 0",
    exceptionTestingFilter5D );
  gridSize5D[2] = 1;
  exceptionTestingFilter5D->SetGridSize( gridSize5D );

  // one region in there - what happens?

  gridSize5D.Fill( sizeLen );
  exceptionTestingFilter5D->SetGridSize( gridSize5D );
  LOCAL_TEST_EXCEPTION_MACRO( "One input region, grid too large",
    exceptionTestingFilter5D );

  // the following shouldn't compile - RGB is a fixed array type, only
  // itk::Vector<type,length> pixels are accepted
  //
  // use itkRGBToVectorImageAdaptor.h to process RGB images
  //
  // // pass in an image where the pixel type is RGB
  //
  // typedef itk::Image< itk::RGBPixel<unsigned char>, NUMDIM2D> ImageTypeRGB2D;
  //
  // ImageTypeRGB2D::Pointer imageRGB2D  = ImageTypeRGB2D::New();
  //
  // ImageTypeRGB2D::SizeType imageSizeRGB2D;
  // imageSizeRGB2D.Fill( sizeLen );
  //
  // ImageTypeRGB2D::IndexType indexRGB2D;
  // indexRGB2D.Fill(0);
  //
  // ImageTypeRGB2D::RegionType regionRGB2D;
  //
  // regionRGB2D.SetSize( imageSizeRGB2D );
  // regionRGB2D.SetIndex( indexRGB2D );
  //
  // imageRGB2D->SetLargestPossibleRegion( regionRGB2D );
  // imageRGB2D->SetBufferedRegion( regionRGB2D );
  // imageRGB2D->Allocate();
  //
  // // Set the filter with valid inputs
  //
  // typedef itk::KLMRegionGrowImageFilter<ImageTypeRGB2D,ImageTypeRGB2D>
  //   KLMRegionGrowImageFilterTypeRGB2D;
  //
  // KLMRegionGrowImageFilterTypeRGB2D::Pointer
  //   exceptionTestingFilterRGB2D = KLMRegionGrowImageFilterTypeRGB2D::New();
  //
  // KLMRegionGrowImageFilterTypeRGB2D::GridSizeType gridSizeRGB2D;
  // gridSizeRGB2D.Fill(1);
  //
  // exceptionTestingFilterRGB2D->SetInput(imageRGB2D);
  // exceptionTestingFilterRGB2D->SetGridSize(gridSizeRGB2D);
  // exceptionTestingFilterRGB2D->SetMaximumNumberOfRegions(2);
  // exceptionTestingFilterRGB2D->SetMaximumLambda(1000);
  //
  // exceptionTestingFilterRGB2D->SetMaximumNumberOfRegions(0);
  // LOCAL_TEST_EXCEPTION_MACRO( "Maximum number of user specified region is 0",
  //   exceptionTestingFilterRGB2D );


  // the following shouldn't compile - vector pixel required
  //
  // // pass in image where pixel is not a vector
  //
  // typedef itk::Image< unsigned char, NUMDIM2D> ImageType2D;
  //
  // ImageType2D::Pointer image2D  = ImageType2D::New();
  //
  // ImageType2D::SizeType imageSize2D;
  // imageSize2D.Fill( sizeLen );
  //
  // ImageType2D::IndexType index2D;
  // index2D.Fill(0);
  //
  // ImageType2D::RegionType region2D;
  //
  // region2D.SetSize( imageSize2D );
  // region2D.SetIndex( index2D );
  //
  // image2D->SetLargestPossibleRegion( region2D );
  // image2D->SetBufferedRegion( region2D );
  // image2D->Allocate();
  //
  // // Set the filter with valid inputs
  //
  // typedef itk::KLMRegionGrowImageFilter<ImageType2D,ImageType2D>
  //   KLMRegionGrowImageFilterType2D;
  //
  // KLMRegionGrowImageFilterType2D::Pointer
  //   exceptionTestingFilter2D = KLMRegionGrowImageFilterType2D::New();
  //
  // KLMRegionGrowImageFilterType2D::GridSizeType gridSize2D;
  // gridSize2D.Fill(1);
  //
  // exceptionTestingFilter2D->SetInput(image2D);
  // exceptionTestingFilter2D->SetGridSize(gridSize2D);
  // exceptionTestingFilter2D->SetMaximumNumberOfRegions(2);
  // exceptionTestingFilter2D->SetMaximumLambda(1000);
  //
  // exceptionTestingFilter2D->SetMaximumNumberOfRegions(0);
  // LOCAL_TEST_EXCEPTION_MACRO( "Maximum number of user specified region is 0",
  //   exceptionTestingFilter2D );


  // the following shouldn't compile - concept checking should be triggered
  //
  // // input/output pixel types don't match
  //
  // typedef itk::Image<itk::Vector<double,NUMBANDS1>,NUMDIM5D> ImageType5Da;
  //
  // typedef itk::KLMRegionGrowImageFilter<ImageType5D,ImageType5Da>
  //   KLMRegionGrowImageFilterTypeBadVecDim;
  //
  // KLMRegionGrowImageFilterTypeBadVecDim::Pointer
  //   exceptionTestingFilterBadVecDim =
  //     KLMRegionGrowImageFilterTypeBadVecDim::New();
  //
  //   KLMRegionGrowImageFilterTypeBadVecDim::GridSizeType gridSizeBadVecDim;
  //   gridSizeBadVecDim.Fill(1);
  //
  //   exceptionTestingFilterBadVecDim->SetInput(image5D);
  //   exceptionTestingFilterBadVecDim->SetGridSize(gridSizeBadVecDim);
  //   exceptionTestingFilterBadVecDim->SetMaximumNumberOfRegions(2);
  //   exceptionTestingFilterBadVecDim->SetMaximumLambda(1000);
  //
  //   LOCAL_TEST_EXCEPTION_MACRO( "Input/Output vector dimensions don't match",
  //     exceptionTestingFilterBadVecDim );


  // the following shouldn't compile - concept checking should be triggered
  //
  // // input/output (5D/4D) dimensions don't match
  //
  // typedef itk::Image<itk::Vector<double,NUMBANDS2>,NUMDIM4D> ImageType4D;
  //
  // typedef itk::KLMRegionGrowImageFilter<ImageType5D,ImageType4D>
  //   KLMRegionGrowImageFilterTypeBadSizeDim;
  //
  // KLMRegionGrowImageFilterTypeBadSizeDim::Pointer
  //   exceptionTestingFilterBadSizeDim =
  //     KLMRegionGrowImageFilterTypeBadSizeDim::New();
  //
  // KLMRegionGrowImageFilterTypeBadSizeDim::GridSizeType gridSizeBadSizeDim;
  // gridSizeBadSizeDim.Fill(2);
  //
  // exceptionTestingFilterBadSizeDim->SetInput(image);
  // exceptionTestingFilterBadSizeDim->SetGridSize(gridSizeBadSizeDim);
  // exceptionTestingFilterBadSizeDim->SetMaximumNumberOfRegions(2);
  // exceptionTestingFilterBadSizeDim->SetMaximumLambda(1000);
  //
  // LOCAL_TEST_EXCEPTION_MACRO( "Input/Output dimensions don't match",
  //    exceptionTestingFilterBadSizeDim );

  std::cout << "Done testing exception handling" << std::endl;

  return EXIT_SUCCESS;
}


unsigned int test_regiongrowKLM1D()
{
  itk::OutputWindow::SetInstance(itk::TextOutput::New().GetPointer());

  std::cout << std::endl << "Begin testing one-dimension images" << std::endl;

  // Manually create an image

  typedef itk::Image<itk::Vector<unsigned char,NUMBANDS3>,NUMDIM1D> ImageType;
  typedef itk::Image<itk::Vector<double,NUMBANDS3>,NUMDIM1D> OutputImageType;

  ImageType::Pointer image  = ImageType::New();

  unsigned int numPixels = 100;
  unsigned int numPixelsHalf = 50;
  ImageType::SizeType imageSize;
  imageSize.Fill( numPixels );

  ImageType::IndexType index;
  index.Fill(0);

  ImageType::RegionType region;
  region.SetSize( imageSize );
  region.SetIndex( index );

  image->SetLargestPossibleRegion( region );
  image->SetBufferedRegion( region );
  image->Allocate();

  typedef ImageType::PixelType ImagePixelType;
  typedef itk::ImageRegionIterator< ImageType > ImageIterator;
  ImageIterator inIt( image, image->GetBufferedRegion() );
  ImageIterator inItEnd = inIt.End();

  typedef ImageType::PixelType::VectorType ImageData;
  ImageData   pixelData;
  unsigned int k = 0;
  while ( inIt != inItEnd ) {
    pixelData[0] = static_cast<unsigned char>( k );
    pixelData[1] = static_cast<unsigned char>( numPixels - k - 1 );
    if ( k < numPixelsHalf ) pixelData[2] = 47;
    else pixelData[2] = 247;
    inIt.Set( pixelData );
    ++inIt;
    ++k;
  }


  // FIRST TEST:
  // If lambda is 0, the number of final regions should equal
  // initial number of regions, the region labels should be consecutive

  // Set up the filter

  typedef itk::KLMRegionGrowImageFilter<ImageType,OutputImageType>
    KLMRegionGrowImageFilterType;

  KLMRegionGrowImageFilterType::Pointer KLMFilter =
    KLMRegionGrowImageFilterType::New();

  KLMRegionGrowImageFilterType::GridSizeType gridSize;
  gridSize.Fill(1);

  KLMFilter->SetInput( image );
  KLMFilter->SetGridSize(gridSize);

#undef LOCAL_TEST_EXCEPTION_MACRO
#define LOCAL_TEST_EXCEPTION_MACRO( FILTER ) \
  try \
    { \
    FILTER->Update(); \
    } \
  catch( itk::ExceptionObject& err ) \
    { \
    std::cout << "Caught unexpected error." << std::endl; \
    std::cout << err << std::endl; \
    return EXIT_FAILURE; \
    } \
  std::cout << std::endl << "Filter has been udpated" << std::endl


  std::cout << std::endl << "First test, lambda = 0" << std::endl;

  KLMFilter->SetMaximumLambda( 0 );
  int nregions = 2;
  KLMFilter->SetMaximumNumberOfRegions( nregions );

  // Kick off the Region grow function
  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );
  KLMFilter->Print(std::cout);

  if( numPixels != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  // KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;
  std::cout << "Extracting and checking label image" << std::endl;

  // Make sure that the labelled image type is set to unsigned integer
  // as labels associated with different regions are always integers
  // This should return unique integer labels of the segmented regions.
  // The region labels should be consecutive integers beginning with 1.

  OutputImageType::Pointer outImage = KLMFilter->GetOutput();
  typedef itk::ImageRegionIterator< OutputImageType > OutputImageIterator;
  OutputImageIterator outIt( outImage, outImage->GetBufferedRegion() );

  typedef KLMRegionGrowImageFilterType::RegionLabelType LabelType;
  typedef itk::Image<LabelType, NUMDIM1D> LabelledImageType;
  LabelledImageType::Pointer labelledImage = KLMFilter->GetLabelledImage();

  typedef OutputImageType::PixelType::VectorType OutputImageData;
  ImageData         pixelIn;
  OutputImageData   pixelOut;

  typedef LabelledImageType::PixelType LabelledImagePixelType;
  typedef itk::ImageRegionIterator< LabelledImageType > LabelImageIterator;
  LabelImageIterator
    labelIt( labelledImage, labelledImage->GetBufferedRegion() );
  LabelType pixelLabel;
  LabelType m = 1;

  inIt.GoToBegin();
  while ( inIt != inItEnd )
    {
    pixelOut = outIt.Get();
    pixelIn  = inIt.Get();
    pixelLabel = labelIt.Get();

    if ( pixelOut[0] != pixelIn[0] ||
         pixelOut[1] != pixelIn[1] ||
         pixelOut[2] != pixelIn[2] ||
         pixelLabel  != m )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt;
    ++inIt;
    ++labelIt;
    ++m;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();



  // SECOND TEST:
  // merge as much as possible

  std::cout << std::endl << "Second test, merge to " << nregions <<
    " regions" << std::endl;

  KLMFilter->SetMaximumLambda( 1e51 );

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  if( KLMFilter->GetNumberOfRegions() !=
      KLMFilter->GetMaximumNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }


  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;
  std::cout << "Extracting and checking label image" << std::endl;

  OutputImageType::Pointer outImage2 = KLMFilter->GetOutput();
  OutputImageIterator outIt2( outImage2, outImage2->GetBufferedRegion() );
  OutputImageIterator outItEnd2 = outIt2.End();
  OutputImageData   pixelOut2a;
  OutputImageData   pixelOut2b;

  LabelledImageType::Pointer labelledImage2 = KLMFilter->GetLabelledImage();
  LabelImageIterator labelIt2( labelledImage2,
    labelledImage2->GetBufferedRegion() );

  pixelOut2a[0] = (numPixelsHalf-1)*numPixelsHalf/2;
  pixelOut2a[1] = (numPixels-1)*numPixels/2 - pixelOut2a[0];
  pixelOut2a[0] /= numPixelsHalf;
  pixelOut2a[1] /= numPixelsHalf;
  pixelOut2a[2] = 47;

  pixelOut2b[0] = pixelOut2a[1];
  pixelOut2b[1] = pixelOut2a[0];
  pixelOut2b[2] = 247;

  LabelType ma = 1;
  LabelType mb = 2;

  k = 0;
  while ( outIt2 != outItEnd2 )
    {
    pixelOut = outIt2.Get();
    pixelLabel = labelIt2.Get();

    if ( k < numPixelsHalf )
      {
      if ( pixelOut != pixelOut2a || pixelLabel != ma )
        {
        std::cout << "Test FAILED" << std::endl;
        return EXIT_FAILURE;
        }
      }
    else
      {
      if ( pixelOut != pixelOut2b || pixelLabel != mb )
        {
        std::cout << "Test FAILED" << std::endl;
        return EXIT_FAILURE;
        }
      }
    ++outIt2;
    ++labelIt2;
    ++k;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();

  // THIRD TEST:
  // merge to 4 regions

  nregions = 4;
  std::cout << std::endl << "Third test, merge to " << nregions <<
    " regions" << std::endl;

  unsigned int numPixelsQtr = numPixelsHalf / 2;
  k = 0;
  inIt.GoToBegin();
  while ( inIt != inItEnd ) {
    pixelData[0] = static_cast<unsigned char>( k );
    pixelData[1] = static_cast<unsigned char>( numPixels - k - 1 );
    if ( k < numPixelsQtr )          pixelData[2] = 127;
    else if ( k < 2 * numPixelsQtr ) pixelData[2] = 27;
    else if ( k < 3 * numPixelsQtr ) pixelData[2] = 127;
    else                             pixelData[2] = 227;
    inIt.Set( pixelData );
    ++inIt;
    ++k;
  }

  KLMFilter->SetMaximumLambda( 1e51 );
  KLMFilter->SetMaximumNumberOfRegions( nregions );

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  if( KLMFilter->GetNumberOfRegions() !=
      KLMFilter->GetMaximumNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }


  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;
  std::cout << "Extracting and checking label image" << std::endl;

  OutputImageType::Pointer outImage3 = KLMFilter->GetOutput();
  OutputImageIterator outIt3( outImage3, outImage3->GetBufferedRegion() );
  OutputImageIterator outItEnd3 = outIt3.End();
  OutputImageData   pixelOut3a;
  OutputImageData   pixelOut3b;
  OutputImageData   pixelOut3c;
  OutputImageData   pixelOut3d;

  LabelledImageType::Pointer labelledImage3 = KLMFilter->GetLabelledImage();
  LabelImageIterator labelIt3( labelledImage3,
    labelledImage3->GetBufferedRegion() );

  pixelOut3a[0] = (numPixelsQtr - 1) * numPixelsQtr / 2;
  pixelOut3a[1] = (numPixels - 1) * numPixels / 2 -
    (3 * numPixelsQtr - 1) * (3 * numPixelsQtr) / 2;
  pixelOut3a[0] /= numPixelsQtr;
  pixelOut3a[1] /= numPixelsQtr;
  pixelOut3a[2] = 127;

  pixelOut3b[0] = (numPixelsHalf - 1) * numPixelsHalf / 2 -
    (numPixelsQtr - 1) * numPixelsQtr / 2;
  pixelOut3b[1] = (3 * numPixelsQtr - 1) * (3 * numPixelsQtr) / 2 -
    (numPixelsHalf - 1) * numPixelsHalf / 2;
  pixelOut3b[0] /= numPixelsQtr;
  pixelOut3b[1] /= numPixelsQtr;
  pixelOut3b[2] = 27;

  pixelOut3c[0] = pixelOut3b[1];
  pixelOut3c[1] = pixelOut3b[0];
  pixelOut3c[2] = 127;

  pixelOut3d[0] = pixelOut3a[1];
  pixelOut3d[1] = pixelOut3a[0];
  pixelOut3d[2] = 227;

  LabelType mc = 3;
  LabelType md = 4;

  k = 0;
  while ( outIt3 != outItEnd3 )
    {
    pixelOut = outIt3.Get();
    pixelLabel = labelIt3.Get();

    if ( k < numPixelsHalf / 2 )
      {
      if ( pixelOut != pixelOut3a || pixelLabel != ma )
        {
        std::cout << "Test FAILED" << std::endl;
        return EXIT_FAILURE;
        }
      }
    else if ( k < numPixelsHalf )
      {
      if ( pixelOut != pixelOut3b || pixelLabel != mb )
        {
        std::cout << "Test FAILED" << std::endl;
        return EXIT_FAILURE;
        }
      }
    else if ( k < 3 * numPixelsHalf / 2 )
      {
      if ( pixelOut != pixelOut3c || pixelLabel != mc )
        {
        std::cout << "Test FAILED" << std::endl;
        return EXIT_FAILURE;
        }
      }
    else
      {
      if ( pixelOut != pixelOut3d || pixelLabel != md )
        {
        std::cout << "Test FAILED" << std::endl;
        return EXIT_FAILURE;
        }
      }

    ++outIt3;
    ++labelIt3;
    ++k;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();

  // FOURTH TEST:
  // set image spacing

  std::cout << std::endl << "Fourth test, spacing set to 0.25: "
    << "area of final regions should be 1 as seen in printed region stats"
    << std::endl;

  ImageType::SpacingType spacing = image->GetSpacing();
  spacing.Fill(.04);
  image->SetSpacing(spacing);

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  if( KLMFilter->GetNumberOfRegions() !=
      KLMFilter->GetMaximumNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }


  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();


  // FIFTH TEST:
  // large gridsize no merging

  int gridWidth = 5;
  gridSize.Fill(gridWidth);
  std::cout << std::endl << "Fifth test, gridSize = "
            << gridWidth << " no merging" << std::endl;

  KLMFilter->SetMaximumLambda( -1 );
  KLMFilter->SetGridSize(gridSize);

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  if( KLMFilter->GetNumberOfRegions() != numPixels / gridWidth )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }


  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();


  std::cout << "Extracting and checking approximation image" << std::endl;
  std::cout << "Extracting and checking label image" << std::endl;

  OutputImageType::Pointer outImage5 = KLMFilter->GetOutput();
  OutputImageIterator outIt5( outImage5, outImage5->GetBufferedRegion() );
  OutputImageIterator outItEnd5 = outIt5.End();

  LabelledImageType::Pointer labelledImage5 = KLMFilter->GetLabelledImage();
  LabelImageIterator labelIt5( labelledImage5,
    labelledImage5->GetBufferedRegion() );

  OutputImageData   pixelOut5in;
  OutputImageData   pixelOut5out;

  k = 1;
  inIt.GoToBegin();
  while ( outIt5 != outItEnd5 )
    {

    pixelOut5in[0] = 0;
    pixelOut5in[1] = 0;
    pixelOut5in[2] = 0;

    for ( int idx = 0; idx < gridWidth; idx++ )
      {
      pixelIn = inIt.Get();
      pixelOut5in[0] += pixelIn[0];
      pixelOut5in[1] += pixelIn[1];
      pixelOut5in[2] += pixelIn[2];

      pixelLabel = labelIt5.Get();

      if ( pixelLabel != k )
        {
        std::cout << "Test FAILED" << std::endl;
        return EXIT_FAILURE;
        }
      ++labelIt5;
      ++inIt;
      }

    pixelOut5in /= gridWidth;
    pixelOut5in /= spacing[0];

    for ( int idx = 0; idx < gridWidth; idx++ )
      {
      pixelOut5out = outIt5.Get();
      if ( pixelOut5in != pixelOut5out )
        {
        std::cout << "Test FAILED" << std::endl;
        return EXIT_FAILURE;
        }
      ++outIt5;
      }

    ++k;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  std::cout << std::endl << "Done testing of one-dimension images" << std::endl;
  return EXIT_SUCCESS;
}


unsigned int test_regiongrowKLM2D()
{
  itk::OutputWindow::SetInstance(itk::TextOutput::New().GetPointer());

  std::cout << std::endl << "Begin testing two-dimension images" << std::endl;

  // Manually create an image

  typedef itk::Image<itk::Vector<int,NUMBANDS2>,NUMDIM2D> ImageType;
  typedef itk::Image<itk::Vector<double,NUMBANDS2>,NUMDIM2D> OutputImageType;

  ImageType::Pointer image  = ImageType::New();

  ImageType::SizeType imageSize;
  imageSize[0] = 10;
  imageSize[1] = 20;
  unsigned int numPixels = 200;

  ImageType::IndexType index;
  index.Fill(0);

  ImageType::RegionType region;
  region.SetSize( imageSize );
  region.SetIndex( index );

  image->SetLargestPossibleRegion( region );
  image->SetBufferedRegion( region );
  image->Allocate();

  typedef ImageType::PixelType ImagePixelType;
  typedef itk::ImageRegionIterator< ImageType > ImageIterator;
  ImageIterator inIt( image, image->GetBufferedRegion() );
  ImageIterator inItEnd = inIt.End();

  /* the key test for merging with duplicate borders will
     come from an image that looks like this:

      x x x x x x x x x x x x x x x x x x x x
      x y y y y y y y y y y y y y y y y y y x
      x y q q q q q q y y y y q q q q q q y x
      x y q q q q q q y y y y q q q q q q y x
      x y q z q q z q y y y y q z q q z q y x
      x y m z m m z m y y y y m z m m z m y x
      x y m m m m m m y y y y m m m m m m y x
      x y m m m m m m y y y y m m m m m m y x
      x y y y y y y y y y y y y y y y y y y x
      x x x x x x x x x x x x x x x x x x x x

      Merge this to 8 regions
       1 with value x
       1 with value y
       2 each with value qm
       4 each with value z

  */
  int inImageVals[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 3, 3, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 3, 3, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 3, 3, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 3, 3, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

  // Set up the filter

  typedef itk::KLMRegionGrowImageFilter<ImageType,OutputImageType>
    KLMRegionGrowImageFilterType;

  KLMRegionGrowImageFilterType::Pointer KLMFilter =
    KLMRegionGrowImageFilterType::New();

  KLMRegionGrowImageFilterType::GridSizeType gridSize;
  gridSize.Fill(1);

  KLMFilter->SetInput( image );
  KLMFilter->SetMaximumNumberOfRegions( 2 );
  KLMFilter->SetGridSize(gridSize);

  typedef KLMRegionGrowImageFilterType::RegionLabelType LabelType;

  LabelType labelVals[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 3, 3, 3, 3, 3, 3, 2, 1,
                            1, 2, 3, 3, 3, 3, 3, 3, 2, 1,
                            1, 2, 3, 3, 4, 4, 3, 3, 2, 1,
                            1, 2, 3, 3, 3, 3, 3, 3, 2, 1,
                            1, 2, 3, 3, 5, 5, 3, 3, 2, 1,
                            1, 2, 3, 3, 3, 3, 3, 3, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 6, 6, 6, 6, 6, 6, 2, 1,
                            1, 2, 6, 6, 7, 7, 6, 6, 2, 1,
                            1, 2, 6, 6, 6, 6, 6, 6, 2, 1,
                            1, 2, 6, 6, 6, 6, 6, 6, 2, 1,
                            1, 2, 6, 6, 8, 8, 6, 6, 2, 1,
                            1, 2, 6, 6, 6, 6, 6, 6, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

  double outImageVals[] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 3.0, 3.0, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 3.0, 3.0, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 3.0, 3.0, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 3.0, 3.0, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1 };

  typedef ImageType::PixelType::VectorType ImageData;
  ImageData   pixelData;
  int k = 0;
  while ( inIt != inItEnd ) {

    pixelData[0] = inImageVals[k];
    pixelData[1] = 100 - pixelData[0];

    inIt.Set( pixelData );
    ++inIt;
    ++k;
  }


  // FIRST TEST:
  // If lambda is negative, the number of final regions should equal
  // initial number of regions, the region labels should be consecutive

  std::cout << std::endl << "First test, lambda = -1" << std::endl;

  KLMFilter->SetMaximumLambda( -1 );

  // Kick off the Region grow function

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );
  KLMFilter->Print(std::cout);

  // This should return unique integer labels of the segmented regions.
  // The region labels should be consecutive integers beginning with 1.

  if( numPixels != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;

  OutputImageType::Pointer outImage = KLMFilter->GetOutput();

  typedef itk::ImageRegionIterator< OutputImageType > OutputImageIterator;

  inIt.GoToBegin();
  OutputImageIterator outIt( outImage, outImage->GetBufferedRegion() );

  typedef OutputImageType::PixelType::VectorType OutputImageData;
  ImageData         pixelIn;
  OutputImageData   pixelOut;

  while ( inIt != inItEnd )
    {
    pixelOut = outIt.Get();
    pixelIn  = inIt.Get();

    if ( pixelOut[0] != pixelIn[0] || pixelOut[1] != pixelIn[1] )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt;
    ++inIt;

    } //end while iterator loop


  // Make sure that the labelled image type is set to unsigned integer
  // as labels associated with different regions are always integers

  std::cout << "Extracting and checking label image" << std::endl;

  typedef itk::Image<LabelType, NUMDIM2D> LabelledImageType;
  LabelledImageType::Pointer labelledImage = KLMFilter->GetLabelledImage();

  // Loop through the approximation image and check if they match the
  // input image

  // setup the iterators

  typedef LabelledImageType::PixelType LabelledImagePixelType;

  typedef itk::ImageRegionIterator< LabelledImageType > LabelImageIterator;

  LabelImageIterator
    labelIt( labelledImage, labelledImage->GetBufferedRegion() );

  LabelType pixelLabel;
  LabelType m = 1;
  while(!labelIt.IsAtEnd())
    {
    pixelLabel = labelIt.Get();

    if(pixelLabel != m)
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++labelIt;
    ++m;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();

  // SECOND TEST:
  // the key test for union borders

  std::cout << std::endl <<
    "Second test, key merging test containing duplicate borders" << std::endl;

  KLMFilter->SetMaximumLambda( 1e45 );
  unsigned int nregions = 8;
  KLMFilter->SetMaximumNumberOfRegions( nregions );

  // Kick off the Region grow function

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  KLMFilter->Print(std::cout);

  // This should return unique integer labels of the segmented regions.
  // The region labels should be consecutive integers beginning with 1.

  if( nregions != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;

  OutputImageType::Pointer outImage2 = KLMFilter->GetOutput();

  OutputImageIterator outIt2( outImage2, outImage2->GetBufferedRegion() );
  OutputImageIterator outItEnd2 = outIt2.End();

  k = 0;
  while ( outIt2 != outItEnd2 )
    {
    pixelOut = outIt2.Get();

    if ( pixelOut[0] != outImageVals[k] ||
         pixelOut[1] != 100 - pixelOut[0] )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt2;
    ++k;

    } //end while iterator loop


  std::cout << "Extracting and checking label image" << std::endl;

  LabelledImageType::Pointer labelledImage2 = KLMFilter->GetLabelledImage();

  LabelImageIterator
    labelIt2( labelledImage2, labelledImage2->GetBufferedRegion() );

  k = 0;
  while(!labelIt2.IsAtEnd())
    {
    pixelLabel = labelIt2.Get();

    if(pixelLabel != labelVals[k])
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++labelIt2;
    ++k;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();


  // THIRD TEST:
  // degenerate case, all 0 image, all constant lambda values -
  // issues related to speed of algorithm

  std::cout << std::endl << "Third test, all 0 image" << std::endl;

  inIt.GoToBegin();
  pixelData[0] = 0;
  pixelData[1] = 0;
  while ( inIt != inItEnd ) {
    inIt.Set( pixelData );
    ++inIt;
  }

  gridSize[0] = 1;
  gridSize[1] = 2;

  KLMFilter->SetInput( image );
  KLMFilter->SetMaximumNumberOfRegions( 25 );
  KLMFilter->SetGridSize( gridSize );
  KLMFilter->SetMaximumLambda( 1e45 );

  // Kick off the Region grow function

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  // KLMFilter->Print(std::cout);

  if( KLMFilter->GetMaximumNumberOfRegions() !=
      KLMFilter->GetNumberOfRegions() )
    {
    std::cout << KLMFilter->GetMaximumNumberOfRegions()
              << KLMFilter->GetNumberOfRegions() << std::endl;
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  // KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;

  OutputImageType::Pointer outImage3 = KLMFilter->GetOutput();

  OutputImageIterator outIt3( outImage3, outImage3->GetBufferedRegion() );
  OutputImageIterator outItEnd3 = outIt3.End();

  k = 0;
  while ( outIt3 != outItEnd3 )
    {
    pixelOut = outIt3.Get();

    if ( pixelOut[0] != 0 || pixelOut[1] != 0 )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt3;
    ++k;

    } //end while iterator loop


  std::cout << "Extracting and checking label image" << std::endl;

  typedef itk::Statistics::ScalarImageToHistogramGenerator<LabelledImageType>
    HistogramGeneratorType;

  HistogramGeneratorType::Pointer histogramGenerator =
    HistogramGeneratorType::New();

  histogramGenerator->SetInput( KLMFilter->GetLabelledImage() );
  histogramGenerator->SetNumberOfBins( KLMFilter->GetNumberOfRegions() );
  histogramGenerator->SetMarginalScale( 1.0 );
  histogramGenerator->Compute();

  typedef HistogramGeneratorType::HistogramType  HistogramType;

  const HistogramType * histogram = histogramGenerator->GetOutput();

  const unsigned int histogramSize = histogram->Size();
  if ( histogramSize != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  HistogramType::ConstIterator histIt = histogram->Begin();
  HistogramType::ConstIterator histItEnd = histogram->End();

  double Sum = histogram->GetTotalFrequency();
  double labelEntropy = 0.0;

  while( histIt != histItEnd )
    {

    double probability = histIt.GetFrequency() / Sum;

    if( probability == 0 )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    labelEntropy -= probability * log( probability );

    ++histIt;
    }

  labelEntropy /= log( 2.0 );
  double idealEntropy = - log( 8.0 / numPixels ) / log( 2.0 );

  std::cout << "Label entropy = " << labelEntropy << " bits " << std::endl;
  std::cout << "Ideal entropy = " << idealEntropy << " bits " << std::endl;

  if ( fabs( idealEntropy - labelEntropy ) > 0.2 )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

  std::cout << "Test PASSED" << std::endl;

  std::cout << std::endl << "Done testing of two-dimension images" << std::endl;
  return EXIT_SUCCESS;

} // End test_regiongrow2D()


unsigned int test_regiongrowKLM3D()
{
  itk::OutputWindow::SetInstance(itk::TextOutput::New().GetPointer());

  std::cout << std::endl << "Begin testing three-dimension images" << std::endl;

  // Manually create an image

  typedef itk::Image<itk::Vector<short int,NUMBANDS2>,NUMDIM3D> ImageType;
  typedef itk::Image<itk::Vector<float,NUMBANDS2>,NUMDIM3D> OutputImageType;

  ImageType::Pointer image  = ImageType::New();

  ImageType::SizeType imageSize;
  imageSize[0] = 10;
  imageSize[1] = 20;
  imageSize[2] = 3;
  unsigned int numPixels = 10 * 20 * 3;

  ImageType::IndexType index;
  index.Fill(0);

  ImageType::RegionType region;
  region.SetSize( imageSize );
  region.SetIndex( index );

  image->SetLargestPossibleRegion( region );
  image->SetBufferedRegion( region );
  image->Allocate();

  typedef ImageType::PixelType ImagePixelType;
  typedef itk::ImageRegionIterator< ImageType > ImageIterator;
  ImageIterator inIt( image, image->GetBufferedRegion() );
  ImageIterator inItEnd = inIt.End();

  /* the key test for merging with duplicate borders will
     come from an image that looks like this:

      x x x x x x x x x x x x x x x x x x x x
      x y y y y y y y y y y y y y y y y y y x
      x y q q q q q q y y y y q q q q q q y x
      x y q q q q q q y y y y q q q q q q y x
      x y q z q q z q y y y y q z q q z q y x
      x y m z m m z m y y y y m z m m z m y x
      x y m m m m m m y y y y m m m m m m y x
      x y m m m m m m y y y y m m m m m m y x
      x y y y y y y y y y y y y y y y y y y x
      x x x x x x x x x x x x x x x x x x x x

      Merge this to 8 regions
       1 with value x
       1 with value y
       2 each with value qm
       4 each with value z

  */
  int inImageVals[] = {
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 30, 30, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 30, 30, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 30, 30, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 7, 7, 30, 30, 6, 6, 9, 1,
                        1, 9, 7, 7, 7, 6, 6, 6, 9, 1,
                        1, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

  // Set up the filter

  typedef itk::KLMRegionGrowImageFilter<ImageType,OutputImageType>
    KLMRegionGrowImageFilterType;

  KLMRegionGrowImageFilterType::Pointer KLMFilter =
    KLMRegionGrowImageFilterType::New();

  KLMRegionGrowImageFilterType::GridSizeType gridSize;
  gridSize.Fill(1);

  KLMFilter->SetInput( image );
  KLMFilter->SetMaximumNumberOfRegions( 2 );
  KLMFilter->SetGridSize(gridSize);

  typedef KLMRegionGrowImageFilterType::RegionLabelType LabelType;

  LabelType labelVals[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 3, 3, 3, 3, 3, 3, 2, 1,
                            1, 2, 3, 3, 3, 3, 3, 3, 2, 1,
                            1, 2, 3, 3, 4, 4, 3, 3, 2, 1,
                            1, 2, 3, 3, 3, 3, 3, 3, 2, 1,
                            1, 2, 3, 3, 5, 5, 3, 3, 2, 1,
                            1, 2, 3, 3, 3, 3, 3, 3, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 2, 6, 6, 6, 6, 6, 6, 2, 1,
                            1, 2, 6, 6, 7, 7, 6, 6, 2, 1,
                            1, 2, 6, 6, 6, 6, 6, 6, 2, 1,
                            1, 2, 6, 6, 6, 6, 6, 6, 2, 1,
                            1, 2, 6, 6, 8, 8, 6, 6, 2, 1,
                            1, 2, 6, 6, 6, 6, 6, 6, 2, 1,
                            1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

  float outImageVals[] = {
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,

                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 30.0, 30.0, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 30.0, 30.0, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 30.0, 30.0, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 30.0, 30.0, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 9.0, 1.0,
                            1.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,

                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

  typedef ImageType::PixelType::VectorType ImageData;
  ImageData   pixelData;
  int k = 0;
  while ( inIt != inItEnd ) {

    pixelData[0] = inImageVals[k];
    pixelData[1] = 100 - pixelData[0];

    inIt.Set( pixelData );
    ++inIt;
    ++k;
  }


  // FIRST TEST:
  // If lambda is negative, the number of final regions should equal
  // initial number of regions, the region labels should be consecutive

  std::cout << std::endl << "First test, lambda = -1" << std::endl;

  KLMFilter->SetMaximumLambda( -1 );

  // Kick off the Region grow function

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  KLMFilter->Print(std::cout);

  // This should return unique integer labels of the segmented regions.
  // The region labels should be consecutive integers beginning with 1.

  if( numPixels != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  // KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;

  OutputImageType::Pointer outImage = KLMFilter->GetOutput();

  typedef itk::ImageRegionIterator< OutputImageType > OutputImageIterator;

  inIt.GoToBegin();
  OutputImageIterator outIt( outImage, outImage->GetBufferedRegion() );

  typedef OutputImageType::PixelType::VectorType OutputImageData;
  ImageData         pixelIn;
  OutputImageData   pixelOut;

  while ( inIt != inItEnd )
    {
    pixelOut = outIt.Get();
    pixelIn  = inIt.Get();

    if ( pixelOut[0] != pixelIn[0] || pixelOut[1] != pixelIn[1] )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt;
    ++inIt;

    } //end while iterator loop


  // Make sure that the labelled image type is set to unsigned integer
  // as labels associated with different regions are always integers

  std::cout << "Extracting and checking label image" << std::endl;

  typedef itk::Image<LabelType, NUMDIM3D> LabelledImageType;
  LabelledImageType::Pointer labelledImage = KLMFilter->GetLabelledImage();

  // Loop through the approximation image and check if they match the
  // input image

  // setup the iterators

  typedef LabelledImageType::PixelType LabelledImagePixelType;

  typedef itk::ImageRegionIterator< LabelledImageType > LabelImageIterator;

  LabelImageIterator
    labelIt( labelledImage, labelledImage->GetBufferedRegion() );

  LabelType pixelLabel;
  LabelType m = 1;
  while(!labelIt.IsAtEnd())
    {
    pixelLabel = labelIt.Get();

    if(pixelLabel != m)
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++labelIt;
    ++m;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();

  // SECOND TEST:
  // the key test for union borders

  std::cout << std::endl <<
    "Second test, key merging test containing duplicate borders" << std::endl;

  KLMFilter->SetMaximumLambda( 1e45 );
  unsigned int nregions = 8;
  KLMFilter->SetMaximumNumberOfRegions( nregions );

  // Kick off the Region grow function

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  KLMFilter->Print(std::cout);

  // This should return unique integer labels of the segmented regions.
  // The region labels should be consecutive integers beginning with 1.

  if( nregions != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  // KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;

  OutputImageType::Pointer outImage2 = KLMFilter->GetOutput();

  OutputImageIterator outIt2( outImage2, outImage2->GetBufferedRegion() );
  OutputImageIterator outItEnd2 = outIt2.End();

  k = 0;
  while ( outIt2 != outItEnd2 )
    {
    pixelOut = outIt2.Get();

    if ( pixelOut[0] != outImageVals[k] ||
         pixelOut[1] != 100 - pixelOut[0] )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt2;
    ++k;

    } //end while iterator loop


  std::cout << "Extracting and checking label image" << std::endl;

  LabelledImageType::Pointer labelledImage2 = KLMFilter->GetLabelledImage();

  LabelImageIterator
    labelIt2( labelledImage2, labelledImage2->GetBufferedRegion() );

  k = 0;
  while(!labelIt2.IsAtEnd())
    {
    pixelLabel = labelIt2.Get();

    if(pixelLabel != labelVals[k])
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++labelIt2;
    ++k;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();


  // THIRD TEST:
  // degenerate case, all 0 image, all constant lambda values -
  // issues related to speed of algorithm

  std::cout << std::endl << "Third test, all 0 image" << std::endl;

  inIt.GoToBegin();
  pixelData[0] = 0;
  pixelData[1] = 0;
  while ( inIt != inItEnd ) {
    inIt.Set( pixelData );
    ++inIt;
  }

  gridSize[0] = 1;
  gridSize[1] = 2;
  gridSize[2] = 1;

  KLMFilter->SetInput( image );
  KLMFilter->SetMaximumNumberOfRegions( 75 );
  KLMFilter->SetGridSize( gridSize );
  KLMFilter->SetMaximumLambda( 1e45 );

  // Kick off the Region grow function

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  // KLMFilter->Print(std::cout);

  if( KLMFilter->GetMaximumNumberOfRegions() !=
      KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  // KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;

  OutputImageType::Pointer outImage3 = KLMFilter->GetOutput();

  OutputImageIterator outIt3( outImage3, outImage3->GetBufferedRegion() );
  OutputImageIterator outItEnd3 = outIt3.End();

  k = 0;
  while ( outIt3 != outItEnd3 )
    {
    pixelOut = outIt3.Get();

    if ( pixelOut[0] != 0 || pixelOut[1] != 0 )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt3;
    ++k;

    } //end while iterator loop


  std::cout << "Extracting and checking label image" << std::endl;

  typedef itk::Statistics::ScalarImageToHistogramGenerator<LabelledImageType>
    HistogramGeneratorType;

  HistogramGeneratorType::Pointer histogramGenerator =
    HistogramGeneratorType::New();

  histogramGenerator->SetInput( KLMFilter->GetLabelledImage() );
  histogramGenerator->SetNumberOfBins( KLMFilter->GetNumberOfRegions() );
  histogramGenerator->SetMarginalScale( 1.0 );
  histogramGenerator->Compute();

  typedef HistogramGeneratorType::HistogramType  HistogramType;

  const HistogramType * histogram = histogramGenerator->GetOutput();

  const unsigned int histogramSize = histogram->Size();
  if ( histogramSize != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  HistogramType::ConstIterator histIt = histogram->Begin();
  HistogramType::ConstIterator histItEnd = histogram->End();

  double Sum = histogram->GetTotalFrequency();
  double labelEntropy = 0.0;

  while( histIt != histItEnd )
    {

    double probability = histIt.GetFrequency() / Sum;

    if( probability == 0 )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    labelEntropy -= probability * log( probability );

    ++histIt;
    }

  labelEntropy /= log( 2.0 );
  double idealEntropy = - log( 8.0/ numPixels ) / log( 2.0 );

  std::cout << "Label entropy = " << labelEntropy << " bits " << std::endl;
  std::cout << "Ideal entropy = " << idealEntropy << " bits " << std::endl;

  if ( fabs( idealEntropy - labelEntropy ) > 0.2 )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

  std::cout << "Test PASSED" << std::endl;

  std::cout << std::endl << "Done testing of three-dimension images" << std::endl;
  return EXIT_SUCCESS;

} // End test_regiongrow3D()


unsigned int test_regiongrowKLM4D()
{
  itk::OutputWindow::SetInstance(itk::TextOutput::New().GetPointer());

  std::cout << std::endl << "Begin testing four-dimension images" << std::endl;

  // Manually create an image

  typedef itk::Image<itk::Vector<short int,NUMBANDS1>,NUMDIM4D> ImageType;
  typedef itk::Image<itk::Vector<float,NUMBANDS1>,NUMDIM4D> OutputImageType;

  ImageType::Pointer image  = ImageType::New();

  ImageType::SizeType imageSize;
  int multVal = 2;
  imageSize[0] = 2 * multVal;
  imageSize[1] = 3 * multVal;
  imageSize[2] = 5 * multVal;
  imageSize[3] = 7 * multVal;
  unsigned int numPixels = imageSize[0] * imageSize[1]  * imageSize[2]  * imageSize[3];

  ImageType::IndexType index;
  index.Fill(0);

  ImageType::RegionType region;
  region.SetSize( imageSize );
  region.SetIndex( index );

  image->SetLargestPossibleRegion( region );
  image->SetBufferedRegion( region );
  image->Allocate();

  typedef ImageType::PixelType ImagePixelType;
  typedef itk::ImageRegionIterator< ImageType > ImageIterator;
  ImageIterator inIt( image, image->GetBufferedRegion() );
  ImageIterator inItEnd = inIt.End();

  // Set up the filter

  typedef itk::KLMRegionGrowImageFilter<ImageType,OutputImageType>
    KLMRegionGrowImageFilterType;

  KLMRegionGrowImageFilterType::Pointer KLMFilter =
    KLMRegionGrowImageFilterType::New();

  KLMRegionGrowImageFilterType::GridSizeType gridSize;
  gridSize[0] = 1;
  gridSize[1] = 1;
  gridSize[2] = 1;
  gridSize[3] = 1;

  KLMFilter->SetInput( image );
  KLMFilter->SetGridSize( gridSize );

  typedef KLMRegionGrowImageFilterType::RegionLabelType LabelType;

  typedef ImageType::PixelType::VectorType ImageData;
  ImageData   pixelData;
  int k = 0;
  while ( inIt != inItEnd ) {

    pixelData[0] = 1;
    if ( k >= 100 && k < 200 ) pixelData[0] = 2;
    if ( k >= 500 && k < 700 ) pixelData[0] = 3;

    inIt.Set( pixelData );
    ++inIt;
    ++k;
  }


  // FIRST TEST:
  // If lambda is 0, the number of final regions should equal
  // initial number of regions, the region labels should be consecutive

  std::cout << std::endl << "First test, lambda = 0" << std::endl;

  KLMFilter->SetMaximumLambda( 0 );

  unsigned int nregions = 3;
  KLMFilter->SetMaximumNumberOfRegions( nregions );

  // Kick off the Region grow function

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  KLMFilter->Print(std::cout);

  // This should return unique integer labels of the segmented regions.
  // The region labels should be consecutive integers beginning with 1.

  if( numPixels != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  // KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;

  OutputImageType::Pointer outImage = KLMFilter->GetOutput();

  typedef itk::ImageRegionIterator< OutputImageType > OutputImageIterator;

  inIt.GoToBegin();
  OutputImageIterator outIt( outImage, outImage->GetBufferedRegion() );

  typedef OutputImageType::PixelType::VectorType OutputImageData;
  ImageData         pixelIn;
  OutputImageData   pixelOut;

  while ( inIt != inItEnd )
    {
    pixelOut = outIt.Get();
    pixelIn  = inIt.Get();

    if ( pixelOut[0] != pixelIn[0] )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt;
    ++inIt;

    } //end while iterator loop


  // Make sure that the labelled image type is set to unsigned integer
  // as labels associated with different regions are always integers

  std::cout << "Extracting and checking label image" << std::endl;

  typedef itk::Image<LabelType, NUMDIM4D> LabelledImageType;
  LabelledImageType::Pointer labelledImage = KLMFilter->GetLabelledImage();

  // Loop through the approximation image and check if they match the
  // input image

  // setup the iterators

  typedef LabelledImageType::PixelType LabelledImagePixelType;

  typedef itk::ImageRegionIterator< LabelledImageType > LabelImageIterator;

  LabelImageIterator
    labelIt( labelledImage, labelledImage->GetBufferedRegion() );

  LabelType pixelLabel;
  LabelType m = 1;
  while(!labelIt.IsAtEnd())
    {
    pixelLabel = labelIt.Get();

    if(pixelLabel != m)
      {
      std::cout << "pixelLabel " << pixelLabel << " m " << m << std::endl;

      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++labelIt;
    ++m;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();

  // SECOND TEST:
  // merge to 3 regions

  std::cout << std::endl <<
    "Second test, merge to " << nregions << " regions" << std::endl;

  KLMFilter->SetMaximumLambda( 1e45 );

  // Kick off the Region grow function

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  KLMFilter->Print(std::cout);

  // This should return unique integer labels of the segmented regions.
  // The region labels should be consecutive integers beginning with 1.

  if( nregions != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  KLMFilter->PrintAlgorithmRegionStats();
  KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;

  OutputImageType::Pointer outImage2 = KLMFilter->GetOutput();

  OutputImageIterator outIt2( outImage2, outImage2->GetBufferedRegion() );
  OutputImageIterator outItEnd2 = outIt2.End();

  k = 0;
  inIt.GoToBegin();
  while ( outIt2 != outItEnd2 )
    {
    pixelOut = outIt2.Get();
    pixelIn = inIt.Get();

    if ( pixelOut[0] != pixelIn[0] )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt2;
    ++inIt;
    ++k;

    } //end while iterator loop


  std::cout << "Extracting and checking label image" << std::endl;

  LabelledImageType::Pointer labelledImage2 = KLMFilter->GetLabelledImage();

  LabelImageIterator
    labelIt2( labelledImage2, labelledImage2->GetBufferedRegion() );

  k = 0;
  inIt.GoToBegin();
  while(!labelIt2.IsAtEnd())
    {
    pixelLabel = labelIt2.Get();
    pixelIn = inIt.Get();

    if(pixelLabel != static_cast<LabelType>(pixelIn[0]))
      {
      std::cout << "k = " << k
                << " pixelLabel = " << pixelLabel
                << " pixelIn[0] = " << pixelIn[0] << std::endl;

      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++labelIt2;
    ++inIt;
    ++k;

    } //end while iterator loop

  std::cout << "Test PASSED" << std::endl;

  KLMFilter->ResetPipeline();


  // THIRD TEST:
  // degenerate case, all 0 image, all constant lambda values -
  // issues related to speed of algorithm

  std::cout << std::endl << "Third test, all 0 image" << std::endl;

  inIt.GoToBegin();
  pixelData[0] = 0;
  while ( inIt != inItEnd ) {
    inIt.Set( pixelData );
    ++inIt;
  }

  gridSize[0] = 2;
  gridSize[1] = 3;
  gridSize[2] = 5;
  gridSize[3] = 7;

  KLMFilter->SetInput( image );
  nregions = 4;
  KLMFilter->SetMaximumNumberOfRegions( nregions );
  KLMFilter->SetGridSize( gridSize );
  KLMFilter->SetMaximumLambda( 1e45 );

  // Kick off the Region grow function

  LOCAL_TEST_EXCEPTION_MACRO( KLMFilter );

  KLMFilter->Print(std::cout);

  if( KLMFilter->GetMaximumNumberOfRegions() !=
      KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  // Test the functions useful to test the region and border statistics
  // as the regions are merged. Primarily useful for debug operations and are
  // called several times, so prudent usage is advisable.
  // KLMFilter->PrintAlgorithmRegionStats();
  // KLMFilter->PrintAlgorithmBorderStats();

  std::cout << "Extracting and checking approximation image" << std::endl;

  OutputImageType::Pointer outImage3 = KLMFilter->GetOutput();

  OutputImageIterator outIt3( outImage3, outImage3->GetBufferedRegion() );
  OutputImageIterator outItEnd3 = outIt3.End();

  k = 0;
  while ( outIt3 != outItEnd3 )
    {
    pixelOut = outIt3.Get();

    if ( pixelOut[0] != 0 )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    ++outIt3;
    ++k;

    } //end while iterator loop


  std::cout << "Extracting and checking label image" << std::endl;

  typedef itk::Statistics::ScalarImageToHistogramGenerator<LabelledImageType>
    HistogramGeneratorType;

  HistogramGeneratorType::Pointer histogramGenerator =
    HistogramGeneratorType::New();

  histogramGenerator->SetInput( KLMFilter->GetLabelledImage() );
  histogramGenerator->SetNumberOfBins( KLMFilter->GetNumberOfRegions() );
  histogramGenerator->SetMarginalScale( 1.0 );
  histogramGenerator->Compute();

  typedef HistogramGeneratorType::HistogramType  HistogramType;

  const HistogramType * histogram = histogramGenerator->GetOutput();

  const unsigned int histogramSize = histogram->Size();
  if ( histogramSize != KLMFilter->GetNumberOfRegions() )
    {
    std::cout << "Test FAILED" << std::endl;
    return EXIT_FAILURE;
    }

  HistogramType::ConstIterator histIt = histogram->Begin();
  HistogramType::ConstIterator histItEnd = histogram->End();

  double Sum = histogram->GetTotalFrequency();
  double labelEntropy = 0.0;

  while( histIt != histItEnd )
    {

    double probability = histIt.GetFrequency() / Sum;

    if( probability == 0 )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

    labelEntropy -= probability * log( probability );

    ++histIt;
    }

  labelEntropy /= log( 2.0 );
  double idealEntropy = - log( 1.0 / KLMFilter->GetNumberOfRegions() ) / log( 2.0 );
  std::cout << "Label entropy = " << labelEntropy << " bits " << std::endl;
  std::cout << "Ideal entropy = " << idealEntropy << " bits " << std::endl;

  if ( fabs( idealEntropy - labelEntropy ) > 0.15 )
      {
      std::cout << "Test FAILED" << std::endl;
      return EXIT_FAILURE;
      }

  std::cout << "Test PASSED" << std::endl;

  std::cout << std::endl << "Done testing of four-dimension images" << std::endl;
  return EXIT_SUCCESS;

}

#undef LOCAL_TEST_EXCEPTION_MACRO
