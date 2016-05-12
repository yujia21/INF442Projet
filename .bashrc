# .bashrc

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

# Prompt
PS1="[\h \W]\\$ "

# User specific aliases and functions
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'
alias mpiboost='/usr/local/openmpi-1.8.3/bin/mpic++ 
-I/usr/local/boost-1.56.0/include/ -L/usr/local/boost-1.56.0/lib/ -lboost_mpi -lboost_serialization'
