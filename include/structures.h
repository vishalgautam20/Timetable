#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <string>
#include <vector>

using namespace std;

// Forward declarations
struct Subject;
struct Lab;
struct Timetable;

// Define Subject first since it's used by others
struct Subject {
    string name;
    string teacher;
    int credits;
    bool is_elective;
};

// Define Lab next
struct Lab {
    string name;
    string center;
    string instructor;
    int duration;
    int day = -1;
    int start_slot = -1;
};

// Define Timetable
struct Timetable {
    vector<vector<int>> subject_id;
    vector<vector<string>> teacher;
    vector<vector<string>> room;
    vector<vector<bool>> is_lab;
};

#endif
