#pragma once

#include "ofMain.h"
#include <queue>
#include <stack>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <chrono>
#include <random>

using namespace std;

// ------------------------------------------------------------
// CellType
// ------------------------------------------------------------
// EMPTY  : a normal movable cell with cost 1
// WALL   : blocked cell that cannot be entered
// START  : starting position of the search
// GOAL   : target position of the search
// WEIGHT : movable cell with a movement cost greater than 1
// VISUAL NOTE: START and GOAL are stored as special types so that
// they can be drawn with unique colors and protected from wall editing.
enum class CellType {
    EMPTY,
    WALL,
    START,
    GOAL,
    WEIGHT
};

// ------------------------------------------------------------
// Node
// ------------------------------------------------------------
// A lightweight coordinate pair used by every search algorithm.
// row and col are separated because the maze is stored as a 2D grid.
struct Node {
    int row;
    int col;

    Node(int r = -1, int c = -1) : row(r), col(c) {}

    bool operator==(const Node& other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Node& other) const {
        return !(*this == other);
    }
};

// ------------------------------------------------------------
// Cell
// ------------------------------------------------------------
// Each Cell represents one square in the maze.
// type : whether this square is wall, empty, start, goal, or weighted.
// cost : movement cost. Normal cells have cost 1, weighted cells use 3/5/7.
// This struct makes the maze easier to explain in the report because
// all information about one grid cell is grouped together.
struct Cell {
    CellType type;
    int cost;

    Cell(CellType t = CellType::EMPTY, int c = 1) : type(t), cost(c) {}
};

// ------------------------------------------------------------
// SearchResult
// ------------------------------------------------------------
// Stores the output of one algorithm execution.
// visitOrder   : exact order of visited nodes, used for animation.
// path         : final reconstructed path from START to GOAL.
// totalCost    : sum of movement costs along the final path.
// visitedCount : number of nodes expanded by the algorithm.
// elapsedMs    : measured execution time in milliseconds.
// found        : whether a valid path exists.
struct SearchResult {
    string algorithmName;
    vector<Node> visitOrder;
    vector<Node> path;
    int totalCost = 0;
    int visitedCount = 0;
    double elapsedMs = 0.0;
    bool found = false;
};

// ------------------------------------------------------------
// PriorityState
// ------------------------------------------------------------
// Used by priority_queue for Dijkstra and A*.
// priority is dist for Dijkstra and f = g + h for A*.
// costSoFar is the actual known cost from start to the current node.
struct PriorityState {
    Node node;
    int priority;
    int costSoFar;
};

// ------------------------------------------------------------
// PriorityCompare
// ------------------------------------------------------------
// C++ priority_queue is max-heap by default.
// This comparator reverses it into a min-heap so that the node with
// the smallest priority is popped first.
struct PriorityCompare {
    bool operator()(const PriorityState& a, const PriorityState& b) const {
        return a.priority > b.priority;
    }
};

// ------------------------------------------------------------
// EditMode
// ------------------------------------------------------------
// Controls what the mouse does when the user clicks a cell.
// WALL_EDIT   : toggle wall/empty
// START_EDIT  : move start node
// GOAL_EDIT   : move goal node
// WEIGHT_EDIT : toggle weighted cell
// This makes the program interactive without needing extra UI libraries.
enum class EditMode {
    WALL_EDIT,
    START_EDIT,
    GOAL_EDIT,
    WEIGHT_EDIT
};

class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    void mousePressed(int x, int y, int button) override;
    void mouseDragged(int x, int y, int button) override;

private:
    // --------------------------
    // Core maze data
    // --------------------------
    // grid is the main 2D data structure of this project.
    // Each element stores cell type and movement cost.
    vector<vector<Cell>> grid;

    // rows and cols define the size of the maze.
    // They are updated whenever a file is loaded or a random maze is generated.
    int rows = 0;
    int cols = 0;

    // startNode and goalNode are the two special positions used by algorithms.
    Node startNode;
    Node goalNode;

    // --------------------------
    // Visualization state
    // --------------------------
    // visibleVisited and visiblePath are separate from algorithm data.
    // They allow step-by-step animation after the algorithm finishes computing.
    vector<vector<bool>> visibleVisited;
    vector<vector<bool>> visiblePath;

    // lastResult stores the latest algorithm result and is used by draw().
    SearchResult lastResult;

    // animationIndex points to how many visited nodes have already been revealed.
    int animationIndex = 0;

    // pathAnimationIndex points to how many final path nodes have been revealed.
    int pathAnimationIndex = 0;

    // cellsPerFrame controls animation speed. Larger value = faster animation.
    int cellsPerFrame = 4;

    // paused stops update() from revealing more nodes.
    bool paused = false;

    // showHelp controls whether a detailed guide is drawn on the side panel.
    bool showHelp = true;

    // --------------------------
    // Interaction state
    // --------------------------
    EditMode editMode = EditMode::WALL_EDIT;
    Node lastMouseCell = Node(-1, -1);
    vector<string> mazeFiles;
    int currentMazeFileIndex = 0;

    // --------------------------
    // Layout values
    // --------------------------
    float mazeX = 30.0f;
    float mazeY = 30.0f;
    float cellSize = 18.0f;
    float panelX = 900.0f;

    // --------------------------
    // Maze loading / generation
    // --------------------------
    bool loadMazeFromFile(const string& filename);
    void loadNextMazeFile();
    void generateRandomMaze(int desiredRows, int desiredCols);
    void addRandomWeights(float probability);
    void clearMazeToEmpty(int newRows, int newCols);

    // --------------------------
    // Drawing helpers
    // --------------------------
    void drawMaze();
    void drawSidePanel();
    void drawLegend(float x, float y);
    void drawCell(int r, int c, float x, float y, float size);
    ofColor getCellColor(int r, int c) const;
    string getEditModeText() const;

    // --------------------------
    // Search algorithms
    // --------------------------
    SearchResult runBFS();
    SearchResult runDFS();
    SearchResult runDijkstra();
    SearchResult runAStar();

    // --------------------------
    // Algorithm utilities
    // --------------------------
    vector<Node> getNeighbors(const Node& node) const;
    bool inBounds(int r, int c) const;
    bool isWalkable(int r, int c) const;
    int getMoveCost(int r, int c) const;
    int heuristicManhattan(const Node& a, const Node& b) const;
    vector<Node> reconstructPath(const vector<vector<Node>>& parent, const Node& endNode) const;
    int computePathCost(const vector<Node>& path) const;

    // --------------------------
    // State utilities
    // --------------------------
    void resetVisualization();
    void prepareAnimation(const SearchResult& result);
    void resizeVisualArrays();
    void runAndStoreResult(int algorithmNumber);
    void cycleEditMode();

    // --------------------------
    // Mouse utilities
    // --------------------------
    bool screenToCell(int x, int y, int& outRow, int& outCol) const;
    void applyEditToCell(int r, int c);
};
