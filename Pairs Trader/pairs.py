import argparse
import pandas as pd
import numpy as np
import alpaca_trade_api as tradeapi
import config
import asyncio
import datetime
import time
from sklearn.metrics import r2_score
from scipy.stats import spearmanr, pearsonr

today = datetime.datetime.today()
DATE_STRING = "{}-{}-{}".format(today.month,today.day,today.year)
PAPER_ENDPOINT_URL = "https://paper-api.alpaca.markets"
LIVE_ENDPOINT_URL = "https://api.alpaca.markets"
overall_pl = 0

"""
Next:
-make trade summary csv


"""


class Spread():
    def __init__(self,api):
        self.long_ticker = "RCL"
        self.short_ticker = "CCL"
        self.api = api
        self.ready = False
        self.wait_periods = 25
        self.short_ewm_periods = 150
        self.long_ewm_periods = 250
        self.sma_periods = 50
        self.profit_per_trade = 1.00
        self.margin_below_trade = .02
        self.loss_per_share =.03
        self.ready = False
        self.wait_periods = 25
        self.short_ewm_periods = 150
        self.long_ewm_periods = 250
        self.sma_periods = 50
        self.profit_per_trade = 1.00
        self.margin_below_trade = .02
        self.loss_per_share =.03
        self.max_shares = 6
        self.long_asset = Asset(self,self.long_ticker,1)#comeback and put in ticker
        self.short_asset = Asset(self,self.short_ticker,4)#comeback and put in ticker
        self.filename = "./data/pairs/{}-{}_log_{}.csv".format(self.long_asset.ticker,self.short_asset.ticker,DATE_STRING)
        self.log = pd.DataFrame([],columns=["time","timestamp","price","price_one","price_two","RSI_15","RSI_30","long_sma","correlation_coef","position_long","position_short","signal"])#comeback
        #Runtime Variables
        self.correlation_coef=0
        self.price = 0
        self.rsi_15 = 0
        self.rsi_30 = 0
        self.long_ewma = 0
        self.short_ewma = 0
        self.long_sma = 0
        self.above_sma = None
        self.last_above_sma = None
        self.signal = None
        self.recorded_trades = 0
        self.pause = False
        self.last_signal = None
        self.current_signal = None

    def record_log(self,data): #comeback
        if data.symbol==self.long_asset.ticker:
            timer = self.long_asset.time
        elif data.symbol==self.short_asset.ticker:
            timer = self.short_asset.time
        #compute spread price
        if self.long_asset.price!=0 and self.short_asset.price!=0:
            self.price = ((self.long_asset.price * self.long_asset.lot_quantity)-
            (self.short_asset.price *self.short_asset.lot_quantity))
        else:
            self.price = None

        new_row = pd.DataFrame.from_dict({
            "time":[timer],
            "timestamp":[data.timestamp],
            "price":[self.price],
            "price_one":[self.long_asset.price],
            "price_two":[self.short_asset.price],
            "RSI_15":[0],#comeback and implement calculating these values
            "RSI_30":[0],
            "long_sma":[0],
            "correlation_coef":[0],
            "position_long":[self.long_asset.shares],
            "position_short":[self.short_asset.shares],
            "signal":[0]
        })

        self.log = self.log.append(new_row,ignore_index=True)

        self.log["RSI_15"].iat[-1] = self.compute_rsi(rsi_length = 15)
        self.log["RSI_30"].iat[-1] = self.compute_rsi(rsi_length = 30)
        self.log["long_sma"].iat[-1] = self.compute_sma()
        self.log["signal"].iat[-1] = self.compute_signal()
        self.log["correlation_coef"].iat[-1]=self.compute_correlation()

        self.recorded_trades += 1
        self.log.to_csv(self.filename)#comeback

    def compute_signal(self): #comeback
        return None

    def compute_rsi(self,rsi_length=15, first_row=True):
        if len(self.log["price"].dropna())>=rsi_length+1:
            delta = self.log["price"].dropna().diff().dropna()
            if(first_row == False):
                delta = delta[1:]
            up, down = delta.copy(), delta.copy()
            up[up < 0] = 0
            down[down > 0] = 0
            roll_up1 = up.ewm(span= rsi_length).mean()
            roll_down1 = down.abs().ewm(span=rsi_length).mean()
            RS1 = roll_up1 / roll_down1
            RSI1 = 100.0 - (100.0 / (1.0 + RS1))
            var1 = RSI1.get(key = ((self.log['price'].index.stop)-1))
            if type(var1)==np.float64:
                return float(np.round(var1,2))
            return var1
        else:
            return None

    def compute_sma(self):
        #(self.trades["price"].rolling(window=self.sma_periods).mean()).iat[-1]
        var1 = self.log["price"].iloc[-self.sma_periods::].mean()
        if type(var1)==np.float64:
            return float(np.round(var1,2))
        return var1

    def compute_correlation(self):
        if self.recorded_trades > 20:
            return float(pearsonr(self.log["price_one"]*self.long_asset.lot_quantity, self.log['price_two']*self.long_asset.lot_quantity)[0])
        else:
            return None

    def handle_trade_update(self,data):
        pass

    def open_position(self):
        pass

