\name{resample, pixel, comression types}
\alias{pixel_type}
\alias{resample_type}
\alias{compression_type}
\title{Raster resample, pixel, comression types}

\description{

The following table shows the \code{pixel_type} value and the range of values stored for different bit depths:
\tabular{lll}{
  \bold{Pixel type} \tab \bold{ Bit depth} \tab \bold{Range of values that each cell can contain}\cr
  \code{"U1"} \tab 1 bit \tab 0 to 1\cr
  \code{"U2"} \tab 2 bits \tab 0 to 3\cr
  \code{"U4"} \tab 4 bits \tab 0 to 15\cr
  \code{"U8"} \tab Unsigned 8 bit integers \tab 0 to 255\cr
  \code{"S8"} \tab 8 bit integers \tab -128 to 128 \cr
  \code{"U16"} \tab Unsigned 16 bit integers \tab 0 to 65535\cr
  \code{"S16"} \tab 16 bit integers \tab -32768 to 32767\cr
  \code{"U32"} \tab Unsigned 32 bit integers \tab 0 to 4294967295\cr
  \code{"S32"} \tab 32 bit integers \tab -2147483648 to 2147483647\cr
  \code{"F32"} \tab 32 bit Single precision floating point \tab -3.402823466e+38 to 3.402823466e+38\cr
  \code{"F64"} \tab 64 bit Double precision floating point \tab 0 to 18446744073709551616\cr
}

The following table shows the \code{resamp_type} value:
\tabular{ll}{
\bold{Resample type } \tab \bold{ Definition} \cr
  \code{"NearestNeighbor"} \tab - Performs a nearest neighbor assignment and
  is the fastest of the interpolation methods.  This is the default.\cr
  \code{"BilinearInterpolation"} \tab - Performs a bilinear interpolation and determines the
  new value of a cell based on a weighted distance average of the four nearest input cell centers.\cr
  \code{"CubicConvolution"} \tab - Performs a cubic convolution and determines the
  new value of a cell based on fitting a smooth curve through the 16 nearest input
  cell centers.\cr
  \code{"Majority"} \tab - Performs a majority algorithm and determines the new value
  of the cell based on the most popular values within the filter window.\cr
  \code{"BilinearInterpolationPlus"} \tab TODO \cr
  \code{"BilinearGaussBlur"} \tab TODO \cr
  \code{"BilinearGaussBlurPlus"} \tab TODO \cr
  \code{"Average"} \tab TODO \cr
  \code{"Minimum"} \tab TODO \cr
  \code{"Average"} \tab TODO \cr
  \code{"VectorAverage"} \tab TODO \cr
}
\bold{Note}
  The Bilinear and Cubic options should not be used with categorical data, since the cell values may be altered.

The following table shows the \code{compression_type} value:
\tabular{lll}{
  \bold{Compression type} \tab \bold{ Lossy or lossless} \tab \bold{Notes}\cr
  \code{"LZ77"} \tab Lossless\cr
  \code{"JPEG"} \tab Lossy \tab Can define a compression quality\cr
  \code{"JPEG 2000"} \tab Lossy or lossless \tab Can define a compression quality\cr
  \code{"PackBits"} \tab Lossless \tab Applies to TIFF only\cr
  \code{"LZW"} \tab Lossless\cr
  \code{"RLE"} \tab Lossless\cr
  \code{"CCITT GROUP 3"} \tab Lossless \tab Applies to TIFF only\cr
  \code{"CCITT GROUP 4"} \tab Lossless \tab Applies to TIFF only\cr
  \code{"CCITT (1D)"} \tab Lossless \tab Applies to TIFF only\cr
  \code{"None"} \tab No data compression\cr
}

}

\seealso{
  \code{\link{arc.raster}},
  \code{\link{arc.raster-class}}
}
\section{References}{
  \enumerate{\item\href{https://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/bit-depth-capacity-for-raster-dataset-cells.htm}{ArcGIS Help: Pixel Types}
  }
}
