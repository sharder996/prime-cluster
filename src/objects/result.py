class Result:
    def __init__(self, op, rank, payload, upper_limit, lower_limit):
        self.op = op
        self.upper_limit = int(upper_limit)
        self.lower_limit = int(lower_limit)
        self.rank = int(rank)
        self.payload = payload
