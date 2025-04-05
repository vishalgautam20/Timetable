#ifndef TIMETABLE_GENERATOR_H
#define TIMETABLE_GENERATOR_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include "constants.h"
#include "structures.h"
#include "utils.h"

using namespace std;

class TimetableGenerator {
private:
    vector<vector<Subject>> subjects;
    vector<vector<Lab>> stream_labs;
    vector<int> subject_count;
    vector<vector<Timetable>> timetables;
    vector<string> stream_names;
    vector<int> sections_per_stream;
    vector<int> slot_durations;
    
    int lunch_break_slot = -1;
    int lunch_break_duration = 60;
    int lunch_break_time = 13 * 60;
    int total_teaching_slots = 4;
    int start_hour = 9;
    int total_rooms = 20;
    const int break_duration = 5;

    map<string, map<int, set<int>>> teacher_schedule;
    map<string, map<int, set<int>>> room_schedule;

    // Input handling
    void get_input();
    void get_streams_info();
    void get_lab_info(int stream_idx);
    void get_subjects_info(int stream_idx);
    void get_college_time();
    void get_room_info();
    void get_slot_configuration();
    void get_lunch_configuration();
    void handle_invalid_input();

    // Core functionality
    void generate_timetables();
    void generate_timetable(Timetable &tt, int stream_idx);
    void calculate_lunch_slot();
    bool can_place_subject(int day, int slot, const string& teacher, const string& room);
    void record_assignment(int day, int slot, const string& teacher, const string& room);
    void assign_regular_subjects(Timetable &tt, int stream_idx);
    void schedule_labs(Timetable &tt, int stream_idx);
    bool place_lab(Lab &lab, Timetable &tt);
    bool can_place_lab(int day, int start_slot, int slots_needed, const Timetable &tt);
    bool subject_already_on_day(const Timetable &tt, int day, int subject_idx);
    void validate_timetable(const Timetable &tt, int stream_idx);
    string find_lab_name(int stream, int day);

    // Output formatting
    void save_to_csv_column_wise();
    void save_to_txt_column_wise();
    void generate_teacher_timetables();
    void print_legend(ofstream& file);
    void print_summary(ofstream& file, const Timetable& tt);
    string format_time_range(int start_time, int current_slot);  // Added current_slot parameter
    string format_class_entry(const Timetable &tt, int day, int slot, int stream);
    string center_text(const string& text, int width);

public:
    void run();
    void initialize();
    void validate_inputs();
    void initialize_data_structures();
};

#endif
