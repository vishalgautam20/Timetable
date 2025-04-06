#include "../include/timetable_generator.h"
#include <iostream>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include "../include/utils.h"

using namespace std;

string format_time(int minutes) {
    int hours = minutes / 60;
    int mins = minutes % 60;
    stringstream ss;
    ss << setfill('0') << setw(2) << hours << ":" 
       << setfill('0') << setw(2) << mins;
    return ss.str();
}


void TimetableGenerator::run() {
    try {
        initialize();
        generate_timetables();
        save_to_csv_column_wise();
        save_to_txt_column_wise();   // Full timetable
        save_individual_section_timetables();  
        generate_teacher_timetables();
        
        cout << "\nTimetables saved to:\n";
        cout << "- 'timetables_column_wise.csv' (spreadsheet format)\n";
        cout << "- 'timetables_column_wise.txt' (full timetable)\n";
        cout << "- Individual section timetables (e.g., cse_sectionA.txt)\n";
        cout << "- 'teacher_timetables.txt' (teacher schedules)\n";
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        cout << "Would you like to try again? (y/n): ";
        char response;
        cin >> response;
        if (tolower(response) == 'y') {
            run();
        } else {
            cout << "Exiting program...\n";
            exit(1);
        }
    }
}

void TimetableGenerator::initialize() {
    srand(static_cast<unsigned int>(time(nullptr)));
    get_input();
    validate_inputs();
    initialize_data_structures();
}

void TimetableGenerator::validate_inputs() {
    if (stream_names.empty() || total_teaching_slots <= 0) {
        throw runtime_error("Invalid configuration: no streams/classes or slots");
    }
    if (lunch_break_time < 0) {
        throw runtime_error("Invalid lunch break time");
    }
    
    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        if (subject_count[stream] == 0) {
            cerr << "Warning: No subjects defined for " << stream_names[stream] << endl;
        }
    }
}

void TimetableGenerator::initialize_data_structures() {
    int total_slots = total_teaching_slots + 1;
    
    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        timetables[stream].resize(sections_per_stream[stream]);
        
        for (int section = 0; section < sections_per_stream[stream]; section++) {
            auto& tt = timetables[stream][section];
            tt.subject_id.resize(DAY_NAMES.size(), vector<int>(total_slots, -1));
            tt.teacher.resize(DAY_NAMES.size(), vector<string>(total_slots, ""));
            tt.room.resize(DAY_NAMES.size(), vector<string>(total_slots, ""));
            tt.is_lab.resize(DAY_NAMES.size(), vector<bool>(total_slots, false));
        }
    }
}

void TimetableGenerator::generate_timetables() {
    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        for (int section = 0; section < sections_per_stream[stream]; section++) {
            generate_timetable(timetables[stream][section], stream);
        }
    }
}

void TimetableGenerator::generate_timetable(Timetable &tt, int stream_idx) {
    calculate_lunch_slot();
    
    teacher_schedule.clear();
    room_schedule.clear();
    
    schedule_labs(tt, stream_idx);
    assign_regular_subjects(tt, stream_idx);
    validate_timetable(tt, stream_idx);
}

void TimetableGenerator::calculate_lunch_slot() {
    int current_time = start_hour * 60;
    lunch_break_slot = -1;
    
    for (int slot = 0; slot < total_teaching_slots; slot++) {
        int slot_end_time = current_time + slot_durations[slot];
        
        if (current_time <= lunch_break_time && lunch_break_time < slot_end_time) {
            lunch_break_slot = slot;
            break;
        }
        current_time = slot_end_time + break_duration;
    }
}

bool TimetableGenerator::can_place_subject(int day, int slot, const string& teacher, const string& room) {
    // Check if teacher is already scheduled for this slot across all streams and sections
    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        for (int section = 0; section < sections_per_stream[stream]; section++) {
            const auto& tt = timetables[stream][section];
            if (!tt.teacher[day][slot].empty() && tt.teacher[day][slot] == teacher) {
                return false;
            }
            if (!tt.room[day][slot].empty() && tt.room[day][slot] == room) {
                return false;
            }
        }
    }
    
    // Check teacher schedule
    if (teacher_schedule[teacher][day].count(slot)) {
        return false;
    }
    
    // Check room schedule  
    if (room_schedule[room][day].count(slot)) {
        return false;
    }
    
    return true;
}

void TimetableGenerator::record_assignment(int day, int slot, const string& teacher, const string& room) {
    teacher_schedule[teacher][day].insert(slot);
    room_schedule[room][day].insert(slot);
}

void TimetableGenerator::assign_regular_subjects(Timetable &tt, int stream_idx) {
    if (stream_idx >= static_cast<int>(subject_count.size()))
        {
        cerr << "Warning: Invalid stream index in assign_regular_subjects\n";
        return;
    }
    
    vector<int> subject_assigned(subject_count[stream_idx], 0);
    int total_subjects = subject_count[stream_idx];
    
    if (total_subjects == 0) {
        cerr << "Warning: No subjects defined for stream " << stream_names[stream_idx] << endl;
        return;
    }

    for (int subject_idx = 0; subject_idx < total_subjects; subject_idx++) {
        int attempts = 0;
        while (subject_assigned[subject_idx] < MAX_ASSIGNMENTS && attempts < 100) {
            attempts++;
            int day = rand() % DAY_NAMES.size();
            int slot = rand() % total_teaching_slots;

            if (slot == lunch_break_slot) continue;
            if (tt.subject_id[day][slot] != -1) continue;
            if (subject_already_on_day(tt, day, subject_idx)) continue;

            const string& teacher = subjects[stream_idx][subject_idx].teacher;
            string room = "Room " + to_string(1 + (rand() % total_rooms));  // Now rooms will be 1 to n
            
            bool teacher_available = true;
            // Check teacher availability
            for (size_t stream = 0; stream < stream_names.size(); stream++) {
                for (int section = 0; section < sections_per_stream[stream]; section++) {
                    const auto& other_tt = timetables[stream][section];
                    if (other_tt.teacher[day][slot] == teacher) {
                        teacher_available = false;
                        break;
                    }
                }
                if (!teacher_available) break;
            }
            
            if (!teacher_available || !can_place_subject(day, slot, teacher, room)) {
                continue;
            }

            tt.subject_id[day][slot] = subject_idx;
            tt.teacher[day][slot] = teacher;
            tt.room[day][slot] = room;
            record_assignment(day, slot, teacher, room);
            subject_assigned[subject_idx]++;
        }
        
        if (subject_assigned[subject_idx] < MAX_ASSIGNMENTS) {
            cerr << "Warning: Could only assign subject " << subjects[stream_idx][subject_idx].name
                 << " " << subject_assigned[subject_idx] << " times (target: " << MAX_ASSIGNMENTS << ")\n";
        }
    }
}

