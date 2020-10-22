import argparse
import pandas as pd
import numpy as np
import alpaca_trade_api as tradeapi
import config
import asyncio
import datetime
import time

today = datetime.datetime.today()
DATE_STRING = "{}-{}-{}".format(today.month, today.day, today.year)
PAPER_ENDPOINT_URL = "https://paper-api.alpaca.markets"
LIVE_ENDPOINT_URL = "https://api.alpaca.markets"
overall_pl = 0


class Context():
    def __init__(self, symbol, api):
        self.ticker = symbol
        self.api = api
        self.trade_stream = 'T.{}'.format(self.ticker)
        self.trades = pd.DataFrame(
            [],
            columns=[
                "time",
                "timestamp",
                "price",
                "size",
                "exchange",
                "conditions",
                "symbol",
                "RSI_15",
                "RSI_30",
                "long_ewm",
                "short_ewm",
                "long_sma",
                "above_sma",
                "position",
                "signal"])
        self.recorded_trades = 0
        self.filename = "data/{}_trades_{}.csv".format(
            self.ticker, DATE_STRING)
        self.last_trade_price = 0
        self.rsi_15 = 0
        self.rsi_30 = 0
        self.long_ewma = 0
        self.short_ewma = 0
        self.long_sma = 0
        self.above_sma = None
        self.last_above_sma = None
        self.short_ewm_periods = 150
        self.long_ewm_periods = 250
        self.long_sma_periods = 50
        self.profit_per_trade = 1.00
        self.margin_below_trade = .02
        self.loss_per_share = .03
        self.trade_size = 10
        self.signal = None
        self.max_shares = 6
        self.ready = False
        self.wait_periods = 25
        self.pause = False
        self.last_signal = None
        self.current_signal = None
        self.position = Position(self)

    def compute_rsi(self, rsi_length=15, first_row=True):
        delta = self.trades["price"].diff().dropna()
        if(first_row == False):
            delta = delta[1:]
        up, down = delta.copy(), delta.copy()
        up[up < 0] = 0
        down[down > 0] = 0
        roll_up1 = up.ewm(span=rsi_length).mean()
        roll_down1 = down.abs().ewm(span=rsi_length).mean()
        RS1 = roll_up1 / roll_down1
        RSI1 = 100.0 - (100.0 / (1.0 + RS1))
        var1 = RSI1.get(key=((self.trades['price'].index.stop) - 1))
        if isinstance(var1, np.float64):
            return float(np.round(var1, 2))
        return var1

    def compute_short_ewma(self):
        var1 = pd.Series.ewm(
            self.trades["price"].iloc[-self.short_ewm_periods::], span=self.short_ewm_periods).mean()
        var1 = var1.iat[-1]
        if isinstance(var1, np.float64):
            return float(np.round(var1, 2))
        return var1

    def compute_long_ewma(self):
        var1 = pd.Series.ewm(
            self.trades["price"].iloc[-self.long_ewm_periods::], span=self.long_ewm_periods).mean()
        var1 = var1.iat[-1]
        if isinstance(var1, np.float64):
            return float(np.round(var1, 2))
        return var1

    def compute_long_sma(self):
        # (self.trades["price"].rolling(window=self.long_sma_periods).mean()).iat[-1]
        var1 = self.trades["price"].iloc[-self.long_sma_periods::].mean()
        if isinstance(var1, np.float64):
            return float(np.round(var1, 2))
        return var1

    def compute_signal(self):
        if self.last_above_sma == False and self.above_sma == True:
            self.signal = 'buy'
        elif self.last_above_sma and self.above_sma == False:
            self.signal = 'sell'
        else:
            self.signal = None

    def record_trade(self, data):
        new_row = pd.DataFrame.from_dict({
            "time": [time.time()],
            "timestamp": [data.timestamp],
            "price": [data.price],
            "size": [data.size],
            'exchange': [data.exchange],
            'conditions': [data.conditions],
            'symbol': [data.symbol],
            'RSI_15': [0],
            'RSI_30': [0],
            'long_ewm': [0],
            'short_ewm': [0],
            'long_sma': [0],
            'above_sma': [None],
            'position': self.position.shares,
            'signal': None
        })
        # Calculate indicator values and put them into the Dataframe
        self.last_trade_price = data.price
        self.trades = self.trades.append(new_row, ignore_index=True)
        self.rsi_15 = self.compute_rsi(rsi_length=15)
        self.trades["RSI_15"].iat[-1] = self.rsi_15
        self.rsi_30 = self.compute_rsi(rsi_length=30)
        self.trades["RSI_30"].iat[-1] = self.rsi_30
        self.long_ewm = self.compute_long_ewma()
        self.trades['long_ewm'].iat[-1] = self.long_ewm
        self.short_ewm = self.compute_short_ewma()
        self.trades['short_ewm'].iat[-1] = self.short_ewm
        self.long_sma = self.compute_long_sma()
        self.trades['long_sma'].iat[-1] = self.long_sma
        # compute above or below sma
        self.last_above_sma = self.above_sma
        if self.long_sma:
            if self.last_trade_price >= self.long_sma:
                self.above_sma = True
            else:
                self.above_sma = False
            self.trades['above_sma'].iat[-1] = self.above_sma

        self.recorded_trades += 1
        if self.recorded_trades >= self.wait_periods:
            self.ready = True
        # calculate buy and sell signals
        self.trades['signal'].iat[-1] = self.compute_signal()
        # Save the updated Dataframe to a csv file
        self.trades.to_csv(self.filename)

    def open_position(self):  # side 'long' or 'sh-ort'
        # check to make sure that the order is not going in the opposite
        # direction

        if self.position.shares == 0 and self.ready and self.pause == False and self.position.pending_id == []:
            self.pause = True
            if self.signal == 'buy':
                limit_price = self.last_trade_price + self.margin_below_trade
            elif self.signal == 'sell':
                limit_price = self.last_trade_price - self.margin_below_trade
            else:
                self.pause = False
                return
                # signal not traded
                # Reminder: possibibly print why the signal was not traded
            try:
                o = self.api.submit_order(
                    symbol=self.ticker,
                    side=self.signal,
                    type='limit',
                    limit_price=str(limit_price),
                    qty=self.trade_size,
                    order_class='simple',
                    time_in_force='ioc')
            except Exception as e:
                print("error opening position, line 218")
                print(e)
            else:
                self.position.pending_id.append(o.id)

            self.pause = False
        return

    def handle_trade_update(self, data):
        event = data.event
        order = data.order
        global overall_pl

        if event == 'new':
            pass

        if event == 'fill':
            order_qty = int(order['qty'])
            filled_price = float(order['filled_avg_price'])
            if order['symbol'] == self.ticker:

                if order['id'] in self.position.pending_id:
                    self.position.pending_id.remove(order['id'])
                    self.position.average_share_price = filled_price
                    self.position.shares = int(data.position_qty)
                    if order['side'] == 'buy':
                        order_side = 'sell'
                        order_limit = self.position.average_share_price + self.profit_per_trade
                        self.position.stop_price = self.position.average_share_price - self.loss_per_share
                        # Reminder: add a stop price
                    if order['side'] == 'sell':
                        order_side = 'buy'
                        order_limit = self.position.average_share_price - self.profit_per_trade
                        self.position.stop_price = self.position.average_share_price + self.loss_per_share
                    # place closing limit_order

                    try:
                        o = self.api.submit_order(
                            symbol=self.ticker,
                            side=order_side,
                            type='limit',
                            limit_price=order_limit,
                            qty=order_qty,
                            order_class='simple',
                            time_in_force='gtc'
                        )
                    except Exception as e:
                        print("error placing take profit order")
                        print(e)
                    else:
                        self.position.pending_limit_id.append(o.id)

                elif order['id'] in self.position.pending_limit_id:
                    self.position.pending_limit_id.remove(order['id'])
                    self.position.shares = int(data.position_qty)
                    print("IMPORTANT: {}".format(int(data.position_qty)))

                    if order['side'] == 'buy':
                        self.position.trade_pl = order_qty * \
                            (self.position.average_share_price - filled_price)

                    if order['side'] == 'sell':
                        self.position.trade_pl = order_qty * \
                            (filled_price - self.position.average_share_price)

                    self.position.total_pl += self.position.trade_pl
                    overall_pl += self.position.trade_pl

                else:
                    self.position.shares = int(data.position_qty)
                    print("order from outside of program filled")

                print(self.position)
                return

        elif event == "partial_fill":
            order_qty = int(order['qty'])
            filled_qty = int(order['filled_qty'])
            filled_price = float(order['filled_avg_price'])
            if order['symbol'] == self.ticker:
                if order['id'] in self.position.pending_id:
                    self.position.shares = int(data.position_qty)
                    self.position.average_share_price = filled_price
                # make sure that this is how partial fills are handled
                elif order['id'] in self.position.pending_limit_id:
                    self.position.shares = int(data.position_qty)
                    # figure out how to calculate the profit when things are partially filled
                    # figure out what to do about this especially when it comes
                    # to the stops
            print(self.position)

        elif event == 'canceled':
            order_qty = int(order['qty'])
            filled_qty = int(order['filled_qty'])
            if order['symbol'] == self.ticker:
                if order["id"] in self.position.pending_id:
                    self.position.pending_id.remove(order["id"])
                    if self.position.shares > 0:
                        order_side = 'sell'
                        order_limit = self.position.average_share_price + self.profit_per_trade

                    elif self.position.shares < 0:
                        order_side = 'buy'
                        order_limit = self.position.average_share_price - self.profit_per_trade
                    else:
                        return
                    try:
                        o = self.api.submit_order(
                            symbol=self.ticker,
                            side=order_side,
                            type='limit',
                            limit_price=order_limit,
                            qty=filled_qty,
                            order_class='simple',
                            time_in_force='gtc'
                        )
                    except Exception as e:
                        print("error placing take profit order, 345")
                        print(e)
                    else:
                        self.position.pending_limit_id.append(o.id)

                if order['id'] in self.position.pending_limit_id:
                    print(
                        "Something went wrong, closing orders are only supposed to be replaced, not cancelled")
            print(self.position)

        elif event == 'replaced':
            print("order was replaced")
            if order['id'] in self.position.pending_limit_id:
                if order['replaced_by'] is not None:
                    self.position.pending_limit_id.remove(order['id'])
                    self.position.pending_limit_id.append(order['replaced_by'])

        elif event == 'rejected':
            print("order was rejected")





