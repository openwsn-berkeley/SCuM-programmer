FILE_NAME   = 'dummy.bin'
FILE_SIZE_B = (64*1024)

# create contents
buf           = []
last_b        = 0
while True:
    b         = (last_b+1)%256
    buf      += [b]
    last_b    = b
    if len(buf)==FILE_SIZE_B:
        break

# write to file
with open(FILE_NAME,'wb') as f:
    f.write(bytes(buf))

input('{} is {} bytes long'.format(FILE_NAME,len(buf)))