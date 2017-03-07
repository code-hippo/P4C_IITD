zero=$0
one=$1
two=$2

echo $zero, $one, $two

echo $#
echo $*
echo $@
echo $?

shift
echo $#
echo $*
echo $@
echo $?
