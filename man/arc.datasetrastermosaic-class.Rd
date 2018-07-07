\name{arc.datasetrastermosaic-class}
\docType{class}
\alias{arc.datasetrastermosaic-class}
\title{Class "arc.datasetrastermosaic"}
\description{
\code{arc.datasetrastermosaic} S4 class. Dataset class for mosaic objects.
}
\details{
Mosaic datasets are made up of a collection of rasters. Mosaic structure efficiently stores and manages multiple rasters for visualization and analysis.
Detailed information about mosaic datasets can be found in \href{https://desktop.arcgis.com/en/arcmap/latest/manage-data/raster-and-images/what-is-a-mosaic-dataset.htm}{ArcGIS reference for mosaic datasets}.

\href{https://r-arcgis.github.io/}{R-ArcGIS bridge} handles mosaic data I/O using the \code{arc.open()} function.
The mosaic dataset opened using \code{arc.open} can be processed on the fly by converting it to a raster object within R using the \code{arc.raster} function.
Properties of a mosaic dataset such as \code{extent}, \code{pixel_type}, \code{nrow}, \code{ncol} and \code{mosaicking rules}.
Mosaicking rules determine how a series of potentially intercepting rasters are displayed as a single raster.
Mosaicking rules go beyond only visualization and can be used to stitch together different rasters making up a mosaic.

Mosaicking rules define how intersections between different rasters within the mosaic dataset are handled and are made up of method and operator.
Simply put, method defines which raster will be placed on top of the other for visualization in cases where they overlap and operator defines how the intersection between overlapping rasters in the mosaic dataset will be handled.
The information on mosaicking rules can be found under \href{https://desktop.arcgis.com/en/arcmap/latest/manage-data/raster-and-images/understanding-the-mosaicking-rules-for-a-mosaic-dataset.htm}{ArcGIS reference for mosaicking rules}.
}
\section{Extends}{
  Class \code{\link{arc.feature-class}}, \code{\link{arc.datasetraster-class}} directly and
  \code{\link{arc.table-class}} by class \code{"arc.feature-class"},
  \code{\link{arc.dataset-class}} by class \code{"arc.table-class"}.
\if{latex}{
  \deqn{ \begin{array}{rcl}
   & \code{arc.dataset-class} &\\
   & \swarrow \searrow &\\
   \code{arc.table-class} & & \\
  \downarrow & & \\
  \code{arc.feature-class} & & \code{arc.datasetraster-class}\\
  \searrow & & \swarrow \\
  & \code{arc.datasetrastermosaic-class} &
  \end{array}
  }
}
}

\section{References}{
  \enumerate{
    \item \href{http://desktop.arcgis.com/en/arcmap/latest/manage-data/raster-and-images/what-is-a-mosaic-dataset.htm}{ArcGIS Help: What is a mosaic dataset?}
  }
}


\seealso{
\code{\link{arc.open}},
\code{\link{arc.raster}},
\code{\link{arc.select}}
}

\keyword{classes}
\keyword{raster}
\keyword{dataset}
\keyword{mosaic}
