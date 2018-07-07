\docType{class}
\name{arc.dataset-class}
\alias{arc.dataset-class}
%\alias{show,arc.dataset-method}
\title{Class "arc.dataset"}
\description{
\code{arc.dataset} S4 class
}
\details{
The \code{dataset_type} slot possible values are described in the
referenced "dataset properties -- data type" documentation. For feature datasets,
\code{extent} contains four \code{double} values: \code{(xmin, ymin, xmax, ymax)}.
The \code{fields} slot includes the details of the ArcGIS data types of the
relevant fields, which include data types not directly representable in \code{R}.
}

\section{Slots}{
  \describe{
    \item{.info}{internal}
    \item{\code{path}}{file path or layer name}
    \item{\code{dataset_type}}{dataset type}
  }
}

\section{Methods}{
  \describe{
%    \item{show}{show object}
    \item{\link{arc.delete}}{}
    \item{\link{arc.metadata}}{}
  }
}

\section{References}{
  \enumerate{
    \item \href{http://desktop.arcgis.com/en/desktop/latest/analyze/arcpy-functions/dataset-properties.htm#GUID-35446E5D-31AF-4B41-B795-783409C641A8}{ArcGIS Help: Dataset properties -- dataset type}
  }
}

\examples{

ozone.file <- system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")
d <- arc.open(ozone.file)
d # print dataset info

}

\seealso{
  \code{\link{arc.open}},
  \code{\link{arc.table-class}},
  \code{\link{arc.feature-class}},
  \code{\link{arc.datasetraster-class}},
  \code{\link{arc.datasetrastermosaic-class}}
}

\keyword{classes}
\keyword{dataset}
