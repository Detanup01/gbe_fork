import os, re, shutil
import urllib.request

# https://stackoverflow.com/a/48336994 # NOTE alternatively we could use 're.findall(str(re.escape(string1))+"(.*)"+str(re.escape(string2)),stringSubject)[0]'    
def GetListOfSubstrings(stringSubject,string1,string2):
    MyList = []
    intstart=0
    strlength=len(stringSubject)
    continueloop = 1

    while(intstart < strlength and continueloop == 1):
        intindex1=stringSubject.find(string1,intstart)
        if(intindex1 != -1): #The substring was found, lets proceed
            intindex1 = intindex1+len(string1)
            intindex2 = stringSubject.find(string2,intindex1)
            if(intindex2 != -1):
                subsequence=stringSubject[intindex1:intindex2]
                MyList.append(subsequence)
                intstart=intindex2+len(string2)
            else:
                continueloop=0
        else:
            continueloop=0
    return MyList

def top_owners():

    if os.path.isfile("top_owners_ids.html"):
        with open("top_owners_ids.html", 'r', encoding='utf-8') as top_own:
            top_own_line = top_own.readlines()

        top_owner_id = ''
        top_owner_no = 0

        if os.path.exists("top_owners_ids.txt"): os.remove("top_owners_ids.txt")
        if os.path.exists("top_owners_ids_alt1.txt"): os.remove("top_owners_ids_alt1.txt")
        if os.path.exists("top_owners_ids_alt2.txt"): os.remove("top_owners_ids_alt2.txt")

        for line in top_own_line:
            if '<tr onclick="location.href=' in line:
                top_owner_no = top_owner_no + 1

                if (top_owner_no >= 1) and (top_owner_no <= 250):
                    top_owner_id = GetListOfSubstrings(line,'profile/','/')[0]

                    if not os.path.exists("top_owners_ids.txt"):
                        with open("top_owners_ids.txt", 'w') as f_txt:
                            f_txt.close()

                    with open("top_owners_ids.txt", 'a') as f_txt:
                        f_txt.write(f'{top_owner_id}\n')
                        f_txt.close()

                    if not os.path.exists("top_owners_ids_alt1.txt"):
                        with open("top_owners_ids_alt1.txt", 'w') as f_txt:
                            f_txt.close()

                    with open("top_owners_ids_alt1.txt", 'a') as f_txt:
                        f_txt.write(f'{top_owner_id},\n')
                        f_txt.close()

                    if not os.path.exists("top_owners_ids_alt2.txt"):
                        with open("top_owners_ids_alt2.txt", 'w') as f_txt:
                            f_txt.close()

                    with open("top_owners_ids_alt2.txt", 'a') as f_txt:
                        f_txt.write(f'#{top_owner_id},\n')
                        f_txt.close()
