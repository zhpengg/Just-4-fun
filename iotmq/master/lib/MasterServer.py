# encoding=utf-8
'''
'''
import tornado.ioloop
import tornado.web

class MessageHandler(tornado.web.RequestHandler):
    def get(self, topic):
        if not topic:
            raise tornado.web.HTTPError('404')
        # find topic
        self.write(topic)

    def post(self):
        pass

class BrokerServer():
    def __init__(self, conf):
        self.conf = conf
        self.app = tornado.web.Application([
            (r'/(.*)', MessageHandler),
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
