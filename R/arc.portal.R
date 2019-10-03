#' @export
arc.check_portal <- function()
{
  info = .call_proxy("arc_portal", NULL, NULL, NULL, NULL)
  class(info) <- c("arc.portalinfo", class(info))
  return (info)
}
#' @export
arc.portal_connect <- function(url, user, password)
{
  if (missing(url))
    stop("require 'url'")

  if (missing(user)) user <- NULL
  if (missing(password)) password <- NULL
  token <- NULL

  info = .call_proxy("arc_portal", url, user, password, token)
  class(info) <- c("arc.portalinfo", class(info))
  return (info)
}

#' @export
format.arc.portalinfo <- function(x, ...)
{
  is_html <- list(...)["fmt"] == "html"
  ret <- ifelse(is_html, "<b>Current</b>", "*** Current")
  if (is.null(x))
  {
    ret <- paste0(ret, "  Empty")
    return (ret)
  }
  if (!is.null(x$url))
  {
    ret <- paste0(ret, "\n  url\t\t: ", x$url)
    ret <- paste0(ret, "\n  version\t: ", x$version)
    if (!is.null(x$user))
      ret <- paste0(ret, "\n  user\t\t: ", x$user)
    if (!is.null(x$organization))
      ret <- paste0(ret, "\n  organization\t: ", x$organization)
    #if (!is.null(x$token))
    #  ret <- paste0(ret, "\n  token\t\t: ", x$token)
  }
  else
    ret <- paste0(ret, "\n  Not signed in")

  if (!is.null(x$portals))
  {
    if (is_html)
      ret <- paste0(ret, "\n<b>Available</b> (signed in)\n")
    else
      ret <- paste0(ret, "\n*** Available (signed in)\n")
    ret <- paste0(ret, paste0("  '", x$portals, "'", collapse="\n"));
  }
  if (!is.null(x$offlines))
  {
    ret <- paste0(ret, ifelse(is_html, "\n<b>Not signed in</b>\n", "\n*** Not signed in\n"))
    ret <- paste0(ret, paste0("  '", x$offlines, "'", collapse="\n"));
  }

  return (ret);
}

#' @export
print.arc.portalinfo <- function(x, ...)
{
  cat(format(x), "\n")
  invisible(x)
}
