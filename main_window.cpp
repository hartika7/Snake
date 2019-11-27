/*
####################################################################
# TIE-02201 Ohjelmointi 2: Perusteet, K2019                        #
# TIE-02207 Programming 2: Basics, S2019                           #
#                                                                  #
# Project4: Snake                                                  #
#                                                                  #
# File: main_window.cpp                                            #
# Description: Defines a class implementing a UI for the game.     #
#                                                                  #
# Author: Timo Hartikainen, timo.hartikainen@tuni.fi               #
####################################################################
*/

#include "main_window.hh"

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent) {

    ui_.setupUi(this);
    ui_.graphicsView->setScene(&scene_);

    // Hide score table
    this->setFixedSize(WINDOW_WIDTH_MIN, this->height());

    ui_.scoreTableWidget->setHorizontalHeaderLabels(QStringList()
                                                    << "Level"
                                                    << "Score"
                                                    << "Time");

    seedRandomNumberGenerator();

    connect(&timer_, &QTimer::timeout, this, &MainWindow::moveSnake);
    connect(&clock_timer_, &QTimer::timeout, this, &MainWindow::countClock);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QMessageBox::StandardButton dialog_result =
            QMessageBox::question(this, WINDOW_TITLE,
                                  "Are you sure?\n",
                                  QMessageBox::Cancel |
                                  QMessageBox::No |
                                  QMessageBox::Yes,
                                  QMessageBox::Yes);

    if (dialog_result != QMessageBox::Yes)
        event->ignore();
    else
        event->accept(); // Exit
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (!timer_.isActive())
        return; // Change snake direction only if game is being played

    switch (event->key())
    {
        case Qt::Key_W:
            if (snake_dir_ != DIR_DOWN)
                snake_dir_ = DIR_UP;
            break;
        case Qt::Key_D:
            if (snake_dir_ != DIR_LEFT)
                snake_dir_ = DIR_RIGHT;
            break;
        case Qt::Key_S:
            if (snake_dir_ != DIR_UP)
                snake_dir_ = DIR_DOWN;
            break;
        case Qt::Key_A:
            if (snake_dir_ != DIR_RIGHT)
                snake_dir_ = DIR_LEFT;
            break;
    }
}

void MainWindow::on_playButton_clicked() {
    if (game_active_)
        stopGame();
    else
        startGame();

    ui_.playButton->setText(game_active_ ? "Stop" : "Restart");

    ui_.levelDial->setEnabled(!game_active_);
    ui_.pauseButton->setEnabled(game_active_);
}

void MainWindow::on_pauseButton_clicked() {
    if (timer_.isActive()) {
        timer_.stop();
        clock_timer_.stop();
        ui_.pauseButton->setText("Resume");
        ui_.playButton->setEnabled(false);
    } else {
        timer_.start(calculateSpeed());
        clock_timer_.start(1000);
        ui_.pauseButton->setText("Pause");
        ui_.playButton->setEnabled(true);
    }
}

void MainWindow::startGame() {
    // Initialize scene only once
    if (!snake_.size())
        adjustSceneArea();

    // Delete old scene objects
    deleteSceneObjects();

    // Reset counters
    ui_.scoreLcdNumber->display(0);
    ui_.timeValueLabel->setText("00:00");
    score_ = 0;
    time_ = 0;

    // Add items to scene
    food_ = scene_.addEllipse(UNIT_RECTANGLE, QPen(Qt::white, 0),
                              QBrush(Qt::yellow));
    wormhole_ = scene_.addEllipse(UNIT_RECTANGLE, QPen(Qt::white, 0),
                                  QBrush(Qt::black));
    snake_.push_back(scene_.addEllipse(UNIT_RECTANGLE, QPen(Qt::white, 0),
                                       QBrush(Qt::darkGreen)));

    // Place items
    snake_.front()->setPos(SNAKE_START);
    placeRandom(food_);
    placeRandom(wormhole_);

    // Start game
    timer_.start(calculateSpeed());
    clock_timer_.start(1000);
    game_active_ = true;
}

void MainWindow::seedRandomNumberGenerator() {
    // Use current time as a seed for random number generator
    const auto time_point_now = std::chrono::system_clock::now();
    const auto elapsed = time_point_now.time_since_epoch();
    const auto secs = std::chrono::duration_cast<std::chrono::seconds>(elapsed);

    std::default_random_engine rng_(secs.count());
}

