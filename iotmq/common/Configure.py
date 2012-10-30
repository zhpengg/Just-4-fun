# encoding=utf-8
'''
##
# @file Configure.py
# @brief 
# @author zhenpeng
# @version 1.0.0
# @date 2012-10-30

'''
import yaml
import logging

class Configure():
    def __init__(self, path = ''):
        logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s')
        self.path = path
        try:
            self.conf = yaml.load(open(self.path))
            logging.info('load conf file(%s) OK' % self.path)
        except:
            logging.warning('load conf file(%s) failed' % self.path)

    def get_conf(self):
        return self.conf

if __name__ == '__main__':
    conf = Configure('../broker/conf/broker.conf')
    print conf.conf
