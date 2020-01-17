import os

maps = [str("preliminary-fields/" + f) for f in os.listdir("preliminary-fields")]
maps.append("samples/sample.dighere")

print("AS BLUE:")
for m in maps:
    exec_statement = "manager/manager {} players/simplePlayer BEKASI-BRAWLER/bekasiBrawler".format(m)
    exec_output = os.popen(exec_statement).read()
    exec_split = exec_output.split("scores\":[")
    scores = exec_split[-1].split("]")
    scores_separated = scores[0].split(",")
    our_score = scores_separated[0]
    enemy_score = scores_separated[1]
    print(m + ";" + our_score + ";" + enemy_score)
    
    # print(m + " RESULT:")
    # if int(our_score) > int(enemy_score):
    #     print("WIN :D " + our_score + " vs " + enemy_score)
    # elif int(our_score < int(enemy_score)):
    #     print("LOSE :( " + our_score + " vs " + enemy_score)
    # else:
    #     print("TIE :| " + our_score + " vs " + enemy_score)