class Asset():
    def __init__(self,spread_ptr,ticker,quantity):
        self.ticker = ticker
        self.lot_quantity = quantity
        self.log= pd.DataFrame([],columns=["time","timestamp","price", "size" ,"exchange","conditions","symbol"])
        self.filename = "data/{}_trades_{}.csv".format(self.ticker,DATE_STRING)
        self.stream = 'T.{}'.format(self.ticker)
        self.spread_ptr = spread_ptr
        self.api = spread_ptr.api
        self.price = 0
        self.time = 0
        self.average_share_price = 0
        self.total_pl = 0
        self.trade_pl = 0
        self.unrealized_trade_pl = 0
        self.pending_id = []
        self.pending_limit_id = []
        self.stop_price = 0
        #self.num_trades = 0 #comeback
        self.shares = 0
        self.recorded_trades = 0



    def record_trade(self,data):

        self.time = time.time()
        new_row = pd.DataFrame.from_dict({
            "time":[self.time],
            "timestamp":[data.timestamp],
            "price":[data.price],
            "size":[data.size],
            'exchange':[data.exchange],
            'conditions':[data.conditions],
            'symbol':[data.symbol]
        })
        self.price = data.price
        self.recorded_trades += 1
        self.log = self.log.append(new_row,ignore_index=True)#comeback
        self.log.to_csv(self.filename)#comeback

    def unrealized_profit_loss(self,data):
        price = float(data.price)
        self.unrealized_trade_pl  = self.shares * (price - self.average_share_price)#shares can be negative



def run(args):
    if args.live_trading:
        conn = tradeapi.StreamConn(key_id=config.Live_trading.api_ID,
            secret_key=config.Live_trading.api_secret_key,base_url=LIVE_ENDPOINT_URL)
        api = tradeapi.REST(config.Live_trading.api_ID, config.Live_trading.api_secret_key,LIVE_ENDPOINT_URL, api_version='v2')
    else:
        conn = tradeapi.StreamConn(key_id=config.Paper_trading.api_ID,
            secret_key=config.Paper_trading.api_secret_key,base_url='https://paper-api.alpaca.markets')
        api = tradeapi.REST(config.Paper_trading.api_ID, config.Paper_trading.api_secret_key,PAPER_ENDPOINT_URL, api_version='v2')
    #clear and close all positions
    api.cancel_all_orders()
    api.close_all_positions()
    time.sleep(7)
    #instantiate spread
    spread = Spread(api)

    @conn.on(spread.long_asset.stream)
    async def on_trade(conn, channel, data):
        spread.long_asset.record_trade(data) #record trade
        spread.record_log(data)
        print(data)

    @conn.on(spread.short_asset.stream)
    async def on_trade(conn, channel, data):
        spread.short_asset.record_trade(data)
        spread.record_log(data)
        print(data)

    @conn.on("trade_updates")
    async def on_trade(conn, channel, data):
        pass #comeback

    print("program will begin in 5 seconds:\n")
    print("Live Trading: {}".format(args.live_trading))

    conn.run([spread.long_asset.stream,spread.short_asset.stream,"trade_updates"])


if __name__=="__main__":
    parser =argparse.ArgumentParser()

    parser.add_argument(
        '--live_trading',action='store_true',
        default = False, help='If flag, use live trading instead of paper trading.'
    )

    args = parser.parse_args()
    run(args)
