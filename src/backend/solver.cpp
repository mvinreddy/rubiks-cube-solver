#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

// Cube mapping representation: 6 faces * 9 stickers = 54 index positions
// Face Order tracking: U(0-8), D(9-17), R(18-26), L(27-35), F(36-44), B(45-53)
struct RubiksCube {
    char state[54]; 

    // Basic representation helper to check if completely solved
    bool isSolved() {
        for (int i = 0; i < 6; ++i) {
            char faceColor = state[i * 9];
            for (int j = 0; j < 9; ++j) {
                if (state[i * 9 + j] != faceColor) return false;
            }
        }
        return true;
    }

    // Rotates the 4 bounding border sticker values when a face turns clockwise
    void rotateFaceClockwise(int faceIdx) {
        int base = faceIdx * 9;
        char temp[9];
        for(int i=0; i<9; i++) temp[i] = state[base + i];
        
        // Matrix transpose and row reverse mapping
        state[base + 0] = temp[6]; state[base + 1] = temp[3]; state[base + 2] = temp[0];
        state[base + 3] = temp[7];                               state[base + 5] = temp[1];
        state[base + 6] = temp[8]; state[base + 7] = temp[5]; state[base + 8] = temp[2];
    }

    void applyMove(const std::string& move) {
        if (move == "U") {
        rotateFaceClockwise(0);
        // Cycle the top rows of the adjacent faces: F -> L -> B -> R
        char temp[3] = {state[36], state[37], state[38]}; // Save Front top row
        for(int i=0; i<3; i++) {
            state[36+i] = state[18+i]; // Right to Front
            state[18+i] = state[45+i]; // Back to Right
            state[45+i] = state[27+i]; // Left to Back
            state[27+i] = temp[i];     // Saved Front to Left
        }
    }
    else if (move == "D") {
        rotateFaceClockwise(1);
        // Cycle the bottom rows of the adjacent faces: F -> R -> B -> L
        char temp[3] = {state[42], state[43], state[44]};
        for(int i=0; i<3; i++) {
            state[42+i] = state[27+6+i];
            state[27+6+i] = state[45+6+i];
            state[45+6+i] = state[18+6+i];
            state[18+6+i] = temp[i];
        }
    }
    // Implement remaining variations for R, L, F, B following similar geometric tracks...
    }
};

// Simple Manhattan-distance styled heuristic or lower-bound calculation for IDA*
int getHeuristicEstimate(RubiksCube& cube) {
    if (cube.isSolved()) return 0;
    // Lowerbound placeholder: if not solved, it requires at least 1 move
    return 1; 
}

// Bounded Depth First Search
bool dfsSearch(RubiksCube& cube, int depth, int g, std::vector<std::string>& path, std::string lastMove) {
    int h = getHeuristicEstimate(cube);
    if (g + h > depth) return false;
    if (cube.isSolved()) return true;

    // Pruned standard half-turn metric moveset
    std::vector<std::string> moves = {"U", "R", "F", "D", "L", "B"};
    
    for (const auto& move : moves) {
        if (!lastMove.empty() && move[0] == lastMove[0]) continue; // Avoid redundant face axis turns

        RubiksCube nextCube = cube;
        nextCube.applyMove(move);
        path.push_back(move);

        if (dfsSearch(nextCube, depth, g + 1, path, move)) return true;

        path.pop_back(); // Backtrack branching
    }
    return false;
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    const char* solveCubeIDAStar(const char* initialState) {
        RubiksCube cube;
        for(int i = 0; i < 54; i++) cube.state[i] = initialState[i];

        std::vector<std::string> path;
        int depth = 0;
        
        // Iterative Deepening Loop threshold limits
        while (depth <= 20) { 
            if (dfsSearch(cube, depth, 0, path, "")) {
                static std::string result;
                result = "";
                if(path.empty()) return "Solved";
                for (const auto& m : path) result += m + " ";
                return result.c_str(); 
            }
            depth++;
        }
        return "Unsolvable";
    }
}