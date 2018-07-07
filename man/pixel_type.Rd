\name{pixel_type}
\alias{pixel_type}
\title{Pixel types}
\description{
Bit depth capacity for raster dataset cells.

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
}

\seealso{
  \code{\link{resample_type}},
  \code{\link{arc.raster}},
  \code{\link{arc.raster-class}}
}
\section{References}{\enumerate{\item\href{https://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/bit-depth-capacity-for-raster-dataset-cells.htm}{ArcGIS Help: Pixel Types}
}
}