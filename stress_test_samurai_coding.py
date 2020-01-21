import os
import sys

maps = [str("preliminary-fields/" + f) for f in os.listdir("preliminary-fields")]
maps.append("samples/sample.dighere")

color = sys.argv[1]
intro = "AS RED: " if color == "RED" else "AS BLUE"
winCount = 0
loseCount = 0
tieCount = 0

print(intro)
for m in maps:
    if color == "RED":
        exec_statement = "manager/manager {} BEKASI-BRAWLER/bekasiBrawler players/simplePlayer".format(m)
        exec_output = os.popen(exec_statement).read()
        exec_split = exec_output.split("scores\":[")
        scores = exec_split[-1].split("]")
        scores_separated = scores[0].split(",")
        our_score = scores_separated[0]
        enemy_score = scores_separated[1]
        print(m + ";" + our_score + ";" + enemy_score)
    else :    
        exec_statement = "manager/manager {} players/simplePlayer BEKASI-BRAWLER/bekasiBrawler".format(m)
        exec_output = os.popen(exec_statement).read()
        exec_split = exec_output.split("scores\":[")
        scores = exec_split[-1].split("]")
        scores_separated = scores[0].split(",")
        our_score = scores_separated[1]
        enemy_score = scores_separated[0]
        print(m + ";" + enemy_score + ";" + our_score)

    if int(our_score) > int(enemy_score):
        winCount = winCount  + 1
    elif int(our_score) < int (enemy_score):
        loseCount = loseCount  + 1
    else:
        tieCount = tieCount  + 1

print("WIN: ", winCount, "LOSE: ", loseCount, "TIE: ", tieCount)
print("WINpct: ", float(winCount)/len(maps)*100, "LOSEpct: ", float(loseCount)/len(maps)*100, "TIEpct: ", float(tieCount)/len(maps)*100)