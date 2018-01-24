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
