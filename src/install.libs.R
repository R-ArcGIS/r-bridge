src <- ifelse(R_ARCH == "/i386",
  file.path(R_PACKAGE_SOURCE, paste0("libs", "/Win32"),"*.dll"),
  file.path(R_PACKAGE_SOURCE, paste0("libs", R_ARCH),"*.dll"))

dest <- file.path(R_PACKAGE_DIR, paste0('libs', R_ARCH))
dir.create(dest, recursive = TRUE, showWarnings = FALSE)
files <- Sys.glob(src)
file.copy(files, dest, overwrite = TRUE)
