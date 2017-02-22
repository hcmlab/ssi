#(c)2013 Andreas Seiderer
#parses an ssi event stream XML and converts it to a Praat TextGrid file;

#notes: 
# - the script converts only events with the state "completed" (see main());
# - for the textgrid output the "long" version is used which is better readable;

import sys
import datetime
from xml.dom import minidom


class PraatTextGridItem:

    def __init__(self, in_xmin, in_xmax, in_text):
        self.xmin = in_xmin
        self.xmax = in_xmax
        self.text = in_text

    def toString(self):
        return """xmin = {xmin} \nxmax = {xmax} \ntext = "{text}" """.format(xmin = self.xmin, xmax = self.xmax, text = self.text)


class PraatTextGridItemList:
    __header = """class = "IntervalTier" \nname = "{name}" \nxmin = {xmin} \nxmax = {xmax} \nintervals: size = {size} \n"""

    def __init__(self, in_name):
        self.items = []
        self.name = in_name
        self.xmin = None
        self.xmax = None

    #update the xmin and xmax value of the list if an item is added
    def addItem(self, item):
        self.items.append(item)

        if self.xmin is None:
            self.xmin = item.xmin
        else:
            self.xmin = min(self.xmin, item.xmin)

        if self.xmax is None:
            self.xmax = item.xmax
        else:
            self.xmax = max(self.xmax, item.xmax)

    def toString(self):
        outputStr = self.__header.format(name=self.name, xmin=self.xmin, xmax=self.xmax, size=len(self.items))

        for i in range(0, len(self.items)):
            outputStr += "intervals [{}]:\n".format(i + 1) + self.items[i].toString() + "\n"

        return outputStr


class PraatTextGrid:
    __header = """File type = "ooTextFile"\nObject class = "TextGrid"\n\nxmin = {xmin} \nxmax = {xmax} \ntiers? <exists> \nsize = {items} \nitem[]: \n"""

    def __init__(self):
        self.itemlist = []
        self.listDict = {}
        self.xmin = None
        self.xmax = None

    #create a new list
    #returns list number
    def addList(self, name):
        self.itemlist.append(PraatTextGridItemList(name))
        self.listDict[name] = len(self.itemlist) - 1
        return len(self.itemlist) - 1

    #add an item object to an existing list with the number of the list
    #returns true if succeeded, false if the list number is out of range
    def addItemToList(self, listNr, item):
        if (len(self.itemlist) - 1 < listNr):
            return False

        self.itemlist[listNr].addItem(item)

        if self.xmin is None:
            self.xmin = item.xmin
        else:
            self.xmin = min(self.xmin, item.xmin)

        if self.xmax is None:
            self.xmax = item.xmax
        else:
            self.xmax = max(self.xmax, item.xmax)

        return True

    def getNumberOfList(self, name):
        if name in self.listDict:
            return self.listDict[name]
        return -1

    def toString(self):
        if len(self.itemlist) > 0:
            outputStr = self.__header.format(xmin=self.xmin, xmax=self.xmax, items=len(self.itemlist))

            for i in range(0, len(self.itemlist)):
                outputStr += "item [{}]:\n".format(i + 1) + self.itemlist[i].toString()

            return outputStr
        return ""


def main():

    #sample code for building a praat textgrid file:
    
    #textGrid = PraatTextGrid()
    #textGrid.addList("test")
    #textGrid.addItemToList(0,PraatTextGridItem(0,150,"testVal1"))
    #textGrid.addItemToList(0,PraatTextGridItem(150,200,"testVal2"))

    #textGrid.addList("test2")
    #textGrid.addItemToList(1,PraatTextGridItem(0,75,"a"))
    #textGrid.addItemToList(1,PraatTextGridItem(150,200,"b"))

    #print(textGrid.toString())


    args = sys.argv[1:]    # ignore first argument (current path)

    if (len(args) != 3):
        sys.stderr.write("Please enter an input, output filename and if the duration of an event has to be subtracted from the start position!")
        return -1

    print("*******************************************************************************")
    print("*                SSI event stream to Praat TextGrid converter                 *")
    print("*                         (c)2013 Andreas Seiderer                            *")
    print("*******************************************************************************\n")

    print(datetime.datetime.now().strftime("%H:%M:%S") + " parsing xml ...")
    textGrid = PraatTextGrid()

    xmldoc = minidom.parse(args[0])

    eventlist = xmldoc.getElementsByTagName("event")

    for e in eventlist:
        if e.attributes["state"].value == "completed":        # only convert events with the state "completed"

            senderValue = e.attributes["sender"].value
            listNr = textGrid.getNumberOfList(senderValue)

            if listNr == -1:
                listNr = textGrid.addList(senderValue)

            eventValue = e.attributes["event"].value
            fromValue = int(e.attributes["from"].value)
            durValue = int(e.attributes["dur"].value)
            
            if (args[1] == "1"):
                textGrid.addItemToList(listNr, PraatTextGridItem((fromValue - durValue) / 1000, (fromValue) / 1000, eventValue))
            else:
                textGrid.addItemToList(listNr, PraatTextGridItem(fromValue / 1000, (fromValue + durValue) / 1000, eventValue))

    print(datetime.datetime.now().strftime("%H:%M:%S") + " writing to file ...")
    f = open(args[1], 'wt', encoding='utf-8')
    f.write(textGrid.toString())
    f.close()

    print(datetime.datetime.now().strftime("%H:%M:%S") + " script finished.")


if __name__ == "__main__":
    main()