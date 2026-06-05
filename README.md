# Maze Pathfinder Lab

An interactive openFrameworks project for visualizing and comparing classic pathfinding algorithms in a grid maze: **BFS**, **DFS**, **Dijkstra**, and **A\***.

This project is designed as both a working C++ application and an educational demonstration. Instead of showing only the final path, it animates the order in which each algorithm visits cells, reconstructs the final path, and reports quantitative results such as visited node count, path length, total path cost, and execution time.

---

## 1. Project Overview

**Maze Pathfinder Lab** is a maze-search visualization program built with **C++ and openFrameworks**. The program treats a maze as a graph, where each walkable cell is a node and movement to neighboring cells is an edge. Users can run different search algorithms, observe how each algorithm expands through the maze, and compare the final path produced by each method.

The main goal of this project is to make graph-search algorithms understandable through direct visual interaction. A user can load a sample maze, generate a random maze, clear the grid, edit the maze with the mouse, place weighted cells, and then run several algorithms on the same map. This makes it possible to compare not only whether an algorithm finds a path, but also how efficiently it searches and whether it considers movement cost.

The project supports both unweighted and weighted maze scenarios. In a normal maze where every cell has the same movement cost, BFS is useful for finding the shortest path in terms of the number of steps. However, when weighted cells are added, the shortest number of steps may not be the minimum-cost route. In that situation, Dijkstra and A\* become more meaningful because they use accumulated movement cost when selecting the next cell to expand.

---

## 2. Main Features

- Visual comparison of **BFS**, **DFS**, **Dijkstra**, and **A\***.
- Animated display of the algorithm's visited order.
- Animated display of the reconstructed final path.
- Support for weighted cells with movement costs.
- Mouse-based maze editing.
- Sample maze loading.
- Random maze generation.
- Clear-grid mode for custom experiments.
- Right-side information panel showing algorithm state, result statistics, complexity, and legend.
- Help guide explaining how to interpret each algorithm.

---

## 3. Why This Project Is Useful

This project is useful for anyone learning data structures, graph traversal, or shortest-path algorithms. Many students understand the definitions of BFS, DFS, Dijkstra, and A\* separately, but it is often difficult to understand how their behaviors differ in the same environment. This program solves that problem by allowing the user to run all four algorithms on the same maze and observe their decisions visually.

The project also connects algorithm theory with actual implementation. It uses important C++ data structures such as `vector`, `queue`, `stack`, `priority_queue`, and parent-tracking arrays. Because the program reports visited node count, path length, path cost, and execution time, it can also be used to explain time complexity, space complexity, and practical algorithm performance.

---

## 4. Development Environment

### Required

- C++ compiler
- openFrameworks
- macOS with Xcode, or Windows with Visual Studio, or a Linux environment supported by openFrameworks
- No additional openFrameworks addons are required

### Recommended

- openFrameworks 0.12.x or later
- Xcode on macOS
- Visual Studio on Windows
- openFrameworks Project Generator for creating or updating the IDE project files

This project uses only the standard openFrameworks application structure and does not depend on external libraries beyond openFrameworks itself.

---

## 5. Project Folder Location

openFrameworks projects should usually be placed inside the `apps/myApps` directory of the openFrameworks installation.

Recommended location:

```text
openFrameworks/apps/myApps/MazePathfinderLab
```

A typical folder structure is:

```text
MazePathfinderLab/
├── README.md
├── README.txt
├── Makefile
├── config.make
├── addons.make
├── src/
│   ├── main.cpp
│   ├── ofApp.h
│   └── ofApp.cpp
└── bin/
    └── data/
        ├── maze_easy.txt
        ├── maze_weighted.txt
        └── maze_large.txt
```

If the `bin/data` sample maze files are not present, the program can still create a fallback empty maze. However, for the best demonstration, it is recommended to include at least one sample maze file.

---

## 6. How to Install openFrameworks

Before running this project, openFrameworks must be installed on the computer.

General process:

1. Download openFrameworks for your operating system.
2. Extract the openFrameworks folder to a stable location.
3. Open the openFrameworks folder and check that it contains folders such as `apps`, `libs`, `addons`, and `projectGenerator`.
4. Place this project inside `apps/myApps`.
5. Use the openFrameworks Project Generator to create or update the IDE project files.

Example final path:

```text
openFrameworks/apps/myApps/MazePathfinderLab
```

