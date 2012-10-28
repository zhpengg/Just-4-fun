import tornado.ioloop
import tornado.web
import json

class ClusterInfoController(tornado.web.RequestHandler):
    def get(self):
        info = [{'name': 'udw-rt', 'id': '0', 'app': ['app1', 'app2', 'app3']},
                {'name': 'rtpe', 'id': '0', 'app': ['1', '3', '11']}
               ]
        self.write(json.dumps(info))
