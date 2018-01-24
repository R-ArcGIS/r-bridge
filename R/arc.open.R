#' @export
arc.open <- function (path)
{
  ds <- new("arc.dataset_impl", path = path)
  ds_new <- if (is(ds, "arc.container")) as(ds, "arc.container")
  else if (is(ds, "arc.feature")) as(ds, "arc.feature")
  else if (is(ds, "arc.table")) as(ds, "arc.table")
  else if (is(ds, "arc.datasetraster")) as(ds, "arc.datasetraster")
  else if (is(ds, "arc.datasetrastermosaic")) as(ds, "arc.datasetrastermosaic")
  else (NULL)
 
  if (is.null(ds_new))
  {
    warning("open as :", ds@dataset_type)
    return (ds)
  }
  .discard(ds)
  return (ds_new)
}
