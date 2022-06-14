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
\item{\dots}{optional additional arguments such as \code{transformation} - datum transformation string}
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

## datum transformation, from NAD_1983 to WGS_1984
## prepare dataset
x <- c(1455535.149968639, 1446183.62438038, 1447950.6349539757)
y <- c(478067.64943164587, 484500.4190463871, 479746.6336064786)
data_path <- file.path(tempdir(), "data.gdb", "test_datum")
## save as NAD_1983
arc.write(data_path, coords=cbind(x, y), shape_info=list(type="Point", WKID=2893))
## read and transform to WGS_1984
df <- arc.select(arc.open(data_path), sr=4326, transformation='NAD_1983_HARN_To_WGS_1984_2')
x <- arc.shape(df)$x
stopifnot(sprintf('\%.8f', x1[1]) == '-76.49626388')

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
