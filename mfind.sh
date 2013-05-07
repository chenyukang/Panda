

if [ $# -gt 0 ]
then
    find ./ -name *\.c -or -name \*.h | xargs grep $1;
fi
