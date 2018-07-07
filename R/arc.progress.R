# Set progressor label for Geoprocessing dialog box
#' @export
arc.progress_label <- function (label)
  invisible(.Call("arc_progress_label", PACKAGE=.arc$dll, as.character(label)))

# Set progressor position for Geoprocessing dialog box
#' @export
arc.progress_pos <- function (pos=-1)
  invisible(.Call("arc_progress_pos", PACKAGE=.arc$dll, as.integer(pos)))
