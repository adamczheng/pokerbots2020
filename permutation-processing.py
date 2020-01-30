#implement permutation distance functions
def reinsert_dist(text1, text2):
    n,m=len(text1),len(text2)
    dp=[[0]*(m+1) for _ in range(n+1)]
    for i in range(n):
        for j in range(m):
            if text1[i]==text2[j]:
                dp[i+1][j+1]=dp[i][j]+1
            else:
                dp[i+1][j+1]=max(dp[i][j+1],dp[i+1][j])
    return len(text1) - dp[-1][-1]
def swap_dist(p, q):
     inverse_p = [p.index(i) for i in range(len(p))]
     invert_q = [q[inverse_p[i]] for i in range(len(q))]
     q = invert_q
     not_visited = [True] * len(q)
     cycles = 0
     for i in range(len(q)):
         curr_pos = i
         if not_visited[curr_pos]:
             cycles += 1
         while not_visited[curr_pos]:
             not_visited[curr_pos] = False
             curr_pos = q[curr_pos]
     return len(q) - cycles

#implement file reading
f = open("perms.txt", "r")
contents = f.read()
separated = contents.split("\n")
perms = [s.split(" ")[:-1] for s in separated][:-1]

#translate cards to indices
ranks = list("23456789TJQKA")
translate = {r: i for i, r in enumerate(ranks)}
perms = [[translate[r] for r in p] for p in perms]

#print reinsertion and swap distances
for i in range(1,len(perms)):
    print(str(i) + ":",reinsert_dist(perms[i-1], perms[i]), swap_dist(perms[i-1], perms[i]), "|", separated[i])
