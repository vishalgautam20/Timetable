#include "../include/timetable_generator.h"
#include <iostream>
#include <limits>
#include <algorithm>

using namespace std;

void TimetableGenerator::get_input() {
    cout << "\n===== COLLEGE TIMETABLE GENERATOR =====" << endl;
    get_college_time();
    get_room_info();
    get_streams_info();
    get_slot_configuration();
    get_lunch_configuration();
}

void TimetableGenerator::get_streams_info() {
    int num_streams;
    while (true) {
        cout << "\nHow many streams? ";
        if (cin >> num_streams && num_streams > 0) break;
        handle_invalid_input();
    }
    cin.ignore();

    stream_names.resize(num_streams);
    sections_per_stream.resize(num_streams);
    subjects.resize(num_streams);
    stream_labs.resize(num_streams);
    subject_count.resize(num_streams);
    timetables.resize(num_streams);

    for (int i = 0; i < num_streams; i++) {
        cout << "\n=== STREAM " << i + 1 << " ===" << endl;
        cout << "Enter stream name: ";
        getline(cin, stream_names[i]);
        
        while (true) {
            cout << "How many sections for " << stream_names[i] << "? ";
            if (cin >> sections_per_stream[i] && sections_per_stream[i] > 0) break;
            handle_invalid_input();
        }
        cin.ignore();
        
        get_subjects_info(i);
        get_lab_info(i);
    }
}

void TimetableGenerator::get_lab_info(int stream_idx) {
    char has_lab;
    cout << "\nDoes " << stream_names[stream_idx] << " have lab sessions? (y/n): ";
    cin >> has_lab;
    cin.ignore();

    if (tolower(has_lab) == 'y') {
        int num_labs;
        while (true) {
            cout << "How many labs? ";
            if (cin >> num_labs && num_labs > 0) break;
            handle_invalid_input();
        }
        cin.ignore();

        stream_labs[stream_idx].resize(num_labs);
        for (int j = 0; j < num_labs; j++) {
            cout << "\nLab " << j + 1 << " details:" << endl;
            cout << "Name: ";
            getline(cin, stream_labs[stream_idx][j].name);
            if (stream_labs[stream_idx][j].name.empty()) {
                stream_labs[stream_idx][j].name = "LAB_" + stream_names[stream_idx] + "_" + to_string(j+1);
            }
            cout << "Lab Center Name: ";
            getline(cin, stream_labs[stream_idx][j].center);
            cout << "Lab Instructor: ";
            getline(cin, stream_labs[stream_idx][j].instructor);

            while (true) {
                cout << "Duration (hours): ";
                float hours;
                if (cin >> hours && hours > 0) {
                    stream_labs[stream_idx][j].duration = static_cast<int>(hours * 60);
                    break;
                }
                handle_invalid_input();
            }
            cin.ignore();
        }
    }
}

void TimetableGenerator::get_subjects_info(int stream_idx) {
    string prompt = "\nEnter number of subjects for " + stream_names[stream_idx] + 
                   " (max " + to_string(MAX_SUBJECTS) + "): ";
        
    int subj_count;
    while (true) {
        cout << prompt;
        if (cin >> subj_count && subj_count > 0 && subj_count <= MAX_SUBJECTS) break;
        handle_invalid_input();
    }
    cin.ignore();

    subject_count[stream_idx] = subj_count;
    subjects[stream_idx].resize(subj_count);
    
    for (int i = 0; i < subj_count; i++) {
        cout << "\nSubject " << i + 1 << " details:" << endl;
        cout << "Name: ";
        getline(cin, subjects[stream_idx][i].name);
        if (subjects[stream_idx][i].name.empty()) {
            subjects[stream_idx][i].name = "SUBJ_" + to_string(i+1);
        }
        cout << "Teacher: ";
        getline(cin, subjects[stream_idx][i].teacher);
        if (subjects[stream_idx][i].teacher.empty()) {
            subjects[stream_idx][i].teacher = "TEACHER_" + to_string(i+1);
        }
    }
}

void TimetableGenerator::get_college_time() {
    string prompt = "At what time should classes begin? (8-12): ";
    while (true) {
        cout << prompt;
        if (cin >> start_hour && start_hour >= 8 && start_hour <= 12) break;
        handle_invalid_input();
    }
    cin.ignore();
}

void TimetableGenerator::get_room_info() {
    string prompt = "How many classrooms are available? ";
    while (true) {
        cout << prompt;
        if (cin >> total_rooms && total_rooms > 0) break;
        handle_invalid_input();
    }
    cin.ignore();
}

void TimetableGenerator::get_slot_configuration() {
    cout << "\n=== TEACHING SLOTS CONFIGURATION ===" << endl;
    while (true) {
        cout << "How many teaching slots per day? ";
        if (cin >> total_teaching_slots && total_teaching_slots > 0) break;
        handle_invalid_input();
    }
    cin.ignore();

    slot_durations.resize(total_teaching_slots);
    for (int i = 0; i < total_teaching_slots; i++) {
        while (true) {
            cout << "Duration (minutes) for teaching slot " << i + 1 << ": ";
            if (cin >> slot_durations[i] && slot_durations[i] > 0) break;
            handle_invalid_input();
        }
        cin.ignore();
    }
}

void TimetableGenerator::get_lunch_configuration() {
    cout << "\n=== LUNCH BREAK CONFIGURATION ===" << endl;
    
    string default_time = "13:00";
    
    while (true) {
        cout << "At what time should lunch break start? (HH:MM, default " << default_time << "): ";
        string lunch_time;
        getline(cin, lunch_time);
        
        if (lunch_time.empty()) lunch_time = default_time;
        
        size_t colon_pos = lunch_time.find(':');
        if (colon_pos != string::npos) {
            try {
                int lunch_hour = stoi(lunch_time.substr(0, colon_pos));
                int lunch_min = stoi(lunch_time.substr(colon_pos + 1));
                if (lunch_hour >= 0 && lunch_hour <= 23 && lunch_min >= 0 && lunch_min <= 59) {
                    lunch_break_time = lunch_hour * 60 + lunch_min;
                    break;
                }
            } catch (...) {}
        }
        cout << "Invalid time format. Please use HH:MM format.\n";
    }

    while (true) {
        cout << "Duration of lunch break (minutes, default " << lunch_break_duration << "): ";
        string input;
        getline(cin, input);
        
        if (!input.empty()) {
            try {
                int duration = stoi(input);
                if (duration > 0) {
                    lunch_break_duration = duration;
                    break;
                }
            } catch (...) {}
            cout << "Invalid duration. Please enter a positive number.\n";
        } else {
            break;
        }
    }
}

void TimetableGenerator::handle_invalid_input() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input. Please try again.\n";
}
