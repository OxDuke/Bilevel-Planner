#
# This is:
#   attachbuilder(what_mosek_bindir, pos=2L, name="Rmosek:builder", warn.conflicts=TRUE)
#
# which mimics the built-in R function 'attach', except that the expected 
# 'what' argument is a path to a system installation of MOSEK. 
#
#

attachbuilder <- function(what_mosek_bindir=init_builder_from_scriptpath(), pos=2L, name="Rmosek:builder", warn.conflicts=TRUE)
{
  stop("Failed to initialize 'attachbuilder'. Please restart the R session and try again.")
}


#
# Initialization of 'attachbuilder'
#  - Encapsulated to avoid polution of execution environment if source()'d.
#
do.call(function(env)
{
  repr <- function(x) {
    ifelse(is.symbol(x), "", ifelse(is.character(x), paste0('"',x,'"'), x))
  }

  #
  # Helper functions for 'attachbuilder'
  #
  init_builder_default <- function() { 
    formals(function(what_mosek_bindir){})$what_mosek_bindir   # create a symbol named 'what_mosek_bindir' to throw standard error on missing function argument.
                                                               # NOTE: the direct creation, as.symbol('what_mosek_bindir'), throws non-standard error messages.
  }

  init_builder_from_scriptpath <- function() {
    out <- quote(init_builder_default())
    tryCatch({
      scriptpath <- function()
      {
        path <- getSrcDirectory(function(){})
        if(length(path)==0) {
          path <- tryCatch(dirname(sys.frame(1)$ofile), error=function(e){ getwd() })
        }
        if( file.exists(file.path(path,'builder.R')) ) { path } else { NA }
      }

      mypath <- scriptpath()
      if( is.na(mypath) ) {
        stop("Tried to make 'what_mosek_bindir' an optional argument, but the scriptpath could not be computed.", call.=FALSE)
      }

      guess <- file.path(mypath, "..", "bin")
      if( !file.exists(guess) ) {
        stop(paste0("Tried to make 'what_mosek_bindir' an optional argument, but '", guess, "' was not recognized as a MOSEK 'bin' directory."), call.=FALSE)
      }
      out <- quote(normalizePath(guess))
    }, error=function(e) warning(e))
    eval(out)
  }

  #
  # Definition of 'attachbuilder'
  #
  attachbuilder <- function(what_mosek_bindir=init_builder_from_scriptpath(), pos=2L, name="Rmosek:builder", warn.conflicts=TRUE)
  {
    #
    # Helper functions for builder utils
    #
    is_windows <- function()
    {
      (.Platform$OS.type=="windows")
    }

    guess_mosek_version_default <- function()
    {
      "<MOSEKMAJORVER>.<MOSEKMINORVER>"
    }

    guess_mosek_bindir_default <- function()
    {
      ""
    }

    guess_mosek_headerdir_default <- function()
    {
      ""
    }

    guess_mosek_libname_default <- function()
    {
      ""
    }

    guess_mosek_version_from_current_installation <- function(silent=FALSE)
    {
      out <- guess_mosek_version_default()
      tryCatch({
        guess <- packageDescription("Rmosek",fields="Configured.MSK_VERSION")
        tryCatch(stopifnot(
          !is.na(guess)), error=function(e) stop("Could not identify MOSEK version from current installation.", call.=FALSE))
        out <- guess
      }, error=function(e) if(!silent){warning(e)} )
      out
    }

    guess_mosek_bindir_from_current_installation <- function(silent=FALSE)
    {
      out <- guess_mosek_bindir_default()
      tryCatch({
        guess <- packageDescription("Rmosek",fields="Configured.MSK_BINDIR")
        tryCatch(stopifnot( 
          !is.na(guess), 
          file.exists(guess)), error=function(e) stop("Could not identify MOSEK 'bin' directory from current installation.", call.=FALSE))
        out <- normalizePath(guess)
      }, error=function(e) if(!silent){warning(e)} )
      out
    }

    guess_mosek_headerdir_from_current_installation <- function(silent=FALSE)
    {
      out <- guess_mosek_headerdir_default()
      tryCatch({
        guess <- packageDescription("Rmosek",fields="Configured.MSK_HEADERDIR")
        tryCatch(stopifnot( 
          !is.na(guess), 
          file.exists(guess)), error=function(e) stop("Could not identify MOSEK 'h' directory from current installation.", call.=FALSE))
        out <- normalizePath(guess)
      }, error=function(e) if(!silent){warning(e)} )
      out
    }

    guess_mosek_libname_from_current_installation <- function(silent=FALSE)
    {
      out <- guess_mosek_libname_default()
      tryCatch({
        arch <- R.Version()$arch
        archinfo <- list(i386="", x86_64="64")[[arch]]
        versioninfo <- ifelse(is_windows(), guess_mosek_version_from_current_installation(silent=TRUE), "")
        versioninfo <- sub("\\.","_",versioninfo)
        tryCatch(stopifnot(
            !is.null(archinfo)), error=function(e) stop(paste0("Could not identify MOSEK library name from current installation (arch=", arch, " was unknown)."), call.=FALSE))
        tryCatch(stopifnot(
            !is_windows() || nchar(versioninfo)>=1), error=function(e) stop("Could not identify MOSEK library name from current installation.", call.=FALSE))
        postfixlist <- list(archinfo,versioninfo)
        postfix <- paste(postfixlist[lapply(postfixlist,nchar)>=1],collapse="_")
        out <- paste0("mosek", postfix)
      }, error=function(e) if(!silent){warning(e)} )
      out
    }

    guess_mosek_version_from_mosek_bindir <- function(silent=FALSE)
    {
      out <- guess_mosek_version_default()
      tryCatch({
        mosekh <- readLines(file.path(what_mosek_bindir,'..','h','mosek.h'))
        major <- sub("[^0-9]*", "", grep("MSK_VERSION_MAJOR",mosekh,value=TRUE))
        minor <- sub("[^0-9]*", "", grep("MSK_VERSION_MINOR",mosekh,value=TRUE))
        guess <- paste0(major,".",minor)
        out <- guess
      }, error=function(e) if(!silent){warning(e)} )
      out
    }

    guess_mosek_bindir_from_mosek_bindir <- function(silent=FALSE)
    {
      out <- guess_mosek_bindir_default()
      tryCatch({
        guess <- what_mosek_bindir
        tryCatch(stopifnot( 
          !is.na(guess), 
          file.exists(guess)), error=function(e) stop("Could not identify MOSEK 'bin' directory from argument 'what_mosek_bindir'.", call.=FALSE))
        out <- normalizePath(guess)
      }, error=function(e) if(!silent){warning(e)} )
      out
    }

    guess_mosek_headerdir_from_mosek_bindir <- function(silent=FALSE)
    {
      out <- guess_mosek_headerdir_default()
      tryCatch({
        guess <- file.path(what_mosek_bindir,'..','h')
        tryCatch(stopifnot( 
          !is.na(guess), 
          file.exists(guess)), error=function(e) stop("Could not identify MOSEK 'h' directory from argument 'what_mosek_bindir'.", call.=FALSE))
        out <- normalizePath(guess)
      }, error=function(e) if(!silent){warning(e)} )
      out
    }

    guess_mosek_libname_from_mosek_bindir <- function(silent=FALSE)
    {
      out <- guess_mosek_libname_default()
      tryCatch({
        arch <- R.Version()$arch
        archinfo <- list(i386="", x86_64="64")[[arch]]
        versioninfo <- ifelse(is_windows(), guess_mosek_version_from_mosek_bindir(silent=TRUE), "")
        versioninfo <- sub("\\.","_",versioninfo)
        tryCatch(stopifnot(
            !is.null(archinfo)), error=function(e) stop(paste0("Could not identify MOSEK library name from argument 'what_mosek_bindir' (arch=", arch, " was unknown)."), call.=FALSE))
        tryCatch(stopifnot(
            !is_windows() || nchar(versioninfo)>=1), error=function(e) stop("Could not identify MOSEK library name from argument 'what_mosek_bindir'.", call.=FALSE))
        postfixlist <- list(archinfo,versioninfo)
        postfix <- paste(postfixlist[lapply(postfixlist,nchar)>=1],collapse="_")
        out <- paste0("mosek", postfix)
      }, error=function(e) if(!silent){warning(e)} )
      out
    }

    if(is.na(what_mosek_bindir))
    {
      if(is.na(packageDescription("Rmosek",fields="Configured.MSK_BINDIR"))) {
        stop(paste0("Failed to load default arguments from the package description, as requested by first input argument 'NA'."))
      }
      guess_mosek_version   = guess_mosek_version_from_current_installation
      guess_mosek_bindir    = guess_mosek_bindir_from_current_installation
      guess_mosek_headerdir = guess_mosek_headerdir_from_current_installation
      guess_mosek_libname   = guess_mosek_libname_from_current_installation
    }
    else
    {
      if( !is.character(what_mosek_bindir) ) {
        stop(paste0("Unexpected input argument type '", typeof(what_mosek_bindir), "'."))
      }
      else if( nchar(what_mosek_bindir)==0 )
      {
        guess_mosek_version   = guess_mosek_version_default
        guess_mosek_bindir    = guess_mosek_bindir_default
        guess_mosek_headerdir = guess_mosek_headerdir_default
        guess_mosek_libname   = guess_mosek_libname_default
      }
      else
      {
        mosekh = file.path(what_mosek_bindir, "..", "h", "mosek.h")
        if (!file.exists(mosekh)) {
          stop(paste0("Failed to load default arguments from the unavailable file: '",mosekh, "', as requested by first input argument '", what_mosek_bindir, "'."))
        }
        guess_mosek_version   = guess_mosek_version_from_mosek_bindir
        guess_mosek_bindir    = guess_mosek_bindir_from_mosek_bindir
        guess_mosek_headerdir = guess_mosek_headerdir_from_mosek_bindir
        guess_mosek_libname   = guess_mosek_libname_from_mosek_bindir
      }
    }

    #
    # Definition of builder utils
    #
    install.rmosek <- function(
      pkgs           = "Rmosek",
      repos          = paste0("https://download.mosek.com/R/", guess_mosek_version()),
      MSK_BINDIR     = guess_mosek_bindir(),
      MSK_HEADERDIR  = guess_mosek_headerdir(),
      MSK_LIB        = guess_mosek_libname(),
      using_pkgbuild = is_windows(),
      using_sysenv   = is_windows(),
      type           = "source",
      INSTALL_opts   = "--no-multiarch",
      configure.vars = ifelse(using_sysenv, "", paste0("MSK_BINDIR=",MSK_BINDIR," ","MSK_HEADERDIR=",MSK_HEADERDIR," ","MSK_LIB=",MSK_LIB," ",
                                                       "PKG_MOSEKHOME=",dirname(MSK_BINDIR)," ","PKG_MOSEKLIB=",MSK_LIB)),  # Legacy support
      ...)
    {
      if( using_sysenv )
      {
        Sys.setenv(MSK_BINDIR=MSK_BINDIR)
        Sys.setenv(MSK_HEADERDIR=MSK_HEADERDIR)
        Sys.setenv(MSK_LIB=MSK_LIB)
        Sys.setenv(PKG_MOSEKHOME=dirname(MSK_BINDIR))   # Legacy support
        Sys.setenv(PKG_MOSEKLIB=MSK_LIB)                # Legacy support
      }

      if( !using_pkgbuild )
      {
        install.packages(pkgs=pkgs, repos=repos, type=type, INSTALL_opts=INSTALL_opts, configure.vars=configure.vars, ...)
      }
      else
      {
        if(!requireNamespace("pkgbuild", quietly=TRUE)) 
        { 
          windowsnote <- ifelse(is_windows(), " if you believe the package is unnecessary (e.g., to locate Rtools on your machine)", "")
          stop(paste0("Package 'pkgbuild' not available. Either install it by calling 'install.packages(\"pkgbuild\")' or add argument 'using_pkgbuild=FALSE' to this function call", windowsnote, "."))
        }
        pkgbuild::with_build_tools({ 
        install.packages(pkgs=pkgs, repos=repos, type=type, INSTALL_opts=INSTALL_opts, configure.vars=configure.vars, ...)
        }, debug=TRUE)
      }

      if( any(is.element(pkgs,loadedNamespaces())) ) {
        warning("Please restart the R session for changes to take effect.", call.=FALSE)
      }
    }

    update.rmosek <- function(
      oldPkgs        = "Rmosek",
      repos          = paste0("https://download.mosek.com/R/", guess_mosek_version()),
      MSK_BINDIR     = guess_mosek_bindir(),
      MSK_HEADERDIR  = guess_mosek_headerdir(),
      MSK_LIB        = guess_mosek_libname(),
      using_pkgbuild = is_windows(),
      using_sysenv   = is_windows(),
      type           = "source",
      INSTALL_opts   = "--no-multiarch",
      configure.vars = ifelse(using_sysenv, "", paste0("MSK_BINDIR=",MSK_BINDIR," ","MSK_HEADERDIR=",MSK_HEADERDIR," ","MSK_LIB=",MSK_LIB," ",
                                                       "PKG_MOSEKHOME=",dirname(MSK_BINDIR)," ","PKG_MOSEKLIB=",MSK_LIB)),  # Legacy support
      ...)
    {
      if( using_sysenv )
      {
        Sys.setenv(MSK_BINDIR=MSK_BINDIR)
        Sys.setenv(MSK_HEADERDIR=MSK_HEADERDIR)
        Sys.setenv(MSK_LIB=MSK_LIB)
        Sys.setenv(PKG_MOSEKHOME=dirname(MSK_BINDIR))   # Legacy support
        Sys.setenv(PKG_MOSEKLIB=MSK_LIB)                # Legacy support
      }

      if( !using_pkgbuild )
      {
        update.packages(oldPkgs=oldPkgs, repos=repos, type=type, INSTALL_opts=INSTALL_opts, configure.vars=configure.vars, ...)
      }
      else
      {
        if(!requireNamespace("pkgbuild", quietly=TRUE)) 
        { 
          windowsnote <- ifelse(is_windows(), " if you believe the package is unnecessary (e.g., to locate Rtools on your machine)", "")
          stop(paste0("Package 'pkgbuild' not available. Either install it by calling 'install.packages(\"pkgbuild\")' or add argument 'using_pkgbuild=FALSE' to this function call", windowsnote, "."))
        }
        pkgbuild::with_build_tools({
        update.packages(oldPkgs=oldPkgs, repos=repos, type=type, INSTALL_opts=INSTALL_opts, configure.vars=configure.vars, ...)
        }, debug=TRUE)
      }

      if( any(is.element(oldPkgs,loadedNamespaces())) ) {
        warning("Please restart the R session for changes to take effect.", call.=FALSE)
      }
    }

    remove.rmosek <- function(
    pkgs = "Rmosek",
    ...)
    {
      remove.packages(pkgs=pkgs, ...)
    }

    #
    # Eager evaluation of default arguments in builder utils
    #
    builderenv <- new.env()
    for (fn in c('install.rmosek','update.rmosek')) {
      f <- get(fn)
      for (arg in c('repos','MSK_BINDIR','MSK_HEADERDIR','MSK_LIB','using_pkgbuild','using_sysenv')) {
        if( !is.symbol(formals(f)[[arg]]) ) {
          formals(f)[[arg]] <- eval(formals(f)[[arg]])
        }
      }
      assign(fn,f,envir=builderenv)
    }
    for (fn in c('remove.rmosek')) {
      f <- get(fn)
      assign(fn,f,envir=builderenv)
    }

    #
    # Attach builder utils
    #
    r <- try({detach(name, character.only=TRUE)}, silent=TRUE)
    if( !inherits(r, "try-error") && warn.conflicts )
    {
      cat(paste0("NOTE: Overwriting the database '", name ,"' on the search path.\n"))
    }
    attach(builderenv,pos,name,warn.conflicts)

    #
    # Introduce builder utils to the user
    #
    cat("
    Functions attached to the search path:
      install.rmosek     - install Rmosek package.
      update.rmosek      - update Rmosek package.
      remove.rmosek      - remove Rmosek package.

    The *.rmosek functions wrap the corresponding utils::*.packages functions
    with the following default argument values:
      pkgs           = ", repr(formals(builderenv$install.rmosek)[['pkgs']]) ,"
      repos          = ", repr(formals(builderenv$install.rmosek)[['repos']]), "

    configuration variables:
      MSK_BINDIR     = ", repr(formals(builderenv$install.rmosek)[['MSK_BINDIR']]), "
                        # if empty, autoconfigured from mosek executable on PATH.

      MSK_HEADERDIR  = ", repr(formals(builderenv$install.rmosek)[['MSK_HEADERDIR']]), "
                        # if empty, autoconfigured from 'MSK_BINDIR'.

      MSK_LIB        = ", repr(formals(builderenv$install.rmosek)[['MSK_LIB']]), "
                        # if empty, autoconfigured from 'MSK_BINDIR'.

    and installation styles:
      using_pkgbuild = ", repr(formals(builderenv$install.rmosek)[['using_pkgbuild']]), "
                        # whether to execute in the 'pkgbuild' environment
                        # as is recommended on Windows to resolve Rtools.
                        
      using_sysenv   = ", repr(formals(builderenv$install.rmosek)[['using_sysenv']]), "
                        # whether to transmit configuration variables via Sys.setenv()
                        # as opposed to configure.vars.

    See, e.g., `show(install.rmosek)` for its implementation.

", sep="")

  }

  #
  # Eager evaluation of default 'attachbuilder' function arguments
  #
  for (fn in c('attachbuilder')) {
    f <- get(fn)
    for (arg in c('what_mosek_bindir')) {
      if( !is.symbol(formals(f)[[arg]]) ) {
        formals(f)[[arg]] <- eval(formals(f)[[arg]])
      }
    }
    assign(fn,f,pos=env)
  }

  #
  # Introduce 'attachbuilder' to the user
  #
  if( TRUE )
  {
    bindirarg <- quote(formals(env$attachbuilder)[['what_mosek_bindir']])
    cat("
    You may now call 'attachbuilder(", ifelse(!is.symbol(eval(bindirarg)),"","what_mosek_bindir") , ")' with optional extra arguments:

        attachbuilder(what_mosek_bindir", ifelse(is.symbol(eval(bindirarg)),"",paste0("=",repr(eval(bindirarg)))), ", pos=2L, name=\"Rmosek:builder\", warn.conflicts=TRUE)

    to attach builder functions (e.g., 'install.rmosek') to the search path.

", sep="")
  }

}, list(env=environment()))

attachbuilder
