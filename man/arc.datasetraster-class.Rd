\name{arc.datasetraster-class}
\docType{class}
\alias{arc.datasetraster-class}
\alias{dim,arc.datasetraster-method}
\alias{names,arc.datasetraster-method}
%\alias{show,arc.datasetraster-method}

\title{Class "arc.datasetraster"}

\description{
\code{arc.datasetraster} S4 class. Dataset class for raster objects. Creates a dataset object with \code{type = raster}.
}

\details{
A raster dataset is any valid raster format organized into one or more bands. Each band consists of an array of pixels (cells), and each pixel has a value. A raster dataset has at least one band. Raster data is a discrete data representation in which space is divided into uniform cells, or pixels.

}

\section{Extends}{
  Class \code{\link{arc.dataset-class}}, directly.
  \deqn{
    \begin{array}{c}
      \code{arc.dataset-class} \\
      \downarrow \\
      \code{arc.datasetraster-class}
      \end{array}
    }
}

\section{Slots}{
  \describe{
    \item{\code{sr}}{Spatial reference.}
    \item{\code{extent}}{Spatial extent of the dataset.  The Extent describes the rectangle (boundary) containing all the raster dataset's data.}
    \item{\code{pixel_type}}{The \link[=pixel_type]{pixel type} of the referenced raster dataset.}
    \item{\code{compression_type}}{The \link[=compression_type]{compression type}.}
    \item{\code{nrow}}{The number of rows.}
    \item{\code{ncol}}{The number of columns.}
    \item{\code{bands}}{raster dataset bands information.}
  }
}

\section{Methods}{
  \describe{
    \item{\link{arc.raster}}{Create a \code{arc.raster} object}
    \item{dim}{retrieves dimensions of a \code{arc.dataset} object}
    \item{names}{return bands names}
%    \item{show}{show object}
    \item{\link{arc.write}}{ TODO }
  }
}

\section{References}{
  \enumerate{
    \item \href{http://desktop.arcgis.com/en/arcmap/latest/manage-data/raster-and-images/raster-dataset-properties.htm}{ArcGIS Help: Raster dataset properties}
  }
}

\seealso{
  \code{\link{arc.raster}},
  \code{\link{arc.write}}
}
\keyword{classes}
\keyword{dataset}
\keyword{open}
\keyword{raster}
\keyword{rasterdataset}
