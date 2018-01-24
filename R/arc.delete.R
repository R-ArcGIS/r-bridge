#' @export
arc.delete <- function (x, ...)
{
  path <- if (inherits(x, "arc.dataset")) x <- x@path else x
  stopifnot(is.character(path))
  ret = .call_proxy("arc_delete", path)
  if (ret && inherits(x, "arc.dataset"))
     .discard(x)
  invisible(ret)
}
