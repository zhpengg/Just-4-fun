# encoding=utf-8
'''
'''

import logging
import socket
import platform
import time
import json

class MasterMetaManager():
    def __init__(self, conf, meta):
        logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s', level = logging.DEBUG)
        self.meta = meta
        self.conf = conf

    def prepare_root_path(self):
        self.master_path = '/Master'
        self.meta.create_node(self.master_path)

        self.backup_master_path = '/BackupMaster'
        self.meta.create_node(self.backup_master_path)

        self.topic_path = '/Topic'
        self.meta.create_node(self.topic_path)

        self.broker_group_path = '/BrokerGroup'
        self.meta.create_node(self.broker_group_path)

        self.cluster_path = '/ClusterPath'
        self.meta.create_node(self.cluster_path)

        logging.info('prepare meta OK')

    def register_master(self):
        host_ip = socket.gethostbyname(platform.node())
        host_port = self.conf['listen-port']

        master_value = "%s:%s" % (host_ip, host_port)
        ret = self.meta.register_node('/Master', master_value)
        if ret:
            logging.info('register master OK')
        else:
            logging.warning('register master failed')

    def create_topic(self, topic_name):
        topic_path = self.topic_path + '/' + topic_name
        ret = self.meta.create_node(topic_path)
        if ret:
            logging.info('create topic(%s) OK', topic_path)
            return True
        else:
            logging.warning('create topic(%s) failed', topic_path)
            return False

    def add_segment(self, topic_name):
        topic_path = self.topic_path + '/' + topic_name
        segment_name = "%d" % int(time.time())
        seg_start = 0
        seg_end = 0xffffffff
        seg_host = ''

        seg_attr = {'start': seg_start, 'end': seg_end, 'host': seg_host}
        seg_path = topic_path + '/' + segment_name
        ret = self.meta.create_node_with_property(seg_path, json.dumps(seg_attr))
        if ret:
            logging.info('add segment(%s) OK' % seg_path)
            return True
        else:
            logging.warning('add segment(%s) failed' % seg_path)
            return False

if __name__ == '__main__':
    import sys
    sys.path.append('../..')
    from common.MetaManager import MetaManager 
    meta = MetaManager('127.0.0.1:2181', '/iotmq')
    mmeta = MasterMetaManager('', meta)

    mmeta.prepare_root_path()
    mmeta.create_topic('test_topic')
    mmeta.add_segment('test_topic')

