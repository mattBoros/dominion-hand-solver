#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <sys/time.h>

using namespace std;

const bool DEBUG = false;

double get_time() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int t = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return t / 1000.0;
}

string spaces(const uint8_t n) {
    string s = "";
    for (int i = 0; i < n; ++i) {
        s += "    ";
    }
    return s;
}

class Card {
public:
    const int8_t card_id;
    const string name;
    const string type;
    const uint8_t actions;
    const uint8_t buys;
    const uint8_t coins;
    const uint8_t cards;

    Card() :
    card_id(-1),
    name("NULL CARD"),
    type("NULL"),
    actions(UINT8_MAX),
    buys(UINT8_MAX),
    coins(UINT8_MAX),
    cards(UINT8_MAX)
    {}

    Card(const int8_t card_id,
         const string name,
         const string type,
         const uint8_t actions,
         const uint8_t buys,
         const uint8_t coins,
         const uint8_t cards)
            :
            card_id(card_id),
            name(name),
            type(type),
            actions(actions),
            buys(buys),
            coins(coins),
            cards(cards) {
    }

//    Card(const Card &other)
//            :
//            name(other.name),
//            type(other.type),
//            actions(other.actions),
//            buys(other.buys),
//            coins(other.coins),
//            cards(other.cards),
//            card_id(other.card_id) {
//        cout << "BEING OVERWRITTEN" << endl;
//    }

//    inline Card &operator=(const Card &other) = default;

};

static const Card NULL_CARD;
static const Card COPPER(0, "Copper", "Treasure", 0, 0, 1, 0);
static const Card SILVER(1, "Silver", "Treasure", 0, 0, 2, 0);
static const Card GOLD(2, "Gold", "Treasure", 0, 0, 3, 0);
static const Card LABORATORY(3, "Laboratory", "Action", 1, 0, 0, 2);
static const Card MARKET(4, "Market", "Action", 1, 1, 1, 1);
static const Card MILITIA(5, "Militia", "Action", 0, 0, 2, 0);
static const Card FESTIVAL(6, "Festival", "Action", 2, 1, 2, 0);
static const Card SMITHY(7, "Smithy", "Action", 0, 0, 0, 3);
static const Card VILLAGE(8, "Village", "Action", 2, 0, 0, 1);
static const Card COUNCIL_ROOM(9, "Council Room", "Action", 0, 1, 0, 4);
static const Card KINGDOM[10] = {COPPER, SILVER, GOLD, LABORATORY, MARKET, MILITIA,
                                 FESTIVAL, SMITHY, VILLAGE, COUNCIL_ROOM};


class Card_Vector {
private:
    mutable uint8_t arr[10] = {0};
    mutable uint16_t _size = 0;

public:
    Card_Vector() {}

    inline uint16_t size() const {
        return _size;
    }

    inline void add_card(const Card c) {
        ++arr[c.card_id];
        ++_size;
    }

    inline void remove_card(const Card c) {
        assert(arr[c.card_id] > 0);
        --arr[c.card_id];
        --_size;
    }

    inline uint16_t num_of(const Card c) const {
        return arr[c.card_id];
    }

    inline Card_Vector copy() const {
        Card_Vector cv;
        for (uint8_t i = 0; i < 10; ++i) {
            cv.arr[i] = arr[i];
        }
        cv._size = _size;
        return cv;
    }

    inline Card_Vector add(const Card_Vector v) const {
        Card_Vector new_vector = copy();
        for (uint8_t i = 0; i < 10; ++i) {
            new_vector.arr[i] += v.arr[i];
        }
        return new_vector;
    }

    void print() const {
        cout << "-- game state --" << endl;
        for (const Card c : KINGDOM) {
            cout << c.name << " : " << num_of(c) << endl;
        }
    }

    static Card_Vector from_vector(vector<Card> v) {
        if (DEBUG) cout << "from_vector" << endl;
        if (DEBUG) cout << v.size() << endl;
        Card_Vector cv;
        for (int i = 0; i < v.size(); ++i) {
            cv.add_card(v.at(i));
        }
        return cv;
    }
};


class Card_And_Value {
public:
    const string card;
    const float value;

    inline Card_And_Value(const string card, const float value)
            : card(card), value(value) {};
};


class GameState {
public:
    const uint8_t actions;
    const uint8_t buys;
    const uint8_t accum_action_coins;

    const Card_Vector deck;
    const Card_Vector hand;
    const Card_Vector in_play;
    const Card_Vector discard;

    inline GameState()
    :
        actions(0),
        buys(0),
        accum_action_coins(0)
    {}

    inline GameState(
            const Card_Vector deck,
            const Card_Vector hand,
            const Card_Vector in_play,
            const Card_Vector discard,
            const uint8_t actions,
            const uint8_t buys,
            const uint8_t coins
    )
            :
            deck(deck),
            hand(hand),
            in_play(in_play),
            discard(discard),
            actions(actions),
            buys(buys),
            accum_action_coins(coins) {}

