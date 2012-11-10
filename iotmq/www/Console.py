# encoding=utf-8

import tornado.ioloop
import tornado.web

class HomeHandler(tornado.web.RequestHandler):
    def get(self):
        self.write('iotmq home')

application = tornado.web.Application([
            (r'/', HomeHandler),
        ]);

application.listen(8888)
tornado.ioloop.IOLoop.instance().start()
