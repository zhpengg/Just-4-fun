import tornado.ioloop
import tornado.web
import json

class BaseController(tornado.web.RequestHandler):
    def get(self):
        cluster_name = self.get_argument('cluster_name')
        app_id = self.get_argument('app_id')
        time_start = self.get_argument('time_start')
        time_end = self.get_argument('time_end')
        # TODO return json string
        #data = [{'2012-10-26 10:00:00': {'1': 100, '2': '200', '3': '500'}}, {'2012-10-26 10:00:10': {'1': 100, '2': '200', '3': '500'}}]
        #data = {'appid_1': [], 'appid2': [], 'appid3': []}
        data = [{'name': 'appid1', 'data': [{'2012-10-26 10:00:00': '100'}, {'2012-10-26 10:00:10': '110'}]},
                {'name': 'appid2', 'data': [{'2012-10-26 10:00:00': '200'}, {'2012-10-26 10:00:10': '310'}]}]
        self.write(json.dumps(data))
        
        
