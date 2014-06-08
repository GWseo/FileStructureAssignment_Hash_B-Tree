#import sys
#import os

f = open("test.in",'r')
count = 0
while(True):
    ss =f.readline().split()
    if not ss : break
    if float(ss[4])>=0.0 and float(ss[4])<=1.0:
        print ss[4]
        count+=1

print count

