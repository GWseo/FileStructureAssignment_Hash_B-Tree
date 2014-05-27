name = "asdf asda"
dept = "fdsafds"
id = 201420011
score = 1.000
f = file("input.in","w+")
for i in range(1024*30):
    st = "i %s %d %f %s\n"%(name,id+i,score,dept)
    f.write(st)
    score=score+0.0001
s = "c 0.0 2.0\n"
f.write(s)

f.close()
