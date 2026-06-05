#include "ofApp.h"

// ============================================================
// ofApp.cpp
// ------------------------------------------------------------
// Maze Pathfinder Lab의 실제 동작을 구현하는 파일입니다.
// 네 가지 탐색 알고리즘(BFS, DFS, Dijkstra, A*)과 화면 시각화,
// 마우스 편집, 랜덤 미로 생성, 실행 통계 계산을 모두 포함합니다.
// ============================================================

namespace {
    // 화면 색상은 한곳에 모아 두면 나중에 디자인을 수정하기 쉽습니다.
    // 색상 자체는 알고리즘 로직에 영향을 주지 않고, 시각적 구분만 담당합니다.
    const ofColor COLOR_BACKGROUND(246, 248, 252);
    const ofColor COLOR_GRID_LINE(220, 224, 230);
    const ofColor COLOR_EMPTY(255, 255, 255);
    const ofColor COLOR_WALL(32, 38, 48);
    const ofColor COLOR_START(52, 168, 83);
    const ofColor COLOR_GOAL(234, 67, 53);
    const ofColor COLOR_WEIGHT(252, 214, 112);
    const ofColor COLOR_VISITED(111, 170, 255, 160);
    const ofColor COLOR_PATH(255, 122, 89, 220);
    const ofColor COLOR_TEXT(30, 36, 46);
    const ofColor COLOR_MUTED_TEXT(90, 99, 115);

    // 도착할 수 없는 상태를 나타내는 충분히 큰 값입니다.
    // Dijkstra와 A*의 dist 배열 초기값으로 사용됩니다.
    constexpr int INF = std::numeric_limits<int>::max() / 4;
}

// ------------------------------------------------------------
// setup()
// ------------------------------------------------------------
// 프로그램 시작 시 한 번만 호출됩니다.
// 샘플 미로 목록, 기본 미로, 화면 배치, 애니메이션 배열을 초기화합니다.
// ------------------------------------------------------------
void ofApp::setup() {
    ofSetWindowTitle("Maze Pathfinder Lab - BFS / DFS / Dijkstra / A*");
    ofSetFrameRate(60);
    ofBackground(COLOR_BACKGROUND);

    setupMazeFileList();

    // 첫 번째 샘플 미로를 읽습니다.
    // 파일이 없거나 형식이 잘못된 경우에도 프로그램이 멈추지 않도록 빈 미로를 생성합니다.
    if (!mazeFiles.empty() && !loadMazeFromFile(mazeFiles[currentMazeIndex])) {
        createEmptyMaze(21, 31);
    }

    updateLayout();
    initializeVisibleArrays();
}

// ------------------------------------------------------------
// update()
// ------------------------------------------------------------
// 매 프레임 호출됩니다.
// 알고리즘 계산은 keyPressed()에서 한 번에 끝내고, update()에서는 계산된
// visitOrder/finalPath를 일정 속도로 화면에 표시하는 애니메이션만 처리합니다.
// ------------------------------------------------------------
void ofApp::update() {
    if (!isAnimating || isPaused) {
        return;
    }

    const float now = ofGetElapsedTimef();

    // animationInterval보다 시간이 적게 지났다면 이번 프레임에서는 새 칸을 표시하지 않습니다.
    // 이렇게 하면 컴퓨터 성능이나 FPS 차이와 무관하게 비슷한 애니메이션 속도를 유지할 수 있습니다.
    if (now - lastAnimationStepTime < animationInterval) {
        return;
    }

    lastAnimationStepTime = now;

    // 1단계: 방문 순서를 먼저 표시합니다.
    // BFS/DFS/Dijkstra/A*가 공간을 어떤 순서로 확장하는지 보여주는 부분입니다.
    if (visitedAnimationIndex < visitOrder.size()) {
        const Node node = visitOrder[visitedAnimationIndex];
        if (isInside(node.row, node.col)) {
            visibleVisited[node.row][node.col] = true;
        }
        ++visitedAnimationIndex;
        return;
    }

    // 2단계: 모든 방문 칸이 표시된 후 최종 경로를 표시합니다.
    // 최종 경로는 parent 배열로 복원된 실제 해답 경로입니다.
    if (pathAnimationIndex < finalPath.size()) {
        const Node node = finalPath[pathAnimationIndex];
        if (isInside(node.row, node.col)) {
            visiblePath[node.row][node.col] = true;
        }
        ++pathAnimationIndex;
        return;
    }

    // 방문 순서와 최종 경로를 모두 표시했으면 애니메이션을 종료합니다.
    isAnimating = false;
}

// ------------------------------------------------------------
// draw()
// ------------------------------------------------------------
// 매 프레임 화면을 다시 그립니다.
// 미로, 우측 통계 패널, 범례, 도움말을 분리된 함수로 나누어 가독성을 높였습니다.
// ------------------------------------------------------------
void ofApp::draw() {
    ofBackground(COLOR_BACKGROUND);

    drawMaze();
    drawSidePanel();
}

// ------------------------------------------------------------
// keyPressed()
// ------------------------------------------------------------
// 키보드 입력을 처리합니다.
// 1~4는 단일 알고리즘 실행, 5는 네 알고리즘 비교 실행, M/L/G/C/R/P/+/-/H는 편집 및 표시 옵션입니다.
// ------------------------------------------------------------
void ofApp::keyPressed(int key) {
    switch (key) {
        case '1':
            comparisonResults.clear();
            runBFS();
            break;
        case '2':
            comparisonResults.clear();
            runDFS();
            break;
        case '3':
            comparisonResults.clear();
            runDijkstra();
            break;
        case '4':
            comparisonResults.clear();
            runAStar();
            break;
        case '5':
            runAllAlgorithmsForComparison();
            break;
        case 'm':
        case 'M': {
            // EditMode를 0~3 정수로 바꾼 뒤 하나 증가시키고 다시 enum으로 변환합니다.
            // 네 가지 모드를 순환시키기 위해 나머지 연산을 사용합니다.
            const int nextMode = (static_cast<int>(editMode) + 1) % 4;
            editMode = static_cast<EditMode>(nextMode);
            break;
        }
        case 'l':
        case 'L':
            if (!mazeFiles.empty()) {
                currentMazeIndex = (currentMazeIndex + 1) % static_cast<int>(mazeFiles.size());
                loadMazeFromFile(mazeFiles[currentMazeIndex]);
            }
            break;
        case 'g':
        case 'G':
            generateRandomMaze();
            break;
        case 'c':
        case 'C':
            createEmptyMaze(21, 31);
            break;
        case 'r':
        case 'R':
            resetSearchState();
            break;
        case 'p':
        case 'P':
            isPaused = !isPaused;
            break;
        case '+':
        case '=':
            // interval이 작을수록 더 빠르게 표시됩니다.
            // 너무 작아지면 눈으로 구분하기 어려우므로 하한을 둡니다.
            animationInterval = std::max(0.003f, animationInterval * 0.75f);
            break;
        case '-':
        case '_':
            // interval이 클수록 더 느리게 표시됩니다.
            // 지나치게 느려지는 것을 막기 위해 상한을 둡니다.
            animationInterval = std::min(0.20f, animationInterval * 1.35f);
            break;
        case 'h':
        case 'H':
            showHelp = !showHelp;
            break;
        default:
            break;
    }
}

