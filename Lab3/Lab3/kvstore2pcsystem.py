#!/usr/bin/python3

'''

Code for Launcher

argv[0] is the name of the program
argv[2] is config file

'''

import sys
from common import si_error
from participant import set_participant
from coordinator import set_coordinator

# read conf file
with open(sys.argv[2], mode='r') as conf:
    # running mode
    for conf_line in conf:
        if conf_line[0] == 'm':
            is_coord = False
            if conf_line.split()[1][0] == 'c':
                is_coord = True
            break

    if is_coord:  # pa_list for coordinator
        part_list = set()

    for conf_line in conf:
        if conf_line[0] == 'c':  # read co
            c_addr, c_port = conf_line.split()[1].split(':')
            c_port = int(c_port)
        elif conf_line[0] == 'p':  # read pa
            si_add, si_port = conf_line.split()[1].split(':')
            si_port = int(si_port)
            if is_coord:  # add pa settings
                part_list.add((si_add,si_port))

# launch co or pa
if is_coord:
    set_coordinator(c_addr, c_port, part_list)
else:
    set_participant(si_add, si_port)

# the procedure never reaches here
exit(8)