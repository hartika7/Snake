/*
####################################################################
# TIE-02201 Ohjelmointi 2: Perusteet, K2019                        #
# TIE-02207 Programming 2: Basics, S2019                           #
#                                                                  #
# Project4: Snake                                                  #
#                                                                  #
# File: main_window.hh                                             #
# Description: Declares a class implementing a UI for the game.    #
#                                                                  #
# Author: Timo Hartikainen, timo.hartikainen@tuni.fi               #
####################################################################
*/

#ifndef PRG2_SNAKE2_MAINWINDOW_HH
#define PRG2_SNAKE2_MAINWINDOW_HH

#include "ui_main_window.h"
#include <QMainWindow>
#include <QCloseEvent>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsItemAnimation>
#include <QTimeLine>
#include <QTimer>
#include <QKeyEvent>
#include <QMessageBox>
#include <QDebug>
#include <fstream>
#include <string>
#include <random>
#include <chrono>
#include <math.h>

const std::string INSTRUCTIONS_FILE = "instructions.txt"; /**< File containing
                                                               instructions. */

const QString WINDOW_TITLE = "Snake 2"; /**< Window title for dialogs. */
const int WINDOW_WIDTH_MIN = 622;       /**< Window width scoretable hidden. */
const int WINDOW_WIDTH_MAX = 890;       /**< Window width scoretable visible. */

const QPointF SNAKE_START = QPointF(45, 45);      /**< Snake start position. */
const QRectF UNIT_RECTANGLE = QRectF(0, 0, 5, 5); /**< Game field
                                                       unit rectangle. */

// Unit displacement vectors.
const QPointF DIR_UP = QPointF(0, -5);          /**< Snake direction up. */
const QPointF DIR_RIGHT = QPointF(5, 0);        /**< Snake direction right. */
const QPointF DIR_DOWN = QPointF(0, 5);         /**< Snake direction down. */
const QPointF DIR_LEFT = QPointF(-5, 0);        /**< Snake direction left. */

/* \class MainWindow
 * \brief Implements the main window through which the game is played.
 */
class MainWindow: public QMainWindow {
    Q_OBJECT

public:

    /* \brief Construct a MainWindow.
     *
     * \param[in] parent The parent widget of this MainWindow.
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /* \brief Destruct a MainWindow.
     */
    ~MainWindow() override = default;

    /* \brief Ask user before exiting the game.
     *
     * \param[in] event Contains data on closing event.
     */
    void closeEvent(QCloseEvent *event) override;

    /* \brief Change the Snake's bearing when certain keys get pressed.
     *
     * \param[in] event Contains data on pressed and released keys.
     */
    void keyPressEvent(QKeyEvent* event) override;


private slots:

    /* \brief Start the game.
     */
    void on_playButton_clicked();

    /* \brief Pause the game.
     */
    void on_pauseButton_clicked();

    /* \brief Show or hide scoretable.
     */
    void on_scoretablePushButton_clicked();

    /* \brief Show instructions.
     */
    void on_instructionsButton_clicked();

    /* \brief Calulate snake speed.
     */
    void on_levelDial_sliderReleased();

    /* \brief Update level label.
     *
     * \param[in] value Dial value.
     */
    void on_levelDial_valueChanged(int value);

    /* \brief Move the Snake by a square and check for collisions.
     *
     * The game ends if the Snake gets in the way.
     * When a food gets eaten a point is gained and the Snake grows.
     */
    void moveSnake();

    /* \brief Count game time.
     */
    void countClock();


private:

    /* \brief Initialize random number generator.
     */
    void seedRandomNumberGenerator();

    /* \brief Make the play field visible and fit it into the view.
     *
     * Should be called every time the field's size changes.
     */
    void adjustSceneArea();

    /* \brief Make scene objects' movement smooth.
     *
     * \param[in] object Scene object to be moved.
     * \param[in] destination Destination point.
     * \param[in] ignore_distance If true, displacement longer than one unit
     *            vector length will be animated, too.
     */
    void animateMove(QGraphicsEllipseItem* object, QPointF destination,
                     bool ignore_distance = false);

    /* \brief Place item randomly in game field.
     *
     * \param[in] object Scene object to be moved.
     * \param[in] exclude_borders If true, object won't be placed near borders.
     */
    void placeRandom(QGraphicsEllipseItem* object,
                     bool exclude_borders = false);

    /* \brief Increase snake length and place food again.
     */
    void eatFood();

    /* \brief Start game.
     */
    void startGame();

    /* \brief Stop game.
     */
    void stopGame();

    /* \brief Delete scene objects.
     */
    void deleteSceneObjects();

    /* \brief Add new result row to scoretable.
     */
    void updateScoreTable();

    /* \brief Calculate snake speed according to selected level.
     *
     * \return New speed.
     */
    int calculateSpeed();

    /* \brief Convert seconds to string containing minutes and seconds.
     *
     * \param[in] seconds Number of seconds.
     *
     * \return Time string.
     */
    QString secondsToTime(int seconds);

    /* \brief Get random snake direction.
     *
     * \return Randomly pointing unit displacement vector.
     */
    QPointF getRandomDir();

    /* \brief Get random corner location.
     *
     * \return Random corner location point.
     */
    QPointF getRandomCorner();

    Ui::MainWindow ui_;                 /**< Accesses the UI widgets. */
    QGraphicsScene scene_;              /**< Manages drawable objects. */
    std::vector<QGraphicsEllipseItem*> snake_ = {}; /**< Contains snake
                                                         parts. */
    QGraphicsEllipseItem* food_ = nullptr;          /**< The food item in the
                                                         scene. */
    QGraphicsEllipseItem* wormhole_ = nullptr;      /**< The wormhole item in
                                                         the scene.  */
    QPointF snake_dir_ = DIR_UP;        /**< Snake moving direction.  */
    QPointF last_tail_ = SNAKE_START;   /**< Last snake tail piece location.  */
    QTimer timer_;                      /**< Triggers the Snake to move. */
    QTimer clock_timer_;                /**< Triggers game time to update. */
    std::default_random_engine rng_;    /**< Randomizes integers. */
    bool game_active_ = false;          /**< Contains game status. */
    int time_ = 0;                      /**< Contains game time in seconds. */
    int score_ = 0;                     /**< Contains game score. */
    int level_ = 1;                     /**< Contains game level. */
    int speed_ = 900;                   /**< Contains snake speed. */

};  // class MainWindow


#endif  // PRG2_SNAKE2_MAINWINDOW_HH
