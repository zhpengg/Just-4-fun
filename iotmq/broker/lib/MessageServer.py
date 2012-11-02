# encoding=utf-8
'''
'''

import logging
import pymongo
from IDGenerator import IDGenerator

class MessageServer():
    def __init__(self):
        logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s')
        self.id_gen = IDGenerator()
        try:
            self.conn = pymongo.Connection()['iotmq']
        except Exception, e:
            logging.error('init db connection fail, %s' % str(e))
        else:
            logging.info('init db connection OK')

    def write(self, topic, message):
        insert_id = self.id_gen.get_id()
        try:
            insert_id = self.conn[topic].insert({'_id': insert_id, 'data': message})
        except Exception, e:
            logging.warning('write %s failed, message: %s' % (topic, message))
        return insert_id

    def read(self, topic, message_id):
        result = None
        try:
            result = self.conn[topic].find({'_id': {'$gt': message_id}}).limit(50)
        except Exception, e:
            logging.info('read message(%s) failed' % message_id)

        return result


if __name__ == '__main__':
    ms = MessageServer()

    print ms.write('test-topic', 'hahhaahha')
    print ms.write('test-topic', 'hahhaahha')
    print ms.write('test-topic', 'hahhaahha')
    print ms.conn.collection_names()

    result = ms.read('test-topic', 0)
    for i in result:
        print i
