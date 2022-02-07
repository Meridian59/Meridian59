# Convert from .tab file format to a sequence of admin commands.
# After recreating the world, e.g. "send o 0 recreate all" (or at initial startup),
# add NPC dialog to the speech library with these commands:
#
# python dialogconvert.py npc1.tab dialog.txt
#
# Blakserv admin command:
# read dialog.txt (file needs to be in server run directory)

import sys

def usage():
    print('Usage:')
    print(sys.argv[0] + ' input_file output_file')
    sys.exit()

def main():
    if len(sys.argv) < 3:
        usage()

    input_filename = sys.argv[1]
    output_filename = sys.argv[2]

    output_file = open(output_filename, "w")
    
    with open(input_filename, "r") as input_file:
        for line in input_file:
            fields = line.split('\t')

            if fields[2]:
                command1 = f"send o 0 speechlibtrigger who class {fields[0]}"
                command2 = f"send o 0 speechlibquote who class {fields[0]} quote q {fields[3]}"
                if fields[1]:
                    command1 += f" mood int LIBRES_{fields[1]}"
                if fields[2]:
                    command1 += f" trigger q {fields[2]}"
                output_file.write(command1)
                output_file.write("\n")
                output_file.write(command2)
                output_file.write("\n")
            else:
                command1 = f"send o 0 randomlibquote who class {fields[0]}"
                if fields[1]:
                    command1 += f" mood int LIBRES_{fields[1]}"
                command1 += f" quote q {fields[3]}"
                output_file.write(command1)
                output_file.write("\n")
            
    output_file.close()

main()