void TimetableGenerator::schedule_labs(Timetable &tt, int stream_idx) {
    if (stream_idx >= static_cast<int>(stream_labs.size())) {
        cerr << "Warning: Invalid stream index in schedule_labs\n";
        return;
    }

    for (Lab &lab : stream_labs[stream_idx]) {
        // Generate a default name if empty
        if (lab.name.empty()) {
            lab.name = "LAB_" + stream_names[stream_idx] + "_" + 
                      to_string(&lab - &stream_labs[stream_idx][0] + 1);
        }

        if (!place_lab(lab, tt)) {
            cerr << "Warning: Could not schedule lab " << lab.name << endl;
        }
    }
}

bool TimetableGenerator::place_lab(Lab &lab, Timetable &tt) {
    vector<int> days(DAY_NAMES.size());
    iota(days.begin(), days.end(), 0);
    random_shuffle(days.begin(), days.end());

    int slots_needed = max(1, lab.duration / slot_durations[0]);
    
    for (int day : days) {
        // Try different start times throughout the day
        vector<int> possible_starts;
        for (int i = 0; i <= total_teaching_slots - slots_needed; i++) {
            possible_starts.push_back(i);
        }
        random_shuffle(possible_starts.begin(), possible_starts.end());

        for (int start : possible_starts) {
            if (can_place_lab(day, start, slots_needed, tt)) {
                bool conflict = false;
                // Check for teacher and room conflicts
                for (int s = start; s < start + slots_needed; s++) {
                    if (!can_place_subject(day, s, lab.instructor, lab.center)) {
                        conflict = true;
                        break;
                    }
                }
                if (conflict) continue;

                // Place the lab
                lab.day = day;
                lab.start_slot = start;

                for (int s = start; s < start + slots_needed; s++) {
                    tt.subject_id[day][s] = -3;  // Special marker for labs
                    tt.teacher[day][s] = lab.instructor;
                    tt.room[day][s] = lab.center;
                    tt.is_lab[day][s] = true;
                    record_assignment(day, s, lab.instructor, lab.center);
                }
                return true;
            }
        }
    }
    return false;
}

bool TimetableGenerator::can_place_lab(int day, int start_slot, int slots_needed, const Timetable &tt) {
    if (day < 0 || day >= static_cast<int>(DAY_NAMES.size())) return false;
    if (start_slot < 0 || start_slot + slots_needed > total_teaching_slots) return false;
    
    // Check for lunch break and existing assignments
    for (int s = start_slot; s < start_slot + slots_needed; s++) {
        if (s == lunch_break_slot || tt.subject_id[day][s] != -1) {
            return false;
        }
    }

    // Check for other labs scheduled at this time
    for (const auto& schedule : room_schedule) {
        if (schedule.second.count(day)) {
            for (int s = start_slot; s < start_slot + slots_needed; s++) {
                if (schedule.second.at(day).count(s)) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool TimetableGenerator::subject_already_on_day(const Timetable &tt, int day, int subject_idx) {
    for (int s = 0; s < total_teaching_slots; s++) {
        if (tt.subject_id[day][s] == subject_idx) {
            return true;
        }
    }
    return false;
}

void TimetableGenerator::validate_timetable(const Timetable &tt, int stream_idx) {
    for (size_t day = 0; day < DAY_NAMES.size(); day++) {
        bool has_class = false;
        for (int slot = 0; slot < total_teaching_slots; slot++) {
            if (slot == lunch_break_slot) continue;
            if (tt.subject_id[day][slot] != -1) {
                has_class = true;
                break;
            }
        }
        if (!has_class) {
            cerr << "Warning: No classes scheduled for " << stream_names[stream_idx]
                 << " on " << DAY_NAMES[day] << endl;
        }
    }
}

string TimetableGenerator::find_lab_name(int stream, int day) {
    if (stream < 0 || stream >= static_cast<int>(stream_labs.size())) {
        return "UNKNOWN LAB";
    }

    for (const Lab &lab : stream_labs[stream]) {
        // Check if lab slot matches the day and also check if lab has a name
        if (lab.day == day) {
            if (!lab.name.empty()) {
                return lab.name;
            }
        }
    }

    // If no matching lab found or lab name is empty, create a default name with stream and section
    string default_name = stream_names[stream] + "_LAB";
    
    // If there are labs configured for this stream, use the first lab's details
    if (!stream_labs[stream].empty()) {
        const Lab& first_lab = stream_labs[stream][0];
        if (!first_lab.name.empty()) {
            return first_lab.name;
        }
    }
    
    return default_name;
}
