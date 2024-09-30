# a random tetris bot. it's not very good.
# general idea:
# stacker.cpp/stacker.h : basic code for a tetris game
# stackerbot.cpp/stackerbot.h : code for a tetris bot. this bot only looks 1 move in advance, and does not pay attention to the next queue. it has many *hand chosen* weights and rules, and is not really a machine learning thing
# game-main.cpp - an sdl2 based ui for the game.
# evolution-main.cpp - uses a subpar genetic algorithm in order to create a better bot than the one i hand designed, by tweaking *only the weights* and not the rules.
