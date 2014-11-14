#!/bin/bash
#
# Make tensor easier.
# Joshua Krueger
# 05_19_2008

export TENSOR_PATH="/home/sherman/tensor"

stopit() {
  ps -fea | while read -r bline ; do
    if [[ ! -z "$(echo $bline | grep $TENSOR_PATH)" ]] ; then
      this="$(echo $bline | while read -r bl bl bl bl bl bl bl oi ; do echo $oi ; done)"
      killall -9 $this 1>&2 2>/dev/null
    fi
  done
  exit 1
}

trap stopit 2

# Check the net
if [[ -z "$(ifconfig | grep eth0:0)" ]] ; then
  sudo ifconfig eth0 add 10.5.1.99
  if [[ -z "$(ifconfig | grep eth0:0)" ]] ; then
    echo "Josh sucks.  This won't work."
    exit 1
  else
    sudo ifconfig eth0:0 netmask 255.255.255.0
  fi
fi

if [[ "$#" == "0" ]] ; then
  echo "Usage: tensor <pattern>"
  echo "       tensor all [<seconds for each>]"
  echo
  echo "  Available patterns:"
  ls ${TENSOR_PATH} | while read -r aline ; do
    echo "    $aline"
  done
  echo
  exit 1
fi

if [[ "$1" == "all" ]] ; then
  if [[ "$#" == "2" ]] ; then
    second="${2}"
  fi

  while [[ 1 ]] ; do
    ls ${TENSOR_PATH} | while read -r aline ; do
      ${TENSOR_PATH}/${aline} &
      sleep ${second:-20}
      killall -9 ${TENSOR_PATH}/${aline} 1>&2 2>/dev/null
    done
  done
fi


if [[ -e "${TENSOR_PATH}/${1}" ]] ; then
  $TENSOR_PATH/${1}
else
  echo "Pattern \"$1\" not recognized.  Valid patterns are:"
  ls ${TENSOR_PATH} | while read -r aline ; do
    echo "    $aline"
  done
  echo
  exit 1
fi

  
