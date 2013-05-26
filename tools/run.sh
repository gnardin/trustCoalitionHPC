#!/bin/bash

if [ $# -lt 1 ]
then
  echo
  echo "usage: run.sh <number of processes>"
  echo
  exit 1
fi

mpirun -np $1 ../Debug/trustCoalitionHPC ../conf/config.props ../conf/model.props
