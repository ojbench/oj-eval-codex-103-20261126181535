#ifndef GAME_H
#define GAME_H

#include <string>

struct Node;
struct GameState;
class Card;
class Pile;

enum SkillCardType { MULTIPLIER, HEAD_BUTT };

class Card {
public:
    explicit Card() = default;
    virtual ~Card() = default;
    virtual void play(GameState& game_state) = 0;
    virtual bool consumed() const { return false; }
};

class ScoreCard : public Card {
private:
    int point_;
public:
    explicit ScoreCard(int point);
    void play(GameState& game_state) override;
    ~ScoreCard() override = default;
};

class SkillCard : public Card {
private:
    SkillCardType type_;
public:
    explicit SkillCard(SkillCardType skill_card_type);
    void play(GameState& game_state) override;
    ~SkillCard() override = default;
};

class PowerCard : public Card {
private:
    int plus_count_;
public:
    explicit PowerCard(int plus_count);
    void play(GameState& game_state) override;
    ~PowerCard() override = default;
    bool consumed() const override { return true; }
};

struct Node {
    Card* card;
    Node* prev = nullptr;
    Node* next = nullptr;
};

// Each pile owns its nodes and cards. Transfers detach before inserting.
class Pile {
private:
    int size_;
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
public:
    friend void outShuffle(GameState&);
    friend void inShuffle(GameState&);
    friend void oddEvenShuffle(GameState&);
    friend class GameController;

    Pile();
    ~Pile();
    Pile(const Pile&) = delete;
    Pile& operator=(const Pile&) = delete;
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    void appendCard(Card* card);
    void appendNode(Node* node);
    void prependNode(Node* node);
    Node* remove(Node* node);
    Node* popFront() { return remove(head_); }
    Node* popBack() { return remove(tail_); }
    void appendPile(Pile& source);
};

struct GameState {
    Pile hand_{};
    Pile draw_pile_{};
    Pile discard_pile_{};
    int score_ = 0;
    int multiplier_ = 1;
    int bonus_ = 0;
};

void outShuffle(GameState& game_state);
void inShuffle(GameState& game_state);
void oddEvenShuffle(GameState& game_state);

class GameController {
private:
    GameState game_state_;
    void (*shuffle_)(GameState&);
public:
    explicit GameController(int mode);
    void draw();
    void play(int card_to_play);
    void finish();
    void shuffle();
    int queryScore();
    int queryDrawPileSize() const { return game_state_.draw_pile_.size(); }
    int queryHandSize() const { return game_state_.hand_.size(); }
    int queryDiscardPileSize() const { return game_state_.discard_pile_.size(); }
    Pile& drawPile() { return game_state_.draw_pile_; }
};

inline Pile::Pile() : size_(0) {}

inline Pile::~Pile() {
    while (Node* node = popFront()) {
        delete node->card;
        delete node;
    }
}

inline Node* Pile::remove(Node* node) {
    if (!node) return nullptr;
    if (node->prev) node->prev->next = node->next;
    else head_ = node->next;
    if (node->next) node->next->prev = node->prev;
    else tail_ = node->prev;
    node->prev = node->next = nullptr;
    --size_;
    return node;
}

inline void Pile::appendNode(Node* node) {
    if (!node) return;
    node->prev = tail_;
    node->next = nullptr;
    if (tail_) tail_->next = node;
    else head_ = node;
    tail_ = node;
    ++size_;
}

inline void Pile::prependNode(Node* node) {
    if (!node) return;
    node->prev = nullptr;
    node->next = head_;
    if (head_) head_->prev = node;
    else tail_ = node;
    head_ = node;
    ++size_;
}

inline void Pile::appendCard(Card* card) {
    appendNode(new Node{card});
}

inline void Pile::appendPile(Pile& source) {
    if (source.empty() || this == &source) return;
    if (tail_) tail_->next = source.head_;
    else head_ = source.head_;
    source.head_->prev = tail_;
    tail_ = source.tail_;
    size_ += source.size_;
    source.head_ = source.tail_ = nullptr;
    source.size_ = 0;
}

inline void outShuffle(GameState& game_state) {
    game_state.draw_pile_.appendPile(game_state.discard_pile_);
}

inline void inShuffle(GameState& game_state) {
    while (Node* node = game_state.discard_pile_.popBack()) {
        game_state.draw_pile_.appendNode(node);
    }
}

inline void oddEvenShuffle(GameState& game_state) {
    Pile even;
    while (Node* odd = game_state.discard_pile_.popFront()) {
        game_state.draw_pile_.appendNode(odd);
        even.appendNode(game_state.discard_pile_.popFront());
    }
    game_state.draw_pile_.appendPile(even);
}

inline ScoreCard::ScoreCard(int point) : point_(point) {}
inline void ScoreCard::play(GameState& game_state) {
    game_state.score_ += (point_ + game_state.bonus_) * game_state.multiplier_;
    game_state.multiplier_ = 1;
}

inline SkillCard::SkillCard(SkillCardType skill_card_type) : type_(skill_card_type) {}
inline void SkillCard::play(GameState& game_state) {
    if (type_ == MULTIPLIER) ++game_state.multiplier_;
    else game_state.draw_pile_.prependNode(game_state.discard_pile_.popBack());
}

inline PowerCard::PowerCard(int plus_count) : plus_count_(plus_count) {}
inline void PowerCard::play(GameState& game_state) {
    game_state.bonus_ += plus_count_;
}

inline GameController::GameController(int mode)
    : shuffle_(mode == 2 ? inShuffle : mode == 3 ? oddEvenShuffle : outShuffle) {}

inline void GameController::draw() {
    for (int i = 0; i < 5; ++i) {
        if (game_state_.draw_pile_.empty()) {
            if (game_state_.discard_pile_.empty()) break;
            shuffle();
        }
        game_state_.hand_.appendNode(game_state_.draw_pile_.popFront());
    }
}

inline void GameController::play(int card_to_play) {
    Pile& hand = game_state_.hand_;
    if (card_to_play < 1 || card_to_play > hand.size()) return;
    Node* node;
    if (card_to_play <= hand.size() / 2) {
        node = hand.head_;
        for (int i = 1; i < card_to_play; ++i) node = node->next;
    } else {
        node = hand.tail_;
        for (int i = hand.size(); i > card_to_play; --i) node = node->prev;
    }
    hand.remove(node);
    // HeadButt must resolve before the played card enters the discard pile.
    node->card->play(game_state_);
    if (node->card->consumed()) {
        delete node->card;
        delete node;
    } else {
        game_state_.discard_pile_.appendNode(node);
    }
}

inline void GameController::finish() {
    game_state_.discard_pile_.appendPile(game_state_.hand_);
}
inline void GameController::shuffle() { shuffle_(game_state_); }
inline int GameController::queryScore() { return game_state_.score_; }

#endif