Do not place only the `src` folder somewhere else. The project should remain inside the openFrameworks directory structure so that the compiler can correctly find the openFrameworks headers, libraries, and build files.

---

## 7. Running the Project on macOS with Xcode

### Step 1. Place the project in openFrameworks

Move the project folder into:

```text
openFrameworks/apps/myApps/MazePathfinderLab
```

### Step 2. Open Project Generator

Run the openFrameworks Project Generator application.

### Step 3. Import the project

Select **Import**, then choose the `MazePathfinderLab` folder.

### Step 4. Check addons

This project does not require additional addons, so the addons list can remain empty.

### Step 5. Update the project

Click **Update**. This generates or refreshes the Xcode project files.

### Step 6. Open the Xcode project

Open the generated `.xcodeproj` file.

### Step 7. Build and run

In Xcode, press the Run button or use:

```text
Command + R
```

The application window should open with the title:

```text
Maze Pathfinder Lab - BFS / DFS / Dijkstra / A*
```

---

## 8. Running the Project from the Terminal on macOS

If your openFrameworks installation supports Makefile builds, you can run the project from the terminal.

Move into the project directory:

```bash
cd openFrameworks/apps/myApps/MazePathfinderLab
```

Build the project:

```bash
make
```

Run the release build:

```bash
make RunRelease
```

Important note:

```text
make RunRelease
```

is case-sensitive. For example, `make Runrelease` is not the same command.

---

## 9. Running the Project on Windows with Visual Studio

### Step 1. Place the project in openFrameworks

Move the project folder into:

```text
openFrameworks/apps/myApps/MazePathfinderLab
```

### Step 2. Open Project Generator

Run the openFrameworks Project Generator for Visual Studio.

### Step 3. Import the project

Select **Import** and choose the `MazePathfinderLab` folder.

### Step 4. Update the project

Click **Update** to generate the Visual Studio solution files.

### Step 5. Open the solution

Open the generated `.sln` file in Visual Studio.

### Step 6. Build and run

Select the appropriate build configuration and run the project. The application window should display the maze grid and the side control panel.

---

## 10. First-Time Execution Guide

When the program starts, it performs the following initialization steps:

1. The openFrameworks window is created.
2. The window title and frame rate are set.
3. The program registers the sample maze file names.
4. The first sample maze is loaded from `bin/data` if available.
5. If no valid sample file is found, an empty bordered maze is created automatically.
6. The grid layout is calculated based on the current window size.
7. The visible visited-cell and path arrays are initialized.
8. The right-side panel is drawn with controls, statistics, complexity, and legend.

After this, the user can immediately press `1`, `2`, `3`, or `4` to run a pathfinding algorithm.

---

## 11. Keyboard Controls

| Key | Action |
|---|---|
| `1` | Run BFS |
| `2` | Run DFS |
| `3` | Run Dijkstra |
| `4` | Run A\* |
| `M` | Change mouse edit mode |
| `L` | Load the next sample maze file |
| `G` | Generate a random maze |
| `C` | Clear the maze and create an empty bordered grid |
| `R` | Reset the current search result |
| `P` | Pause or resume the animation |
| `+` or `=` | Increase animation speed |
| `-` or `_` | Decrease animation speed |
| `H` | Show or hide the interpretation help guide |

---

## 12. Mouse Controls and Edit Modes

The mouse behavior depends on the current edit mode. Press `M` to cycle through the four edit modes.

### 1. Wall Toggle

Click or drag on cells to switch them between wall and empty cell. Walls cannot be passed by any algorithm.

### 2. Move Start

Click a walkable cell to move the start position. The project keeps exactly one start point on the grid.

### 3. Move Goal

Click a walkable cell to move the goal position. The project keeps exactly one goal point on the grid.

### 4. Weight Toggle

Click or drag on cells to switch them between weighted and empty cell. Weighted cells are passable, but they have a higher movement cost than normal cells.

During mouse dragging, the program ignores repeated edits on the same cell so that a wall or weighted cell does not toggle back and forth accidentally.

---

## 13. How to Use the Program After Launch

A recommended demonstration sequence is:

1. Run the program.
2. Press `1` to run BFS and observe the visited order.
3. Press `2` to run DFS and compare its deep-search behavior.
4. Press `3` to run Dijkstra and observe cost-aware exploration.
5. Press `4` to run A\* and compare its goal-directed behavior.
6. Press `M` until the edit mode becomes `Weight Toggle`.
7. Add weighted cells between the start and goal.
8. Run BFS again and check whether it still follows the shortest number of steps.
9. Run Dijkstra or A\* and check whether the algorithm avoids high-cost cells.
10. Compare the result panel values: visited count, path length, path cost, and time.

