# 
import re
import glob

path = 'area*.log'
value = '([\w.]+)'

arch_name = 'area_([\w]+).log'


arch = [None]*12
area = [None]*12

files=glob.glob(path)


for i, file in enumerate(files):
    f=open(file, 'r')
    m = f.read()
    data = re.findall(value, m)
    s = f.name
    ss = re.findall(arch_name, s)
    print(ss)
    arch[i] = ss
    area[i] = data
    f.close()

table = zip(arch, area)

with open('area_data.csv', 'w') as f:
    for item in table:
        print >> f, item

