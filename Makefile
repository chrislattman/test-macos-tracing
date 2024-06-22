default:
	clang -Wall -Wextra -pedantic -std=c99 -o child child.c
	clang -Wall -Wextra -Werror -pedantic -std=c99 -sectcreate __TEXT __info_plist Info.plist -o parent parent.c
	codesign -s chrislattman ./parent

clean:
	rm -f parent child
