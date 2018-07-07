\name{resample_type}
\alias{resample_type}
\title{Resample types}
\description{
Resample types
}
\section{Supported}{

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
}
\note{
  The Bilinear and Cubic options should not be used with categorical data, since the cell values may be altered.
}

\section{References}{
  \enumerate{
    \item \href{http://desktop.arcgis.com/en/arcmap/latest/extensions/spatial-analyst/performing-analysis/cell-size-and-resampling-in-analysis.htm}{Cell size and resampling in analysis}
  }
}


\seealso{
  \code{\link{pixel_type}}
  \code{\link{arc.raster}}
  \code{\link{arc.raster-class}}
}