    inline GameState move_card_from_hand_to_in_play(const Card card, const uint8_t depth) const {
        if(DEBUG) cout << spaces(depth) << "playing " << card.name << endl;
        // TODO: make discard go into deck if deck size is 0

        Card_Vector new_hand = hand.copy();
        Card_Vector new_in_play = in_play.copy();
        new_hand.remove_card(card);
        new_in_play.add_card(card);

        return GameState(
                    deck,
                    new_hand,
                    new_in_play,
                    discard,
                    actions + card.actions - 1,
                    buys + card.buys,
                    accum_action_coins + card.coins
                );
    }

    inline GameState move_card_from_deck_to_hand(const Card card) const {
        GameState gs_copy = copy();

        Card_Vector new_deck = deck.copy();
        Card_Vector new_hand = hand.copy();
        new_deck.remove_card(card);
        new_hand.add_card(card);

        return GameState(
                new_deck,
                new_hand,
                in_play,
                discard,
                actions,
                buys,
                accum_action_coins
                );
    }

    inline uint8_t coin_value(const uint8_t depth) const {
        // TODO: max at 8 * buys
        uint8_t coin_value = accum_action_coins;

        for (const Card c : KINGDOM) {
            if (c.type == "Treasure") {
                coin_value += c.coins * hand.num_of(c);
            }
        }

        if(DEBUG) cout << spaces(depth) << "---" << endl;
        if(DEBUG) cout << spaces(depth) << "total coin val    : " << (int) coin_value << endl;
        if(DEBUG) cout << spaces(depth) << "action coin val   : " << (int) accum_action_coins << endl;
        if(DEBUG) cout << spaces(depth) << "treasure coin val : " << (int) (coin_value - accum_action_coins) << endl;

        return coin_value;
    }

    inline uint8_t num_total_cards() const {
        return deck.size() + hand.size() + in_play.size() + discard.size();
    }

    inline GameState copy() const {
        return GameState(
                deck,
                hand,
                in_play,
                discard,
                actions,
                buys,
                accum_action_coins
        );
    }

    inline GameState move_deck_into_hand() const {
        Card_Vector new_hand = hand.copy();
        new_hand.add(deck);
        return GameState(
                Card_Vector(),
                new_hand,
                in_play,
                discard,
                actions,
                buys,
                accum_action_coins
        );;
    }

//    static GameState get_starting_game_state(const uint8_t num_copper_in_hand) {
//        Card_Vector deck();
//        Card_Vector hand();
//        Card_Vector in_play();
//        Card_Vector discard();
//
//        return GameState(
//                deck,
//                hand,
//                in_play,
//                discard,
//                1,
//                1,
//                0
//        );
//    }

};

class GameState_and_Freq {
public:
    const GameState gs;
    const uint16_t freq;

    GameState_and_Freq(GameState gs, uint16_t freq)
            : gs(gs), freq(freq) {}
};

class DominionSolver {
public:
    inline vector<GameState_and_Freq> combinations(const GameState game_state, const Card card_being_played, const uint8_t depth) const {
        if (DEBUG) cout << spaces(depth) << "---" << endl;
        if (DEBUG) cout << spaces(depth) << "combinations" << endl;
//        if (DEBUG) cout << "card : " << card.name << endl;
//        if (DEBUG) game_state.deck.print();

        if(card_being_played.cards >= game_state.deck.size()){
//            cout << "+cards >= deck size" << endl;
//            cout << (int) card.cards << endl;
//            cout << game_state.deck.size() << endl;
            vector<GameState_and_Freq> new_game_states;
            const GameState new_gs = game_state.move_deck_into_hand();
            new_game_states.emplace_back(new_gs, 1);
            return new_game_states;
        }

        if (card_being_played.cards == 0) {
            vector<GameState_and_Freq> new_game_states;
            new_game_states.emplace_back(game_state, 1);
            return new_game_states;

        } else if (card_being_played.cards == 1) {
            vector<GameState_and_Freq> new_game_states;
            for (const Card c : KINGDOM) {
                if(game_state.deck.num_of(c) > 0){
                    const GameState new_gs = game_state.move_card_from_deck_to_hand(c);
                    new_game_states.emplace_back(new_gs, game_state.deck.num_of(c));
                }
            }
            return new_game_states;

        } else if (card_being_played.cards == 2) {
            vector<GameState_and_Freq> new_game_states;

//            vector<Card> all_cards;
//            for (Card c : KINGDOM) {
//                for (int i = 0; i < game_state.deck.num_of(c); ++i) {
//                    all_cards.push_back(c);
//                }
//            }
//            for (int j = 0; j < all_cards.size(); ++j) {
//                Card c1 = all_cards.at(j);
//                for (int k = 0; k < all_cards.size(); ++k) {
//                    if (j != k) {
//                        Card c2 = all_cards.at(k);
//                        GameState new_gs = game_state.move_card_from_deck_to_hand(c1).move_card_from_deck_to_hand(c2);
//                        new_game_states.emplace_back(new_gs, 1);
//                    }
//                }
//            }

            for (uint8_t i = 0; i < 10; ++i) {
                const Card c1 = KINGDOM[i];
                if(game_state.deck.num_of(c1) == 0){
                    continue;
                }
                const GameState one_card_moved = game_state.move_card_from_deck_to_hand(c1);
                for (uint8_t j = 0; j < 10; ++j) {
                    const Card c2 = KINGDOM[j];
                    if(one_card_moved.deck.num_of(c2) == 0){
                        continue;
                    }
                    const GameState both_cards_moved = one_card_moved.move_card_from_deck_to_hand(c2);
                    const uint16_t freq = game_state.deck.num_of(c1) * one_card_moved.deck.num_of(c2);
                    new_game_states.emplace_back(both_cards_moved, freq);
                    if (DEBUG) cout << spaces(depth) << "combo : " << c1.name << (int) i << " : " << c2.name << (int) j << " : freq : " << freq << endl;
                }
            }
//            exit(0);
            return new_game_states;

        } else {
            cout << "Need to implement +card >= 3" << endl;
            exit(0);
        }
    }


