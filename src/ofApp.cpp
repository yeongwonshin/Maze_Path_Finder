#include "ofApp.h"

// ------------------------------------------------------------
// setup
// ------------------------------------------------------------
// Initializes sample file list, loads the first maze, and sets drawing options.
// This function is called once at program start by OpenFrameworks.
void ofApp::setup() {
    ofSetWindowTitle("Maze Pathfinder Lab - BFS / DFS / Dijkstra / A*");
    ofSetFrameRate(60);
    ofBackground(24, 26, 31);

    // Sample maze files are stored in bin/data.
    // The user can cycle through them by pressing L.
    mazeFiles = {
        "maze_easy.txt",
        "maze_weighted.txt",
        "maze_large.txt"
    };

    // Load the first sample maze. If loading fails, build a safe default maze
    // so that the program still runs during grading.
    if (!loadMazeFromFile(mazeFiles[currentMazeFileIndex])) {
        generateRandomMaze(25, 35);
    }

    resetVisualization();
}

// ------------------------------------------------------------
// update
// ------------------------------------------------------------
// Reveals the search result gradually for visualization.
// The actual algorithm is already computed when the user presses 1/2/3/4.
void ofApp::update() {
    if (paused || lastResult.algorithmName.empty()) {
        return;
    }

    // First reveal visited nodes in the order produced by the algorithm.
    for (int i = 0; i < cellsPerFrame && animationIndex < (int)lastResult.visitOrder.size(); ++i) {
        Node n = lastResult.visitOrder[animationIndex];
        if (inBounds(n.row, n.col)) {
            visibleVisited[n.row][n.col] = true;
        }
        animationIndex++;
    }

    // After all visited nodes are displayed, reveal the final path.
    if (animationIndex >= (int)lastResult.visitOrder.size()) {
        for (int i = 0; i < cellsPerFrame && pathAnimationIndex < (int)lastResult.path.size(); ++i) {
            Node n = lastResult.path[pathAnimationIndex];
            if (inBounds(n.row, n.col)) {
                visiblePath[n.row][n.col] = true;
            }
            pathAnimationIndex++;
        }
    }
}

// ------------------------------------------------------------
// draw
// ------------------------------------------------------------
// Draws the maze, animated search result, statistics, and guide text.
void ofApp::draw() {
    ofBackground(24, 26, 31);

    // Recalculate layout so the program adapts to different window sizes.
    float availableW = ofGetWidth() * 0.68f;
    float availableH = ofGetHeight() - 60.0f;
    if (rows > 0 && cols > 0) {
        cellSize = min(availableW / cols, availableH / rows);
        cellSize = ofClamp(cellSize, 8.0f, 28.0f);
    }
    panelX = mazeX + cols * cellSize + 35.0f;

    drawMaze();
    drawSidePanel();
}

// ------------------------------------------------------------
// keyPressed
// ------------------------------------------------------------
// Handles all keyboard shortcuts. The shortcuts are shown in the side panel.
void ofApp::keyPressed(int key) {
    if (key == '1') {
        runAndStoreResult(1);
    } else if (key == '2') {
        runAndStoreResult(2);
    } else if (key == '3') {
        runAndStoreResult(3);
    } else if (key == '4') {
        runAndStoreResult(4);
    } else if (key == 'r' || key == 'R') {
        resetVisualization();
    } else if (key == 'l' || key == 'L') {
        loadNextMazeFile();
    } else if (key == 'g' || key == 'G') {
        generateRandomMaze(31, 45);
        resetVisualization();
    } else if (key == 'm' || key == 'M') {
        cycleEditMode();
    } else if (key == 'p' || key == 'P') {
        paused = !paused;
    } else if (key == 'h' || key == 'H') {
        showHelp = !showHelp;
    } else if (key == '+' || key == '=') {
        cellsPerFrame = min(100, cellsPerFrame + 2);
    } else if (key == '-' || key == '_') {
        cellsPerFrame = max(1, cellsPerFrame - 2);
    } else if (key == 'c' || key == 'C') {
        clearMazeToEmpty(25, 35);
        resetVisualization();
    }
}

