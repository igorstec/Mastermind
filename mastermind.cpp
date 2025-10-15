#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <sstream>
using namespace std;

/*
 * mastermind.cpp
 *
 * Author: Igor Stec
 * Date: 2025-10-14
 * Description: Interactive Mastermind game in codebreaker/codemaker modes.
 *
 */

// Parse string into integers, optionally check expected count.
bool parseNumbers(const string &line, vector<int> &out, int expectedCount = -1) {
    istringstream iss(line);
    out.clear();
    int num;
    while (iss >> num) out.push_back(num);
    if (!iss.eof()) return false;
    if (expectedCount != -1 && (int) out.size() != expectedCount) return false;
    return !out.empty();
}

// Validate Mastermind parameter constraints.
bool validateConstraints(int k, int n) {
    if (k < 2 || k > 256 || n < 2 || n > 10) return false;
    int squereTimes = n;
    long long k_n = 1, mnoznik = k;
    while (squereTimes > 0) {
        if (squereTimes % 2 == 1) {
            k_n *= mnoznik;
            if (k_n > (1LL << 24)) return false;
        }
        mnoznik *= mnoznik;
        squereTimes /= 2;
    }
    return true;
}


// Calculate black and white pegs for a guess against a secret.
pair<int, int> calculateResponse(const vector<int> &secret, const vector<int> &guess) {
    int n = secret.size(), black = 0, white = 0;
    vector<int> secretCount(256, 0), guessCount(256, 0);
    for (int i = 0; i < n; i++) {
        if (secret[i] == guess[i]) black++;
        else {
            secretCount[secret[i]]++;
            guessCount[guess[i]]++;
        }
    }
    for (int color = 0; color < 256; color++)
        white += min(secretCount[color], guessCount[color]);
    return {black, white};
}

// Print guess vector.
void outputGuess(const vector<int> &guess) {
    for (size_t i = 0; i < guess.size(); i++) {
        if (i) cout << " ";
        cout << guess[i];
    }
    cout << endl;
}

// Read response pair (black, white). Returns {-1, -1} on error.
pair<int, int> readResponse(int n) {
    string line;
    vector<int> res;
    if (!getline(cin, line)) return {-1, -1};
    if (!parseNumbers(line, res, 2)) return {-1, -1};
    int black = res[0], white = res[1];
    if (black < 0 || white < 0 || black + white > n || black > n || white > n)
        return {-1, -1};
    return {black, white};
}

// Ask for a guess and get feedback.
bool guessAnswer(int n, vector<int> &guess, pair<int, int> &response) {
    outputGuess(guess);
    response = readResponse(n);
    if (response.first == -1 && response.second == -1)
        return false;
    return true;
}

// Find unique colors via testing each color individually.
// Returns false if not enough unique colors found.
bool lookForUniqueColours(int &k, int n, set<int> &uniqueColours, int &firstColorNumber) {
    int counter = 0;
    for (int i = 0; i < k; i++) {
        vector<int> guess(n, i);
        pair<int, int> response;
        if (!guessAnswer(n, guess, response))
            return false;
        if (response.first == n) {
            k = 1;
            return true;
        }
        if (response.second > 0) return false;
        if (response.first != 0) {
            uniqueColours.insert(i);
            if (counter == 0) firstColorNumber = response.first;
        }
        counter += response.first;
        if (counter >= n) break;
    }
    if (counter != n) return false;
    return true;
}

// Validate a guess.
bool isValidGuess(const vector<int> &guess, int k, int n) {
    if (guess.size() != (size_t) n) return false;
    for (int color: guess) {
        if (color < 0 || color >= k) return false;
    }
    return true;
}

// Unified argument parsing and validation.
bool initializeGame(int argc, char *argv[], int &k, int &n, vector<int> &secret) {
    try {
        k = stoi(argv[1]);
        n = (argc == 3) ? stoi(argv[2]) : argc - 2;
    } catch (...) { return false; }
    if (!validateConstraints(k, n)) return false;

    // Parse secret if provided (codemaker mode).
    if (argc > 3) {
        secret.clear();
        for (int i = 2; i < argc; i++) {
            try {
                int color = stoi(argv[i]);
                if (color < 0 || color >= k) return false;
                secret.push_back(color);
            } catch (...) { return false; }
        }
        if (secret.size() != (size_t) n) return false;
    }
    return true;
}

// --- Main ---
int main(int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "ERROR" << endl;
        return 1;
    }

    int k, n;
    vector<int> secret;
    // Catch invalid arguments or secret.
    if (!initializeGame(argc, argv, k, n, secret)) {
        cerr << "ERROR" << endl;
        return 1;
    }

    if (argc == 3) {
        // --- Codebreaker mode ---
        set<int> uniqueColors;
        vector<int> answer(n, -1);
        int firstColorNumber = 1;

        if (!lookForUniqueColours(k, n, uniqueColors, firstColorNumber)) {
            cerr << "ERROR" << endl;
            return 1;
        }
        if (k == 1) { return 0; }
        for (int pos = 0; pos < n; ++pos) {
            vector<int> guess(n, *uniqueColors.begin());
            for (int color: uniqueColors) {
                if (color == *uniqueColors.begin()) continue;
                guess[pos] = color;
                pair<int, int> response;
                if (!guessAnswer(n, guess, response)) {
                    cerr << "ERROR" << endl;
                    return 1;
                }
                if (response.first == n) {
                    return 0;
                }
                if (response.first > firstColorNumber) {
                    answer[pos] = color;
                    break;
                }
                if (response.first < firstColorNumber) {
                    answer[pos] = *uniqueColors.begin();
                    break;
                }
            }
            if (answer[pos] == -1) {
                cerr << "ERROR" << endl;
                return 1;
            }
        }
        outputGuess(answer);
        auto [black, white] = readResponse(n);
        if (black != n || white != 0) {
            cerr << "ERROR" << endl;
            return 1;
        }
    } else {
        // --- Codemaker mode ---
        string line;
        while (getline(cin, line)) {
            vector<int> guess;
            if (!parseNumbers(line, guess) || !isValidGuess(guess, k, n)) {
                cerr << "ERROR" << endl;
                return 1;
            }

            auto [black, white] = calculateResponse(secret, guess);
            cout << black << " " << white << endl;
            if (black == n && white == 0) return 0;
        }
    }
    return 0;
}