This sequence clearly shows the difference between unweighted pathfinding and weighted shortest-path search.

---

## 14. Maze File Format

Maze files are plain text files placed inside:

```text
bin/data/
```

Supported characters:

| Character | Meaning |
|---|---|
| `#` | Wall |
| `.` | Empty cell with movement cost 1 |
| `S` | Start cell |
| `G` | Goal cell |
| `2` to `9` | Weighted cell with the corresponding movement cost |
| `W` | Weighted cell with movement cost 5 |

Example:

```text
#########
#S..2...#
#.###.#G#
#...W...#
#########
```

Explanation:

- `S` is the start position.
- `G` is the goal position.
- `#` is a wall.
- `.` is a normal walkable cell.
- `2` is a weighted cell with movement cost 2.
- `W` is a weighted cell with movement cost 5.

The loader also handles lines of different lengths by padding shorter lines as empty cells so that the internal grid remains rectangular.

---

## 15. Visual Legend

The application uses different colors to distinguish cell types and algorithm states.

| Visual Element | Meaning |
|---|---|
| Start cell | The cell where the search begins |
| Goal cell | The target cell the algorithm tries to reach |
| Wall cell | A blocked cell that cannot be visited |
| Weighted cell | A passable cell with higher movement cost |
| Visited order | Cells expanded by the algorithm during search |
| Final path | The reconstructed path from start to goal |

The visited-order animation appears before the final path animation. This separation helps the viewer distinguish between the search process and the final result.

---

## 16. Algorithm Explanation

### BFS: Breadth-First Search

BFS uses a `queue`. It explores cells in increasing distance from the start point. In an unweighted maze, BFS guarantees the shortest path in terms of the number of steps. However, BFS does not consider different movement costs. Therefore, in a weighted maze, BFS may find a path with fewer steps but a higher total cost.

### DFS: Depth-First Search

DFS uses a `stack`. It explores deeply in one direction before backtracking. DFS is useful for demonstrating recursive or stack-based graph traversal behavior, but it does not guarantee the shortest path or the minimum-cost path. In this project, DFS is mainly useful as a contrast to BFS and cost-aware algorithms.

### Dijkstra's Algorithm

Dijkstra uses a `priority_queue`. It always expands the cell with the smallest accumulated distance from the start. Because it considers movement cost, it can find the minimum-cost path when all movement costs are non-negative. Weighted cells make the advantage of Dijkstra especially clear.

### A\* Search

A\* also uses a `priority_queue`, but it selects cells based on:

```text
f(n) = g(n) + h(n)
```

where `g(n)` is the accumulated cost from the start and `h(n)` is the estimated distance to the goal. This project uses Manhattan distance as the heuristic because the maze uses four-directional grid movement. A\* usually visits fewer cells than Dijkstra when the heuristic guides the search effectively.

---

## 17. Important Data Structures

| Data Structure | Used For |
|---|---|
| `vector<vector<Cell>> grid` | Stores the maze as a 2D grid |
| `Node` | Stores row and column coordinates |
| `Cell` | Stores cell type and movement cost |
| `queue<Node>` | Used by BFS |
| `stack<Node>` | Used by DFS |
| `priority_queue<PriorityState>` | Used by Dijkstra and A\* |
| `parent` array | Reconstructs the final path |
| `visited` array | Prevents repeated expansion of the same cell |
| `distance` array | Stores minimum known cost for Dijkstra and A\* |
| `visitOrder` | Stores the exact order of visited cells for animation |
| `finalPath` | Stores the reconstructed path from start to goal |
| `SearchStats` | Stores result metrics for the side panel |

---

## 18. Complexity Analysis

Let:

```text
V = number of cells in the maze
E = number of valid movement edges between cells
```

In a rectangular grid, each cell has at most four neighbors, so `E` is proportional to `V`.

| Algorithm | Time Complexity | Space Complexity | Explanation |
|---|---|---|---|
| BFS | `O(V + E)` | `O(V)` | Visits each reachable cell and edge at most once |
| DFS | `O(V + E)` | `O(V)` | Uses stack and visited array |
| Dijkstra | `O((V + E) log V)` | `O(V)` | Priority queue operations add logarithmic cost |
| A\* | `O((V + E) log V)` in the general case | `O(V)` | Uses priority queue and heuristic-guided expansion |

