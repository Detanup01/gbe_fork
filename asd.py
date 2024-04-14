import os
import glob


if __name__ == "__main__":
    with open("cc.ini", "w", encoding="utf-8") as fw:
        for gg in glob.glob('post_build/steam_settings.EXAMPLE/*.txt'):
            kk = os.path.basename(gg[:-12])
            lns=[]
            with open(gg, "r", encoding="utf-8") as f:
                lns=['# ' + ll.strip('\n').strip('\r') for ll in f.readlines()]
            for ln in lns:
                fw.write(ln + '\n')
            fw.write(kk + '=false' + '\n')
            fw.write('' + '\n')