"""
    def compute_signal(self):
        self.last_signal = self.current_signal
        if self.recorded_trades >= 50:
            if self.rsi_15 >= 70:
                self.current_signal = ' buy'
            elif self.rsi_15 <= 30:
                self.current_signal = 'sell'
            else:
                self.current_signal = None
        else:
            self.current_signal = None

        if self.last_signal !=None and self.current_signal == None:
            self.signal = self.last_signal
        else:
            self.signal = None
        return self.signal

"""

"""
    def compute_signal(self):
        self.last_signal = self.current_signal
        if self.recorded_trades >= 50:
            if self.rsi_30 >= 65 and self.long_ewma >= self.short_ewma:
                self.current_signal = 'sell'
            elif self.rsi_30 <= 25 and self.long_ewma <= self.short_ewma:
                self.current_signal = 'buy'
            else:
                self.current_signal = None
        else:
            self.current_signal = None

        if self.last_signal !=None and self.current_signal == None:
            self.signal = self.last_signal
        else:
            self.signal = None
        return self.signal
"""



class Position():
    def __init__(self, context):
        self.shares = 0
        self.api = context.api
        self.average_share_price = 0
        self.total_pl = 0
        self.trade_pl = 0
        self.unrealized_trade_pl = 0
        self.pending_id = []
        self.pending_limit_id = []
        self.stop_price = 0
        self.context = context
        self.num_trades = 0

    def __str__(self):
        global overall_pl
        return 'symbol: {} trade_pl: {} total_pl: {} overall_pl: {}'.format(
            self.context.ticker, self.trade_pl, self.total_pl, overall_pl)
        # return 'shares: {} \n average share price: {} \n trade_pl: {} \n
        # total pl: {} \n pending ids: {} \n pending limit ids: {}
        # \n'.format(self.shares,self.average_share_price,self.trade_pl,self.total_pl,self.pending_id,self.pending_limit_id)

    def unrealized_profit_loss(self, data):
        price = float(data.price)
        self.unrealized_trade_pl = self.shares * \
            (price - self.average_share_price)
        if self.shares > 0 and self.pending_limit_id:
            if (self.average_share_price -
                    data.price) > self.context.loss_per_share:
                # replace the order with a market order to sell
                try:
                    self.api.replace_order(
                        self.pending_limit_id[0],
                        limit_price=price - .10)  # limit price set under last trade price
                except Exception as e:
                    print("failed to place stop, sell order")
        elif self.shares < 0:
            if (price - self.average_share_price) > self.context.loss_per_share:
                # replae the order with a market order to buy
                try:
                    self.api.replace_order(
                        self.pending_limit_id[0],
                        limit_price=price + .10)  # limti price set under last trade price
                except Exception as e:
                    print("failed to place stop, buy order")

    def trailing_stop(self, data):
        price = float(data.price)
        self.unrealized_trade_pl = self.shares * \
            (price - self.average_share_price)
        if self.shares > 0 and self.pending_limit_id:
            if price <= self.stop_price:
                # replace the order with a market order to sell
                try:
                    self.api.replace_order(
                        self.pending_limit_id[0],
                        limit_price=price - .10)  # limit price set under last trade price
                except Exception as e:
                    print("failed to place stop, sell order")
            elif (price - self.context.loss_per_share) > self.stop_price:
                # move stop loss up
                self.stop_price = price - self.context.loss_per_share

        elif self.shares < 0:
            if self.stop_price <= price:
                # replae the order with a market order to buy
                try:
                    self.api.replace_order(
                        self.pending_limit_id[0],
                        limit_price=price + .10)  # limti price set under last trade price
                except Exception as e:
                    print("failed to place stop, buy order")
            elif (price + self.context.loss_per_share) < self.stop_price:
                # move stop loss up
                self.stop_price = price + self.context.loss_per_share


