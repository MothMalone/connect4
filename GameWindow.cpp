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

    gameOver = false; // initialize gameOver flag
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
                position = GameSolver::Connect4::Position();
                gameOver = false;
                playerTurn = true;
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
    if (playerTurn) {
        statusText.setString("Your Turn");
    } else {
        int score = solver.solve(position);
        statusText.setString("AI's Turn (Evaluation: " + std::to_string(score) + ")");
    }
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
    
    // Update status text with position evaluation
    if (bestScore != GameSolver::Connect4::Solver::INVALID_MOVE) {
        std::string evalString = (playerTurn ? "Your" : "AI's") + std::string(" turn (Eval: ") + 
                               std::to_string(bestScore) + ")";
        statusText.setString(evalString);
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
        window.clear(sf::Color::Blue);
        // Draw the captured board snapshot
        window.draw(boardSprite);
        // Draw the overlay and pop-up elements
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
    statusText.setString("Your Turn");
}