// ------------------------------------------------------------
// mousePressed(), mouseDragged()
// ------------------------------------------------------------
// 미로를 마우스로 편집합니다.
// 클릭 또는 드래그 좌표를 미로의 row/col로 변환한 뒤 현재 EditMode에 맞게 수정합니다.
// ------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    hasLastEditedCell = false;

    Node cell;
    if (screenToCell(x, y, cell)) {
        applyEditToCell(cell.row, cell.col);
        lastEditedCell = cell;
        hasLastEditedCell = true;
    }
}

void ofApp::mouseDragged(int x, int y, int button) {
    Node cell;
    if (!screenToCell(x, y, cell)) {
        return;
    }

    // 같은 칸에서 drag 이벤트가 여러 번 발생할 수 있습니다.
    // 벽/가중치 토글이 반복되어 원래 상태로 돌아가는 것을 막기 위해 중복 칸은 무시합니다.
    if (hasLastEditedCell && cell == lastEditedCell) {
        return;
    }

    applyEditToCell(cell.row, cell.col);
    lastEditedCell = cell;
    hasLastEditedCell = true;
}

// ------------------------------------------------------------
// windowResized()
// ------------------------------------------------------------
// 창 크기가 바뀌면 미로 한 칸의 픽셀 크기와 시작 위치를 다시 계산합니다.
// ------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    updateLayout();
}

// ------------------------------------------------------------
// setupMazeFileList()
// ------------------------------------------------------------
// bin/data 폴더에 들어갈 샘플 미로 파일명을 등록합니다.
// 파일명만 저장하는 이유는 OpenFrameworks의 ofBufferFromFile이 기본적으로
// data 폴더를 기준으로 파일을 찾기 때문입니다.
// ------------------------------------------------------------
void ofApp::setupMazeFileList() {
    mazeFiles = {
        "maze_easy.txt",
        "maze_weighted.txt",
        "maze_large.txt"
    };
    currentMazeIndex = 0;
}

// ------------------------------------------------------------
// loadMazeFromFile()
// ------------------------------------------------------------
// 텍스트 파일의 각 문자를 Cell로 변환하여 grid에 저장합니다.
// #은 벽, .은 빈 칸, S는 시작점, G는 도착점, 2~9/W는 가중치 칸입니다.
// ------------------------------------------------------------
bool ofApp::loadMazeFromFile(const std::string& fileName) {
    ofBuffer buffer = ofBufferFromFile(fileName);
    if (buffer.size() == 0) {
        ofLogWarning("MazePathfinderLab") << "Cannot read maze file: " << fileName;
        return false;
    }

    std::vector<std::string> lines;

    // 파일의 모든 줄을 순회하면서 빈 줄을 제외합니다.
    // 줄마다 길이가 다를 수 있으므로, 뒤에서 가장 긴 줄 길이에 맞추어 빈 칸을 보충합니다.
    for (const std::string& rawLine : buffer.getLines()) {
        std::string line = rawLine;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    if (lines.empty()) {
        return false;
    }

    rows = static_cast<int>(lines.size());
    cols = 0;

    // 가장 긴 줄의 길이를 cols로 사용합니다.
    // 이렇게 하면 파일에 일부 짧은 줄이 있어도 grid가 직사각형 형태를 유지합니다.
    for (const std::string& line : lines) {
        cols = std::max(cols, static_cast<int>(line.size()));
    }

    grid.assign(rows, std::vector<Cell>(cols, Cell{}));

    bool hasStart = false;
    bool hasGoal = false;

    // 각 문자 하나하나를 CellType과 cost로 변환합니다.
    // 이중 반복문은 모든 칸을 한 번씩 읽으므로 시간복잡도는 O(rows*cols)입니다.
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const char ch = (c < static_cast<int>(lines[r].size())) ? lines[r][c] : '.';
            Cell cell;

            if (ch == '#') {
                cell.type = CellType::Wall;
                cell.cost = 1;
            } else if (ch == 'S' || ch == 's') {
                cell.type = CellType::Start;
                cell.cost = 1;
                start = {r, c};
                hasStart = true;
            } else if (ch == 'G' || ch == 'g') {
                cell.type = CellType::Goal;
                cell.cost = 1;
                goal = {r, c};
                hasGoal = true;
            } else if (ch >= '2' && ch <= '9') {
                cell.type = CellType::Weight;
                cell.cost = ch - '0';
            } else if (ch == 'W' || ch == 'w') {
                cell.type = CellType::Weight;
                cell.cost = 5;
            } else {
                cell.type = CellType::Empty;
                cell.cost = 1;
            }

            grid[r][c] = cell;
        }
    }

    // 파일에 S 또는 G가 없을 경우 안전한 기본 위치를 지정합니다.
    // 제출 환경에서 잘못된 파일을 읽어도 프로그램이 바로 종료되지 않게 하기 위한 방어 코드입니다.
    if (!hasStart) {
        start = {1, 1};
        if (isInside(start.row, start.col)) {
            grid[start.row][start.col] = {CellType::Start, 1};
        }
    }
    if (!hasGoal) {
        goal = {std::max(1, rows - 2), std::max(1, cols - 2)};
        if (isInside(goal.row, goal.col)) {
            grid[goal.row][goal.col] = {CellType::Goal, 1};
        }
    }

    updateLayout();
    resetSearchState();
    return true;
}