// ------------------------------------------------------------
// mousePressed / mouseDragged
// ------------------------------------------------------------
// The mouse edits the maze according to the current EditMode.
void ofApp::mousePressed(int x, int y, int button) {
    int r = 0;
    int c = 0;
    if (screenToCell(x, y, r, c)) {
        lastMouseCell = Node(r, c);
        applyEditToCell(r, c);
    }
}

void ofApp::mouseDragged(int x, int y, int button) {
    int r = 0;
    int c = 0;
    if (screenToCell(x, y, r, c)) {
        // Dragging is useful mainly for wall drawing and weighted-cell drawing.
        // lastMouseCell prevents the same cell from being toggled repeatedly.
        if ((editMode == EditMode::WALL_EDIT || editMode == EditMode::WEIGHT_EDIT) && Node(r, c) != lastMouseCell) {
            lastMouseCell = Node(r, c);
            applyEditToCell(r, c);
        }
    }
}

// ------------------------------------------------------------
// loadMazeFromFile
// ------------------------------------------------------------
// Reads a text maze from bin/data.
// Supported symbols:
// # = wall, . or space = empty, S = start, G = goal,
// 2~9 = weighted cell with that movement cost, W = weighted cell with cost 5.
bool ofApp::loadMazeFromFile(const string& filename) {
    ofBuffer buffer = ofBufferFromFile(ofToDataPath(filename, true));
    if (!buffer.size()) {
        ofLogWarning("loadMazeFromFile") << "Could not load " << filename;
        return false;
    }

    vector<string> lines;
    int maxCols = 0;

    // Read all non-empty lines first so that the grid can be rectangular.
    for (const string& rawLine : buffer.getLines()) {
        string line = rawLine;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        lines.push_back(line);
        maxCols = max(maxCols, (int)line.size());
    }

    if (lines.empty() || maxCols == 0) {
        return false;
    }

    rows = (int)lines.size();
    cols = maxCols;
    grid.assign(rows, vector<Cell>(cols, Cell(CellType::WALL, 1)));

    bool hasStart = false;
    bool hasGoal = false;

    // Convert characters into Cell objects.
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            char ch = (c < (int)lines[r].size()) ? lines[r][c] : '#';

            if (ch == '#') {
                grid[r][c] = Cell(CellType::WALL, 1);
            } else if (ch == 'S' || ch == 's') {
                grid[r][c] = Cell(CellType::START, 1);
                startNode = Node(r, c);
                hasStart = true;
            } else if (ch == 'G' || ch == 'g' || ch == 'E' || ch == 'e') {
                grid[r][c] = Cell(CellType::GOAL, 1);
                goalNode = Node(r, c);
                hasGoal = true;
            } else if (ch >= '2' && ch <= '9') {
                grid[r][c] = Cell(CellType::WEIGHT, ch - '0');
            } else if (ch == 'W' || ch == 'w') {
                grid[r][c] = Cell(CellType::WEIGHT, 5);
            } else {
                grid[r][c] = Cell(CellType::EMPTY, 1);
            }
        }
    }

    // Safety fallback if the text file forgot to mark S or G.
    if (!hasStart) {
        startNode = Node(1, 1);
        if (inBounds(startNode.row, startNode.col)) {
            grid[startNode.row][startNode.col] = Cell(CellType::START, 1);
        }
    }
    if (!hasGoal) {
        goalNode = Node(rows - 2, cols - 2);
        if (inBounds(goalNode.row, goalNode.col)) {
            grid[goalNode.row][goalNode.col] = Cell(CellType::GOAL, 1);
        }
    }

    resetVisualization();
    return true;
}

// ------------------------------------------------------------
// loadNextMazeFile
// ------------------------------------------------------------
// Cycles through sample maze files.
void ofApp::loadNextMazeFile() {
    if (mazeFiles.empty()) {
        return;
    }
    currentMazeFileIndex = (currentMazeFileIndex + 1) % mazeFiles.size();
    loadMazeFromFile(mazeFiles[currentMazeFileIndex]);
}

