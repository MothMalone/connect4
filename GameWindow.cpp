#include "GameWindow.hpp"
#include <iostream>

GameWindow::GameWindow()
    : window(sf::VideoMode(700, 600), "Connect 4"), playerTurn(true) {
    // Load font for status text
    if (!font.loadFromFile("assets/fonts/DejaVuSansMono.ttf")) {
        std::cerr << "Error loading font!" << std::endl;
    }


    statusText.setFont(font);
    statusText.setCharacterSize(24);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(10, 10);

    // Compute layout based on window size
    int boardCols = GameSolver::Connect4::Position::WIDTH;
    int boardRows = GameSolver::Connect4::Position::HEIGHT;
    topMargin = 50.f; // reserve 50 pixels for status text
    cellWidth = window.getSize().x / static_cast<float>(boardCols);
    cellHeight = (window.getSize().y - topMargin) / static_cast<float>(boardRows);

    // Initialize board cells 
    for (int row = 0; row < boardRows; ++row) {
        for (int col = 0; col < boardCols; ++col) {
            boardCells[row][col].setSize(sf::Vector2f(cellWidth, cellHeight));
            boardCells[row][col].setFillColor(sf::Color::White);
            boardCells[row][col].setOutlineThickness(2);
            boardCells[row][col].setOutlineColor(sf::Color::Black);
            boardCells[row][col].setPosition(col * cellWidth, row * cellHeight + topMargin);
        }
    }

    // Initialize solver
    solver.reset(); // Reset the solver's state
    solver.loadBook("7x6.book"); // Load an opening book (optional)

    playerTurn = showStartMenu();
    gameOver = false; // initialize gameOver flag
    if (!playerTurn) {
        // If AI goes first, make its move
        aiMove();
    }
}

void GameWindow::resetGame(bool playerGoesFirst) {

    position = GameSolver::Connect4::Position();
    gameOver = false;
    playerTurn = playerGoesFirst;

    if (!playerTurn) {
        aiMove();
    }
}

bool GameWindow::showStartMenu() {
    sf::RectangleShape firstButton(sf::Vector2f(300, 80));
    sf::RectangleShape secondButton(sf::Vector2f(300, 80));

    // Position buttons
    firstButton.setPosition(window.getSize().x/2 - 150, 200);
    secondButton.setPosition(window.getSize().x/2 - 150, 300);

    // Setup button styling
    firstButton.setFillColor(sf::Color(50, 50, 250));
    secondButton.setFillColor(sf::Color(50, 50, 250));
    firstButton.setOutlineThickness(2);
    secondButton.setOutlineThickness(2);
    firstButton.setOutlineColor(sf::Color::White);
    secondButton.setOutlineColor(sf::Color::White);

    // Setup text
    sf::Text title, firstText, secondText;
    title.setFont(font);
    firstText.setFont(font);
    secondText.setFont(font);

    title.setString("Choose Your Turn Order");
    firstText.setString("Play First");
    secondText.setString("Play Second");

    title.setCharacterSize(32);
    firstText.setCharacterSize(24);
    secondText.setCharacterSize(24);

    // Center the text
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.width/2, titleBounds.height/2);
    title.setPosition(window.getSize().x/2, 100);

    sf::FloatRect firstBounds = firstText.getLocalBounds();
    firstText.setOrigin(firstBounds.width/2, firstBounds.height/2);
    firstText.setPosition(window.getSize().x/2, 240);

    sf::FloatRect secondBounds = secondText.getLocalBounds();
    secondText.setOrigin(secondBounds.width/2, secondBounds.height/2);
    secondText.setPosition(window.getSize().x/2, 340);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return true;
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                
                if (firstButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    return true;  // Player goes first
                }
                if (secondButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    return false; // AI goes first
                }
            }
        }
        
        // Update button hover effects
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        updateButtonHover(firstButton, mousePos);
        updateButtonHover(secondButton, mousePos);
        
        // Adjust text positions to match button scaling
        firstText.setPosition(firstButton.getPosition().x + firstButton.getSize().x / 2.f,
                            firstButton.getPosition().y + firstButton.getSize().y / 2.f);
        secondText.setPosition(secondButton.getPosition().x + secondButton.getSize().x / 2.f,
                             secondButton.getPosition().y + secondButton.getSize().y / 2.f);
        
        // Draw menu
        window.clear(sf::Color::Blue);
        window.draw(title);
        window.draw(firstButton);
        window.draw(secondButton);
        window.draw(firstText);
        window.draw(secondText);
        window.display();
    }

    return true; // Default to player first if window is closed
}

