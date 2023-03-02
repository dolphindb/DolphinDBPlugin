import readline
import sys
from datetime import datetime

result = dict()

filename = sys.argv[1]
file = open(filename, "r")
for line in file:
    mdtime = line[11:25]
    info = line[35:]
    info = info.split()
    if line[35:46] == "InsightPerf":
        if info[1] not in result:
            result[info[1]] = dict()
        if info[2] not in result[info[1]]:
            result[info[1]][info[2]] = []
        result[info[1]][info[2]].append(mdtime)
file.close()

for marketType in result:
    for mdtime in result[marketType]:
        maxRecieveTime = max(result[marketType][mdtime]);
        prefix = '2000-01-01T'
        start = datetime.strptime(prefix+mdtime, "%Y-%m-%dT%H:%M:%S.%f")
        end = datetime.strptime(prefix+maxRecieveTime, "%Y-%m-%dT%H:%M:%S.%f")
        print("{} | {} | {} | {}".format(marketType, mdtime, maxRecieveTime, end-start))