// ------------------------------------------------------------
// generateRandomMaze
// ------------------------------------------------------------
// Generates a maze using recursive backtracking.
// This algorithm starts with all walls and carves passages two cells at a time.
// A stack is used instead of recursive function calls to make the data structure
// explicit for the report and to avoid recursion-depth problems.
void ofApp::generateRandomMaze(int desiredRows, int desiredCols) {
    // Maze generation works best with odd sizes.
    rows = (desiredRows % 2 == 0) ? desiredRows + 1 : desiredRows;
    cols = (desiredCols % 2 == 0) ? desiredCols + 1 : desiredCols;

    grid.assign(rows, vector<Cell>(cols, Cell(CellType::WALL, 1)));

    vector<vector<bool>> carved(rows, vector<bool>(cols, false));
    stack<Node> st;

    Node start(1, 1);
    grid[start.row][start.col] = Cell(CellType::EMPTY, 1);
    carved[start.row][start.col] = true;
    st.push(start);

    const int dr[4] = {-2, 2, 0, 0};
    const int dc[4] = {0, 0, -2, 2};

    while (!st.empty()) {
        Node current = st.top();
        vector<int> dirs = {0, 1, 2, 3};
        static std::mt19937 rng(std::random_device{}());
        std::shuffle(dirs.begin(), dirs.end(), rng);

        bool moved = false;
        for (int dir : dirs) {
            int nr = current.row + dr[dir];
            int nc = current.col + dc[dir];

            // Continue carving only if the target cell is inside the border.
            if (nr > 0 && nr < rows - 1 && nc > 0 && nc < cols - 1 && !carved[nr][nc]) {
                int wallR = current.row + dr[dir] / 2;
                int wallC = current.col + dc[dir] / 2;

                grid[wallR][wallC] = Cell(CellType::EMPTY, 1);
                grid[nr][nc] = Cell(CellType::EMPTY, 1);
                carved[nr][nc] = true;

                st.push(Node(nr, nc));
                moved = true;
                break;
            }
        }

        // If there is no unvisited neighbor, backtrack.
        if (!moved) {
            st.pop();
        }
    }

    startNode = Node(1, 1);
    goalNode = Node(rows - 2, cols - 2);
    grid[startNode.row][startNode.col] = Cell(CellType::START, 1);
    grid[goalNode.row][goalNode.col] = Cell(CellType::GOAL, 1);

    addRandomWeights(0.14f);
}

// ------------------------------------------------------------
// addRandomWeights
// ------------------------------------------------------------
// Adds weighted cells to empty passages. Weighted cells make Dijkstra and A*
// meaningfully different from BFS because the best path is not always the
// path with the smallest number of steps.
void ofApp::addRandomWeights(float probability) {
    for (int r = 1; r < rows - 1; ++r) {
        for (int c = 1; c < cols - 1; ++c) {
            if (grid[r][c].type == CellType::EMPTY && ofRandom(1.0f) < probability) {
                int randomCost = (ofRandom(1.0f) < 0.5f) ? 3 : 5;
                grid[r][c] = Cell(CellType::WEIGHT, randomCost);
            }
        }
    }
}

// ------------------------------------------------------------
// clearMazeToEmpty
// ------------------------------------------------------------
// Builds an empty bordered grid. Useful when the user wants to design a custom
// maze with the mouse.
void ofApp::clearMazeToEmpty(int newRows, int newCols) {
    rows = newRows;
    cols = newCols;
    grid.assign(rows, vector<Cell>(cols, Cell(CellType::EMPTY, 1)));

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (r == 0 || c == 0 || r == rows - 1 || c == cols - 1) {
                grid[r][c] = Cell(CellType::WALL, 1);
            }
        }
    }

    startNode = Node(1, 1);
    goalNode = Node(rows - 2, cols - 2);
    grid[startNode.row][startNode.col] = Cell(CellType::START, 1);
    grid[goalNode.row][goalNode.col] = Cell(CellType::GOAL, 1);
}

// ------------------------------------------------------------
// drawMaze
// ------------------------------------------------------------
// Draws every cell of the maze with borders.
void ofApp::drawMaze() {
    ofPushStyle();

    // Title above the maze.
    ofSetColor(235);
    ofDrawBitmapString("Maze Pathfinder Lab", mazeX, 18);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float x = mazeX + c * cellSize;
            float y = mazeY + r * cellSize;
            drawCell(r, c, x, y, cellSize);
        }
    }

    // Border around the entire maze.
    ofNoFill();
    ofSetColor(210);
    ofDrawRectangle(mazeX, mazeY, cols * cellSize, rows * cellSize);
    ofFill();

    ofPopStyle();
}