// ------------------------------------------------------------
// createEmptyMaze()
// ------------------------------------------------------------
// 테두리만 벽으로 막힌 빈 미로를 생성합니다.
// 사용자가 마우스로 직접 벽과 가중치 칸을 배치하여 알고리즘을 실험할 수 있습니다.
// ------------------------------------------------------------
void ofApp::createEmptyMaze(int newRows, int newCols) {
    rows = std::max(5, newRows);
    cols = std::max(5, newCols);
    grid.assign(rows, std::vector<Cell>(cols, Cell{}));

    // 테두리를 벽으로 만드는 반복문입니다.
    // 미로 밖으로 탐색이 나가는 경우를 시각적으로 막고, 실험 환경을 명확히 합니다.
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const bool boundary = (r == 0 || c == 0 || r == rows - 1 || c == cols - 1);
            grid[r][c].type = boundary ? CellType::Wall : CellType::Empty;
            grid[r][c].cost = 1;
        }
    }

    start = {1, 1};
    goal = {rows - 2, cols - 2};
    grid[start.row][start.col] = {CellType::Start, 1};
    grid[goal.row][goal.col] = {CellType::Goal, 1};

    updateLayout();
    resetSearchState();
}

// ------------------------------------------------------------
// updateLayout()
// ------------------------------------------------------------
// 현재 창 크기와 미로 크기를 기준으로 셀 크기와 출력 위치를 계산합니다.
// 우측 패널 공간을 제외한 영역 안에 미로가 최대한 크게 들어가도록 합니다.
// ------------------------------------------------------------
void ofApp::updateLayout() {
    if (rows <= 0 || cols <= 0) {
        return;
    }

    const int padding = 28;
    const int availableWidth = std::max(200, ofGetWidth() - sidePanelWidth - padding * 3);
    const int availableHeight = std::max(200, ofGetHeight() - padding * 2);

    const int sizeByWidth = availableWidth / cols;
    const int sizeByHeight = availableHeight / rows;

    cellSize = std::max(8, std::min(sizeByWidth, sizeByHeight));
    offsetX = padding;
    offsetY = padding;
}

// ------------------------------------------------------------
// resetSearchState()
// ------------------------------------------------------------
// 탐색 결과와 애니메이션 상태를 초기화합니다.
// 미로를 편집하거나 새 파일을 로드하면 기존 경로가 더 이상 유효하지 않으므로 반드시 호출합니다.
// ------------------------------------------------------------
void ofApp::resetSearchState() {
    visitOrder.clear();
    finalPath.clear();
    visitedAnimationIndex = 0;
    pathAnimationIndex = 0;
    isAnimating = false;
    isPaused = false;
    currentAlgorithmName = "None";
    lastStats = SearchStats{};
    comparisonResults.clear();
    initializeVisibleArrays();
}

// ------------------------------------------------------------
// initializeVisibleArrays()
// ------------------------------------------------------------
// 화면 표시용 bool 배열을 grid 크기에 맞추어 다시 생성합니다.
// 계산 결과와 표시 결과를 분리해 두면 애니메이션 구현이 단순해집니다.
// ------------------------------------------------------------
void ofApp::initializeVisibleArrays() {
    visibleVisited.assign(rows, std::vector<bool>(cols, false));
    visiblePath.assign(rows, std::vector<bool>(cols, false));
}

// ------------------------------------------------------------
// isInside(), isWalkable(), getMoveCost()
// ------------------------------------------------------------
// 네 알고리즘이 공통으로 사용하는 격자 검사 함수입니다.
// 이동 가능성 판단을 한곳에 모아 중복 코드를 줄였습니다.
// ------------------------------------------------------------
bool ofApp::isInside(int row, int col) const {
    return row >= 0 && row < rows && col >= 0 && col < cols;
}

bool ofApp::isWalkable(int row, int col) const {
    return isInside(row, col) && grid[row][col].type != CellType::Wall;
}

int ofApp::getMoveCost(int row, int col) const {
    if (!isInside(row, col)) {
        return INF;
    }
    return std::max(1, grid[row][col].cost);
}

// ------------------------------------------------------------
// manhattanDistance()
// ------------------------------------------------------------
// A*의 휴리스틱 함수입니다.
// 상하좌우 네 방향 이동만 허용하는 격자에서는 |dr|+|dc|가 자연스러운 거리 추정값입니다.
// ------------------------------------------------------------
int ofApp::manhattanDistance(const Node& a, const Node& b) const {
    return std::abs(a.row - b.row) + std::abs(a.col - b.col);
}

// ------------------------------------------------------------
// getNeighbors()
// ------------------------------------------------------------
// 현재 칸에서 상하좌우로 이동 가능한 이웃 칸을 반환합니다.
// 벽과 미로 범위 밖 좌표는 제외합니다.
// ------------------------------------------------------------
std::vector<Node> ofApp::getNeighbors(const Node& node) const {
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    std::vector<Node> neighbors;
    neighbors.reserve(4);

    // 네 방향을 하나씩 검사합니다.
    // 격자 미로에서 각 정점의 최대 차수는 4이므로, 이 반복문은 상수 시간으로 볼 수 있습니다.
    for (int i = 0; i < 4; ++i) {
        const int nr = node.row + dr[i];
        const int nc = node.col + dc[i];
        if (isWalkable(nr, nc)) {
            neighbors.push_back({nr, nc});
        }
    }

    return neighbors;
}

