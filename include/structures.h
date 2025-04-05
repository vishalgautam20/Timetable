#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <string>
#include <vector>

using namespace std;

struct Subject {
    string name;
    string teacher;
};

struct Lab {
    string name;
    string center;
    string instructor;
    int duration;
    int day = -1;
    int start_slot = -1;
};

struct Timetable {
    vector<vector<int>> subject_id;
    vector<vector<string>> teacher;
    vector<vector<string>> room;
    vector<vector<bool>> is_lab;
};

#endif
