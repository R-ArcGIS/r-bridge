\name{arc.select}
\docType{methods}
\alias{arc.select}
\alias{arc.select,arc.table-method}
%\alias{arc.select,arc.feature-method}
%\alias{arc.select,arc.datasetrastermosaic}
\concept{feature}
\concept{table}
\concept{spatial}

\concept{shape}

\title{Load dataset to "data.frame"}

\usage{
\S4method{arc.select}{arc.table}(object, fields, where_clause, selected, sr, \dots)
%\S4method{arc.select}{arc.feature}(object, fields, where_clause, selected, sr, \dots)
%\S4method{arc.select}{arc.datasetrastermosaic}(object, \dots)
}
\arguments{
\item{object}{\link{arc.dataset-class} object}
\item{fields}{string, or list of strings, containing fields to include (default: all)}
\item{where_clause}{SQL where clause}
\item{selected}{use only selected records (if any) when dataset is a layer or standalone table}
\item{sr}{transform geometry to Spatial Reference (default: object@sr)}
\item{\dots}{Additional arguements.}
}
\value{
\code{arc.select} returns a \code{data.frame} object (type of \code{arc.data}).
}
\description{
Load dataset to a standard data frame.
}

\note{
If \code{object} is \code{\link{arc.feature-class}}, the "shape" of class
\code{\link{arc.shape-class}} will be attached to the resulting
\code{arc.data} object.
}
\examples{

## read all fields
ozone.file <- system.file("extdata", "ca_ozone_pts.shp",
                          package="arcgisbinding")
d <- arc.open(ozone.file)
df <- arc.select(d, names(d@fields))
head(df, n=3)

## read 'name', 'fid' and geometry
df <- arc.select(d, c('fid', 'ozone'), where_clause="fid < 5")
nrow(df)

## transform points to "+proj=eqc"
df <- arc.select(d,"fid", where_clause="fid<5", sr="+proj=eqc")
arc.shape(df)

}
\seealso{
\code{\link{arc.data}},
\code{\link{arc.open}},
\code{\link{arc.write}}
}

\keyword{method}
\keyword{datasets}
\keyword{feature}
\keyword{spatial}
\keyword{select}
\keyword{table}
