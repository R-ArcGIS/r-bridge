#' ArcGIS product and license information
#'
#' Initialize connection to ArcGIS. Any script running directly from R (i.e.
#' without being called from a Geoprocessing script) should first call
#' \code{arc.check_product} to create a connection with ArcGIS. Provides
#' installation details on the version of ArcGIS installed that
#' \code{arcgisbinding} can communicate with.
#'
#' Returned details include:
#' \itemize{
#'   \item Product: ArcGIS Desktop (i.e. ArcMap), or ArcGIS Pro. The name of the
#'         product connected to.
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
#'        unsupported by this binding.
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
#' @export
arc.check_product <- function()
{
  prod <- .call_proxy("arc_AoInitialize")
  class(prod) <- append(class(prod), "arc.product")
  prod$dll <- .arc$dll
  prod$app <- if (.arc$dll == "rarcproxy_pro") "ArcGIS Pro" else "ArcGIS Desktop"
#TODO: test more i386
#  if (!is.null(prod$path) && is.na(Sys.getenv("GDAL_DATA",unset=NA)))
#  {
#    gdal_data = file.path(prod$path, ifelse(.arc$dll == "rarcproxy_pro", "Resources\\pedata\\gdaldata", "pedata\\gdaldata"), fsep='\\')
#    Sys.setenv(GDAL_DATA=gdal_data)
#  }
  return(prod)
}

.set_repo_mirrors <- function()
{
  repos_mirror <- getOption("repos")
  if (repos_mirror["CRAN"] == "@CRAN@")
  {
    #TODO allow customization for URL mirror
    repos_mirror["CRAN"] <- "http://cran.cnr.Berkeley.edu"
    options(repos = repos_mirror)
  }
  invisible()
}

#' @export
print.arc.product <- function(x, ...)
{
  cat("product:", x$app, "\n")
  cat("license:", x$license, "\n")
  cat("build number:", x$version, "\n")
  cat("binding dll:", x$dll, "\n")
  invisible(x)
}
#' Set progressor label for Geoprocessing dialog box
#'
#' Geoprocessing tools have a progressor, which includes both a progress
#' label and a progress bar. The default progressor continuously moves back
#' and forth to indicate the script is running. Using
#' \code{\link{arc.progress_label}} and \code{\link{arc.progress_pos}}
#' allows fine control over the script progress. Updating the progressor
#' isn't necessary, but is useful in situations where solely outputting messages
#' to the dialog is insufficient to communicate script progress.
#'
#' Using \code{\link{arc.progress_label}} allows control over the label that is
#' displayed at the top of the running script. For example, it might be used
#' to display the current step of the analysis taking place.
#'
#' @note
#'  \itemize{
#'    \item Currently only functions in ArcGIS Pro, and has no effect in
#'          ArcGIS Desktop.
#'    \item This function is only available from within an ArcGIS session, and
#'          has no effect when run from the command line or in background geoprocessing.
#' }
#' @section References:
#'   \href{https://pro.arcgis.com/en/pro-app/arcpy/geoprocessing_and_python/understanding-the-progress-dialog-in-script-tools.htm}{Understanding the progressor in script tools}
#'
#' @param label Progress Label
#' @seealso \code{\link{arc.progress_pos}},
#'   "Progress Messages" example Geoprocessing script
#' @examples
#' \dontrun{
#' arc.progress_label("Calculating bootstrap samples...")
#' }
#' @export
arc.progress_label <- function (label)
  invisible(.Call("arc_progress_label", PACKAGE=.arc$dll, as.character(label)))

#' Set progressor position for Geoprocessing dialog box
#'
#' Geoprocessing tools have a progressor, which includes both a progress
#' label and a progress bar. The default progressor continuously moves back
#' and forth to indicate the script is running. Using
#' \code{\link{arc.progress_label}} and \code{\link{arc.progress_pos}} allow
#' fine control over the script progress. Updating the progressor isn't
#' necessary, but is useful in situations where solely outputting messages to
#' the dialog is insufficient to communicate script progress.
#'
#' Using \code{\link{arc.progress_pos}} allows control over the progrssor position
#' displayed at the top of the running script. The position is an integer percentage,
#' 0 to 100, that the progress bar should be set to, with 100 indicating
#' the script has completed (100\%).
#'
#' Setting the position to -1 resets the progressor to the default progressor,
#' which continuously moves to indicate the script is running.
#'
#' @note
#'  \itemize{
#'    \item Currently only functions in ArcGIS Pro, and has no effect in ArcGIS Desktop.
#'    \item This function is only available from within an ArcGIS session, and
#'          has no effect when run from the command line or in background geoprocessing.
#' }
#' @section References:
#'   \href{https://pro.arcgis.com/en/pro-app/arcpy/geoprocessing_and_python/understanding-the-progress-dialog-in-script-tools.htm}{Understanding the progressor in script tools}
#
#' @param pos Progress position (in percent)
#' @seealso \code{\link{arc.progress_label}},
#'   "Progress Messages" example Geoprocessing script
#' @examples
#' \dontrun{
#' arc.progress_pos(55)
#' }
#' @export
arc.progress_pos <- function (pos=-1)
  invisible(.Call("arc_progress_pos", PACKAGE=.arc$dll, as.integer(pos)))

.onLoad <- function (libname, pkgname)
{
  .arc$inproc <- exists(".arcgisbinding_inproc", envir=.GlobalEnv)
  if (exists(".arcgisbinding_inproc", envir=.GlobalEnv))
    rm(".arcgisbinding_inproc", envir=.GlobalEnv)
  if (.arc$inproc)
  {
    .arc$dll = unlist(strsplit(commandArgs()[[1]], ':', TRUE))[2]
    .set_repo_mirrors()
  }
  else
    .arc$dll <- if (version$arch == "x86_64") "rarcproxy_pro" else "rarcproxy"
  library.dynam(.arc$dll, pkgname, libname)
  return(TRUE)
}

.onAttach <- function(...)
{
  if (!.arc$inproc)
  {
    packageStartupMessage(paste("*** Please call arc.check_product() to",
                                " define a desktop license."))
  }
}

.onUnload <- function(libpath)
{
  if (!is.null(.arc$dll))
  {
    library.dynam.unload(.arc$dll, libpath)
    rm("dll", envir=.arc)
  }
}
