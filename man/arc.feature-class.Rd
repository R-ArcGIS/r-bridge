\name{arc.feature-class}
\docType{class}
\alias{arc.feature-class}
\alias{names,arc.feature-method}
%\alias{show,arc.feature-method}

\title{Class "arc.feature"}
\description{
\code{arc.feature} S4 class.
}
\details{
  Container for shape information pertaining to extent and shape from a table class.
}

\section{Extends}{
  Class \code{\link{arc.table-class}}, directly and
  \code{\link{arc.dataset-class}} by class \code{"arc.table"}.
  \if{latex}{
    \deqn{
      \begin{array}{c}
      \code{arc.dataset-class} \\
      \downarrow \\
      \code{arc.table-class} \\
      \downarrow \\
      \code{arc.feature-class}
      \end{array}
    }
  }
}

\section{Slots}{
  \describe{
    \item{\code{shapeinfo}}{geometry information (see \code{\link{arc.shapeinfo}})}
    \item{\code{extent}}{spatial extent of the dataset}
  }
}

\section{Methods}{
  \describe{
    \item{\link{arc.select}}{TODO}
    \item{names}{return names of columns}
%    \item{show}{\code{signature(object = "arc.feature")} show object}
    \item{\link{arc.shapeinfo}}{return geometry information}
  }
}
\examples{

ozone.file <- system.file("extdata", "ca_ozone_pts.shp", package="arcgisbinding")
d <- arc.open(ozone.file)
names(d@fields) # get all field names
arc.shapeinfo(d) # print shape info
d                # print dataset info

}

\seealso{
  \code{\link{arc.open}},
  \code{\link{arc.dataset-class}},
  \code{\link{arc.table-class}},
  \code{\link{arc.datasetraster-class}},
  \code{\link{arc.datasetrastermosaic-class}}
}

\keyword{classes}
\keyword{dataset}
\keyword{feature}