void GameWindow::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

// Modified processEvents to allow resetting the game when over
void GameWindow::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (gameOver) {
                // Reset the game on click if game is over
                bool playerGoesFirst = showStartMenu();
                resetGame(playerGoesFirst);
            } else if (playerTurn && event.mouseButton.button == sf::Mouse::Left) {
                int column = event.mouseButton.x / cellWidth; 
                handlePlayerMove(column);
            }
        }
    }
}

// Modified update to skip evaluation updates during game over
void GameWindow::update() {
    if (gameOver) return;
}

void GameWindow::render() {
    window.clear(sf::Color::Blue);

    int boardCols = GameSolver::Connect4::Position::WIDTH;
    int boardRows = GameSolver::Connect4::Position::HEIGHT;
    // Draw board cells (flip row order)
    for (int drawRow = 0; drawRow < boardRows; ++drawRow) {
        int posRow = boardRows - 1 - drawRow;
        for (int col = 0; col < boardCols; ++col) {
            int cell = position.getCell(posRow, col);
            if (cell == 1) {
                boardCells[drawRow][col].setFillColor(sf::Color::Red);
            } else if (cell == 2) {
                boardCells[drawRow][col].setFillColor(sf::Color::Yellow);
            } else {
                boardCells[drawRow][col].setFillColor(sf::Color::White);
            }
            window.draw(boardCells[drawRow][col]);
        }
    }

    // In the render() method, replace the evaluation bar code:
    if (!gameOver) {
        int score = solver.solve(position);
        float maxScore = 42.f; // Maximum theoretical score
        float barWidth = 500.f;
        float barHeight = 20.f;
        
        // Center the bar horizontally
        float barX = (window.getSize().x - barWidth) / 2.f;
        float barY = 10.f;
        
        // Background bar with outline
        sf::RectangleShape evalBarBg(sf::Vector2f(barWidth, barHeight));
        evalBarBg.setFillColor(sf::Color(50, 50, 50));
        evalBarBg.setOutlineColor(sf::Color::White);
        evalBarBg.setOutlineThickness(1);
        evalBarBg.setPosition(barX, barY);
        
        // Use exponential scaling for more dramatic changes near winning positions
        float normalizedScore;
        if (score > 0) {
            normalizedScore = 0.5f + 0.5f * std::min(1.f, std::pow(score / maxScore, 0.5f));
        } else {
            normalizedScore = 0.5f - 0.5f * std::min(1.f, std::pow(-score / maxScore, 0.5f));
        }
        
        float centerWidth = barWidth * normalizedScore;
        
        // Left side (First player - Red)
        sf::RectangleShape leftBar(sf::Vector2f(centerWidth, barHeight));
        sf::Color leftColor = sf::Color::Red;
        // Make color more intense when close to winning
        if (normalizedScore > 0.75f) {
            leftColor = sf::Color(255, 0, 0, 255); // Bright red
            // Add pulsing effect for near-win
            float pulse = (1 + std::sin(animationClock.getElapsedTime().asSeconds() * 5)) / 2;
            leftColor.a = 128 + static_cast<sf::Uint8>(127 * pulse);
        }
        leftBar.setFillColor(leftColor);
        leftBar.setPosition(barX, barY);
        
        // Right side (Second player - Yellow)
        sf::RectangleShape rightBar(sf::Vector2f(barWidth - centerWidth, barHeight));
        sf::Color rightColor = sf::Color::Yellow;
        // Make color more intense when close to winning
        if (normalizedScore < 0.25f) {
            rightColor = sf::Color(255, 255, 0, 255); // Bright yellow
            // Add pulsing effect for near-win
            float pulse = (1 + std::sin(animationClock.getElapsedTime().asSeconds() * 5)) / 2;
            rightColor.a = 128 + static_cast<sf::Uint8>(127 * pulse);
        }
        rightBar.setFillColor(rightColor);
        rightBar.setPosition(barX + centerWidth, barY);
        
        // Add markers for critical thresholds
        sf::RectangleShape centerLine(sf::Vector2f(2, barHeight));
        centerLine.setFillColor(sf::Color(200, 200, 200, 128));
        centerLine.setPosition(barX + barWidth/2, barY);
        
        // Score text with additional info
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setCharacterSize(16);
        scoreText.setFillColor(sf::Color::Black);
        
        // Add winning chance percentage
        float winChance = std::clamp((normalizedScore - 0.5f) * 2.f, -1.f, 1.f);
        std::string scoreStr = std::to_string(score);
        if (winChance > 0) {
            scoreStr += " (+" + std::to_string(static_cast<int>(winChance * 100)) + "%)";
        } else if (winChance < 0) {
            scoreStr += " (" + std::to_string(static_cast<int>(winChance * 100)) + "%)";
        }
        scoreText.setString(scoreStr);
        
        // Center the score text above the bar
        sf::FloatRect textBounds = scoreText.getLocalBounds();
        scoreText.setOrigin(textBounds.width / 2.f, textBounds.height - 13);
        scoreText.setPosition(barX + barWidth / 2.f, barY);
        
        // Draw everything
        window.draw(evalBarBg);
        window.draw(centerLine);
        window.draw(leftBar);
        window.draw(rightBar);
        window.draw(scoreText);
    }


    window.draw(statusText);

    window.display();
}

