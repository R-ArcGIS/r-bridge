# ArcGIS product and license information
#' @export
arc.check_product <- function()
{
  prod <- .call_proxy("arc_AoInitialize")
  if (!is.null(prod$error))
    stop(prod$error, call. = FALSE)

  prod$dll <- .arc$dll
  prod$app <- if (.arc$dll == "rarcproxy_pro") "ArcGIS Pro" else "ArcGIS Desktop"
  prod$pkg_ver = paste0(utils::packageVersion('arcgisbinding'))
#TODO: test more i386
#  if (!is.null(prod$path) && is.na(Sys.getenv("GDAL_DATA",unset=NA)))
#  {
#    gdal_data = file.path(prod$path, ifelse(.arc$dll == "rarcproxy_pro", "Resources\\pedata\\gdaldata", "pedata\\gdaldata"), fsep='\\')
#    Sys.setenv(GDAL_DATA=gdal_data)
#  }
  class(prod) <- c("arc.productinfo", class(prod))
  return(prod)
}

format.arc.productinfo <- function(x, ...)
{
  fmt <- list(...)["fmt"]
  if (fmt == "html")
    return (sprintf("<b>product:</b> %s (%s)\n<b>license:</b> %s\n<b>version:</b> %s", x$app, x$version, x$license, x$pkg_ver))
  return (sprintf("product: %s (%s)\nlicense: %s\nversion: %s", x$app, x$version, x$license, x$pkg_ver))
}

#' @export
print.arc.productinfo <- function(x, ...)
{
  cat(format(x), "\n")
  invisible(x)
}
