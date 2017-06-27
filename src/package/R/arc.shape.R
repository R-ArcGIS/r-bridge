#' Get arc.shape object
#'
#' Get \code{\link{arc.shape-class}} from \code{arc.dataframe}
#'
#'
#' @param df \code{arc.dataframe}
#' @seealso \code{\link{arc.select}}
#' @examples
#'
#' d <- arc.open(system.file("extdata", "ca_ozone_pts.shp",
#'                           package="arcgisbinding"))
#'
#' df <- arc.select(d, 'ozone')
#' shp <- arc.shape(df)
#' length(shp$x)
#' @export
arc.shape <- function (df) UseMethod("arc.shape")

#' @export
arc.shape.default <- function(df) attr(df, "shape")

#' @export
arc.shape.arc.data <- function (df) attr(df, "shape")

#' @export
setMethod("show", "arc.shape", function(object)
{
  info <- arc.shapeinfo(object)
  show(info)
  cat("length       : ")
  if (info$type == "Point")
    cat(length(object[[1]]), "\n")
  else
    cat(length(object), "\n")
  invisible(object)
})

