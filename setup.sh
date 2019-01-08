# setup.sh

# Setup file for protodune-adc-calib.
#
# If protodune-adc-calib is installed at SOMEDIR, run this
# from the directory where you plan to run Root:
#
# > source SOMEDIR/protodune-adc-calib/setup.sh [ACLICBASE]
#
# The variable ACLICBASE is the prefix for build directory.

# Function that returns true if $1 is in path $2
# This definition is a side effect of sourcing this script.
isInPath() {
  case ":$2:" in
    *":$1:"* ) return 0; ;;
  esac
  return 1;
}

ACLICBASE=$1

# Do some work to get the directory where this script resides.
CALLED=$0
if [ "$CALLED" = bash -o "$CALLED" = "-bash" ]; then
  CALLED=$BASH_SOURCE
fi
#echo $CALLED
CALLDIR=`dirname $CALLED`
# Handle absolute and relative path names.
if [ ${CALLED:0:1} = / ]; then
  PKGDIR=$CALLDIR
else
  PKGDIR=`pwd`/$CALLDIR
fi
PKGDIR=`readlink -f $PKGDIR`
LOCAL_SETUP_NAME=setup_protodune-adc-calib.sh
LOCAL_SETUP_FILE=`pwd`/$LOCAL_SETUP_NAME
if false; then
  echo "Local setup file is $LOCAL_SETUP_FILE"
  echo "PKGDIR is $PKGDIR"
elif [ ! -r $PKGDIR/setup.sh ]; then
  echo Unable to locate the sourced package script
else
  echo Setting up package at $PKGDIR

  if [ -z "$DUNETPC_VERSION" ]; then
    echo "Please set up dunetpc."
  else
    CURDIR=`readlink -f .`
    if [ -z "$ACLICBASE" ]; then
      #ACLICDIR=$PKGDIR/.aclic
      ACLICDIR=$CURDIR/.aclic
    else
      ACLICDIR=$ACLICBASE
    fi
    mkdir -p $ACLICDIR
    ACLICDIR=`readlink -f $ACLICDIR`
    ACLICDIRNORES=$ACLICDIR/\$DUNETPC_VERSION
    ACLICDIR=$ACLICDIR/$DUNETPC_VERSION

    # Add lib dir to library path.
    LIBDIR=$ACLICDIR/lib
    if ! isInPath $LIBDIR $LD_LIBRARY_PATH; then
      LD_LIBRARY_PATH=$LIBDIR:$LD_LIBRARY_PATH
      echo Added $LIBDIR to library path
    fi
    # Add bin dir to exe path.
    BINDIR=$PKGDIR/bin
    if ! isInPath $BINDIR $PATH; then
      PATH=$BINDIR:$PATH
      echo Added $BINDIR to exe path
    fi
    # Add fcl dir to fcl path.
    FCLDIR=$PKGDIR/fcl
    if ! isInPath $FCLDIR $FHICL_FILE_PATH; then
      FHICL_FILE_PATH=$FCLDIR:$FHICL_FILE_PATH
      echo Added $FCLDIR to fcl path
    fi

    # Create pkginit.C if not already present.
    FOUT=pkginit.C
    if [ ! -r $FOUT ]; then
      echo 'void pkginit() {' >> $FOUT
      echo '  gROOT->ProcessLine(' >> $FOUT
      echo '    ".x '$PKGDIR'/init.C("' >> $FOUT
      echo '      "\"'$PKGDIR'\","' >> $FOUT
      echo '      "\"'$ACLICDIRNORES'\""' >> $FOUT
      echo '    ")"' >> $FOUT
      echo '  );' >> $FOUT
      echo '}' >> $FOUT
      echo Created $FOUT
    fi

    # Create rootlogon.C if not already present.
    FOUT=rootlogon.C
    if [ ! -r $FOUT ]; then
      echo '{' >> $FOUT
      echo '  gROOT->ProcessLine(".x pkginit.C");' >> $FOUT
      echo '  gROOT->ProcessLine("ArtServiceHelper::load(\"run_dataprep.fcl\")");' >> $FOUT
      echo '  gROOT->ProcessLine("DuneToolManager::instance(\"run_dataprep.fcl\")");' >> $FOUT
      echo '}' >> $FOUT
      echo Created $FOUT
    fi

    # Create local setup file.
    if [ ! -r $LOCAL_SETUP_FILE ]; then
      echo "source $PKGDIR/setup.sh" >$LOCAL_SETUP_FILE
      echo "In future, set up with"
      echo "> source $LOCAL_SETUP_NAME"
    fi
  fi
fi
