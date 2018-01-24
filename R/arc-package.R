#' Bindings for ArcGIS
#'
#' This package provides classes for loading, converting and exporting ArcGIS datasets and layers in R.
#'
#' @name arcgisbinding
#' @aliases arcgisbinding arcgisbinding-package arc
#' @docType package
#' @section Introduction: For a complete list of exported functions, use
#' \code{library(help = "arcgisbinding")}.
#' @section References:
#' \itemize{
#'  \item \href{https://r-forge.r-project.org/projects/rspatial/}{sp package}
#'  \item \href{http://cran.r-project.org/web/views/Spatial.html}{CRAN Task View: Analysis of Spatial Data}
#' }
#' @import methods
#' @rdname arcgisbinding
NULL

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
                                "define a desktop license."))
    #TODDO remove before release
    #arc.check_product()
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