// ------------------------------------------------------------
// drawCell
// ------------------------------------------------------------
// Draws one square. Weighted cells show their cost number.
void ofApp::drawCell(int r, int c, float x, float y, float size) {
    ofColor color = getCellColor(r, c);

    ofSetColor(color);
    ofDrawRectangle(x, y, size, size);

    // Small grid line makes the maze easier to read.
    ofNoFill();
    ofSetColor(50, 53, 60);
    ofDrawRectangle(x, y, size, size);
    ofFill();

    // Draw cost number for weighted cells.
    if (grid[r][c].type == CellType::WEIGHT && size >= 13.0f) {
        ofSetColor(255);
        ofDrawBitmapString(ofToString(grid[r][c].cost), x + size * 0.35f, y + size * 0.68f);
    }
}

// ------------------------------------------------------------
// getCellColor
// ------------------------------------------------------------
// Chooses color by combining static cell type and current animation state.
ofColor ofApp::getCellColor(int r, int c) const {
    if (grid[r][c].type == CellType::WALL) {
        return ofColor(38, 41, 48);
    }

    if (visiblePath[r][c]) {
        return ofColor(255, 210, 60);
    }

    if (grid[r][c].type == CellType::START) {
        return ofColor(80, 220, 120);
    }

    if (grid[r][c].type == CellType::GOAL) {
        return ofColor(255, 90, 90);
    }

    if (visibleVisited[r][c]) {
        return ofColor(80, 145, 240);
    }

    if (grid[r][c].type == CellType::WEIGHT) {
        return ofColor(145, 95, 210);
    }

    return ofColor(225, 228, 235);
}

// ------------------------------------------------------------
// drawSidePanel
// ------------------------------------------------------------
// Draws algorithm statistics, current mode, keyboard guide, and legend.
void ofApp::drawSidePanel() {
    ofPushStyle();

    float x = panelX;
    float y = 40.0f;
    float line = 20.0f;

    ofSetColor(255);
    ofDrawBitmapString("[ Project Summary ]", x, y);
    y += line;
    ofSetColor(220);
    ofDrawBitmapString("Interactive pathfinding visualizer", x, y); y += line;
    ofDrawBitmapString("Grid: " + ofToString(rows) + " x " + ofToString(cols), x, y); y += line;
    ofDrawBitmapString("Edit Mode: " + getEditModeText(), x, y); y += line;
    ofDrawBitmapString("Speed: " + ofToString(cellsPerFrame) + " cells/frame", x, y); y += line;
    ofDrawBitmapString(string("Status: ") + (paused ? "Paused" : "Running"), x, y); y += line * 1.5f;

    ofSetColor(255);
    ofDrawBitmapString("[ Latest Algorithm Result ]", x, y);
    y += line;

    if (lastResult.algorithmName.empty()) {
        ofSetColor(220);
        ofDrawBitmapString("Press 1, 2, 3, or 4 to start.", x, y); y += line;
    } else {
        ofSetColor(235);
        ofDrawBitmapString("Algorithm : " + lastResult.algorithmName, x, y); y += line;
        ofDrawBitmapString("Found     : " + string(lastResult.found ? "YES" : "NO"), x, y); y += line;
        ofDrawBitmapString("Visited   : " + ofToString(lastResult.visitedCount), x, y); y += line;
        ofDrawBitmapString("Path Len  : " + ofToString((int)lastResult.path.size()), x, y); y += line;
        ofDrawBitmapString("Path Cost : " + ofToString(lastResult.totalCost), x, y); y += line;
        ofDrawBitmapString("Time      : " + ofToString(lastResult.elapsedMs, 4) + " ms", x, y); y += line;

        int totalReveal = (int)lastResult.visitOrder.size() + (int)lastResult.path.size();
        int doneReveal = min(animationIndex, (int)lastResult.visitOrder.size()) + min(pathAnimationIndex, (int)lastResult.path.size());
        ofDrawBitmapString("Animation : " + ofToString(doneReveal) + " / " + ofToString(totalReveal), x, y); y += line;
    }

    y += line * 0.7f;
    drawLegend(x, y);
    y += 150.0f;

    if (showHelp) {
        ofSetColor(255);
        ofDrawBitmapString("[ Controls ]", x, y); y += line;
        ofSetColor(220);
        ofDrawBitmapString("1 : Run BFS", x, y); y += line;
        ofDrawBitmapString("2 : Run DFS", x, y); y += line;
        ofDrawBitmapString("3 : Run Dijkstra", x, y); y += line;
        ofDrawBitmapString("4 : Run A*", x, y); y += line;
        ofDrawBitmapString("M : Change mouse edit mode", x, y); y += line;
        ofDrawBitmapString("L : Load next sample maze", x, y); y += line;
        ofDrawBitmapString("G : Generate random maze", x, y); y += line;
        ofDrawBitmapString("C : Clear to empty grid", x, y); y += line;
        ofDrawBitmapString("R : Reset visualization", x, y); y += line;
        ofDrawBitmapString("P : Pause / resume animation", x, y); y += line;
        ofDrawBitmapString("+/- : Animation speed", x, y); y += line;
        ofDrawBitmapString("H : Hide/show guide", x, y); y += line;
        ofDrawBitmapString("Mouse click/drag : edit cell", x, y); y += line;
    }

    ofPopStyle();
}

