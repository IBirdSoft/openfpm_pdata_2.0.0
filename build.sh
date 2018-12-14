#! /bin/bash

# Make a directory in /tmp/OpenFPM_pdata

workspace=$1
hostname=$(hostname)
target=$3
comp_type=$4
branch=$5

echo "Directory: $workspace"
echo "Machine: $hostname"
echo "make target: $target"
echo "compilation type: $comp_type"
echo "Branch name: $branch"

rm -rf $HOME/openfpm_dependencies/openfpm_pdata/0

if [ x"$hostname" == x"cifarm-centos-node.mpi-cbg.de"  ]; then
	./install_MPI_mpich.sh $HOME/openfpm_dependencies/openfpm_pdata/$branch/ 4
	echo 4 > $HOME/openfpm_dependencies/openfpm_pdata/$branch/MPI/version
	rm -rf $HOME/openfpm_dependencies/openfpm_pdata/full
fi

if [ x"$hostname" == x"cifarm-ubuntu-node.mpi-cbg.de"  ]; then
	rm -rf $HOME/openfpm_dependencies/openfpm_pdata/full
fi

if [ x"$hostname" == x"cifarm-mac-node.mpi-cbg.de"  ]; then
	export PATH="/usr/local/bin:$PATH"
	rm -rf $HOME/openfpm_dependencies/openfpm_pdata/full
fi


if [ x"$branch" == x"" ]; then
  branch=$(git ls-remote --heads origin | grep $(git rev-parse HEAD) | cut -d / -f 3)
fi

#### If you have a dep_dir file change the branch name to the dep_dir

dep_dir=$(cat dep_dir)
if [ x"$dep_dir" != x"" ]; then
  branch=$dep_dir
fi

mkdir src/config
mkdir openfpm_numerics/src/config

if [ "$hostname" == "gin" ]
then
 echo "Compiling on gin\n"

 source "$HOME/.bashrc"

 ## Check if MPI folder exist if not copy MPICH

 if [ ! -d $HOME/$branch/MPI ]; then
   echo "COPY MPICH"
   cp -R $HOME/MPI $HOME/$branch/MPI
   echo 2 > $HOME/$branch/MPI/version
 fi

 ### Activate MPI and binutils ###

 export PATH="$PATH:$HOME/$branch/MPI/bin"
 export PATH="/usr/local/binutils/bin/:$PATH"

 mkdir $HOME/$branch
 if [ x"$comp_type" == x"full" ]; then
  CC=gcc-4.9.2 CXX=g++-4.9.2 FC=gfortran-4.9.2 F77=gfortran-4.9.2 ./install -i $HOME/$branch  -s -c "--prefix=/home/jenkins/openfpm_install"
  echo "Moving environment variable"
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
 elif [ x"$comp_type" == x"numerics" ]; then
  branch=$(git ls-remote --heads origin | grep $(git rev-parse HEAD) | cut -d / -f 3)
  CC=gcc-4.9.2 CXX=g++-4.9.2 FC=gfortran-4.9.2 F77=gfortran-4.9.2 ./install -i $HOME/$branch  -m -s -c "--prefix=/home/jenkins/openfpm_install"
  echo "Moving environment variable"
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
  make -j 12
 else
  CC=gcc-4.9.2 CXX=g++-4.9.2 FC=gfortran-4.9.2 F77=gfortran-4.9.2 ./install -i $HOME/$branch  -m -s -c "--prefix=/home/jenkins/openfpm_install --no-recursion"
  echo "Moving environment variables"
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
  make -j 12
 fi

 if [ $? -ne 0 ]; then
   curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$hostname failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
   exit 1 ;
 fi


 if [ $? -ne 0 ]; then
   curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$hostname failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
   exit 1 ; 
 fi

elif [ "$hostname" == "taurus" ]
then
 echo "Compiling on taurus"

 source /etc/profile
 echo "$PATH"
 module load gcc/7.1.0
 module load openmpi/3.0.0-gnu7.1
 
 export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/incard/PARMETIS/lib:/home/incard/METIS/lib:/home/incard/HDF5/lib"

 mkdir /scratch/p_ppm/$branch
 ./install -m -i "/scratch/p_ppm/$branch" -s -c"CXX=mpic++ --no-recursion"
 mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
 source $HOME/openfpm_vars_$branch
 make -j 24


 if [ $? -ne 0 ]; then
   curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$hostname failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
   exit 1 ; 
 fi
else
 echo "Compiling general"

 source ~/.bashrc
 
 installation_dir=""
 if [ x"$hostname" == x"sbalzarini-mac-15" ]; then
  installation_dir="--prefix=/Users/jenkins/openfpm_install"
 else
  installation_dir="--prefix=$HOME/openfpm_install/$branch"
 fi

 # force ssh to not use HostKey verification
 echo "StrictHostKeyChecking=no" > $HOME/.ssh/config
 chmod 600 $HOME/.ssh/config

 mkdir $HOME/openfpm_dependencies/openfpm_pdata/$branch
 if [ x"$comp_type" == x"full" ]; then
  echo "Installing with: ./install -i $HOME/openfpm_dependencies/openfpm_pdata/$branch  -s -c \"$installation_dir\"  "
  ./install -i $HOME/openfpm_dependencies/openfpm_pdata/$branch  -s -c "$installation_dir"
  if [ $? -ne 0 ]; then
    curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$hostname failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
    exit 1 ;
  fi
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
 elif [ x"$comp_type" == x"numerics" ]; then
  branch=$(git ls-remote --heads origin | grep $(git rev-parse HEAD) | cut -d / -f 3)
  ./install -i $HOME/openfpm_dependencies/openfpm_pdata/$branch  -m -s -c "$installation_dir"
  if [ $? -ne 0 ]; then
    curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$hostname failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
    exit 1 ;
  fi
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
  make VERBOSE=1  -j 8
 else
  echo "Installing with: ./install -i $HOME/openfpm_dependencies/openfpm_pdata/$branch -m -s -c \"$installation_dir --no-recursion\""
  ./install -i $HOME/openfpm_dependencies/openfpm_pdata/$branch -m -s -c "$installation_dir --no-recursion"
  if [ $? -ne 0 ]; then
    curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$hostname failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
    exit 1 ;
  fi
  mv $HOME/openfpm_vars $HOME/openfpm_vars_$branch
  source $HOME/openfpm_vars_$branch
  make VERBOSE=1  -j 8
 fi

 if [ $? -ne 0 ]; then
   curl -X POST --data "payload={\"icon_emoji\": \":jenkins:\", \"username\": \"jenkins\"  , \"attachments\":[{ \"title\":\"Error:\", \"color\": \"#FF0000\", \"text\":\"$hostname failed to complete the openfpm_pdata test \" }] }" https://hooks.slack.com/services/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
   exit 1 ;
 fi

fi


