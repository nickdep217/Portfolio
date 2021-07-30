#Simple Scalper
Simple_scalper:
    Simple_scalper.py is a simple trading program that I wrote to trade stocks on the
  market based off of technical indicators computed by the program. My main purpose in
  doing this was to get acquainted with using event based programing to trade. To do this I used an api provided by Alpaca, more info about Alpaca can be found at https://alpaca.markets/docs/.
    The idea of the program is to open a long or short position based off of a signal
  that is computed by the compute_signal() function that is called every time that new live trade history data is recieved by websocket. Once a position is sucsessfully  opened by the
  program, the position is left open until it reaches the take profit or stop loss amount
  as defined in the context variables. A trailing_stop can also be used in place of the stop_loss. Only once all positions of a given asset are closed can a new position of that asset be opened. Up to 4 assets can be traded at one
  time.
  At the moment simple_scalper is trading based off of the last trade price crossing over a calculated simple moving average of the last 150 trades. However, the signal can easily be modified in the compute_signal function to trade based off some of the other indicators that the program calculates. Furthermore, context variables such as the take profit, stop_loss, etc, can be modified within the Context class to alter how the program trades. More info about can be found in the pdf attatched

  To Run:
    $ python3 simple_scalper.py --symbol [symbol] --trailing_stop

    -to add multiple ticker symbols add another "--symbol" flag and another ticker
    -Make sure to input alpaca keys into the config.py file
    -Make sure all packages specified in requirments.txt are installed
    -I recommend running it on a server rather than a local computer as you will get slightly faster quotes and will have less connection errors.
