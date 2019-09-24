# dominion-hand-solver
Brute force method to calculate which card to play in a [Dominion](https://en.wikipedia.org/wiki/Dominion_(card_game)) game.

Given a deck, a hand, and a discard pile, you might not know what card to play first to obtain the greatest amount of money to spend in your
buy phase. This can get especially confusing when [Throne Rooms](http://wiki.dominionstrategy.com/index.php/Throne_Room) come into play.
With a deck with more than 10 cards it's difficult to keep track what's left in your deck that would contribute to your buy phase.

This is a solvable problem using probabilities and expected values. There is a best card play order, and it is calculable.