// ------------------------------------------------------------
// reconstructPath()
// ------------------------------------------------------------
// parent 배열을 따라 goal에서 start까지 거꾸로 이동한 뒤 reverse하여
// start -> goal 순서의 최종 경로를 만듭니다.
// ------------------------------------------------------------
std::vector<Node> ofApp::reconstructPath(const std::vector<std::vector<Node>>& parent) const {
    std::vector<Node> path;

    if (!isInside(goal.row, goal.col)) {
        return path;
    }

    Node cur = goal;

    // parent가 {-1,-1}이면 더 이상 이전 칸이 없다는 뜻입니다.
    // 정상적으로 goal에서 start까지 연결되어 있으면 마지막에 start를 만나게 됩니다.
    while (isInside(cur.row, cur.col)) {
        path.push_back(cur);
        if (cur == start) {
            break;
        }

        const Node next = parent[cur.row][cur.col];
        if (next.row == -1 && next.col == -1) {
            path.clear();
            return path;
        }
        cur = next;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

// ------------------------------------------------------------
// computePathCost()
// ------------------------------------------------------------
// 경로의 총 이동 비용을 계산합니다.
// 시작점에 서 있는 것은 이동이 아니므로 두 번째 칸부터 비용을 더합니다.
// ------------------------------------------------------------
int ofApp::computePathCost(const std::vector<Node>& path) const {
    if (path.empty()) {
        return 0;
    }

    int cost = 0;

    // i=1부터 시작하는 이유는 path[0]이 시작점이기 때문입니다.
    // 이동 비용은 "다음 칸으로 들어가는 비용"으로 정의했습니다.
    for (std::size_t i = 1; i < path.size(); ++i) {
        cost += getMoveCost(path[i].row, path[i].col);
    }

    return cost;
}

// ------------------------------------------------------------
// prepareAnimation()
// ------------------------------------------------------------
// 알고리즘 계산 결과를 화면 애니메이션 상태로 복사합니다.
// 알고리즘 로직과 렌더링 로직을 분리하기 위한 중간 단계입니다.
// ------------------------------------------------------------
void ofApp::prepareAnimation(const std::string& algorithmName,
                             const std::vector<Node>& visited,
                             const std::vector<Node>& path,
                             const SearchStats& stats) {
    currentAlgorithmName = algorithmName;
    visitOrder = visited;
    finalPath = path;
    lastStats = stats;

    visitedAnimationIndex = 0;
    pathAnimationIndex = 0;
    lastAnimationStepTime = ofGetElapsedTimef();
    isAnimating = true;
    isPaused = false;

    initializeVisibleArrays();
}

// ------------------------------------------------------------
// runBFS()
// ------------------------------------------------------------
// BFS(Breadth-First Search)는 queue를 사용하여 시작점에서 가까운 깊이의 노드부터 탐색합니다.
// 모든 간선 비용이 동일한 일반 미로에서는 최단 칸 수 경로를 보장합니다.
// 단, 가중치 칸이 있으면 이동 비용을 고려하지 않으므로 최소 비용 경로는 보장하지 않습니다.
// 시간복잡도: O(V+E), 공간복잡도: O(V)
// ------------------------------------------------------------
void ofApp::runBFS() {
    const auto t0 = std::chrono::high_resolution_clock::now();

    std::queue<Node> q;
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<Node>> parent(rows, std::vector<Node>(cols, {-1, -1}));
    std::vector<Node> visitedOrder;

    q.push(start);
    visited[start.row][start.col] = true;

    int edgeChecks = 0;
    int maxFrontierSize = static_cast<int>(q.size());
    bool found = false;

    // queue가 빌 때까지 탐색합니다.
    // 각 칸은 visited가 true가 된 뒤 다시 queue에 들어가지 않으므로 전체 시간은 O(V+E)입니다.
    while (!q.empty()) {
        Node cur = q.front();
        q.pop();
        visitedOrder.push_back(cur);

        if (cur == goal) {
            found = true;
            break;
        }

        // 현재 칸의 상하좌우 이웃을 확인합니다.
        // 아직 방문하지 않은 통과 가능 칸만 queue에 넣고 parent를 기록합니다.
        for (const Node& next : getNeighbors(cur)) {
            ++edgeChecks;
            if (!visited[next.row][next.col]) {
                visited[next.row][next.col] = true;
                parent[next.row][next.col] = cur;
                q.push(next);
                maxFrontierSize = std::max(maxFrontierSize, static_cast<int>(q.size()));
            }
        }
    }

    std::vector<Node> path = found ? reconstructPath(parent) : std::vector<Node>{};

    const auto t1 = std::chrono::high_resolution_clock::now();
    const double elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    SearchStats stats;
    stats.found = found;
    stats.visitedCount = static_cast<int>(visitedOrder.size());
    stats.pathLength = static_cast<int>(path.size());
    stats.pathCost = computePathCost(path);
    stats.edgeChecks = edgeChecks;
    stats.maxFrontierSize = maxFrontierSize;
    stats.auxiliaryCells = rows * cols * 2 + maxFrontierSize;
    stats.elapsedMs = elapsedMs;

    prepareAnimation("BFS", visitedOrder, path, stats);
}

// ------------------------------------------------------------
// runDFS()
// ------------------------------------------------------------
// DFS(Depth-First Search)는 stack을 사용하여 한 방향으로 최대한 깊게 들어간 뒤 되돌아옵니다.
// 경로를 찾을 수는 있지만, 최단 칸 수 또는 최소 비용 경로를 보장하지 않습니다.
// BFS와 비교하면 탐색 순서의 차이를 시각적으로 보여주기 좋습니다.
// 시간복잡도: O(V+E), 공간복잡도: O(V)
// ------------------------------------------------------------
void ofApp::runDFS() {
    const auto t0 = std::chrono::high_resolution_clock::now();

    std::stack<Node> st;
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<Node>> parent(rows, std::vector<Node>(cols, {-1, -1}));
    std::vector<Node> visitedOrder;

    st.push(start);
    visited[start.row][start.col] = true;

    int edgeChecks = 0;
    int maxFrontierSize = static_cast<int>(st.size());
    bool found = false;

    // stack의 LIFO 특성 때문에 가장 최근에 넣은 이웃을 먼저 탐색합니다.
    // 방문 처리를 push 시점에 수행하여 같은 칸이 stack에 중복으로 들어가는 것을 막습니다.
    while (!st.empty()) {
        Node cur = st.top();
        st.pop();
        visitedOrder.push_back(cur);

        if (cur == goal) {
            found = true;
            break;
        }

        std::vector<Node> neighbors = getNeighbors(cur);

        // DFS의 화면 흐름이 위/왼쪽으로만 치우치지 않도록 이웃 순서를 반전합니다.
        // 알고리즘의 본질은 stack 기반 깊이 우선 탐색이라는 점에서 변하지 않습니다.
        std::reverse(neighbors.begin(), neighbors.end());

        for (const Node& next : neighbors) {
            ++edgeChecks;
            if (!visited[next.row][next.col]) {
                visited[next.row][next.col] = true;
                parent[next.row][next.col] = cur;
                st.push(next);
                maxFrontierSize = std::max(maxFrontierSize, static_cast<int>(st.size()));
            }
        }
    }

    std::vector<Node> path = found ? reconstructPath(parent) : std::vector<Node>{};

    const auto t1 = std::chrono::high_resolution_clock::now();
    const double elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    SearchStats stats;
    stats.found = found;
    stats.visitedCount = static_cast<int>(visitedOrder.size());
    stats.pathLength = static_cast<int>(path.size());
    stats.pathCost = computePathCost(path);
    stats.edgeChecks = edgeChecks;
    stats.maxFrontierSize = maxFrontierSize;
    stats.auxiliaryCells = rows * cols * 2 + maxFrontierSize;
    stats.elapsedMs = elapsedMs;

    prepareAnimation("DFS", visitedOrder, path, stats);
}

// ------------------------------------------------------------
// runDijkstra()
// ------------------------------------------------------------
// Dijkstra 알고리즘은 priority_queue를 사용하여 현재까지의 누적 비용이 가장 작은 칸을 먼저 확장합니다.
// 모든 이동 비용이 0 이상일 때 시작점에서 각 칸까지의 최소 비용을 구할 수 있습니다.
// 가중치 셀이 있는 미로에서 BFS와 다른 결과가 나오는 핵심 알고리즘입니다.
// 시간복잡도: O((V+E)logV), 공간복잡도: O(V)
// ------------------------------------------------------------
void ofApp::runDijkstra() {
    const auto t0 = std::chrono::high_resolution_clock::now();

    std::priority_queue<PriorityState> pq;
    std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, INF));
    std::vector<std::vector<bool>> finalized(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<Node>> parent(rows, std::vector<Node>(cols, {-1, -1}));
    std::vector<Node> visitedOrder;

    dist[start.row][start.col] = 0;
    pq.push({start.row, start.col, 0, 0});

    int edgeChecks = 0;
    int maxFrontierSize = static_cast<int>(pq.size());
    bool found = false;

    // priority_queue에서 꺼낸 칸은 현재까지 알려진 비용이 가장 작은 후보입니다.
    // finalized 배열은 이미 확정된 칸을 다시 처리하지 않도록 막습니다.
    while (!pq.empty()) {
        PriorityState state = pq.top();
        pq.pop();

        if (finalized[state.row][state.col]) {
            continue;
        }

        finalized[state.row][state.col] = true;
        Node cur{state.row, state.col};
        visitedOrder.push_back(cur);

        if (cur == goal) {
            found = true;
            break;
        }

        // Relaxation 단계입니다.
        // 현재 칸을 거쳐 이웃으로 가는 비용이 기존 dist보다 작으면 dist와 parent를 갱신합니다.
        for (const Node& next : getNeighbors(cur)) {
            ++edgeChecks;
            const int newDist = dist[cur.row][cur.col] + getMoveCost(next.row, next.col);
            if (newDist < dist[next.row][next.col]) {
                dist[next.row][next.col] = newDist;
                parent[next.row][next.col] = cur;
                pq.push({next.row, next.col, newDist, newDist});
                maxFrontierSize = std::max(maxFrontierSize, static_cast<int>(pq.size()));
            }
        }
    }

    std::vector<Node> path = found ? reconstructPath(parent) : std::vector<Node>{};

    const auto t1 = std::chrono::high_resolution_clock::now();
    const double elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    SearchStats stats;
    stats.found = found;
    stats.visitedCount = static_cast<int>(visitedOrder.size());
    stats.pathLength = static_cast<int>(path.size());
    stats.pathCost = found ? dist[goal.row][goal.col] : 0;
    stats.edgeChecks = edgeChecks;
    stats.maxFrontierSize = maxFrontierSize;
    stats.auxiliaryCells = rows * cols * 3 + maxFrontierSize;
    stats.elapsedMs = elapsedMs;

    prepareAnimation("Dijkstra", visitedOrder, path, stats);
}

// ------------------------------------------------------------
// runAStar()
// ------------------------------------------------------------
// A*는 실제 누적 비용 g(n)과 목표까지의 추정 비용 h(n)을 더한 f(n)=g(n)+h(n)을 우선순위로 사용합니다.
// 여기서는 h(n)으로 Manhattan Distance를 사용합니다.
// 휴리스틱이 과대평가하지 않으면 Dijkstra보다 적은 노드를 방문하면서도 최적 경로를 찾을 수 있습니다.
// 시간복잡도: 최악 O((V+E)logV), 공간복잡도: O(V)
// ------------------------------------------------------------
void ofApp::runAStar() {
    const auto t0 = std::chrono::high_resolution_clock::now();

    std::priority_queue<PriorityState> pq;
    std::vector<std::vector<int>> gScore(rows, std::vector<int>(cols, INF));
    std::vector<std::vector<bool>> closed(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<Node>> parent(rows, std::vector<Node>(cols, {-1, -1}));
    std::vector<Node> visitedOrder;

    gScore[start.row][start.col] = 0;
    pq.push({start.row, start.col, manhattanDistance(start, goal), 0});

    int edgeChecks = 0;
    int maxFrontierSize = static_cast<int>(pq.size());
    bool found = false;

    // priority_queue는 f(n)=g(n)+h(n)이 작은 칸을 먼저 꺼냅니다.
    // h(n)이 목표 방향 정보를 제공하기 때문에 Dijkstra보다 탐색 범위가 줄어드는 경우가 많습니다.
    while (!pq.empty()) {
        PriorityState state = pq.top();
        pq.pop();

        if (closed[state.row][state.col]) {
            continue;
        }

        closed[state.row][state.col] = true;
        Node cur{state.row, state.col};
        visitedOrder.push_back(cur);

        if (cur == goal) {
            found = true;
            break;
        }

        // 현재 칸을 통해 이웃으로 이동하는 실제 비용 g를 계산합니다.
        // f는 이 실제 비용에 goal까지의 Manhattan Distance를 더한 값입니다.
        for (const Node& next : getNeighbors(cur)) {
            ++edgeChecks;
            const int tentativeG = gScore[cur.row][cur.col] + getMoveCost(next.row, next.col);
            if (tentativeG < gScore[next.row][next.col]) {
                gScore[next.row][next.col] = tentativeG;
                parent[next.row][next.col] = cur;

                const int fScore = tentativeG + manhattanDistance(next, goal);
                pq.push({next.row, next.col, fScore, tentativeG});
                maxFrontierSize = std::max(maxFrontierSize, static_cast<int>(pq.size()));
            }
        }
    }

    std::vector<Node> path = found ? reconstructPath(parent) : std::vector<Node>{};

    const auto t1 = std::chrono::high_resolution_clock::now();
    const double elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    SearchStats stats;
    stats.found = found;
    stats.visitedCount = static_cast<int>(visitedOrder.size());
    stats.pathLength = static_cast<int>(path.size());
    stats.pathCost = found ? gScore[goal.row][goal.col] : 0;
    stats.edgeChecks = edgeChecks;
    stats.maxFrontierSize = maxFrontierSize;
    stats.auxiliaryCells = rows * cols * 3 + maxFrontierSize;
    stats.elapsedMs = elapsedMs;

    prepareAnimation("A*", visitedOrder, path, stats);
}


// ------------------------------------------------------------
// runAllAlgorithmsForComparison()
// ------------------------------------------------------------
// 같은 미로, 같은 시작점, 같은 도착점에 대해 네 알고리즘을 연속 실행합니다.
// 각 알고리즘의 방문 수, 경로 길이, 비용, 간선 검사 수, 최대 frontier 크기를 저장하여
// 우측 패널의 비교표로 보여 줍니다. 마지막 화면 애니메이션은 비용 최적 경로를 보여주기 쉬운 A* 결과입니다.
// ------------------------------------------------------------
void ofApp::runAllAlgorithmsForComparison() {
    comparisonResults.clear();

    runBFS();
    comparisonResults.push_back({"BFS", lastStats});

    runDFS();
    comparisonResults.push_back({"DFS", lastStats});

    runDijkstra();
    comparisonResults.push_back({"Dijkstra", lastStats});

    runAStar();
    comparisonResults.push_back({"A*", lastStats});

    currentAlgorithmName = "A* after Compare All";
}

// ------------------------------------------------------------
// generateRandomMaze()
// ------------------------------------------------------------
// Recursive Backtracking 방식으로 랜덤 미로를 생성합니다.
// 명시적 stack을 사용하므로 재귀 호출 깊이 문제 없이 DFS 기반 미로 생성을 수행할 수 있습니다.
// ------------------------------------------------------------
void ofApp::generateRandomMaze() {
    rows = 25;
    cols = 35;
    grid.assign(rows, std::vector<Cell>(cols, {CellType::Wall, 1}));

    std::stack<Node> st;
    std::vector<std::vector<bool>> carved(rows, std::vector<bool>(cols, false));

    const Node seed{1, 1};
    st.push(seed);
    carved[seed.row][seed.col] = true;
    grid[seed.row][seed.col] = {CellType::Empty, 1};

    std::random_device rd;
    std::mt19937 rng(rd());

    // DFS와 비슷하게 stack의 top에서 갈 수 있는 방향을 고르고, 막히면 pop하여 되돌아갑니다.
    // 두 칸씩 이동하는 이유는 중간 칸을 벽으로 남겨 두어 미로의 통로와 벽 구조를 만들기 위함입니다.
    while (!st.empty()) {
        Node cur = st.top();

        std::vector<Node> directions = {
            {-2, 0}, {2, 0}, {0, -2}, {0, 2}
        };
        std::shuffle(directions.begin(), directions.end(), rng);

        bool moved = false;

        for (const Node& d : directions) {
            const int nr = cur.row + d.row;
            const int nc = cur.col + d.col;

            if (nr <= 0 || nr >= rows - 1 || nc <= 0 || nc >= cols - 1) {
                continue;
            }
            if (carved[nr][nc]) {
                continue;
            }

            const int betweenR = cur.row + d.row / 2;
            const int betweenC = cur.col + d.col / 2;

            grid[betweenR][betweenC] = {CellType::Empty, 1};
            grid[nr][nc] = {CellType::Empty, 1};
            carved[nr][nc] = true;

            st.push({nr, nc});
            moved = true;
            break;
        }

        if (!moved) {
            st.pop();
        }
    }

    start = {1, 1};
    goal = {rows - 2, cols - 2};

    // 일부 빈 칸을 가중치 칸으로 바꿉니다.
    // 이 단계 덕분에 BFS와 Dijkstra/A*의 차이를 랜덤 미로에서도 관찰할 수 있습니다.
    for (int r = 1; r < rows - 1; ++r) {
        for (int c = 1; c < cols - 1; ++c) {
            if (grid[r][c].type == CellType::Empty && ofRandom(1.0f) < 0.07f) {
                grid[r][c] = {CellType::Weight, 5};
            }
        }
    }

    grid[start.row][start.col] = {CellType::Start, 1};
    grid[goal.row][goal.col] = {CellType::Goal, 1};

    updateLayout();
    resetSearchState();
}

// ------------------------------------------------------------
// screenToCell()
// ------------------------------------------------------------
// 마우스의 픽셀 좌표를 미로의 row/col 좌표로 변환합니다.
// 미로 영역 밖을 클릭한 경우 false를 반환합니다.
// ------------------------------------------------------------
bool ofApp::screenToCell(int x, int y, Node& outCell) const {
    const int col = (x - offsetX) / cellSize;
    const int row = (y - offsetY) / cellSize;

    if (!isInside(row, col)) {
        return false;
    }

    const int localX = x - offsetX;
    const int localY = y - offsetY;

    if (localX < 0 || localY < 0 || localX >= cols * cellSize || localY >= rows * cellSize) {
        return false;
    }

    outCell = {row, col};
    return true;
}

// ------------------------------------------------------------
// applyEditToCell()
// ------------------------------------------------------------
// 현재 editMode에 따라 클릭한 칸을 수정합니다.
// 시작점과 도착점은 항상 하나씩만 존재하도록 기존 위치를 Empty로 되돌린 뒤 새 위치를 지정합니다.
// ------------------------------------------------------------
void ofApp::applyEditToCell(int row, int col) {
    if (!isInside(row, col)) {
        return;
    }

    // 테두리 벽은 미로 구조를 안정적으로 유지하기 위해 시작점/도착점 이동 대상에서 제외합니다.
    const bool boundary = (row == 0 || col == 0 || row == rows - 1 || col == cols - 1);
    if (boundary) {
        return;
    }

    Cell& cell = grid[row][col];

    switch (editMode) {
        case EditMode::ToggleWall:
            if (Node{row, col} == start || Node{row, col} == goal) {
                break;
            }
            cell.type = (cell.type == CellType::Wall) ? CellType::Empty : CellType::Wall;
            cell.cost = 1;
            break;

        case EditMode::MoveStart:
            if (Node{row, col} == goal) {
                break;
            }
            grid[start.row][start.col] = {CellType::Empty, 1};
            start = {row, col};
            grid[start.row][start.col] = {CellType::Start, 1};
            break;

        case EditMode::MoveGoal:
            if (Node{row, col} == start) {
                break;
            }
            grid[goal.row][goal.col] = {CellType::Empty, 1};
            goal = {row, col};
            grid[goal.row][goal.col] = {CellType::Goal, 1};
            break;

        case EditMode::ToggleWeight:
            if (Node{row, col} == start || Node{row, col} == goal) {
                break;
            }
            if (cell.type == CellType::Weight) {
                cell = {CellType::Empty, 1};
            } else if (cell.type == CellType::Empty) {
                cell = {CellType::Weight, 5};
            }
            break;
    }

    // 미로가 바뀌면 기존 방문 순서와 경로는 더 이상 현재 미로의 결과가 아닙니다.
    // 따라서 즉시 초기화하여 잘못된 결과가 화면에 남지 않도록 합니다.
    resetSearchState();
}

// ------------------------------------------------------------
// editModeToString()
// ------------------------------------------------------------
// 현재 편집 모드를 사람이 읽기 쉬운 문자열로 변환합니다.
// ------------------------------------------------------------
std::string ofApp::editModeToString() const {
    switch (editMode) {
        case EditMode::ToggleWall:
            return "Wall Toggle";
        case EditMode::MoveStart:
            return "Move Start";
        case EditMode::MoveGoal:
            return "Move Goal";
        case EditMode::ToggleWeight:
            return "Weight Toggle";
    }
    return "Unknown";
}

// ------------------------------------------------------------
// drawMaze()
// ------------------------------------------------------------
// grid와 애니메이션 배열을 사용해 미로 전체를 그립니다.
// 기본 칸 색상 위에 방문 색, 경로 색, 시작/도착 표시를 순서대로 덮어 씁니다.
// ------------------------------------------------------------
void ofApp::drawMaze() const {
    ofPushStyle();

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const int x = offsetX + c * cellSize;
            const int y = offsetY + r * cellSize;

            ofColor color = COLOR_EMPTY;
            const Cell& cell = grid[r][c];

            if (cell.type == CellType::Wall) {
                color = COLOR_WALL;
            } else if (cell.type == CellType::Weight) {
                color = COLOR_WEIGHT;
            } else if (cell.type == CellType::Start) {
                color = COLOR_START;
            } else if (cell.type == CellType::Goal) {
                color = COLOR_GOAL;
            }

            ofSetColor(color);
            ofDrawRectangle(x, y, cellSize, cellSize);

            // 방문 칸은 기본 칸 색 위에 반투명 파란색을 덮어 표현합니다.
            // 시작/도착/벽은 의미가 강하므로 방문 색을 덮지 않습니다.
            if (visibleVisited[r][c] && cell.type != CellType::Wall && cell.type != CellType::Start && cell.type != CellType::Goal) {
                ofSetColor(COLOR_VISITED);
                ofDrawRectangle(x, y, cellSize, cellSize);
            }

            // 최종 경로는 방문 색보다 더 진한 색으로 표시합니다.
            if (visiblePath[r][c] && cell.type != CellType::Wall) {
                ofSetColor(COLOR_PATH);
                ofDrawRectangle(x + 2, y + 2, cellSize - 4, cellSize - 4);
            }

            // 가중치 칸에는 비용 숫자를 표시하여 이동 비용의 의미를 분명하게 보여줍니다.
            if (cell.type == CellType::Weight && cellSize >= 16) {
                ofSetColor(COLOR_TEXT);
                ofDrawBitmapString(std::to_string(cell.cost), x + cellSize / 2 - 3, y + cellSize / 2 + 4);
            }

            // 격자선을 그려 각 칸의 경계를 구분합니다.
            ofNoFill();
            ofSetColor(COLOR_GRID_LINE);
            ofDrawRectangle(x, y, cellSize, cellSize);
            ofFill();
        }
    }

    ofPopStyle();
}

