'''
##
# @file Server.py
# @brief 
# @author zhenpeng
# @version 1.0.0
# @date 2012-10-31
'''
import tornado.ioloop
import tornado.web
from MessageServer import MessageServer
import json

db = MessageServer()
class MessageHandler(tornado.web.RequestHandler):
    def get(self, topic):
        message_id = self.get_argument('id')
        result = db.read(topic, message_id)
        json_result = []
        for item in result:
            json_result.append(item)
        self.write(json.dumps(json_result))

    def post(self, topic):
        message = self.get_argument('data')
        write_id = db.write(topic, message)
        if not write_id:
            self.write(json.dumps({'ret': 'Error', 'code': 0x1234}))
        else:
            self.write(json.dumps({'ret': 'OK', 'id': write_id}))

class BrokerServer():
    def __init__(self, conf):
        self.conf = conf
        self.app = tornado.web.Application([
            (r'/message/(.*)', MessageHandler),
            (r'/testing', TestHandler),
        ])

    def run(self):
        self.app.listen(self.conf['listen-port'])
        tornado.ioloop.IOLoop.instance().start()

class TestHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("Hello, world")

if __name__ == "__main__":
    conf = {}
    conf['listen-port'] = 12321
    server = BrokerServer(conf)
    server.run()
