# encoding=utf-8

from optparse import OptionParser
from SQLDumper import SQLDumper
import json

class QPSDumper():
    def __init__(self, conf_path):
        self.conf_path = conf_path

        conf = None
        try:
            print open(conf_path).read()
            conf = json.loads(open(conf_path).read())
            print conf
        except Exception, e:
            print 'load conf failed, %s' % str(e)
            exit()

        self.dumper = SQLDumper(conf)

    def get_processors(self, appid):
        cmd = 'select distinct c_attr2 where c_attr3=%s' % appid
        plist = self.dumper.dump(cmd)
        return plist


if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-i', '--appid', dest = 'appid', help='app id to dump')
    (options, args) = parser.parse_args()

    dumper = QPSDumper('./conf/db.conf')
