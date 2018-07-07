\name{arc.check_product}
\alias{arc.check_product}
\title{ArcGIS product and license information}
\usage{
arc.check_product()
}

\description{
Initialize connection to ArcGIS. Any script running directly from R (i.e.
without being called from a Geoprocessing script) should first call
\code{arc.check_product} to create a connection with ArcGIS. Provides
installation details on the version of ArcGIS installed that
\code{arcgisbinding} is communicating with. Failure to run this function successfuly implies a problem with ArcGIS installation or environment variables for ArcGIS.
}
\value{
a named list is returned with the following components:
  \describe{
    \item{\code{app}}{Product: ArcGIS Desktop (i.e. ArcMap), or ArcGIS Pro. The name of the product connected.}
    \item{\code{license}}{License level: Basic, Standard, or Advanced are the three licensing
            levels available. Each provides progressively more functionality within the software.
            See the "Desktop Functionality Matrix" link for details.}
    \item{\code{version}}{Build number: The build number of the release being used.
            Useful in debugging and when creating error reports.}
    \item{\code{dll}}{DLL: The dynamic linked library (DLL) in use allowing
            ArcGIS to communicate with R.}
  }
}
\note{
Additional license levels are available on ArcGIS Desktop: Server,
EngineGeoDB, and Engine. These license levels are currently
unsupported by this package.
}
\section{References}{
 \href{https://assets.esri.com/content/dam/esrisites/media/pdf/product/desktop/ArcGIS-10.6-Desktop-ArcMap-Functionality-Matrix.pdf}{ArcGIS Desktop Functionality Matrix}
}
\examples{

info <- arc.check_product()
info$license # ArcGIS license level
info$version # ArcGIS build number
info$app # product name
info$dll # binding DLL in use
}

