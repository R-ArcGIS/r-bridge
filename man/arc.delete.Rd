\name{arc.delete}
\alias{arc.delete}
\alias{arc.delete,arc.dataset-method}
\title{Delete dataset}
\usage{
arc.delete(x, \dots)
\S4method{arc.delete}{arc.dataset}(x, \dots)
}
\arguments{
\item{x}{\code{string} full path or \code{arc.dataset} object}
\item{\dots}{reserved}
}
\description{
delete dataset
}
\value{
  logical, \code{TRUE} on success.
}

\examples{
  table_path <- file.path(tempdir(), "data.gdb", "mytable")
  arc.write(table_path, data=list('f1'=c(23,45), 'f2'=c('hello', 'bob')))

  # delete table
  arc.delete(table_path)

  # delete database
  arc.delete(dirname(table_path))
}

\keyword{method}
\keyword{delete}
