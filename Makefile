.PHONY: shell clean
shell: myShell.c ; gcc -o shell myShell.c && ./shell
clean: ; rm -f shell
