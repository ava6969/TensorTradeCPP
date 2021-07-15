import os
import pandas as pd
import talib
from talib import abstract


class DataWrapper:
    def __init__(self, ticker, market, args=None, **kwargs):

        self.ticker = ticker
        self.market = market

        if market == 'crypto':
            filepath = f'/home/dewe/data_center/{market}/X:{ticker}.csv'
        elif market == 'stocks':
            filepath = f'/home/dewe/data_center/{market}/{ticker}.csv'
        elif market == 'fx':
            filepath = f'/home/dewe/data_center/{market}/C:{ticker}.csv'
        else:
            raise FileNotFoundError(f'{market} is an invalid market type use either [crypto, stocks, fx] ')

        if not os.path.exists(filepath):
            raise FileNotFoundError(f'{filepath} not found {ticker}')

        self._data = pd.read_csv(filepath)

        if args:
            self.add_tech_indicators(list(args))

    def get_series(self, columns):
        datalist = self._data[columns].tolist()
        return datalist

    def get_columns(self):
        return list(self._data.columns)

    def fn(self, fn_name, **kwargs):
        fnc = talib.abstract.Function(fn_name)
        result = fnc(self._data, **kwargs)
        result = result.fillna(0)
        return result

    def add_tech_indicators(self, tech_indicators):
        names = {}
        for name, kwargs in tech_indicators:
            df = self.fn(name, **kwargs)
            names[name] = names.get(name, 0)
            n = name + '_' + str(names[name]) if names[name] > 0 else name
            self._data[n] = df
            names[name] += 1

    def __repr__(self):
        return self.ticker + "\n" + str(self._data)


def make_wrapper( ticker, market, args):
    return DataWrapper(ticker, market, args)


if __name__ == '__main__':

    wrapper = make_wrapper("BTCUSD", "crypto", ['SMA', {}])
    print(wrapper)