// Modified handlePlayerMove to stop processing moves when gameOver is true 
void GameWindow::handlePlayerMove(int column) {
    int boardCols = GameSolver::Connect4::Position::WIDTH;
    int boardRows = GameSolver::Connect4::Position::HEIGHT;
    if (column < 0 || column >= boardCols || !position.canPlay(column)) {
        return; // Invalid move
    }
    if (gameOver) return; // ignore moves if game is already over
    // Compute final row for the move
    int finalRow = 0;
    while(finalRow < boardRows &&
          position.getCell(finalRow, column) != 0) {
        finalRow++;
    }

    // animateMove(column, finalRow, sf::Color::Red);
    position.playCol(column); 
    checkGameOver(column);    // Check if the game is over
    if (!gameOver) {
        playerTurn = false;       // Switch to AI's turn
        aiMove();
    }
}

// Modified aiMove so that if a win occurs during the AI move, it does not switch back turn
void GameWindow::aiMove() {
    if (gameOver) return;
    // Analyze all possible moves
    std::vector<int> scores = solver.analyze(position);
    // Find the best move
    int bestMove = -1;
    int bestScore = GameSolver::Connect4::Solver::INVALID_MOVE;
    int boardCols = GameSolver::Connect4::Position::WIDTH;
    for (int col = 0; col < boardCols; ++col) {
        if (scores[col] > bestScore) {
            bestScore = scores[col];
            bestMove = col;
        }
    }
    // Play the best move 
    if (bestMove != -1) {
        int boardRows = GameSolver::Connect4::Position::HEIGHT;
        int finalRow = 0;
        while(finalRow < boardRows &&
              position.getCell(finalRow, bestMove) != 0) {
            finalRow++;
        }
        // animateMove(bestMove, finalRow, sf::Color::Yellow);
        position.playCol(bestMove);
        checkGameOver(bestMove); // Check game over after AI's move
    }
    if (!gameOver)
        playerTurn = true; // Switch back to the player's turn if game not over
}

// Modified checkGameOver signature and updated logic
void GameWindow::checkGameOver([[maybe_unused]] int lastColumn) {
    int boardCols = GameSolver::Connect4::Position::WIDTH;
    if (position.wins()) {
        statusText.setString(playerTurn ? "You Win!" : "AI Wins!");
        gameOver = true;
        // Render the final move before showing the pop-up
        render();
        window.display();
        sf::sleep(sf::milliseconds(200));
        showEndGamePopup(statusText.getString()); // Use end-game pop-up
        return;
    }
    // Check for a tie - occurs when all columns are full
    bool isTie = true;
    for (int col = 0; col < boardCols; col++) {
        if (position.canPlay(col)) {
            isTie = false;
            break;
        }
    }

    if (isTie) {
        statusText.setString("Game Tied!");
        gameOver = true;
        // Render the final move before showing the pop-up
        render();
        window.display();
        sf::sleep(sf::milliseconds(200));
        showEndGamePopup("Game Tied!");
        return;
    }
    evaluateState(); 
}


void GameWindow::animateMove(int column, int finalRow, sf::Color discColor) {
    // Animate the disc falling down
}




void GameWindow::showNotification(const std::string& message) {
    // Create notification text
}

