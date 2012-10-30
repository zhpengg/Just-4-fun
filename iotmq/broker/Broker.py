# encoding=utf-8
'''
# @file broker.py
# @brief 
# @author zhenpeng
# @version 1.0.0
# @date 2012-10-30
'''
import sys
sys.path.append('..')
from common.MetaManager import MetaManager
from common.Configure import Configure
import logging

class Broker():
    def __init__(self, conf_path = ''):
        logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s')
        self.conf = Configure(conf_path).conf
        self.meta = MetaManager(self.conf['meta-server'], self.conf['meta-root']).zk

    def run(self):
        logging.info('Broker running...')


if __name__ == '__main__':
    # 载入配置文件 
    broker = Broker('./conf/broker.conf')
    # 启动Broker主进程
    broker.run()
