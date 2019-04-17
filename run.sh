#! /bin/bash

# Make a directory in /tmp/OpenFPM_pdata

workspace=$1
hostname=$(hostname)
nproc=$3
ntask_per_node=$5
nodes=$4
branch=$6

if [ x"$branch" == x"" ]; then
  branch=$(git ls-remote --heads origin | grep $(git rev-parse HEAD) | cut -d / -f 3)
fi

echo "Directory: workspace"
echo "Machine: $hostname"
echo "Num of processors: $nproc"
echo "Branch: $branch"

if [ x"$hostname" == x"cifarm-mac-node.mpi-cbg.de"  -o x"$hostname" == x"suitcase"  ]; then
        mpi_options="--oversubscribe"
fi

if [ x"$hostname" == x"cifarm-ubuntu-node.mpi-cbg.de"  ]; then
        mpi_options="--mca btl self,vader"
fi


echo "Running general machine"
if [ x"$hostname" == x"no_test" ]; then
   exit 0;
fi

source $HOME/openfpm_vars_$branch

mpirun $mpi_options -np $nproc ./build/src/pdata
if [ $? -ne 0 ]; then
   curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$hostname failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
   exit 1 ;
fi