void MainWindow::adjustSceneArea() {
    const QRectF area(0, 0, 100, 100);
    scene_.setSceneRect(area);
    ui_.graphicsView->fitInView(area);

    ui_.graphicsView->setRenderHints(QPainter::Antialiasing);
}

void MainWindow::stopGame() {
    timer_.stop();
    clock_timer_.stop();

    updateScoreTable();

    game_active_ = false;
}

void MainWindow::deleteSceneObjects() {
    for (auto part : snake_)
        delete part;
    snake_.clear();

    delete food_;
    delete wormhole_;
}

void MainWindow::updateScoreTable() {
    if (score_ == 0)
        return; // Don't store empty results

    // Next row index
    int row_number = ui_.scoreTableWidget->rowCount();

    // Add new row
    ui_.scoreTableWidget->setRowCount(row_number + 1);

    ui_.scoreTableWidget->setItem(row_number, 0,
                                  new QTableWidgetItem(
                                      QString::number(level_)));
    ui_.scoreTableWidget->setItem(row_number, 1,
                                  new QTableWidgetItem(
                                      QString::number(score_)));
    ui_.scoreTableWidget->setItem(row_number, 2,
                                  new QTableWidgetItem(
                                      secondsToTime(time_)));
}

void MainWindow::moveSnake() {
    // Update position for the next possible piece (if food will be eaten)
    last_tail_ = snake_.back()->scenePos();

    // Store new positions temporarily (snake is animated once in the end)
    std::vector<std::pair<QGraphicsEllipseItem*, QPointF>> temp_snake = {};
    for (uint i=snake_.size()-1; i >= 1; i--) {
        temp_snake.push_back({snake_.at(i), snake_.at(i - 1)->scenePos()});
    }

    // Move head first
    QPointF old_head_pos = snake_.front()->scenePos();
    QPointF new_head_pos = old_head_pos + snake_dir_;

    // Cross walls
    if (new_head_pos.x() > 95)
        new_head_pos.setX(0);
    else if (new_head_pos.x() < 0)
        new_head_pos.setX(95);

    if (new_head_pos.y() > 95)
        new_head_pos.setY(0);
    else if (new_head_pos.y() < 0)
        new_head_pos.setY(95);

    // Handle food and wormhole
    if (old_head_pos == wormhole_->scenePos()) {
        // Move head
        placeRandom(snake_.front(), true);
        new_head_pos = snake_.front()->scenePos();

        // Change direction
        snake_dir_ = getRandomDir();

        // Move wormhole behind head
        wormhole_->setPos(new_head_pos - snake_dir_);

        // Check food
        if (new_head_pos== food_->scenePos())
            eatFood();
    } else {
        // Check food
        if (old_head_pos == food_->scenePos())
            eatFood();

        // Move head
        animateMove(snake_.front(), new_head_pos);
    }

    // Move parts
    for (auto part : temp_snake) {
        animateMove(part.first, part.second);
    }

    // Check self-crossing
    for (auto part : temp_snake) {
        if (new_head_pos == part.second) {
            // Stop game
            on_playButton_clicked();

            // Paint snake red
            for (auto piece : snake_) {
                piece->setBrush(QBrush(Qt::red));
            }

            // Display losing message
            QMessageBox::information(0, WINDOW_TITLE, "You Lost!");

            break;
        }
    }
}

void MainWindow::animateMove(QGraphicsEllipseItem* object, QPointF destination,
                             bool ignore_distance) {
    // Calculate coordinate changes
    int d_x = destination.x() - object->scenePos().x();
    int d_y = destination.y() - object->scenePos().y();

    // Don't animate long displacements by default
    if ((abs(d_x) > 5 || abs(d_y) > 5) && !ignore_distance) {
        object->setPos(destination);
        return;
    }

    // Animate moving
    QGraphicsItemAnimation* animation = new QGraphicsItemAnimation(this);
    QTimeLine* timer = new QTimeLine(calculateSpeed() - 100, this);
    animation->setItem(object);
    animation->setTimeLine(timer);
    animation->setPosAt(1, destination);
    timer->start();
}

