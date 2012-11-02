# encoding=utf-8
'''
'''
import time

class IDGenerator():
    def __init__(self):
        self.counter = 0
        self.max_counter = 1000000
        self.last_sec = int(time.time())

    def get_id(self):
        now_sec = int(time.time())
        self.counter += 1
        if now_sec != self.last_sec or self.counter >= self.max_counter:
            self.last_sec = now_sec
            self.counter = 0

        time_part= "%d" % now_sec 
        counter_part = "%06d" % self.counter
        return time_part + counter_part
