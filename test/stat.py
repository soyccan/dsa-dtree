for nam in ('rand2', 'min', 'max'):
    print(nam)
    thr = 0
    s = 0
    n = 0
    for line in open(nam+'.csv'):
        eps, depth = line.split(',')
        if depth == '\n':
            continue
        if float(eps) - thr < 1e-10:
            s += int(depth)
            n += 1
        else:
            thr += 0.01
            print(eps, s/n)
            s = 0
            n = 0