    inline float move_value(const GameState game_state, const Card card, const uint8_t depth) const {
        if (DEBUG) cout << spaces(depth) << "---" << endl;
        if (DEBUG) cout << spaces(depth) << "move_value" << endl;
        float sum = 0;
        uint16_t num_combos = 0;

        const vector<GameState_and_Freq> combos = combinations(game_state, card, depth+1);
        if(DEBUG) cout << spaces(depth) << "num combinations : " << combos.size() << endl;
        for (uint16_t i = 0; i < combos.size(); ++i) {
            const GameState_and_Freq gs_and_freq = combos.at(i);
            const GameState new_game_state = gs_and_freq.gs;
            const uint16_t freq = gs_and_freq.freq;
            num_combos += freq;
            sum += freq * card_to_play(new_game_state, depth+1).value;
        }

        return sum / num_combos;
    }

    inline Card_And_Value card_to_play(const GameState game_state, const uint8_t depth) const {
        if (DEBUG) cout << spaces(depth) << "---" << endl;
        if (DEBUG) cout << spaces(depth) << "card_to_play" << endl;

        if (game_state.actions == 0) {
            if (DEBUG) cout << spaces(depth) << "actions are 0" << endl;
            return Card_And_Value("", game_state.coin_value(depth+1));
        }

        float best_value = -1;
        string best_card_to_play = "";
        bool has_action_in_hand = false;

        for (const Card card : KINGDOM) {
            if (card.type == "Action" && game_state.hand.num_of(card) > 0) {
                has_action_in_hand = true;
                const GameState new_game_state = game_state.move_card_from_hand_to_in_play(card, depth+1);
                const float play_value = move_value(new_game_state, card, depth+1);
                if (play_value > best_value) {
                    best_value = play_value;
                    best_card_to_play = card.name;
                }
            }
        }

        if (!has_action_in_hand) {
            if(DEBUG) cout << spaces(depth) << "no actions left in hand" << endl;
            return Card_And_Value("", game_state.coin_value(depth+1));
        }

        return Card_And_Value(best_card_to_play, best_value);
    }
};


int main() {
    const DominionSolver ds;

    Card_Vector deck = Card_Vector::from_vector({
                                                        SILVER, COPPER, COPPER, COPPER,
                                                        SILVER, LABORATORY, MARKET, MARKET
                                                });
    Card_Vector hand = Card_Vector::from_vector({
                                                        MILITIA, MARKET, LABORATORY, COPPER, COPPER
                                                });

//    Card_Vector deck = Card_Vector::from_vector(
//            {SILVER, COPPER, MARKET});
//    Card_Vector hand = Card_Vector::from_vector({MILITIA, MARKET, LABORATORY, COPPER, COPPER});
    Card_Vector in_play = Card_Vector::from_vector({});
    Card_Vector discard = Card_Vector::from_vector({COPPER, COPPER, COPPER});


    GameState gs(
            deck,
            hand,
            in_play,
            discard,
            1,
            1,
            0
    );

    double t1 = get_time();
    Card_And_Value card_and_value = ds.card_to_play(gs, 0);
    double t2 = get_time();
    cout << "Best card to play : " << card_and_value.card << endl;
    cout << "Average value : " << card_and_value.value << endl;
    cout << "Time taken : " << (t2 - t1) << endl;


    return 0;
}