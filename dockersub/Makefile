LDLIBS += -lcurl -ljsmn -lnats

dockersub: dockersub.o

image:
	ragel -G2 -Vp test.rl -o out.dot && dot out.dot -Tpng > diagram.png && rm out.dot
