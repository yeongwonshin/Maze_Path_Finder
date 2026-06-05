#pragma once

#include "ofMain.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <queue>
#include <random>
#include <stack>
#include <string>
#include <vector>

// ============================================================
// ofApp.h
// ------------------------------------------------------------
// Maze Pathfinder Lab의 전체 자료구조와 함수 원형을 정의하는 헤더입니다.
// 평가 기준에서 요구하는 자료구조 설명이 코드만 보아도 드러나도록
// 각 enum, struct, 멤버 변수, 함수 그룹의 목적을 주석으로 상세히 적었습니다.
// ============================================================

// 미로 한 칸의 논리적 종류를 표현합니다.
// drawMaze(), isWalkable(), getMoveCost(), applyEditToCell()에서
// 이 값을 기준으로 렌더링, 이동 가능 여부, 비용, 편집 동작이 결정됩니다.
enum class CellType {
    Empty,   // 통과 가능한 일반 칸입니다. 이동 비용은 1입니다.
    Wall,    // 통과할 수 없는 벽입니다. 탐색 알고리즘의 후보에서 제외됩니다.
    Start,   // 탐색 시작점입니다. 프로그램 전체에서 하나만 유지합니다.
    Goal,    // 탐색 도착점입니다. 프로그램 전체에서 하나만 유지합니다.
    Weight   // 통과는 가능하지만 일반 칸보다 이동 비용이 큰 가중치 칸입니다.
};

// 마우스 편집 모드를 표현합니다.
// M 키를 누를 때마다 모드가 순환하며, 현재 모드에 따라 클릭한 칸의 의미가 달라집니다.
enum class EditMode {
    ToggleWall,    // 클릭한 칸을 벽 또는 빈 칸으로 전환합니다.
    MoveStart,     // 클릭한 칸으로 시작점을 이동합니다.
    MoveGoal,      // 클릭한 칸으로 도착점을 이동합니다.
    ToggleWeight   // 클릭한 칸을 가중치 칸 또는 빈 칸으로 전환합니다.
};

// 미로 좌표를 저장하는 가장 기본적인 노드 구조체입니다.
// row는 행 번호, col은 열 번호입니다. 2차원 vector의 인덱스와 직접 대응됩니다.
struct Node {
    int row = 0;
    int col = 0;

    // 두 노드가 같은 칸을 가리키는지 비교합니다.
    // 시작점/도착점 판정, 마우스 드래그 중복 처리, 경로 복원 등에 사용됩니다.
    bool operator==(const Node& other) const {
        return row == other.row && col == other.col;
    }
};

// 미로의 각 칸이 저장하는 정보입니다.
// type은 칸의 역할을, cost는 해당 칸으로 진입할 때 필요한 이동 비용을 뜻합니다.
struct Cell {
    CellType type = CellType::Empty;
    int cost = 1;
};

// Dijkstra와 A*에서 priority_queue에 넣을 상태 정보입니다.
// priority는 우선순위 큐 정렬 기준이고, distance는 시작점에서 현재 칸까지의 실제 누적 비용입니다.
struct PriorityState {
    int row = 0;
    int col = 0;
    int priority = 0;
    int distance = 0;

    // std::priority_queue는 기본적으로 큰 값이 먼저 나오는 max-heap입니다.
    // 따라서 priority가 작은 칸이 먼저 나오도록 비교식을 반대로 작성했습니다.
    bool operator<(const PriorityState& other) const {
        return priority > other.priority;
    }
};