// ------------------------------------------------------------
// drawLegend
// ------------------------------------------------------------
// Small legend for the colors used in the maze.
void ofApp::drawLegend(float x, float y) {
    struct LegendItem {
        string name;
        ofColor color;
    };

    vector<LegendItem> items = {
        {"Wall", ofColor(38, 41, 48)},
        {"Empty", ofColor(225, 228, 235)},
        {"Visited", ofColor(80, 145, 240)},
        {"Final Path", ofColor(255, 210, 60)},
        {"Weighted", ofColor(145, 95, 210)},
        {"Start", ofColor(80, 220, 120)},
        {"Goal", ofColor(255, 90, 90)}
    };

    ofSetColor(255);
    ofDrawBitmapString("[ Legend ]", x, y);
    y += 18.0f;

    for (const LegendItem& item : items) {
        ofSetColor(item.color);
        ofDrawRectangle(x, y - 11.0f, 14.0f, 14.0f);
        ofSetColor(230);
        ofDrawBitmapString(item.name, x + 22.0f, y);
        y += 18.0f;
    }
}

// ------------------------------------------------------------
// getEditModeText
// ------------------------------------------------------------
// Converts EditMode enum into readable text.
string ofApp::getEditModeText() const {
    switch (editMode) {
        case EditMode::WALL_EDIT: return "Wall Toggle";
        case EditMode::START_EDIT: return "Move Start";
        case EditMode::GOAL_EDIT: return "Move Goal";
        case EditMode::WEIGHT_EDIT: return "Weight Toggle";
    }
    return "Unknown";
}

// ------------------------------------------------------------
// runBFS
// ------------------------------------------------------------
// Breadth-First Search.
// BFS uses queue and explores nodes in increasing number of steps.
// In an unweighted maze, BFS gives the shortest path by number of cells.
SearchResult ofApp::runBFS() {
    SearchResult result;
    result.algorithmName = "BFS (Queue)";

    auto startTime = chrono::high_resolution_clock::now();

    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<Node>> parent(rows, vector<Node>(cols, Node(-1, -1)));
    queue<Node> q;

    q.push(startNode);
    visited[startNode.row][startNode.col] = true;

    while (!q.empty()) {
        Node current = q.front();
        q.pop();

        result.visitOrder.push_back(current);

        if (current == goalNode) {
            result.found = true;
            break;
        }

        vector<Node> neighbors = getNeighbors(current);
        for (const Node& next : neighbors) {
            if (!visited[next.row][next.col]) {
                visited[next.row][next.col] = true;
                parent[next.row][next.col] = current;
                q.push(next);
            }
        }
    }

    if (result.found) {
        result.path = reconstructPath(parent, goalNode);
        result.totalCost = computePathCost(result.path);
    }

    auto endTime = chrono::high_resolution_clock::now();
    result.elapsedMs = chrono::duration<double, std::milli>(endTime - startTime).count();
    result.visitedCount = (int)result.visitOrder.size();
    return result;
}

