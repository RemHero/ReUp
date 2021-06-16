#!/usr/bin/python3
'''

Code for Coordinator


ConnectionError:
A base class for connection-related issues.
Subclasses are BrokenPipeError, ConnectionAbortedError, ConnectionRefusedError and ConnectionResetError.

'''

from xmlrpc.client import ServerProxy
from common import si_error
from socketserver import TCPServer, StreamRequestHandler
from typing import List
from concurrent.futures import ThreadPoolExecutor, as_completed

single = None
multi = None


class PreProcess(StreamRequestHandler):
    '''
    parse the request and forward to pa
    '''
    def handle(self):
        global single  
        global multi

        batch = self.get_batch_string()
        # si_error('[CO] got batch:' + str(batch))
        cmd = batch[0]

        # get parameter
        if cmd == 'SET':
            value = ' '.join(batch[2:])
        elif cmd == 'GET':
            assert len(batch) == 2
        elif cmd == 'DEL':
            key_list = batch[1:]
        else:
            raise ValueError(cmd)

      
        dead_test = False   #测试掉线的
        dead_si_conf = set()   

        # 线性
        if cmd == 'GET':
            for si_add, si_port in single:
                try:
                    get_result = self.op_got(
                        batch[1], PreProcess.got_rpc_agent(si_add, si_port))
                    break
                except ConnectionError:
                    # let out dead PA
                    dead_test = True
                    si_error('[CO] PA {}:{} is dead'.format(si_add, si_port))
                    dead_si_conf.add((si_add, si_port))
        # 并行
        elif cmd == 'SET':
            total_mission = [
                multi.submit(PreProcess.op_make, si_add, si_port, batch[1], value)
                for si_add, si_port in single
            ]
            for ft in as_completed(total_mission):
                data = ft.result()
                if data is not None:  # connection error
                    # record dead PA
                    dead_test = True
                    si_error('[CO] PA {}:{} is dead'.format(data[0], data[1]))
                    dead_si_conf.add((data[0], data[1]))
        else:
            total_mission = [
                multi.submit(PreProcess.op_del, si_add, si_port, key_list)
                for si_add, si_port in single
            ]
            for ft in as_completed(total_mission):
                data = ft.result()
                if type(data) == tuple:  # connection error
                    # record dead PA
                    dead_test = True
                    si_error('[CO] PA {}:{} is dead'.format(data[0], data[1]))
                    dead_si_conf.add((data[0], data[1]))

        # remove dead PA
        if dead_test:
            single -= dead_si_conf

        # reply
        if not single:  # PA set is empty. all PA are dead
            self.wfile.write('-ERROR\r\n'.encode())
        elif cmd == 'GET':
            self.wfile.write(get_result)
        elif cmd == 'SET':
            self.wfile.write('+OK\r\n'.encode())
        elif cmd == 'DEL':
            self.wfile.write(':{}\r\n'.format(data).encode())

      

    def get_batch_string(self) -> List[str]:
        n_lines = int(self.rfile.readline().decode().strip('*\r\n'))
        batch_string = []
        for i in range(n_lines):
            self.rfile.readline()  # ignore string length
            batch_string.append(self.rfile.readline().decode().strip())
        return batch_string

    @staticmethod
    def got_rpc_agent(si_add: str, si_port: int) -> ServerProxy:
        pa = ServerProxy('http://{}:{}/'.format(si_add, si_port))
        si_error('[CO] connected to PA {}, port {}'.format(si_add, si_port))
        return pa

    def op_got(self, key: str, pa: ServerProxy) -> bytes:
        # RPC: get value
        value = pa.get_key(key)
        # encode result
        sp_value = value.split()
        meta = '*{}\r\n'.format(len(sp_value))
        batch = map(lambda s: '${}\r\n{}\r\n'.format(len(s), s), sp_value)
        return meta.encode() + ''.join(batch).encode()

    @staticmethod
    def op_make(si_add: str, si_port: int, key: str, value: str):
        try:
            # get RPC proxy
            pa = PreProcess.got_rpc_agent(si_add, si_port)
            # SET: modify ALL PA
            pa.set_key(key, value)
            return None
        except ConnectionError:
            return (si_add, si_port)

    @staticmethod
    def op_del(si_add: str, si_port: int, key_list: List[str]):
        try:
            # get RPC proxy
            pa = PreProcess.got_rpc_agent(si_add, si_port)
            # DEL: modify ALL PA
            return pa.del_key(key_list)
        except ConnectionError:
            return (si_add, si_port)


def set_coordinator(addr: str, port: int, pa_list: set) -> None:
    global single
    global multi

    TCPServer.allow_reuse_address = True
    tcp_server = TCPServer((addr, port), PreProcess)
    single = pa_list
    multi = ThreadPoolExecutor(max_workers=len(pa_list))

    si_error("[CO] started coordinator at {}, port {}".format(addr, port))
    si_error('[CO] pa: ' + str(pa_list))

    tcp_server.serve_forever()

    exit(0)