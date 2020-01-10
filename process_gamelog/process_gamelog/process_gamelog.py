# UPLOAD GAME_LOG.TXT TO THIS FOLDER, THIS FILE WRITES TO OUTPUT.CSV IN FORMAT (WINNING HANG, LOSING HANG, [WINNING HAND], [LOSING HAND])

gamelog = open('game_log.txt', 'r')
output = open('output.csv', 'w') 

lines=gamelog.readlines()

count=0
flag = False

for index, line in enumerate(lines):
    if 'River' in line:
        currentRiver = line
    if 'shows' in line and flag == False:
        flag = True

        # print(currentRiver)
        # print(lines[index:index+4])

        hands = {}

        if line[0]=='A':
            hands['A'] = line
            hands['B'] = lines[index+1]

        if line[0]=='B':
            hands['B'] = line
            hands['A'] = lines[index+1]

        if '-' in lines[index+2]:
            winner = lines[index+3][0]
            loser = lines[index+2][0]

        if '-' in lines[index+3]:
            winner = lines[index+2][0]
            loser = lines[index+3][0]

        winner_hand = hands[winner]
        loser_hand = hands[loser]

        # for char in winner_hand[8:13]:
        #     if char in {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'}:
        #         dumb[char] = dumb.get(char, 0) + 1

        # for char in loser_hand[8:13]:
        #     if char in {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'}:
        #         dumb[char] = dumb.get(char, 0) - 1

############## ONLY IMPORTANT LINE BELOW ######## TOUCH NOTHING ELSE

        output.write(( currentRiver[6:21]+winner_hand[8:13] + ', ' + currentRiver[6:21]+loser_hand[8:13]+ ', ' + currentRiver[21:36] + ' ' + winner_hand[15:21] + ', ' + currentRiver[21:36] + ' ' + loser_hand[15:21] + '\n'))

############## ONLY IMPORTANT LINE ABOVE ########

    if 'awarded' in line:
        flag = False

output.write(lines[2]+lines[3])

''' IRRELEVANT TRASH BELOW

fuckthis = {}
fuckthis2 ={}


for index in range(len(lines[2])):
    if lines[2][index] in {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'}:
        fuckthis[lines[2][index]]=lines[3][index]


print(fuckthis)
for element in fuckthis:
    fuckthis2[fuckthis[element]]=element

actual =''

for card in ['2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A']:
    actual += fuckthis2[card] + ' '

print(actual)

dumblist=[]
for char in dumb:
    dumblist.append((char, dumb[char]))
    dumblist.sort(key = lambda x: x[1])

guess=''
for item in dumblist:
    guess+=(item[0]+' ')

print(guess)

'''