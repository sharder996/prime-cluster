class Job:
    def __init__(self, op, upper_limit, lower_limit, rank):
        self.op = op
        self.upper_limit = upper_limit
        self.lower_limit = lower_limit
        self.rank = rank
