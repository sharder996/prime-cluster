class Result:
    def __init__(self, rank, payload, upper_limit, lower_limit):
        self.upper_limit = upper_limit
        self.lower_limit = lower_limit
        self.rank = rank
        self.payload = payload
