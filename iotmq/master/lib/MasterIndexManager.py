# encoding=utf-8
'''
'''

import logging

class MasterIndexManager():
    def __init__(self, conf, meta):
        self.conf = conf
        self.meta = meta

    def build_index(self):
        index = {}
        index['topic'] = self.build_topic_index()
        index['bgroup'] = self.build_bgroup_index()

        return index

    def build_topic_index(self):
        topic_path = '/Topic'
        topic_index = {}
        topic_list = self.meta.get_children(topic_path)
        for topic in topic_list:
            segment_index = []
            sub_topic_path = topic_path + '/' + topic
            segment_list = self.meta.get_children(sub_topic_path)
            for segment in segment_list:
                segment_path = sub_topic_path + '/' + segment
                segment_property = self.meta.get_property(segment_path)
                segment_index.append(segment_property)
                print segment_property
            topic_index[str(topic)] = segment_index
        return topic_index

    def build_bgroup_index(self):
        bgroup_path = '/BrokerGroup'

if __name__ == '__main__':
    import sys
    sys.path.append('../..')
    from common.MetaManager import MetaManager
    meta = MetaManager('127.0.0.1:2181', '/iotmq')
    index_manager = MasterIndexManager('', meta)
    topic_index = index_manager.build_topic_index()
    print topic_index
