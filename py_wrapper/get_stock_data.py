import os
import pickle
import sys
import threading
import time
from collections import namedtuple

import pandas as pd
import datetime
import tqdm
import requests
from ordered_set import OrderedSet
import multiprocessing as mp
DEFAULT_HOST = "api.polygon.io"


class Exchange:
    def __init__(self, id, type, name, mic, **kwargs):
        self.id = id
        self.name = name
        self.mic = mic
        self.tickers = list()
        self.type = type

    def __repr__(self):
        return f'Exchange(id: {self.id}, name: {self.name}, mic: {self.mic}, num tickers: {len(self.tickers)}\n'


class Ticker:
    def __init__(self, ticker, name, market, **kwargs):
        self.id = id
        self.name = name
        self.market = market
        self.currency_name = kwargs.get('currency_name', None)
        self.primary_exchange = kwargs.get('primary_exchange', None)
        self.ticker = ticker
        self.type = kwargs.get('type', None)
        self.data = None


def run(id, t):

    start = "1970-01-01"
    _session = requests.Session()
    save_file = f'/home/dewe/data_center/{t.market}/{t.ticker}.pkl'
    if os.path.exists(save_file):
        return
    prev = None

    try:
        end = "2021-07-05"
        while start != end:
            if prev == start:
                break
            endpoint = f"{url}/v2/aggs/ticker/{t.ticker}/range/5/minute/{start}/{end}?adjusted=true&sort=asc&limit=50000&&apiKey={key}"
            resp = _session.get(endpoint)
            if resp.status_code == 200:
                data = resp.json()
                data = data['results']
                results = pd.DataFrame(data)
                results['date'] = pd.to_datetime(results.t, unit='ms').dt.tz_localize('UTC').dt.tz_convert(tz='US/Eastern')
                prev = start
                start = str(results.date.iloc[-1].date())
                t.data = results if t.data is None else t.data.append(results)
            else:
                print('response for ', t.ticker, 'failed')
                if t.data is None:
                    print(f'{t.ticker} was also empty')
                    return
                break

        t.data.drop_duplicates(inplace=True)
        with open(save_file, 'wb') as pkl:
            pickle.dump(t, pkl)
        print('id:', id, ' [created] ', save_file)

    except Exception as e:
        print('type:', t.market, 'ticker:', t.ticker )


if __name__ == '__main__':
    key = "DoU6br9Hxy2VAx_ZgxUqO6mQyYvLBzgi"
    url = "https://" + DEFAULT_HOST

    all_exchanges = []
    exchange_dict = dict()
    all_tickers = []

    endpoint = f"{url}/v1/meta/exchanges?apikey={key}"

    try:
        df = pd.read_json(endpoint)
        for i in range(len(df)):
            kwargs = df.iloc[i].to_dict()
            all_exchanges.append(Exchange(**kwargs))
            exchange_dict[all_exchanges[-1].mic] = all_exchanges[-1]
    except Exception as s:
        print(s)

    endpoint = f"{url}/v3/reference/tickers?active=true&sort=ticker&order=asc&limit=1000&apiKey={key}"

    good = True
    try:
        while good:
            df = pd.read_json(endpoint)
            results = df.results.tolist()
            temp = [Ticker(**result) for result in results]
            all_tickers += temp
            endpoint = str(df['next_url'][len(df) - 1]) + f"&apiKey={key}"
            good = all(df['status'].tolist())
    except Exception as e:
        print(e)

    for t in all_tickers:
        if t.primary_exchange in exchange_dict.keys():
            exchange_dict[t.primary_exchange].tickers.append(t.ticker)

    with open("/home/dewe/data_center/exchanges.pkl", 'wb') as pkl:
        pickle.dump(exchange_dict, pkl)

    # all_tickers_ = [(i, t_) for i, t_ in enumerate(all_tickers)]
    #
    # with mp.Pool() as pool:
    #     pool.starmap(run, all_tickers_, 128)