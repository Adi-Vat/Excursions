.var foo 2
LD R2, @foo
STR @foo, 15
.var bar $10
LD R0, @bar
ADD R0, 2
STR @bar, R0