// 마지막으로 실행한 탐색의 정량적 결과를 저장합니다.
// 우측 패널과 보고서용 비교표에서 방문 노드 수, 경로 길이, 총 비용, 실행 시간을 표시합니다.
struct SearchStats {
    bool found = false;       // 도착점에 도달했는지 여부입니다.
    int visitedCount = 0;     // 탐색 과정에서 방문 처리된 칸 수입니다.
    int pathLength = 0;       // 최종 경로에 포함된 칸 수입니다. 시작점과 도착점을 포함합니다.
    int pathCost = 0;         // 최종 경로의 총 이동 비용입니다. 가중치 칸을 지나면 증가합니다.
    int edgeChecks = 0;       // 이웃 간선을 검사한 횟수입니다. 실제 시간복잡도 비교 근거로 사용합니다.
    int maxFrontierSize = 0;  // queue, stack, priority_queue가 동시에 보관한 최대 후보 수입니다.
    int auxiliaryCells = 0;   // visited, parent, dist 등 보조 배열이 사용하는 대략적 셀 단위 공간입니다.
    double elapsedMs = 0.0;   // 알고리즘 계산에 걸린 시간입니다. 애니메이션 시간은 제외합니다.
};

// Compare All 모드에서 알고리즘별 결과를 한 줄씩 저장하기 위한 구조체입니다.
// 같은 미로에 대해 BFS, DFS, Dijkstra, A*를 연속 실행한 뒤 표 형태로 출력합니다.
struct AlgorithmSnapshot {
    std::string name;
    SearchStats stats;
};

class ofApp : public ofBaseApp {
public:
    // OpenFrameworks 기본 생명주기 함수입니다.
    void setup() override;
    void update() override;
    void draw() override;

    // 사용자 입력 처리 함수입니다.
    void keyPressed(int key) override;
    void mousePressed(int x, int y, int button) override;
    void mouseDragged(int x, int y, int button) override;
    void windowResized(int w, int h) override;

private:
    // ------------------------------------------------------------
    // 1. 미로 상태 저장 자료구조
    // ------------------------------------------------------------

    // 전체 미로를 2차원 배열 형태로 저장합니다.
    // grid[r][c]는 r행 c열의 칸을 의미하므로, 그래프의 정점 집합 V를 배열로 표현한 것입니다.
    std::vector<std::vector<Cell>> grid;

    // rows와 cols는 미로의 행/열 개수입니다.
    // 반복문 범위, 화면 좌표 변환, 시간복잡도 V=rows*cols 계산에 모두 사용됩니다.
    int rows = 0;
    int cols = 0;

    // start와 goal은 시작점과 도착점의 좌표입니다.
    // 알고리즘 실행 전 반드시 유효한 칸을 가리켜야 하며, 편집 모드에서도 하나씩만 유지됩니다.
    Node start{1, 1};
    Node goal{1, 1};

    // 샘플 미로 파일 목록입니다.
    // L 키를 누르면 currentMazeIndex를 증가시켜 다음 파일을 로드합니다.
    std::vector<std::string> mazeFiles;
    int currentMazeIndex = 0;

    // ------------------------------------------------------------
    // 2. 탐색 결과 및 애니메이션 상태
    // ------------------------------------------------------------

    // visitOrder는 알고리즘이 실제로 노드를 방문한 순서입니다.
    // update()가 이 vector를 조금씩 화면에 공개하여 탐색 과정을 애니메이션으로 보여줍니다.
    std::vector<Node> visitOrder;

    // finalPath는 parent 배열을 통해 복원된 시작점부터 도착점까지의 최종 경로입니다.
    // 방문 애니메이션이 끝난 뒤 순차적으로 표시됩니다.
    std::vector<Node> finalPath;

    // visibleVisited[r][c]는 현재 프레임까지 화면에 표시된 방문 칸인지 저장합니다.
    // 알고리즘 계산 결과와 화면 표시 상태를 분리하기 위해 별도 배열로 둡니다.
    std::vector<std::vector<bool>> visibleVisited;

    // visiblePath[r][c]는 현재 프레임까지 화면에 표시된 최종 경로 칸인지 저장합니다.
    // 방문 영역과 최종 경로를 서로 다른 색으로 표현하기 위해 사용합니다.
    std::vector<std::vector<bool>> visiblePath;

    // 애니메이션 인덱스입니다.
    // visitedAnimationIndex는 visitOrder의 다음 표시 위치를,
    // pathAnimationIndex는 finalPath의 다음 표시 위치를 가리킵니다.
    std::size_t visitedAnimationIndex = 0;
    std::size_t pathAnimationIndex = 0;

