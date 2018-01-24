#' @export
arc.fromP4ToWkt <- function (proj4)
{
  if (class(proj4) == "CRS")
    str <- as.character(proj4@projargs)
  else
  {
    if (is.null(proj4) || is.na(proj4))
      return(as.character(NA))
    str <- as.character(proj4)
  }

  if (length(str) == 0)
    return(as.character(NA))

  .call_proxy("arc_fromP42Wkt", str)
}

#' @export
arc.fromWktToP4 <- function (wkt)
{
  .call_proxy("arc_fromWkt2P4", wkt)
}
