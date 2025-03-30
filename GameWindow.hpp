#ifndef GAMEWINDOW_HPP
#define GAMEWINDOW_HPP

#include <SFML/Graphics.hpp>
#include "Position.hpp"
#include "Solver.hpp"

class GameWindow {
public:
    GameWindow();
    void run(); // Main game loop

private:
    sf::RenderWindow window;
    sf::RectangleShape boardCells[GameSolver::Connect4::Position::HEIGHT][GameSolver::Connect4::Position::WIDTH];
    sf::Font font;
    sf::Text statusText;

    GameSolver::Connect4::Position position;
    GameSolver::Connect4::Solver solver;
    bool playerTurn; 
    bool gameOver; 


    bool showStartMenu(); // Returns true if player wants to go first
    void resetGame(bool playerGoesFirst);
    
  
    float topMargin;  
    float cellWidth;   
    float cellHeight; 

    void processEvents();
    void update();        
    void render();       
    void handlePlayerMove(int column); 
    void aiMove();                     
    void checkGameOver([[maybe_unused]] int lastColumn); 

    // Animation and notification functions
    void animateMove(int column, int finalRow, sf::Color discColor);
    void showNotification(const std::string &message); 
    void evaluateState();
    void showEndGamePopup(const std::string &message);

    void updateButtonHover(sf::RectangleShape& button, const sf::Vector2i& mousePos);
    sf::Clock animationClock;  // Add this line for timing animations

};

#endif // GAMEWINDOW_HPP