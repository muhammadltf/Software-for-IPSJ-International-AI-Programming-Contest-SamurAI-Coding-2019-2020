import os, fnmatch
def find(pattern, path):
    result = []
    for root, dirs, files in os.walk(path):
        for name in files:
            if fnmatch.fnmatch(name, pattern):
                result.append(os.path.join(root, name))
    return result

files = find('*.dighere.summary', '/home/luthfi/Downloads/practice2019 (1)/logs')

for f in files:
    players = f.split('_')[1].split('.')[0].split('vs')

    summary = open(f, 'r')
    summary_content = summary.readlines()
    last_line = summary_content[-1]

    if "Scores" in last_line:
        scores = last_line.split(" ")[-1].split(":")
        scores = [int(s) for s in scores]

        if scores[0] > scores[1]:
            print(players[0] + ";" + players[1] + "| WIN" )
        elif scores[0] < scores[1]:
            print(players[0] + ";" + players[1] + "| LOSE")
        else:
            print(players[0] + ";" + players[1] + "| TIE")
    else:
        print(players[0] + ";" + players[1] + "| ERROR!!!")