// ------------------------------------------------------------
// runDFS
// ------------------------------------------------------------
// Depth-First Search.
// DFS uses stack and goes as deep as possible before backtracking.
// It can find a path, but the path is not guaranteed to be shortest.
SearchResult ofApp::runDFS() {
    SearchResult result;
    result.algorithmName = "DFS (Stack)";

    auto startTime = chrono::high_resolution_clock::now();

    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<Node>> parent(rows, vector<Node>(cols, Node(-1, -1)));
    stack<Node> st;

    st.push(startNode);
    visited[startNode.row][startNode.col] = true;

    while (!st.empty()) {
        Node current = st.top();
        st.pop();

        result.visitOrder.push_back(current);

        if (current == goalNode) {
            result.found = true;
            break;
        }

        vector<Node> neighbors = getNeighbors(current);
        // Reverse order only to make DFS path visually different from BFS.
        reverse(neighbors.begin(), neighbors.end());

        for (const Node& next : neighbors) {
            if (!visited[next.row][next.col]) {
                visited[next.row][next.col] = true;
                parent[next.row][next.col] = current;
                st.push(next);
            }
        }
    }

    if (result.found) {
        result.path = reconstructPath(parent, goalNode);
        result.totalCost = computePathCost(result.path);
    }

    auto endTime = chrono::high_resolution_clock::now();
    result.elapsedMs = chrono::duration<double, std::milli>(endTime - startTime).count();
    result.visitedCount = (int)result.visitOrder.size();
    return result;
}

// ------------------------------------------------------------
// runDijkstra
// ------------------------------------------------------------
// Dijkstra's algorithm.
// It uses priority_queue to always expand the lowest-cost node first.
// Unlike BFS, it correctly handles weighted cells.
SearchResult ofApp::runDijkstra() {
    SearchResult result;
    result.algorithmName = "Dijkstra (Priority Queue)";

    auto startTime = chrono::high_resolution_clock::now();

    const int INF = numeric_limits<int>::max() / 4;
    vector<vector<int>> dist(rows, vector<int>(cols, INF));
    vector<vector<bool>> closed(rows, vector<bool>(cols, false));
    vector<vector<Node>> parent(rows, vector<Node>(cols, Node(-1, -1)));
    priority_queue<PriorityState, vector<PriorityState>, PriorityCompare> pq;

    dist[startNode.row][startNode.col] = 0;
    pq.push({startNode, 0, 0});

    while (!pq.empty()) {
        PriorityState state = pq.top();
        pq.pop();

        Node current = state.node;
        if (closed[current.row][current.col]) {
            continue;
        }

        closed[current.row][current.col] = true;
        result.visitOrder.push_back(current);

        if (current == goalNode) {
            result.found = true;
            break;
        }

        for (const Node& next : getNeighbors(current)) {
            int newCost = dist[current.row][current.col] + getMoveCost(next.row, next.col);
            if (newCost < dist[next.row][next.col]) {
                dist[next.row][next.col] = newCost;
                parent[next.row][next.col] = current;
                pq.push({next, newCost, newCost});
            }
        }
    }

    if (result.found) {
        result.path = reconstructPath(parent, goalNode);
        result.totalCost = dist[goalNode.row][goalNode.col];
    }

    auto endTime = chrono::high_resolution_clock::now();
    result.elapsedMs = chrono::duration<double, std::milli>(endTime - startTime).count();
    result.visitedCount = (int)result.visitOrder.size();
    return result;
}

