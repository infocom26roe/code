import random
import string
import sys
import os

def generate_file(size, unit, num):
    folder_path = './../data/'
    if not os.path.exists(folder_path):
        os.makedirs(folder_path)
    n = size
    if unit == 'K':
        n = n * 1024
    elif unit == 'M':
        n = n * 1048576
    chars = ''.join([random.choice(string.ascii_letters) for i in range(n)])
    filename = folder_path + 'Object' + str(num)
    if num < 10:
        filename = folder_path + 'Object0' + str(num)
    with open(filename, 'w') as f:
        f.write(chars)

if len(sys.argv) == 4:
    file_num = int(sys.argv[1])
    for i in range(file_num):
        generate_file(int(sys.argv[2]), sys.argv[3], i) 
else:
    print('Invalid arguments! Usage. $python generate_file.py file_num size unit(b/K/M)')

# 67108864 64MiB
# 4194304 4MiB
# 1048576 1MiB