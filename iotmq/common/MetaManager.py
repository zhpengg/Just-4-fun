# encoding=utf-8
'''
##
# @file MetaManager.py
# @brief ZooKeeper 常用操作封装
# @author zhenpeng
# @version 1.0.0
# @date 2012-09-30
'''
import zc.zk
import logging

class MetaManager():
    def __init__(self, meta_addr = '', meta_root = ''):
        logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s')
        self.addr = meta_addr
        self.root = meta_root
        try:
            self.zk = zc.zk.ZooKeeper(self.addr)
        except Exception, e:
            logging.warning('zk connect failed, %s' % str(e))
        else:
            logging.info('zk connect OK')

    def create_node(self, path = ''):
        try:
            self.zk.create(self.root + path, '', zc.zk.OPEN_ACL_UNSAFE)
            return True
        except Exception, e:
            logging.warning('zk create node(%s) failed, %s' % (path, str(e)))
            return False

    def create_node_with_property(self, path = '', properties = ''):
        try:
            self.zk.create_recursive(self.root + path, properties, zc.zk.OPEN_ACL_UNSAFE)
            return True
        except Exception, e:
            logging.warning('create recursive node(%s) failed, %s' % (path, str(e)))
            return False

    def register_node(self, path = '', value = ''):
        try:
            path = self.root + path
            logging.debug('register node, path: %s, value: %s' % (path, value))
            self.zk.register_server(path, value, zc.zk.OPEN_ACL_UNSAFE)
            return True
        except Exception, e:
            logging.warning('register node(%s) failed, %s' % (path + value, str(e)))
            return False

    def get_children(self, path):
        try:
            path = self.root + path
            children = self.zk.children(path)
            return sorted(children)
        except Exception, e:
            logging.warning('get children(%s) error, %s' % (path, str(e)))
            return []
    def get_property(self, path):
        try:
            path = self.root + path
            prop = self.zk.get_properties(path)
            return prop
        except Exception, e:
            logging.warning('get properties(%s) failed' % path)
            return {}


# for testing
if __name__ == '__main__':
    meta = MetaManager('127.0.0.1:2181', '/iotmq')

    print 'test create node--------------------'
    print meta.create_node('/iotmq')

    print 'test create node with propt-----------'
    print meta.create_node_with_property('/iotmq/broker', '{"name": "test-create-recursive"}')

    print 'test register node--------------------'
    print meta.regist_node('/iotmq/broker', '127.0.0.1:2233')
