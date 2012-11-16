# encoding=utf-8

import mysql.connector

class SQLDumper():
    conn = None

    def __init__(self, conf):
        self.conf = conf

    def connect(self):
        if self.conn and self.conn.is_connected():
            return

        try:
            self.conn = mysql.connector.connect(
                    user = self.conf['user'],
                    password = self.conf['password'],
                    host = self.conf['host'],
                    database = self.conf['database']
                    )
        except Exception, e:
            print "connect db failed %s " % str(e)

    def dump(self, sql):
        self.connect()
        result = []
        try:
            cursor = self.conn.cursor(buffered = True)
            cursor.execute(sql)
            result = cursor.fetchall()
            cursor.close()
        except Exception, e:
            print "dump sql failed %s" % str(e)

        return result
        

if __name__ == '__main__':
    conf = {}
    conf['user'] = 'root'
    conf['password'] = 'lovemm'
    conf['host'] = 'localhost'
    conf['database'] = 'ddup_blog'

    dumper = SQLDumper(conf)
    result = dumper.dump('select count(*) from wp_posts')
    print result