    // lastAnimationStepTime은 마지막으로 한 칸을 표시한 시각입니다.
    // animationInterval과 비교하여 프레임 속도와 무관하게 일정한 애니메이션 속도를 유지합니다.
    float lastAnimationStepTime = 0.0f;
    float animationInterval = 0.018f;

    // isAnimating은 표시할 방문/경로가 남아 있는지, isPaused는 사용자가 P 키로 일시정지했는지 나타냅니다.
    bool isAnimating = false;
    bool isPaused = false;

    // 마지막으로 실행된 알고리즘 이름과 통계입니다.
    // 우측 패널에서 현재 결과를 설명하는 데 사용됩니다.
    std::string currentAlgorithmName = "None";
    SearchStats lastStats;

    // 5 키를 눌렀을 때 네 알고리즘의 결과를 한 번에 비교하여 저장합니다.
    std::vector<AlgorithmSnapshot> comparisonResults;

    // ------------------------------------------------------------
    // 3. 화면 배치 및 입력 상태
    // ------------------------------------------------------------

    // cellSize는 한 칸을 화면에 그릴 픽셀 크기입니다.
    // 창 크기가 바뀌면 updateLayout()에서 다시 계산됩니다.
    int cellSize = 24;
    int offsetX = 30;
    int offsetY = 30;
    int sidePanelWidth = 360;

    // 현재 마우스 편집 모드입니다.
    // M 키 입력에 따라 순환하며, 우측 패널에도 현재 모드가 표시됩니다.
    EditMode editMode = EditMode::ToggleWall;

    // 드래그 중 같은 칸이 반복 토글되는 문제를 막기 위한 상태입니다.
    // 같은 셀 위에서 mouseDragged가 여러 번 발생해도 한 번만 편집되도록 합니다.
    bool hasLastEditedCell = false;
    Node lastEditedCell{-1, -1};

    // H 키로 도움말 패널을 표시하거나 숨길 수 있습니다.
    bool showHelp = true;

    // ------------------------------------------------------------
    // 4. 초기화, 파일 입출력, 화면 배치 함수
    // ------------------------------------------------------------
    void setupMazeFileList();
    bool loadMazeFromFile(const std::string& fileName);
    void createEmptyMaze(int newRows, int newCols);
    void updateLayout();
    void resetSearchState();
    void initializeVisibleArrays();

    // ------------------------------------------------------------
    // 5. 탐색 알고리즘 공통 유틸리티 함수
    // ------------------------------------------------------------
    bool isInside(int row, int col) const;
    bool isWalkable(int row, int col) const;
    int getMoveCost(int row, int col) const;
    int manhattanDistance(const Node& a, const Node& b) const;
    std::vector<Node> getNeighbors(const Node& node) const;
    std::vector<Node> reconstructPath(const std::vector<std::vector<Node>>& parent) const;
    int computePathCost(const std::vector<Node>& path) const;
    void prepareAnimation(const std::string& algorithmName,
                          const std::vector<Node>& visited,
                          const std::vector<Node>& path,
                          const SearchStats& stats);

    // ------------------------------------------------------------
    // 6. 네 가지 탐색 알고리즘
    // ------------------------------------------------------------
    void runBFS();
    void runDFS();
    void runDijkstra();
    void runAStar();
    void runAllAlgorithmsForComparison();

    // ------------------------------------------------------------
    // 7. 랜덤 미로 생성 및 마우스 편집 함수
    // ------------------------------------------------------------
    void generateRandomMaze();
    bool screenToCell(int x, int y, Node& outCell) const;
    void applyEditToCell(int row, int col);
    std::string editModeToString() const;

    // ------------------------------------------------------------
    // 8. 화면 출력 함수
    // ------------------------------------------------------------
    void drawMaze() const;
    void drawSidePanel() const;
    void drawLegend(int x, int y) const;
    void drawComparisonTable(int x, int& y) const;
    void drawHelpBox(int x, int y) const;
};
