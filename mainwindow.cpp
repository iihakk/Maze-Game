#include "mainwindow.h"
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <vector>
#include <queue>
#include <functional>
#include <QBrush>

using namespace std;

// hashing function
struct pair_hash {
    template <class T1, class T2>
    size_t operator () (const pair<T1,T2> &pair) const {
        auto hash1 = hash<T1>{}(pair.first);
        auto hash2 = hash<T2>{}(pair.second);
        return hash1 ^ hash2;
    }
};
// this is the widget represeting the maze which inherits from qwidgets
class MazeWidget : public QWidget {
public:
    MazeWidget(QWidget *parent = nullptr) : QWidget(parent), playerPosition(1, 1), goalPosition(18, 18) {
        initMaze();
        findPath();
        setFocusPolicy(Qt::StrongFocus);  // Ensuring widget can capture keyboard events
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        drawMaze(painter);
        drawPath(painter);
        drawPlayer(painter);
    }
           // move the player based on the user input from the keyboard
    void keyPressEvent(QKeyEvent *event) override {
        int x = playerPosition.first;
        int y = playerPosition.second;
        switch (event->key()) {
        case Qt::Key_Up:    y--; break;
        case Qt::Key_Down:  y++; break;
        case Qt::Key_Left:  x--; break;
        case Qt::Key_Right: x++; break;
        default: return;
        }

        if (isValidPosition(x, y)) {
            playerPosition = {x, y};
            findPath();
            update();  // updates the map after the changes
        }
    }

private:

    vector<vector<int>> maze; // representing the maze as a 2d vector
    pair<int, int> playerPosition; // the current position of the player
     pair<int, int> goalPosition; // the target position in the maze
    vector<pair<int, int>> path; // stores the path from the player to the goal

     unordered_map<pair<int, int>, pair<int, int>, pair_hash> prev;
    unordered_map<pair<int, int>, int, pair_hash> dist;
        // a function to initialize the map with random walls everytime of running
    void initMaze() {
        maze.resize(20, vector<int>(20, 0));

        // setting the perimeter of the walls
        for (int i = 0; i < 20; ++i) {
            maze[0][i] = maze[19][i] = maze[i][0] = maze[i][19] = 1;
        }

        // Seed for random number generation
        srand(time(0));

        // randomly placing walls in the maze while ensuring at least one open paths
        for (int i = 1; i < 19; ++i) {
            for (int j = 1; j < 19; ++j) {
                if (rand() % 5 == 0) { //this will ensure 20% of walls in the maze
                    maze[i][j] = 1;
                }
            }
            // to ensure a path from start to end always exsist
        } vector<pair<int, int>> forcedPath = {{1, 1}, {1, 2}, {1, 3}, {2, 3}, {3, 3}, {3, 4}, {3, 5}, {4, 5}, {5, 5}, {6, 5}, {7, 5}, {8, 5}, {9, 5}, {10, 5}, {11, 5}, {12, 5}, {13, 5}, {14, 5}, {15, 5}, {16, 5}, {17, 5}, {18, 5}, {18, 6}, {18, 7}, {18, 8}, {18, 9}, {18, 10}, {18, 11}, {18, 12}, {18, 13}, {18, 14}, {18, 15}, {18, 16}, {18, 17}, {18, 18}};
        for (size_t i = 0; i < forcedPath.size(); ++i) {
            int x = forcedPath[i].first;
            int y = forcedPath[i].second;
            maze[x][y] = 0;
        }
    }
        // dijkstra's algrothim to find the shortest path
    void findPath() {
        // Clear previous state before running the pathfinder
        dist.clear();
        prev.clear();
        priority_queue<pair<int, pair<int, int>>,
                            vector<pair<int, pair<int, int>>>,
                            greater<pair<int, pair<int, int>>>> pq;
        pq.push({0, playerPosition});
        dist[playerPosition] = 0;

        while (!pq.empty()) {
            auto [d, v] = pq.top(); pq.pop();
            if (dist[v] < d) continue;
            vector<pair<int, int>> directions = {{0,1}, {1,0}, {0,-1}, {-1,0}};
            for (size_t i = 0; i < directions.size(); ++i) {
                int dx = directions[i].first;
                int dy = directions[i].second;
                int x = v.first + dx;
                int y = v.second + dy;
                if (!isValidPosition(x, y)) continue;
                int newDist = d + 1;
                if (dist.find({x, y}) == dist.end() || newDist < dist[{x, y}]) {
                    dist[{x, y}] = newDist;
                    prev[{x, y}] = v;
                    pq.push({newDist, {x, y}});
                }
            }
        }

        // Trace path back from goal to player
        path.clear();
        pair<int, int> at = goalPosition;
        while (at != playerPosition) {
            path.push_back(at);
            at = prev[at]; // This line could be problematic if there's no path found causing a crash
        }
        path.push_back(playerPosition);
       reverse(path.begin(), path.end());
    }

    bool isValidPosition(int x, int y) {
        return x >= 0 && y >= 0 && x < 20 && y < 20 && maze[x][y] == 0;
    }
    // a function to draw the maze
    void drawMaze(QPainter &painter) {
        painter.setBrush(Qt::black);
        for (int i = 0; i < 20; ++i) {
            for (int j = 0; j < 20; ++j) {
                if (maze[i][j] == 1) {
                    painter.drawRect(i * 20, j * 20, 20, 20);
                }
            }
        }
    }
    // a function to draw the player at the current position
    void drawPlayer(QPainter &painter) {
        painter.setBrush(Qt::blue);
        painter.drawEllipse(playerPosition.first * 20, playerPosition.second * 20, 20, 20);
    }
        // this function prints the shortest path using dijkstra's algrothim
    void drawPath(QPainter &painter) {
        painter.setPen(QPen(Qt::yellow, 2));
        QPoint lastPoint = QPoint(playerPosition.first * 20 + 10, playerPosition.second * 20 + 10);
        for (size_t i = 0; i < path.size(); ++i) {
            QPoint nextPoint(path[i].first * 20 + 10, path[i].second * 20 + 10);
            painter.drawLine(lastPoint, nextPoint);
            lastPoint = nextPoint;
        }
    }
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setCentralWidget(new MazeWidget(this));
    resize(420, 440); //
}

MainWindow::~MainWindow() {
    // destructor
}

