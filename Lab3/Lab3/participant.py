#!/usr/bin/python3

'''

Code for participant

'''

from xmlrpc.server import SimpleXMLRPCServer
from typing import List
from common import si_error


# dict
database = {}


# set
def set_key(key: str, value: str) -> bool:
    database[key] = value
    # si_error('[PA] {}'.format(database))
    return True


# get
def get_key(key: str) -> str:
    # si_error('[PA] querying {}'.format(key))
    if key in database.keys():
        return database[key]
    else:
        return 'nil'


# del
def del_key(key: List[str]) -> int:
    count = 0
    for s in key:  # enum each key
        if s in database.keys():
            del database[s]
            count += 1
    # si_error('[PA] {}'.format(database))
    return count


def set_participant(addr: str, port: int) -> None:
    '''
    The function blocks the program and runs forever
    '''

    # start server
    server = SimpleXMLRPCServer((addr, port))

    # register functions
    server.register_function(get_key, 'get_key')
    server.register_function(set_key, 'set_key')
    server.register_function(del_key, 'del_key')

    si_error("[PA] started participant at {}, port {}".format(addr, port))
    server.serve_forever()  # run RPC server

    exit(0)
