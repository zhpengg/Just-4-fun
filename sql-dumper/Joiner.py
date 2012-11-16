# encoding = utf-8

class Joiner():
    table = {}

    def join(self, rcd):
        if not self.table:
            self.first_join(rcd)
        else:
            self.normal_join(rcd)

    def first_join(self, rcd):
        for row in rcd:
            time_str = str(row[0])
            self.table[time_str] = [row[1]]

    def normal_join(self, rcd):
        for row in rcd:
            time_str = str(row[0])
            if time_str in self.table:
                self.table[time_str].append(row[1])

    def result(self):
        return self.table



if __name__ == '__main__':
    rcd = [('1', 10), ('2',15), ('3', 9), ('4', 22), ('6', 7)]
    rcd2 = [('1', 8), ('2', 12), ('3', 7), ('5', 22), ('6', 7)]

    joiner = Joiner()
    joiner.join(rcd)
    joiner.join(rcd2)

    print joiner.result()