void MainWindow::eatFood() {
    // Calculate new piece color
    double piece_color = 200 * atan(snake_.size() / 3.5) * 2 / M_PI;
    const QBrush piece_brush(QColor(piece_color, 255, piece_color));

    // Add to snake
    snake_.push_back(scene_.addEllipse(UNIT_RECTANGLE, QPen(Qt::white, 0),
                                       piece_brush));

    // Animate adding
    snake_.back()->setPos(getRandomCorner());
    animateMove(snake_.back(), last_tail_, true);

    // Update score
    score_ += 1;
    ui_.scoreLcdNumber->display(score_);

    // Check if snake fills the whole game field except wormhole
    if (snake_.size() == 399) {
        stopGame();
        QMessageBox::information(0, WINDOW_TITLE, "Congratulations! You Won!");
        return;
    }

    // Change food location
    placeRandom(food_);

    // Update snake speed
    timer_.stop();
    timer_.start(calculateSpeed());
}

void MainWindow::placeRandom(QGraphicsEllipseItem* object,
                             bool exclude_borders) {
    std::uniform_int_distribution<int> int_dist;

    if (exclude_borders)
        int_dist = std::uniform_int_distribution<int>(1, 18);
    else
        int_dist = std::uniform_int_distribution<int>(0, 19);

    // Find spare spots
    while (true) {
        bool skip_point = false;
        QPointF new_pos = QPointF(int_dist(rng_) * 5, int_dist(rng_) * 5);

        if (food_->scenePos() == new_pos || wormhole_->scenePos() == new_pos)
            continue;

        for (auto part : snake_) {
            if (part->scenePos() == new_pos)
                skip_point = true;
        }

        if (!skip_point) {
            object->setPos(new_pos);
            break;
        }
    }
}

QPointF MainWindow::getRandomDir() {
    std::uniform_int_distribution<int> int_dist(1, 4);

    switch (int_dist(rng_)) {
        case 1:
            return DIR_UP;
        case 2:
            return DIR_RIGHT;
        case 3:
            return DIR_DOWN;
        case 4:
            return DIR_LEFT;
        default:
            return DIR_UP;
    }
}

QPointF MainWindow::getRandomCorner() {
    std::uniform_int_distribution<int> int_dist(1, 4);

    switch (int_dist(rng_)) {
        case 1:
            return QPointF(-6,-6);
        case 2:
            return QPointF(101, -6);
        case 3:
            return QPointF(101, 101);
        case 4:
            return QPointF(-6, 101);
        default:
            return QPointF(-6, -6);
    }
}

void MainWindow::countClock() {
    time_ += 1;
    ui_.timeValueLabel->setText(secondsToTime(time_));
}

int MainWindow::calculateSpeed() {
    return std::max((int)(speed_ * 0.5), speed_ - 20 * score_);
}

QString MainWindow::secondsToTime(int seconds) {
    int min = seconds / 60;
    int sec = seconds % 60;

    QString min_str = min < 10 ? "0" + QString::number(min) :
                                 QString::number(min);
    QString sec_str = sec < 10 ? "0" + QString::number(sec) :
                                 QString::number(sec);

    return min_str + ":" + sec_str;
}

void MainWindow::on_scoretablePushButton_clicked() {
    // Show or hide scoretable
    this->setFixedSize(this->width() == WINDOW_WIDTH_MIN ? WINDOW_WIDTH_MAX :
                                                           WINDOW_WIDTH_MIN,
                                                           this->height());

    ui_.scoretablePushButton->setText(this->width() == WINDOW_WIDTH_MIN ?
                                          "Scores >>" : "Scores <<");
}

void MainWindow::on_levelDial_sliderReleased() {
    level_ = ui_.levelDial->value();
    speed_ = (5 - level_) * 225;
}

void MainWindow::on_levelDial_valueChanged(int value) {
    ui_.levelLabel->setText("Level: " + QString::number(value));
}

void MainWindow::on_instructionsButton_clicked() {
    QString contents = "";
    std::string line = "";
    std::ifstream instructions_file(INSTRUCTIONS_FILE);

    if (instructions_file.is_open()) {
        while (std::getline(instructions_file, line)) {
            if (line == "##")
                break; // Skip the rest of file (details for returning)

            contents += QString::fromStdString(line + "\n");
        }

        instructions_file.close();
    }

    QMessageBox::information(0, WINDOW_TITLE, contents.length() ?
                                              contents.trimmed() :
                                              "File not found.");
}