// ------------------------------------------------------------
// drawSidePanel()
// ------------------------------------------------------------
// 우측 패널에 조작법, 현재 알고리즘, 통계, 복잡도 정보를 표시합니다.
// 제출 동영상에서 이 패널을 함께 보여주면 구현 기능과 알고리즘 설명이 명확해집니다.
// ------------------------------------------------------------
void ofApp::drawSidePanel() const {
    const int panelX = offsetX + cols * cellSize + 28;
    const int panelY = offsetY;
    const int line = 21;
    int y = panelY;

    ofPushStyle();
    ofSetColor(255);
    ofDrawRectangle(panelX - 14, panelY - 18, sidePanelWidth - 24, ofGetHeight() - panelY * 2 + 12);

    ofSetColor(COLOR_TEXT);
    ofDrawBitmapString("Maze Pathfinder Lab", panelX, y);
    y += line;
    ofSetColor(COLOR_MUTED_TEXT);
    ofDrawBitmapString("Weighted search visualizer + comparison", panelX, y);
    y += line * 2;

    ofSetColor(COLOR_TEXT);
    ofDrawBitmapString("[Algorithm Keys]", panelX, y);
    y += line;
    ofSetColor(COLOR_MUTED_TEXT);
    ofDrawBitmapString("1 BFS        2 DFS", panelX, y); y += line;
    ofDrawBitmapString("3 Dijkstra   4 A*", panelX, y); y += line;
    ofDrawBitmapString("5 Compare All Algorithms", panelX, y); y += line;
    ofDrawBitmapString("L Load Maze  G Random Maze", panelX, y); y += line;
    ofDrawBitmapString("C Clear      R Reset Result", panelX, y); y += line;
    ofDrawBitmapString("P Pause      +/- Speed", panelX, y); y += line;
    ofDrawBitmapString("M Edit Mode  H Help", panelX, y); y += line * 2;

    ofSetColor(COLOR_TEXT);
    ofDrawBitmapString("[Current State]", panelX, y);
    y += line;
    ofSetColor(COLOR_MUTED_TEXT);
    ofDrawBitmapString("Algorithm : " + currentAlgorithmName, panelX, y); y += line;
    ofDrawBitmapString("Edit Mode : " + editModeToString(), panelX, y); y += line;
    ofDrawBitmapString("Maze Size : " + std::to_string(rows) + " x " + std::to_string(cols), panelX, y); y += line;
    ofDrawBitmapString("Animation : " + std::string(isPaused ? "Paused" : (isAnimating ? "Running" : "Stopped")), panelX, y); y += line * 2;

    ofSetColor(COLOR_TEXT);
    ofDrawBitmapString("[Last Result]", panelX, y);
    y += line;
    ofSetColor(COLOR_MUTED_TEXT);
    ofDrawBitmapString("Found       : " + std::string(lastStats.found ? "Yes" : "No"), panelX, y); y += line;
    ofDrawBitmapString("Visited     : " + std::to_string(lastStats.visitedCount), panelX, y); y += line;
    ofDrawBitmapString("Path Length : " + std::to_string(lastStats.pathLength), panelX, y); y += line;
    ofDrawBitmapString("Path Cost   : " + std::to_string(lastStats.pathCost), panelX, y); y += line;
    ofDrawBitmapString("Edge Checks : " + std::to_string(lastStats.edgeChecks), panelX, y); y += line;
    ofDrawBitmapString("Max Frontier: " + std::to_string(lastStats.maxFrontierSize), panelX, y); y += line;
    ofDrawBitmapString("Aux Space   : " + std::to_string(lastStats.auxiliaryCells) + " cells", panelX, y); y += line;
    ofDrawBitmapString("Time        : " + ofToString(lastStats.elapsedMs, 4) + " ms", panelX, y); y += line * 2;

    ofSetColor(COLOR_TEXT);
    ofDrawBitmapString("[Complexity]", panelX, y);
    y += line;
    ofSetColor(COLOR_MUTED_TEXT);
    ofDrawBitmapString("BFS / DFS     : O(V + E)", panelX, y); y += line;
    ofDrawBitmapString("Dijkstra / A* : O((V+E)logV)", panelX, y); y += line;
    ofDrawBitmapString("Space         : O(V)", panelX, y); y += line;
    ofDrawBitmapString("Measured frontier/space shown above", panelX, y); y += line * 2;

    drawComparisonTable(panelX, y);

    drawLegend(panelX, y);
    y += line * 7;

    if (showHelp) {
        drawHelpBox(panelX, y);
    }

    ofPopStyle();
}