def run(args):
    ticker_objects = {}
    _tickers = [symbol.upper() for symbol in args.symbol]
    _streams = ["T.{}".format(symbol) for symbol in _tickers]
    _streams.append("trade_updates")

    if args.live_trading:
        conn = tradeapi.StreamConn(
            key_id=config.Live_trading.api_ID,
            secret_key=config.Live_trading.api_secret_key,
            base_url=LIVE_ENDPOINT_URL)
        api = tradeapi.REST(
            config.Live_trading.api_ID,
            config.Live_trading.api_secret_key,
            LIVE_ENDPOINT_URL,
            api_version='v2')
    else:
        conn = tradeapi.StreamConn(
            key_id=config.Paper_trading.api_ID,
            secret_key=config.Paper_trading.api_secret_key,
            base_url='https://paper-api.alpaca.markets')
        api = tradeapi.REST(
            config.Paper_trading.api_ID,
            config.Paper_trading.api_secret_key,
            PAPER_ENDPOINT_URL,
            api_version='v2')

    _trailing_stop = args.trailing_stop
    api.cancel_all_orders()
    api.close_all_positions()
    time.sleep(7)

    for ticker in _tickers:

        ticker_objects[ticker] = Context(ticker, api)

        @conn.on(ticker_objects[ticker].trade_stream)
        async def on_trade(conn, channel, data):
            # On trades records data and computes indicator values
            ticker_objects[data.symbol].record_trade(data)
            ticker_objects[data.symbol].open_position()
            if _trailing_stop:
                ticker_objects[data.symbol].position.trailing_stop(data)
            else:
                ticker_objects[data.symbol].position.unrealized_profit_loss(
                    data)
            # print(data)
            print(ticker_objects[data.symbol].position)

    @conn.on("trade_updates")
    async def on_trade_update(conn, channel, data):
        if data.order['symbol'] in ticker_objects:
            ticker_objects[data.order['symbol']].handle_trade_update(
                data)
        else:
            return  # not a traded symbol
        print(data)

    print("program will begin in 5 seconds:\n")
    print("Live Trading: {}".format(args.live_trading))
    print("Symbols: {}".format(_tickers))
    print("Trailing Stop: {}".format(_trailing_stop))

    conn.run(_streams)
    """
    @conn.on("account_updates")
    async def on_account_updates(conn,channel,updates):
        print(updates)
    """


# Reminder: Can't open order in 2 directions at the same time, two orders
# stop and a limit cannot be open at the same time
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--symbol', nargs='+', default='SPY',
        help='stock ticker'
    )
    parser.add_argument(
        '--trailing_stop', action='store_true',
        default=False, help='trailing stop on or off'
    )
    parser.add_argument(
        '--live_trading', action='store_true',
        default=False, help='If flag, use live trading instead of paper trading.'
    )  # Reminder: Finish this
    args = parser.parse_args()
    run(args)
