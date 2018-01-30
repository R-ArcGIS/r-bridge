% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/AA.classes.R
\docType{class}
\name{arc.dataset-class}
\alias{arc.container-class}
\alias{arc.dataset-class}
\alias{arc.feature-class}
\alias{arc.metadata,arc.container-method}
\alias{arc.metadata,arc.dataset-method}
\alias{arc.table-class}
\alias{names,arc.table-method}
\alias{show,arc.dataset-method}
\alias{show,arc.feature-method}
\alias{show,arc.table-method}
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
\item{\code{path}}{file path or layer name}

\item{\code{dataset_type}}{dataset type}

\item{\code{extent}}{spatial extent of the dataset}

\item{\code{fields}}{list of field names}

\item{\code{shapeinfo}}{geometry information (see \code{\link{arc.shapeinfo}})}
}}
\section{References}{

\enumerate{
  \item \href{http://desktop.arcgis.com/en/desktop/latest/analyze/arcpy-functions/dataset-properties.htm#GUID-35446E5D-31AF-4B41-B795-783409C641A8}{ArcGIS Help: Dataset properties -- dataset type}
}
}

\section{Methods}{

\describe{
  \item{\link{arc.select}}{}
  \item{\link{names}}{}
}
}
\examples{

ozone.file <- system.file("extdata", "ca_ozone_pts.shp",
                          package="arcgisbinding")
d <- arc.open(ozone.file)
names(d@fields) # get all field names
arc.shapeinfo(d) # print shape info
d                # print dataset info

}
\keyword{classes}
\keyword{dataset}