void GameWindow::evaluateState() {
    std::vector<int> scores = solver.analyze(position);
    
    // Create evaluation text
    std::string eval = "Column scores:";
    for (int i = 0; i < GameSolver::Connect4::Position::WIDTH; i++) {
        eval += "\nColumn " + std::to_string(i + 1) + ": ";
        if (scores[i] == GameSolver::Connect4::Solver::INVALID_MOVE) {
            eval += "Invalid";
        } else {
            eval += std::to_string(scores[i]);
        }
    }
    
    // Update console output
    std::cout << "\n" << eval << "\n" << std::endl;
    
    // Calculate overall position evaluation
    int bestScore = GameSolver::Connect4::Solver::INVALID_MOVE;
    for (int score : scores) {
        if (score > bestScore && score != GameSolver::Connect4::Solver::INVALID_MOVE) {
            bestScore = score;
        }
    }

}


void GameWindow::showEndGamePopup(const std::string& message) {
    // Capture the current board state into a texture
    sf::Texture boardSnapshot;
    boardSnapshot.create(window.getSize().x, window.getSize().y);
    boardSnapshot.update(window);
    sf::Sprite boardSprite(boardSnapshot);

    // Create a semi-transparent overlay
    sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));

    // Create the pop-up background rectangle
    sf::RectangleShape popup(sf::Vector2f(400, 200));
    popup.setFillColor(sf::Color(50, 50, 50, 230));
    popup.setOutlineThickness(2);
    popup.setOutlineColor(sf::Color::White);
    popup.setPosition((window.getSize().x - 400) / 2, (window.getSize().y - 200) / 2);

    // Create text for the result message
    sf::Text popupText;
    popupText.setFont(font);
    popupText.setCharacterSize(32);
    popupText.setFillColor(sf::Color::White);
    popupText.setString(message);
    sf::FloatRect textBounds = popupText.getLocalBounds();
    popupText.setOrigin(textBounds.width / 2.f, textBounds.height / 2.f);
    popupText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 30);

    // Create a "Replay" button
    sf::RectangleShape replayButton(sf::Vector2f(150, 50));
    replayButton.setFillColor(sf::Color(100, 100, 250));
    replayButton.setOutlineThickness(2);
    replayButton.setOutlineColor(sf::Color::White);
    replayButton.setPosition(window.getSize().x / 2 - 75, window.getSize().y / 2 + 30);

    sf::Text replayText;
    replayText.setFont(font);
    replayText.setCharacterSize(24);
    replayText.setFillColor(sf::Color::White);
    replayText.setString("Replay");
    sf::FloatRect replayBounds = replayText.getLocalBounds();
    replayText.setOrigin(replayBounds.width / 2, replayBounds.height / 2);
    replayText.setPosition(window.getSize().x / 2, window.getSize().y / 2 + 55);

    // Display the pop-up until the user clicks the "Replay" button
    bool waiting = true;
    while (waiting && window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                waiting = false;
            } else if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                if (replayButton.getGlobalBounds().contains(mousePos)) {
                    waiting = false;
                }
            }
        }
        // Update replay button hover effect
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        updateButtonHover(replayButton, mousePos);
        
        // Adjust replay text position to match button scaling
        replayText.setPosition(replayButton.getPosition().x + replayButton.getSize().x / 2.f,
                             replayButton.getPosition().y + replayButton.getSize().y / 2.f);

        window.clear(sf::Color::Blue);
        window.draw(boardSprite);
        window.draw(overlay);
        window.draw(popup);
        window.draw(popupText);
        window.draw(replayButton);
        window.draw(replayText);
        window.display();
    }
    // After clicking replay, reset the game state
    position = GameSolver::Connect4::Position();
    gameOver = false;
    playerTurn = true;
    bool playerGoesFirst = showStartMenu();
    resetGame(playerGoesFirst);
}


void GameWindow::updateButtonHover(sf::RectangleShape& button, const sf::Vector2i& mousePos) {
    if (button.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
        button.setFillColor(sf::Color(80, 80, 255)); // Lighter blue when hovering
        button.setScale(1.05f, 1.05f);  // Slightly larger
        button.setOutlineThickness(3);   // Thicker outline
    } else {
        button.setFillColor(sf::Color(50, 50, 250)); // Normal blue
        button.setScale(1.0f, 1.0f);     // Normal size
        button.setOutlineThickness(2);    // Normal outline
    }
}

