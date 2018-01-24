R-ArcGIS bridge
===============
#### Requirements
 - [ArcGIS 10.3.1 or later](http://desktop.arcgis.com/en/desktop/) or [ArcGIS Pro 1.1 or later](http://pro.arcgis.com/en/pro-app/) ([don't have it? try a 60 day trial](http://www.esri.com/software/arcgis/arcgis-for-desktop/free-trial))
 - [R Statistical Computing Software, 3.3.2 or later](http://cran.cnr.berkeley.edu/bin/windows/base/) ([What is R?](http://www.r-project.org/about.html))
  + 32-bit version required for ArcMap, 64-bit version required for ArcGIS Pro (Note: the installer installs both by default).
  + 64-bit version can be used with ArcMap by installing [Background Geoprocessing](http://desktop.arcgis.com/en/desktop/latest/analyze/executing-tools/64bit-background.htm) and configuring scripts to [run in the background](http://desktop.arcgis.com/en/desktop/latest/analyze/executing-tools/foreground-and-background-processing.htm). NOTE: Background geoprocessing only allows using the bridge from ArcGIS, not from within R itself.

#### Installation

For most users, the easiest way to install is using the [installation toolbox](https://github.com/R-ArcGIS/r-bridge-install) which will install and configure the bridge for ArcGIS 10.3.1+ and Pro 1.1+. Alternatively, [downloading the release](https://github.com/R-ArcGIS/r-bridge/releases/latest) can be manually installed into R, as shown [in this screencast](https://4326.us/R/zipinst/).

#### Basic GP Tool script
```R
tool_exec <- function (in_params, out_params)
{
  value0 <- in_params[[1]]
  print(value0)
# ...
  return (out_params)
}
```
- `tool_exec(in_params, out_params)` main function
- `in_params` list of all input parameters. You can get parameter value by index `in_params[[1]]` or by parameter name `in_params$param1`
- `output_params` list of all output and derived parameters.

#### Using `arcgisbinding` in standalone R script
```R
> library(arcgisbinding)
> arc.check_product()
> d <- arc.open("c:/mydb.gdb/points")
```

#### Documentation
 - The [arcgisbinding vignette](https://r-arcgis.github.io/assets/arcgisbinding-vignette.html) providesan overview of what the bridge does, and how to use it effectively.
 - The [arcgisbinding manual](https://r-arcgis.github.io/assets/arcgisbinding.pdf) provides details on each of the functions included within the package.

#### Build
```bash
> git clone https://github.com/R-ArcGIS/r-bridge.git`
> cd r-bridge\buid\
> build.bat pro desktop
```

###### Build dependencies:
- [Rtools 3.3](http://cran.r-project.org/bin/windows/Rtools)
- (optional) Visual Studio 2017 Update 2. [Get free Visual Studio Community](https://www.visualstudio.com/products/free-developer-offers-vs)

###### Documentation dependencies:
- [roxygen2](https://github.com/yihui/roxygen2), install from R: `install.packages('roxygen2')`
- [MiKTeX 2.9](http://miktex.org/), to build package PDF

## Project Details

Check out the [R-ArcGIS Website](https://r-arcgis.github.io) for related projects and extensions built on this library.

## Credits

This package depends on the R Statistical Computing Software:

> Copyright (C) 2015 The R Foundation for Statistical Computing
> R is free software and comes with ABSOLUTELY NO WARRANTY.
> See the [COPYRIGHTS file](https://github.com/wch/r-source/blob/trunk/doc/COPYRIGHTS) for details.

## License
[Apache 2.0](LICENSE)
