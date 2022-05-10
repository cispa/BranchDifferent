from sys import argv
import sys

maximum = 2000

file_in = "out.txt"
if len(argv) > 1:
    file_in = argv[1]

text = open(file_in, "r").read()

results = []

for t in text.split("--- ")[1:]:
    name = t.split(" ---")[0]
    result = [name]
    for line in t.split("\n")[1:-1]:
        result.append(line)
    results.append(result)
        
data = [
    [0 for i in range(maximum)] for j in range(len(results))
] 
     
for i in range(len(results)):
    for j in range(1, len(results[i])):
        value = min(int(results[i][j]), maximum - 1)
        data[i][value] += 1

print(",".join(["time"] + list(map(lambda x: x[0], results))))
for i in range(len(data[0])):
    result_line = [str(i)]
    for k in range(len(data)):
        result_line.append(str(data[k][i]))
    print(",".join(result_line))
