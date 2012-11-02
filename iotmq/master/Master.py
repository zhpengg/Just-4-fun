# encoding = utf-8
'''
'''

import sys
sys.path.append('..')
import logging
from common.MetaManager import MetaManager
from common.Configure import Configure
from lib.MasterMetaManager import MasterMetaManager

class Master():
    def __init__(self, conf_path = ''):
        logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s', level = logging.DEBUG)
        self.conf = Configure(conf_path).conf
        self.meta = MetaManager(self.conf['meta-server'], self.conf['meta-root'])
        self.mmeta = MasterMetaManager(self.conf, self.meta)

    def register_node(self):
        self.mmeta.prepare_root_path()
        self.mmeta.register_master()
        return True

    def build_index(self):
        pass

    def start_to_serve(self):
        pass


if __name__ == '__main__':
    master = Master('./conf/master.conf')
    # register node
    master.register_node()
    logging.info('master register node OK')

    # start to serve
    master.start_to_serve()
    logging.info('master stat to serve OK')
