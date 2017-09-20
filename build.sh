#! /bin/bash

# Make a directory in /tmp/OpenFPM_pdata

echo "Directory: $1"
echo "Machine: $2"
echo "Branch name: $5"


if [ x"$5" == x"" ]; then
  branch=$(git ls-remote --heads origin | grep $(git rev-parse HEAD) | cut -d / -f 3)
else
  branch=$5
fi

#### If you have a dep_dir file change the branch name to the dep_dir

dep_dir=$(cat dep_dir)
if [ x"$dep_dir" != x"" ]; then
  set -- "${@:1:4}" "$dep_dir"
fi

mkdir src/config
mkdir openfpm_numerics/src/config


if [ "$2" == "gin" ]
then
 echo "Compiling on gin\n"

 source "$HOME/.bashrc"

 ## Check if MPI folder exist if not copy MPICH

 if [ ! -d $HOME/$branch/MPI ]; then
   echo "COPY MPICH"
   cp -R $HOME/MPI $HOME/$branch
   echo 2 > $HOME/$branch/MPI/version
 fi

 ### Activate MPI and binutils ###

 export PATH="$PATH:$HOME/$branch/MPI/bin"
 export PATH="/usr/local/binutils/bin/:$PATH"

 mkdir $HOME/$branch
 if [ x"$4" == x"full" ]; then
  CC=gcc-4.9.2 CXX=g++-4.9.2 FC=gfortran-4.9.2 F77=gfortran-4.9.2 ./install -i $HOME/$branch  -s -c "--prefix=/home/jenkins/openfpm_install"
  echo "Moving environment variable"
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
 elif [ x"$3" == x"numerics" ]; then
  branch=$(git ls-remote --heads origin | grep $(git rev-parse HEAD) | cut -d / -f 3)
  CC=gcc-4.9.2 CXX=g++-4.9.2 FC=gfortran-4.9.2 F77=gfortran-4.9.2 ./install -i $HOME/$branch  -m -s -c "--prefix=/home/jenkins/openfpm_install"
  echo "Moving environment variable"
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
  make $3
 else
  CC=gcc-4.9.2 CXX=g++-4.9.2 FC=gfortran-4.9.2 F77=gfortran-4.9.2 ./install -i $HOME/$branch  -m -s -c "--prefix=/home/jenkins/openfpm_install --no-recursion"
  echo "Moving environment variables"
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
  make $3
 fi

 if [ $? -ne 0 ]; then
   curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$2 failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
   exit 1 ;
 fi


 if [ $? -ne 0 ]; then
   curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$2 failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
   exit 1 ; 
 fi

elif [ "$2" == "taurus" ]
then
 echo "Compiling on taurus"

 source /etc/profile
 echo "$PATH"
 module load eigen/3.2.0
 module load suitesparse/4.2.1-gnu-multimkl
 module load boost/1.60.0
 module load gcc/5.3.0
 module load openmpi/1.10.2-gnu
 module unload bullxmpi
 
 export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/incard/PARMETIS/lib:/home/incard/METIS/lib:/home/incard/HDF5/lib"

 mkdir /scratch/p_ppm/$branch
 ./install -m -i "/scratch/p_ppm/$branch" -s -c"CXX=mpic++ --no-recursion"
 mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
 source $HOME/openfpm_vars_$branch
 make $3


 if [ $? -ne 0 ]; then
   curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$2 failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
   exit 1 ; 
 fi
else
 echo "Compiling general"

 source ~/.bashrc

 mkdir $HOME/$branch
 if [ x"$4" == x"full" ]; then
  ./install -i $HOME/$branch  -s -c "--prefix=/Users/jenkins/openfpm_install"
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
 elif [ x"$3" == x"numerics" ]; then
  branch=$(git ls-remote --heads origin | grep $(git rev-parse HEAD) | cut -d / -f 3)
  ./install -i $HOME/$branch  -m -s -c "--prefix=/home/jenkins/openfpm_install"
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
  make $3
 else
  ./install -i $HOME/$branch -m -s -c "--prefix=/Users/jenkins/openfpm_install --no-recursion"
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
  make $3
 fi

 if [ $? -ne 0 ]; then
   curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$2 failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
   exit 1 ;
 fi

fi


