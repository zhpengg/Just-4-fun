import tornado.ioloop
import tornado.web
from api.BaseController import BaseController
from api.ClusterInfoController import ClusterInfoController

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.render('templates/index.html')

application = tornado.web.Application([
    (r"/", MainHandler),
    (r"/api/base", BaseController),
    (r"/api/clusterInfo", ClusterInfoController),
    (r"/static/(.*)", tornado.web.StaticFileHandler, {"path": "static"}),
])

if __name__ == "__main__":
    application.listen(8888)
    tornado.ioloop.IOLoop.instance().start()
