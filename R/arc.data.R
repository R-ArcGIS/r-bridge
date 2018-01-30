#' @export
print.arc.data <- function(x, ...)
{
  s <- arc.shape(x)
  if (is.null(s))
    return (NextMethod())
  show(arc.shapeinfo(s))
  if (nrow(x) < 20)
    return(NextMethod())
  if (.shape_len(s) != nrow(x))
    return (NextMethod())
  if (!all(class(x) %in% c("arc.data", "data.frame")))
    return (NextMethod())

  class(x) <- setdiff(class(x), "arc.data")
  head5tail3<-c(1:5,NA,((nrow(x)-2):nrow(x)))
  x<-format(x[head5tail3,,drop=FALSE])
  x[6,]<-rep("...", ncol(x))
  rownames(x)[6]<-"..."
  #print.data.frame(x, ...)
  NextMethod("print", x, ...)
}

# subsetting
#' @export
"[.arc.data" <- function(x, i, j, drop)
{
  cl <- oldClass(x)
  d <- NextMethod("[")
  if (is.data.frame(d))
  {
    if (!missing(i) && mode(i)=="numeric")
      attr(d, "shape") <- arc.shape(x)[i]
    else
      attr(d, "shape") <- arc.shape(x)
    oldClass(d) <- cl
  }
  d
}

#"[<-.arc.data" <- function(x, i, j, value) stop("'[<-' unsupported")

#' dplyr support
#'
#' dplyr methods for arc.data objects: filter, arrange, mutate, select, group_by
#'
#' @name dplyr
#' @rdname dplyr.support
#' @export
filter.arc.data <- function(.data, ..., .dots)
{
  .data[["..old.index"]] <- seq_len(nrow(.data))
  cl <- oldClass(.data)
  d <- NextMethod()
  attr(d, "shape") <- arc.shape(d)[d[["..old.index"]]]
  d[["..old.index"]]<-NULL
  oldClass(d) <- cl
  return (d)
}

#' @name dplyr
#' @rdname dplyr.support
#' @export
arrange.arc.data <- function(.data, ..., .dots)
{
  .data[["..old.index"]] <- seq_len(nrow(.data))
  cl <- oldClass(.data)
  d <- NextMethod()
  attr(d, "shape") <- arc.shape(d)[d[["..old.index"]]]
  d[["..old.index"]]<-NULL
  oldClass(d) <- cl
  return (d)
}

#' @name dplyr
#' @rdname dplyr.support
#' @export
mutate.arc.data <- function(.data, ..., .dots)
{
  cl <- oldClass(.data)
  d <- NextMethod()
  attr(d, "shape") <- arc.shape(.data)
  oldClass(d) <- cl
  return (d)
}

#' @name dplyr
#' @rdname dplyr.support
#' @export
group_by.arc.data <- function(.data, ..., add)
{
  d <- NextMethod()
  attr(d, "shape") <- arc.shape(.data)
  oldClass(d) <- c("arc.data", oldClass(d))
  return (d)
}

#' @name dplyr
#' @rdname dplyr.support
#' @export
ungroup.arc.data <- function(x, ...)
{
  d <- NextMethod()
  attr(d, "shape") <- arc.shape(x)
  oldClass(d) <- c("arc.data", oldClass(d))
  return (d)
}
