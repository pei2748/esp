# 
import re
import glob

path = 'latency*.log'
value = '([\w.]+)'

arch_name = 'latency_([\w]+).log'


arch = [None]*12
latency = [None]*12

files=glob.glob(path)


for i, file in enumerate(files):
    f=open(file, 'r')
    m = f.read()
    data = re.findall(value, m)
    s = f.name
    ss = re.findall(arch_name, s)
 
    arch[i] = ss
    latency[i] = data
    f.close()


table = zip(arch, latency)

with open('latency_data.txt', 'w') as f:
    for item in table:
        print >> f, item