Although A\* has the same general worst-case complexity class as Dijkstra, it often visits fewer nodes in practice when the heuristic is effective.

---

## 19. Result Metrics Shown in the Program

The side panel displays the result of the most recently executed algorithm.

| Metric | Meaning |
|---|---|
| `Found` | Whether a path from start to goal was found |
| `Visited` | Number of cells expanded by the algorithm |
| `Path Length` | Number of cells in the final path, including start and goal |
| `Path Cost` | Total movement cost of the final path |
| `Time` | Algorithm computation time in milliseconds |

These metrics are useful for reports, presentations, and demonstrations because they connect the visual behavior of the algorithm to measurable performance.

---

## 20. Source File Description

### `src/main.cpp`

Creates the openFrameworks application window and starts the `ofApp` instance. This file is the entry point of the program.

### `src/ofApp.h`

Defines the main data structures, enumerations, class members, and function declarations. This file explains the logical structure of the project, including cell types, edit modes, algorithm states, animation state, and result statistics.

### `src/ofApp.cpp`

Implements all main functionality, including setup, update, drawing, keyboard input, mouse editing, maze loading, random maze generation, BFS, DFS, Dijkstra, A\*, path reconstruction, animation preparation, and side-panel rendering.

---

## 21. Suggested Demo Scenario

For a clear project demonstration, use the following flow:

1. Show the initial maze and explain that the maze is represented as a graph.
2. Run BFS and explain that it expands level by level.
3. Run DFS and explain why the visited order looks deeper and less balanced.
4. Run Dijkstra and explain how accumulated cost changes the search behavior.
5. Run A\* and explain how the heuristic guides the search toward the goal.
6. Add weighted cells with the mouse.
7. Compare BFS with Dijkstra and A\* on the weighted map.
8. Use the result panel to compare visited count, path length, path cost, and time.
9. Explain the data structures used by each algorithm.
10. Connect the implementation to time and space complexity.

This sequence demonstrates both the visual features and the algorithmic meaning of the project.

---

## 22. Troubleshooting

### The project does not compile

Use the openFrameworks Project Generator to update the project files. Make sure the project folder is inside `openFrameworks/apps/myApps`.

### The program cannot find maze files

Check that the text files are placed inside:

```text
bin/data/
```

If the files are missing, the program should still create a fallback empty maze, but the sample loading feature will not show the intended examples.

### The window opens but the maze looks too small or too large

Resize the window. The program recalculates the cell size and layout when the window size changes.

### The animation is too fast or too slow

Use `+` or `=` to make the animation faster. Use `-` or `_` to make it slower.

### Mouse editing changes a cell unexpectedly

Check the current edit mode on the right-side panel. Press `M` to cycle through Wall Toggle, Move Start, Move Goal, and Weight Toggle.

---

## 23. Project Strengths

This project goes beyond a basic maze solver because it includes multiple algorithms, visual animation, weighted pathfinding, interactive editing, result statistics, and a clear educational interface. It is suitable for explaining how abstract graph algorithms are implemented in an actual C++ program.

The strongest part of the project is that it allows direct comparison. A viewer can see that different algorithms may visit different areas of the maze, find different kinds of paths, and produce different costs. This makes the project effective for both final project evaluation and algorithm learning.

---

## 24. Final Submission Checklist

Before submitting or presenting the project, check the following:

- The project folder is placed inside the openFrameworks `apps/myApps` directory.
- Project files have been updated with the openFrameworks Project Generator.
- The program builds successfully.
- The application window opens correctly.
- Keys `1`, `2`, `3`, and `4` run BFS, DFS, Dijkstra, and A\*.
- Mouse edit mode works correctly.
- Weighted cells can be created and tested.
- The side panel displays result statistics.
- The README file is included.
- Source code comments explain variables, functions, loops, and algorithms.
- A demonstration video shows both the running program and the algorithm comparisons.

---

## 25. Summary

Maze Pathfinder Lab is an interactive C++/openFrameworks visualization tool for understanding and comparing pathfinding algorithms. It provides a practical demonstration of graph representation, traversal data structures, shortest-path search, heuristic search, animation, user interaction, and algorithm analysis. By combining BFS, DFS, Dijkstra, and A\* in one editable maze environment, the project gives viewers a clear and detailed way to understand how pathfinding algorithms behave in practice.
