#(c)2013 Andreas Seiderer

import sys, os, subprocess

def main():
    #get all files in current directory
    files = [f for f in os.listdir() if os.path.isfile(f)]

    outputFile = list()
    separator = ";"
    outputFile.append(""+separator)
    firstRun = True

    i = 0
    for fileInDir in files:
        #get the file extension
        fileName, fileExt = os.path.splitext(fileInDir)

        #only .wav files are usable
        if fileExt == '.wav':
            i+=1
            print("({0})".format(i)+" processing file \""+fileInDir+"\" ...")

            p = subprocess.Popen(["praatcon", "-a", "voicereport_speechrate_formatted.praat", "0", "2", "yes", fileInDir], stdout=subprocess.PIPE)
            out, err = p.communicate()
            #print(out)

            lines = out.decode("utf-8").splitlines()

            outputFile[0] += fileInDir+separator
            lineNr = 1
            for line in lines:
                name, value = line.split(": ")
                if (firstRun):
                    outputFile.append(name+separator+value+separator)
                else:
                    outputFile[lineNr] += value+separator
                lineNr+=1

            firstRun = False

    #print(outputFile)

    csv_file = open("Output.csv", "w")
    csv_file.write('\n'.join(outputFile))
    csv_file.close()

    print("\n=========================== script finished. ===========================")


if __name__ == "__main__":
    main()