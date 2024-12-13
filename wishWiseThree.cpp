#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <unordered_map>

using namespace std;

// Utility function to split a string by a delimiter
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

class DataLoader {
public:
    static vector<vector<int>> loadRatingsMatrix(const string &filePath) {
        vector<vector<int>> matrix;
        ifstream file(filePath);
        string line;

        while (getline(file, line)) {
            vector<string> tokens = split(line, ',');
            vector<int> row;
            for (const string &token : tokens) {
                row.push_back(stoi(token));
            }
            matrix.push_back(row);
        }

        return matrix;
    }

    static void printMatrix(const vector<vector<int>> &matrix) {
        cout << "Ratings Matrix:\n";
        for (const auto &row : matrix) {
            for (int rating : row) {
                cout << rating << " ";
            }
            cout << "\n";
        }
    }
};

class SimilarityCalculator {
public:
    static double calculateCosineSimilarity(const vector<int> &user1, const vector<int> &user2) {
        double dotProduct = 0.0, norm1 = 0.0, norm2 = 0.0;
        for (size_t i = 0; i < user1.size(); i++) {
            dotProduct += user1[i] * user2[i];
            norm1 += user1[i] * user1[i];
            norm2 += user2[i] * user2[i];
        }
        if (norm1 == 0 || norm2 == 0) return 0; // Avoid division by zero
        return dotProduct / (sqrt(norm1) * sqrt(norm2));
    }
};

class RecommenderSystem {
private:
    vector<vector<int>> ratingsMatrix;

public:
    explicit RecommenderSystem(const vector<vector<int>> &matrix) : ratingsMatrix(matrix) {}

    vector<pair<int, double>> predictRatings(int userIndex) {
        vector<pair<int, double>> predictedRatings;
        size_t movieCount = ratingsMatrix[0].size();
        size_t userCount = ratingsMatrix.size();

        for (size_t movie = 0; movie < movieCount; ++movie) {
            if (ratingsMatrix[userIndex][movie] != 0) continue; // Skip already rated movies

            double weightedSum = 0.0, similaritySum = 0.0;
            for (size_t otherUser = 0; otherUser < userCount; ++otherUser) {
                if (otherUser == userIndex || ratingsMatrix[otherUser][movie] == 0) continue;

                double similarity = SimilarityCalculator::calculateCosineSimilarity(
                    ratingsMatrix[userIndex], ratingsMatrix[otherUser]);

                weightedSum += similarity * ratingsMatrix[otherUser][movie];
                similaritySum += abs(similarity);
            }

            double predictedRating = (similaritySum == 0) ? 0 : weightedSum / similaritySum;
            predictedRatings.emplace_back(movie, predictedRating);
        }

        sort(predictedRatings.begin(), predictedRatings.end(), [](const pair<int, double> &a, const pair<int, double> &b) {
            return a.second > b.second;
        });

        return predictedRatings;
    }

    void printPredictedRatings(int userIndex, const vector<pair<int, double>> &predictedRatings) {
        cout << "Predicted Ratings for User " << userIndex + 1 << ":\n";
        for (const auto &pred : predictedRatings) {
            cout << "  Movie " << pred.first + 1 << " predicted rating: " << fixed << setprecision(2) << pred.second << "\n";
        }
    }

    void suggestTopNMovies(int userIndex, const vector<pair<int, double>> &predictedRatings, int topN) {
        cout << "Top " << topN << " Recommended Movies for User " << userIndex + 1 << ":\n";
        for (int i = 0; i < min(topN, static_cast<int>(predictedRatings.size())); ++i) {
            cout << "  Movie " << predictedRatings[i].first + 1 << " with predicted rating: " << fixed << setprecision(2) << predictedRatings[i].second << "\n";
        }
    }

    void suggestTopNMoviesOverall(const vector<vector<pair<int, double>>> &allPredictedRatings, int topN) {
        unordered_map<int, pair<double, int>> movieAggregates; // movieId -> {totalRating, count}

        for (const auto &userPredictions : allPredictedRatings) {
            for (const auto &pred : userPredictions) {
                int movieId = pred.first;
                double rating = pred.second;
                movieAggregates[movieId].first += rating;
                movieAggregates[movieId].second++;
            }
        }

        vector<pair<int, double>> averagedRatings;
        for (const auto &entry : movieAggregates) {
            int movieId = entry.first;
            double averageRating = entry.second.first / entry.second.second;
            averagedRatings.emplace_back(movieId, averageRating);
        }

        sort(averagedRatings.begin(), averagedRatings.end(), [](const pair<int, double> &a, const pair<int, double> &b) {
            return a.second > b.second;
        });

        cout << "Top " << topN << " Movies Overall:\n";
        for (int i = 0; i < min(topN, static_cast<int>(averagedRatings.size())); ++i) {
            cout << "  Movie " << averagedRatings[i].first + 1 << " with average predicted rating: " << fixed << setprecision(2) << averagedRatings[i].second << "\n";
        }
    }
};

int main() {
    string filePath = "ratings.csv"; // Replace with the actual file path

    // Step 1: Load and print the ratings matrix
    vector<vector<int>> ratingsMatrix = DataLoader::loadRatingsMatrix(filePath);
    DataLoader::printMatrix(ratingsMatrix);

    // Step 2: Initialize recommender system
    RecommenderSystem recommender(ratingsMatrix);

    // Step 3: Predict ratings for each user and store results
    vector<vector<pair<int, double>>> allPredictedRatings;
    for (size_t userIndex = 0; userIndex < ratingsMatrix.size(); ++userIndex) {
        vector<pair<int, double>> predictedRatings = recommender.predictRatings(userIndex);
        allPredictedRatings.push_back(predictedRatings);
        recommender.printPredictedRatings(userIndex, predictedRatings);

        // Suggest top N movies for the user
        int topN = 3; // Customize the number of recommendations per user
        recommender.suggestTopNMovies(userIndex, predictedRatings, topN);
        cout << "\n";
    }

    // Step 4: Suggest top N movies overall
    int topNOverall = 5; // Customize the number of overall recommendations
    recommender.suggestTopNMoviesOverall(allPredictedRatings, topNOverall);

    return 0;
}

