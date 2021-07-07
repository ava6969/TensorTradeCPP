
import cdd


class DataWrapper:
    def __init__(self, prefix=None):
        cdd_ = cdd.CryptoDataDownload()
        self.data = cdd_.fetch("Bitfinex", "USD", "BTC", "1h").add_prefix("BTC:") if prefix else \
            cdd_.fetch("Bitfinex", "USD", "BTC", "1h")

    def get_series(self, id):
        datalist = self.data[id].tolist()
        return datalist

    def get_columns(self):
        return list(self.data.columns)


if __name__ == '__main__':

    wrapper = DataWrapper()
    d = wrapper.get_series("BTC:close")
    print(wrapper.get_columns())