// ------------------------------------------------------------
// runAStar
// ------------------------------------------------------------
// A* pathfinding.
// A* uses f(n) = g(n) + h(n).
// g(n) is the known cost from the start, and h(n) is Manhattan distance
// to the goal. With this heuristic, A* usually visits fewer nodes than
// Dijkstra while still finding an optimal path for this grid setting.
SearchResult ofApp::runAStar() {
    SearchResult result;
    result.algorithmName = "A* (g + Manhattan h)";

    auto startTime = chrono::high_resolution_clock::now();

    const int INF = numeric_limits<int>::max() / 4;
    vector<vector<int>> gCost(rows, vector<int>(cols, INF));
    vector<vector<bool>> closed(rows, vector<bool>(cols, false));
    vector<vector<Node>> parent(rows, vector<Node>(cols, Node(-1, -1)));
    priority_queue<PriorityState, vector<PriorityState>, PriorityCompare> openSet;

    gCost[startNode.row][startNode.col] = 0;
    int startPriority = heuristicManhattan(startNode, goalNode);
    openSet.push({startNode, startPriority, 0});

    while (!openSet.empty()) {
        PriorityState state = openSet.top();
        openSet.pop();

        Node current = state.node;
        if (closed[current.row][current.col]) {
            continue;
        }

        closed[current.row][current.col] = true;
        result.visitOrder.push_back(current);

        if (current == goalNode) {
            result.found = true;
            break;
        }

        for (const Node& next : getNeighbors(current)) {
            int tentativeG = gCost[current.row][current.col] + getMoveCost(next.row, next.col);
            if (tentativeG < gCost[next.row][next.col]) {
                gCost[next.row][next.col] = tentativeG;
                parent[next.row][next.col] = current;
                int fCost = tentativeG + heuristicManhattan(next, goalNode);
                openSet.push({next, fCost, tentativeG});
            }
        }
    }

    if (result.found) {
        result.path = reconstructPath(parent, goalNode);
        result.totalCost = gCost[goalNode.row][goalNode.col];
    }

    auto endTime = chrono::high_resolution_clock::now();
    result.elapsedMs = chrono::duration<double, std::milli>(endTime - startTime).count();
    result.visitedCount = (int)result.visitOrder.size();
    return result;
}

// ------------------------------------------------------------
// getNeighbors
// ------------------------------------------------------------
// Returns four-directional walkable neighbors.
// The order is up, right, down, left so that results are deterministic.
vector<Node> ofApp::getNeighbors(const Node& node) const {
    vector<Node> neighbors;
    const int dr[4] = {-1, 0, 1, 0};
    const int dc[4] = {0, 1, 0, -1};

    for (int i = 0; i < 4; ++i) {
        int nr = node.row + dr[i];
        int nc = node.col + dc[i];
        if (isWalkable(nr, nc)) {
            neighbors.push_back(Node(nr, nc));
        }
    }

    return neighbors;
}

bool ofApp::inBounds(int r, int c) const {
    return r >= 0 && r < rows && c >= 0 && c < cols;
}

bool ofApp::isWalkable(int r, int c) const {
    return inBounds(r, c) && grid[r][c].type != CellType::WALL;
}

int ofApp::getMoveCost(int r, int c) const {
    if (!inBounds(r, c)) {
        return 999999;
    }
    return max(1, grid[r][c].cost);
}

int ofApp::heuristicManhattan(const Node& a, const Node& b) const {
    return abs(a.row - b.row) + abs(a.col - b.col);
}

// ------------------------------------------------------------
// reconstructPath
// ------------------------------------------------------------
// Rebuilds the final path by following parent links backward from the goal.
vector<Node> ofApp::reconstructPath(const vector<vector<Node>>& parent, const Node& endNode) const {
    vector<Node> path;
    Node current = endNode;

    while (inBounds(current.row, current.col)) {
        path.push_back(current);
        if (current == startNode) {
            break;
        }
        current = parent[current.row][current.col];
    }

    reverse(path.begin(), path.end());
    return path;
}

// ------------------------------------------------------------
// computePathCost
// ------------------------------------------------------------
// Sums movement costs along the path. The start cell cost is excluded because
// the algorithm pays cost when it enters the next cell.
int ofApp::computePathCost(const vector<Node>& path) const {
    int cost = 0;
    for (int i = 1; i < (int)path.size(); ++i) {
        cost += getMoveCost(path[i].row, path[i].col);
    }
    return cost;
}

// ------------------------------------------------------------
// resetVisualization
// ------------------------------------------------------------
// Clears animation state but keeps the maze itself.
void ofApp::resetVisualization() {
    resizeVisualArrays();
    lastResult = SearchResult();
    animationIndex = 0;
    pathAnimationIndex = 0;
    paused = false;
}

