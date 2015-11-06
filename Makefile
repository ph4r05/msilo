# $Id$
#
# example module makefile
#
#
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=msilo.so

#
# Dynamic linking of rabbitmq library:
#  1. Create a new file: /etc/ld.so.conf.d/rabbitmq-x86_64.conf
#    /usr/local/lib64
#  2. Clear ldconfig cache:
#    rm /etc/ld.so.cache
#  3. Rebuild ld cache
#    ldconfig
#  4. test:
#    ldd modules/msilo/msilo.so
#
LIBS=-L/usr/local -L/usr/local/lib64 -fPIC -lrabbitmq

include ../../Makefile.modules
