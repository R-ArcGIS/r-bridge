\name{Enterprise and Online portals}
\docType{methods}
\alias{arc.check_portal}
\alias{arc.portal_connect}
\concept{portal}


\title{ArcGIS Enterprise and Online portals}
\usage{
  arc.portal_connect(url, user, password)
  arc.check_portal()
}

\arguments{
\item{url}{The URL of the portal to be signed in to. (character) }
\item{user}{The user name of the user signing in to the portal. (character) }
\item{password}{The password of the user signing in to the portal. (character) }
}

\details{
If \code{url} already in active list of portals connections then \code{user} and \code{password} parameters are optional
}

\value{
An named list of portal connections.
\itemize{
  \item url - The URL of the current portal.
  \item user - The user name.
  \item version - The portal version.
  \item organization - The organization name.
  \item session - TODO.
  \item token - This is the Enterprise token for built-in logins.
  \item portals - \code{list} of active portals.
  \item offlines - \code{list} of offline portals.
}
}

\description{
The \code{arc.portal_connect()} function to sign in to a portal. To check available portals call \code{arc.check_portal()}.
Functions returns a list that contains active info and available portals.
}

\keyword{method}
\keyword{portal}
