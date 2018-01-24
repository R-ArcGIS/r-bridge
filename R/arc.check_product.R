#' ArcGIS product and license information
#'
#' Initialize connection to ArcGIS. Any script running directly from R (i.e.
#' without being called from a Geoprocessing script) should first call
#' \code{arc.check_product} to create a connection with ArcGIS. Provides
#' installation details on the version of ArcGIS installed that
#' \code{arcgisbinding} is communicating with.
#'
#' Returned details include:
#' \itemize{
#'   \item Product: ArcGIS Desktop (i.e. ArcMap), or ArcGIS Pro. The name of the
#'         product connected.
#'   \item License level: Basic, Standard, or Advanced are the three licensing
#'         levels available. Each provides progressively more functionality
#'         within the software. See the "Desktop Functionality Matrix" link for
#'         details.
#'   \item Build number: The build number of the release being used.
#'         Useful in debugging and when creating error reports.
#'   \item DLL: The dynamic linked library (DLL) in use allowing
#'         ArcGIS to communicate with R.
#' }
#' @note  Additional license levels are available on ArcGIS Desktop: Server,
#'        EngineGeoDB, and Engine. These license levels are currently
#'        unsupported by this package.
#' @section References:
#'  \href{http://www.esri.com/~/media/Files/Pdfs/library/brochures/pdfs/arcgis1021-desktop-functionality-matrix.pdf}{ArcGIS Desktop Functionality Matrix}
#'
#' @examples
#'
#' info <- arc.check_product()
#' info$license # ArcGIS license level
#' info$version # ArcGIS build number
#' info$app # product name
#' info$dll # binding DLL in use
#' @name arc.check_product
#' @rdname arc.check_product
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
  class(prod) <- c("arc.product", class(prod))
  return(prod)
}

#' @export
print.arc.product <- function(x, ...)
{
  cat("product:", x$app, "(", x$version, ")\n")
  cat("license:", x$license, "\n")
  cat("version:", x$pkg_ver, "\n")
  invisible(x)
}