// ------------------------------------------------------------
// drawComparisonTable()
// ------------------------------------------------------------
// Compare All 모드에서 저장한 정량 결과를 표 형태로 출력합니다.
// 단순히 경로가 보이는 것을 넘어서 시간복잡도와 공간복잡도 관찰 지표를 화면에 남깁니다.
// ------------------------------------------------------------
void ofApp::drawComparisonTable(int x, int& y) const {
    const int line = 18;

    if (comparisonResults.empty()) {
        return;
    }

    ofSetColor(COLOR_TEXT);
    ofDrawBitmapString("[Compare All]", x, y);
    y += line;

    ofSetColor(COLOR_MUTED_TEXT);
    ofDrawBitmapString("Alg       Visited  Cost  Edges  Frontier", x, y);
    y += line;

    for (const AlgorithmSnapshot& item : comparisonResults) {
        const SearchStats& s = item.stats;
        const std::string row = item.name +
            "  V:" + std::to_string(s.visitedCount) +
            " C:" + std::to_string(s.pathCost) +
            " E:" + std::to_string(s.edgeChecks) +
            " F:" + std::to_string(s.maxFrontierSize);
        ofDrawBitmapString(row, x, y);
        y += line;
    }

    y += line;
}

// ------------------------------------------------------------
// drawLegend()
// ------------------------------------------------------------
// 색상 범례를 출력합니다.
// ------------------------------------------------------------
void ofApp::drawLegend(int x, int y) const {
    const int box = 13;
    const int line = 20;

    ofSetColor(COLOR_TEXT);
    ofDrawBitmapString("[Legend]", x, y);
    y += line;

    const std::vector<std::pair<ofColor, std::string>> items = {
        {COLOR_START, "Start"},
        {COLOR_GOAL, "Goal"},
        {COLOR_WALL, "Wall"},
        {COLOR_WEIGHT, "Weight Cost"},
        {COLOR_VISITED, "Visited Order"},
        {COLOR_PATH, "Final Path"}
    };

    for (const auto& item : items) {
        ofSetColor(item.first);
        ofDrawRectangle(x, y - box + 3, box, box);
        ofSetColor(COLOR_MUTED_TEXT);
        ofDrawBitmapString(item.second, x + box + 8, y);
        y += line;
    }
}

// ------------------------------------------------------------
// drawHelpBox()
// ------------------------------------------------------------
// 보고서/발표에서 설명하기 좋은 핵심 해석 포인트를 화면에도 표시합니다.
// ------------------------------------------------------------
void ofApp::drawHelpBox(int x, int y) const {
    const int line = 19;

    ofSetColor(COLOR_TEXT);
    ofDrawBitmapString("[Interpretation Guide]", x, y);
    y += line;

    ofSetColor(COLOR_MUTED_TEXT);
    ofDrawBitmapString("BFS: shortest number of steps", x, y); y += line;
    ofDrawBitmapString("DFS: deep exploration, not optimal", x, y); y += line;
    ofDrawBitmapString("Dijkstra: minimum total cost", x, y); y += line;
    ofDrawBitmapString("A*: Dijkstra + goal heuristic", x, y); y += line;
    ofDrawBitmapString("Weighted cells reveal cost-aware search.", x, y); y += line;
    ofDrawBitmapString("Use mouse edit mode to create cases.", x, y);
}
