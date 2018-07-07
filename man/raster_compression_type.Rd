\name{raster_compression_type}
\alias{raster_compression_type}
\alias{compression_type}
\title{Compression types}
\description{
Raster compression types

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
  \code{\link{resample_type}},
  \code{\link{pixel_type}},
  \code{\link{arc.datasetraster-class}}
}