void ofApp::prepareAnimation(const SearchResult& result) {
    resizeVisualArrays();
    lastResult = result;
    animationIndex = 0;
    pathAnimationIndex = 0;
    paused = false;
}

void ofApp::resizeVisualArrays() {
    visibleVisited.assign(rows, vector<bool>(cols, false));
    visiblePath.assign(rows, vector<bool>(cols, false));
}

// ------------------------------------------------------------
// runAndStoreResult
// ------------------------------------------------------------
// Dispatches to the selected algorithm and prepares animation.
void ofApp::runAndStoreResult(int algorithmNumber) {
    SearchResult result;

    if (algorithmNumber == 1) {
        result = runBFS();
    } else if (algorithmNumber == 2) {
        result = runDFS();
    } else if (algorithmNumber == 3) {
        result = runDijkstra();
    } else if (algorithmNumber == 4) {
        result = runAStar();
    }

    prepareAnimation(result);
}

// ------------------------------------------------------------
// cycleEditMode
// ------------------------------------------------------------
// Rotates mouse editing behavior.
void ofApp::cycleEditMode() {
    if (editMode == EditMode::WALL_EDIT) {
        editMode = EditMode::START_EDIT;
    } else if (editMode == EditMode::START_EDIT) {
        editMode = EditMode::GOAL_EDIT;
    } else if (editMode == EditMode::GOAL_EDIT) {
        editMode = EditMode::WEIGHT_EDIT;
    } else {
        editMode = EditMode::WALL_EDIT;
    }
}

// ------------------------------------------------------------
// screenToCell
// ------------------------------------------------------------
// Converts mouse screen position into grid coordinate.
bool ofApp::screenToCell(int x, int y, int& outRow, int& outCol) const {
    if (cellSize <= 0.0f) {
        return false;
    }

    int c = (int)((x - mazeX) / cellSize);
    int r = (int)((y - mazeY) / cellSize);

    if (!inBounds(r, c)) {
        return false;
    }

    outRow = r;
    outCol = c;
    return true;
}

// ------------------------------------------------------------
// applyEditToCell
// ------------------------------------------------------------
// Modifies the selected cell depending on EditMode.
void ofApp::applyEditToCell(int r, int c) {
    if (!inBounds(r, c)) {
        return;
    }

    // Border walls are kept fixed so the maze remains visually clean.
    if (r == 0 || c == 0 || r == rows - 1 || c == cols - 1) {
        return;
    }

    if (editMode == EditMode::WALL_EDIT) {
        if (Node(r, c) == startNode || Node(r, c) == goalNode) {
            return;
        }
        if (grid[r][c].type == CellType::WALL) {
            grid[r][c] = Cell(CellType::EMPTY, 1);
        } else {
            grid[r][c] = Cell(CellType::WALL, 1);
        }
    } else if (editMode == EditMode::START_EDIT) {
        if (Node(r, c) == goalNode || grid[r][c].type == CellType::WALL) {
            return;
        }
        grid[startNode.row][startNode.col] = Cell(CellType::EMPTY, 1);
        startNode = Node(r, c);
        grid[r][c] = Cell(CellType::START, 1);
    } else if (editMode == EditMode::GOAL_EDIT) {
        if (Node(r, c) == startNode || grid[r][c].type == CellType::WALL) {
            return;
        }
        grid[goalNode.row][goalNode.col] = Cell(CellType::EMPTY, 1);
        goalNode = Node(r, c);
        grid[r][c] = Cell(CellType::GOAL, 1);
    } else if (editMode == EditMode::WEIGHT_EDIT) {
        if (Node(r, c) == startNode || Node(r, c) == goalNode || grid[r][c].type == CellType::WALL) {
            return;
        }
        if (grid[r][c].type == CellType::WEIGHT) {
            grid[r][c] = Cell(CellType::EMPTY, 1);
        } else {
            grid[r][c] = Cell(CellType::WEIGHT, 5);
        }
    }

    // Editing changes the maze, so old algorithm visualization is cleared.
    resetVisualization();
}
