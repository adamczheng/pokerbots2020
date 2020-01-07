import numpy as np
def permute_values():
	'''
	Selects a value permutation for the whole game according the prior distribution.
	'''
	orig_perm = list(range(13))[::-1]
	prop_perm = []
	seed = np.random.geometric(p=0.25, size=13) - 1
	for s in seed:
		pop_i = len(orig_perm) - 1 - (s % len(orig_perm))
		prop_perm.append(orig_perm.pop(pop_i))
	return prop_perm
cnt = []
cntt= []
for i in range(13):
	cnt2 = []
	for j in range(13):
		cnt2.append(0)
	cnt.append(cnt2)
	cntt.append(cnt2.copy())
for i in range(10000):
	x = permute_values()
	for j in range(13):
		cnt[x[j]][j] += 1
	for a in range(13):
		for b in range(13):
			if x[a] > x[b]:
				cntt[a][b]+=1
for i in range(13):
	#print(cnt[i])
	sum = 0
	for j in range(13):
		sum += (2+j)*cnt[i][j]
	print(cnt[i], sum/10000)
for i in range(13):
	for j in range(13):
		print("%.2f" %(cntt[i][j]/10000), end=' ')
	print()
	
