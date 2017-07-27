if [ -z "$1" ]
then
  echo "USAGE: $0 <.c files to go through>"
  exit 1
fi

# get included headers of objects used (incl. nested)
clang -M $@ | 
# make slashes and spaces newlines
sed -e 's/[\\ ]/\n/g' | 
# delete empty lines and lines containing object files
sed -e '/^$/d' -e '/\.o:[ \t]*$/d' |
# feed dat into ctags
ctags -L - 
