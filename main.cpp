#include <iostream>
#include <stdexcept>
#include "game.h"

void loadCards(Pile& draw_pile) {
    int n;
    std::cin >> n;
    for (int i = 0; i < n; ++i) {
        std::string type;
        std::cin >> type;
        Card* card = nullptr;
        if (type == "SCORE") {
            int point;
            std::cin >> point;
            card = new ScoreCard(point);
        } else if (type == "POWER") {
            int bonus;
            std::cin >> bonus;
            card = new PowerCard(bonus);
        } else if (type == "SKILL") {
            std::string skill;
            std::cin >> skill;
            if (skill == "Multiplier") card = new SkillCard(MULTIPLIER);
            else if (skill == "HeadButt") card = new SkillCard(HEAD_BUTT);
        }
        if (!card) throw std::runtime_error("Invalid Operation");
        draw_pile.appendCard(card);
    }
}

void processCommands(GameController& game) {
    std::string command;
    while (std::cin >> command) {
        if (command == "draw") game.draw();
        else if (command == "finish") game.finish();
        else if (command == "exit") break;
        else if (command == "play") {
            int index;
            std::cin >> index;
            game.play(index);
        } else if (command == "query") {
            std::string type;
            std::cin >> type;
            if (type == "score") std::cout << game.queryScore() << '\n';
            else if (type == "hand") std::cout << game.queryHandSize() << '\n';
            else if (type == "draw_pile") std::cout << game.queryDrawPileSize() << '\n';
            else if (type == "discard_pile") std::cout << game.queryDiscardPileSize() << '\n';
            else throw std::runtime_error("Invalid Operation");
        } else throw std::runtime_error("Invalid Operation");
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int mode;
    if (!(std::cin >> mode)) return 0;
    GameController game(mode);
    loadCards(game.drawPile());
    processCommands(game);
}
