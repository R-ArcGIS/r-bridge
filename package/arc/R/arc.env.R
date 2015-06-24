#' Get geoprocessing environment settings
#'
#' Geoprocessing environment settings are additional parameters that affect a
#' tool's results. Instead, they are values configured in a separate dialog box,
#' and interrogated and used by the script when run.
#'
#' The geoprocessing environment can control a variety of attributes relating
#' to where data is stored, the extent and projection of analysis outputs,
#' tolerances of output values, and parallel processing, among other attributes.
#  Commonly used environment settings include \code{workspace}, which controls
#' the default location for geoprocessing tool inputs and outputs. See the
#' topics listed under "References" for details on the full range of
#' environment settings that Geoprocessing scripts can utilize.
#'
#' @note \itemize{
#'   \item This function is only available from within an ArcGIS session.
#'         Usually, it is used to get local Geoprocessing tool environment
#'         settings within the executing tool.
#'   \item This function can only read current geoprocessing settings. Settings,
#'         such as the current workspace, must be configured in the calling
#'         Geoprocessing script, not within the body of the R script.
#' }
#' @section References:
#' \itemize{
#'   \item \href{http://desktop.arcgis.com/en/desktop/latest/tools/environments/what-is-a-geoprocessing-environment.htm}{ArcGIS Help: What is a geoprocessing environment setting?}
#'   \item \href{http://desktop.arcgis.com/en/desktop/latest/tools/environments/setting-geoprocessing-environment.htm}{ArcGIS Help: Setting geoprocessing environments}
#' }
#' @examples
#' \dontrun{
#'   tool_exec <- function(in_para, out_params)
#'   {
#'     env = arc.env()
#'     wkspath <- env$workspace
#'     ...
#'     return(out_params)
#'   }
#' }
#' @export
arc.env <- function ()
{
  .call_proxy("arc_getEnv")
}
