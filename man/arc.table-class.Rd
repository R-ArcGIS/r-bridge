\docType{class}
\name{arc.table-class}
\alias{arc.table-class}
\alias{names,arc.table-method}
%\alias{show,arc.table-method}
\title{Class "arc.table"}
\description{\code{arc.table} S4 class}
\details{
The \code{fields} slot includes the details of the ArcGIS data types of the
relevant fields, which include data types not directly representable in \code{R}.
}

\section{Extends}{
  Class \code{arc.dataset-class}, directly.
    \if{latex}{
    \deqn{
      \begin{array}{c}
      \code{arc.dataset-class} \\
      \downarrow \\
      \code{arc.table-class}
      \end{array}
    }
  }

}

\section{Slots}{
  \describe{
    \item{\code{fields}}{named list of field types.}
  }
}

\section{Methods}{
  \describe{
    \item{\link{arc.select}}{return \code{data.frame}. TODO}
    \item{names}{return names of columns}
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

\seealso{
  \code{\link{arc.open}},
  \code{\link{arc.dataset-class}},
  \code{\link{arc.feature-class}}
}

\keyword{classes}
\keyword{dataset}
