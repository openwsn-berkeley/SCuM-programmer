
def handleCrash(name,err):
    output  = []
    output += ['============= CRASH in thread "{}" =============================='.format(name)]
    output += ['']
    output += ['type of exception:']
    output += ['']
    output += ['{}'.format(type(err))]
    output += ['']
    output += ['backtrace:']
    output += ['{}'.format(err)]
    output  = '\n'.join(output)
    print(output)
    raise(err)