#include "../include/timetable_generator.h"
#include <fstream>
#include <iomanip>
#include <sstream>

// Add these color definitions at the top
const string RESET = "\033[0m";
const string HEADER_COLOR = "\033[1;36m";  // Cyan
const string FREE_COLOR = "\033[0;37m";    // Gray
const string CLASS_COLOR = "\033[0;32m";   // Green
const string LUNCH_COLOR = "\033[0;33m";   // Yellow
const string BORDER_COLOR = "\033[0;34m";  // Blue

// Add these constants for box drawing
const string BOX_HORIZONTAL = "─";
const string BOX_VERTICAL = "│";
const string BOX_TOP_LEFT = "┌";
const string BOX_TOP_RIGHT = "┐";
const string BOX_BOTTOM_LEFT = "└";
const string BOX_BOTTOM_RIGHT = "┘";
const string BOX_T_DOWN = "┬";
const string BOX_T_UP = "┴";
const string BOX_T_RIGHT = "├";
const string BOX_T_LEFT = "┤";
const string BOX_CROSS = "┼";

using namespace std;

void TimetableGenerator::save_to_csv_column_wise() {
    ofstream file("timetables_column_wise.csv");
    if (!file) {
        throw runtime_error("Could not open CSV file for writing");
    }

    file << "Institution Type,College\n";
    file << "Start Time," << format_time(start_hour * 60) << "\n";
    file << "Lunch Time," << format_time(lunch_break_time) << "\n";
    file << "Lunch Duration," << lunch_break_duration << " minutes\n\n";

    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        for (int section = 0; section < sections_per_stream[stream]; section++) {
            const Timetable &tt = timetables[stream][section];
            char sec_char = 'A' + section;

            file << "Stream:," << stream_names[stream] << "\n";
            file << "Section:," << sec_char << "\n\n";

            file << "Time Slot,";
            for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                file << DAY_NAMES[day] << ",";
                if (day < DAY_NAMES.size() - 1) file << ",";
            }
            file << "\n";

            file << ",";
            for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                file << "Subject,Teacher,Room,";
                if (day < DAY_NAMES.size() - 1) file << ",";
            }
            file << "\n";

            int current_time = start_hour * 60;
            for (int slot = 0; slot < total_teaching_slots; slot++) {
                if (slot == lunch_break_slot) {
                    file << format_time(lunch_break_time) << "-" 
                         << format_time(lunch_break_time + lunch_break_duration) << ",";
                    for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                        file << "LUNCH BREAK,,,";
                        if (day < DAY_NAMES.size() - 1) file << ",";
                    }
                    file << "\n";
                    current_time = lunch_break_time + lunch_break_duration;
                    continue;
                }

                int end_time = current_time + slot_durations[slot];
                file << format_time(current_time) << "-" << format_time(end_time) << ",";

                for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                    if (tt.subject_id[day][slot] == -3) {
                        file << "\"" << find_lab_name(stream, day) << "\","
                             << "\"" << tt.teacher[day][slot] << "\","
                             << "\"" << tt.room[day][slot] << "\",";
                    } else if (tt.subject_id[day][slot] == -1) {
                        file << "FREE,,,";
                    } else {
                        int subj_idx = tt.subject_id[day][slot];
                        file << "\"" << subjects[stream][subj_idx].name << "\","
                             << "\"" << subjects[stream][subj_idx].teacher << "\","
                             << "\"" << tt.room[day][slot] << "\",";
                    }

                    if (day < DAY_NAMES.size() - 1) file << ",";
                }
                file << "\n";
                current_time = end_time + break_duration;
            }
            file << "\n\n";
        }
    }
}

// Modify the save_to_txt_column_wise() function
void TimetableGenerator::save_to_txt_column_wise() {
    ofstream file("timetables_column_wise.txt");
    if (!file) {
        throw runtime_error("Could not open TXT file for writing");
    }

    file << HEADER_COLOR << "COLLEGE TIMETABLE (COLUMN-WISE)\n";
    file << string(60, '=') << RESET << "\n\n";
    file << "Start Time: " << format_time(start_hour * 60) << "\n";
    file << "Lunch Time: " << format_time(lunch_break_time) << " (" << lunch_break_duration << " mins)\n\n";

    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        for (int section = 0; section < sections_per_stream[stream]; section++) {
            const Timetable &tt = timetables[stream][section];
            char sec_char = 'A' + section;

            file << HEADER_COLOR << "STREAM: " << stream_names[stream];
            file << " - SECTION " << sec_char << RESET << "\n";
            file << BORDER_COLOR << string(60, '=') << RESET << "\n\n";

            const int time_width = 14;
            const int day_width = 30;

            file << HEADER_COLOR << left << setw(time_width) << "TIME";
            for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                file << BORDER_COLOR << "| " << HEADER_COLOR 
                     << left << setw(day_width - 2) << DAY_NAMES[day] << " ";
            }
            file << RESET << "\n";

            file << BORDER_COLOR;
            file << string(time_width, ' ');
            for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                file << "+" << string(day_width - 1, '-');
            }
            file << RESET << "\n";

            int current_time = start_hour * 60;
            for (int slot = 0; slot < total_teaching_slots; slot++) {
                if (slot == lunch_break_slot) {
                    file << left << setw(time_width) 
                         << (format_time(lunch_break_time) + "-" + format_time(lunch_break_time + lunch_break_duration));
                    for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                        file << BORDER_COLOR << "| " << LUNCH_COLOR 
                             << left << setw(day_width - 2) << "LUNCH BREAK" << " ";
                    }
                    file << RESET << "\n";
                    current_time = lunch_break_time + lunch_break_duration;
                    continue;
                }

                int end_time = current_time + slot_durations[slot];
                file << left << setw(time_width) 
                     << format_time_range(current_time, slot);  // Pass slot as parameter

                for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                    string entry;
                    if (tt.subject_id[day][slot] == -3) {
                        entry = find_lab_name(stream, day) + " (LAB) - " + tt.room[day][slot];
                    } else if (tt.subject_id[day][slot] == -1) {
                        entry = "FREE PERIOD";
                    } else {
                        int subj_idx = tt.subject_id[day][slot];
                        entry = subjects[stream][subj_idx].name + " (" + 
                                subjects[stream][subj_idx].teacher + ") - " +
                                tt.room[day][slot];
                    }
                    file << BORDER_COLOR << "| ";
                    if (tt.subject_id[day][slot] == -1) {
                        file << FREE_COLOR << left << setw(day_width - 2) << "FREE PERIOD";
                    } else {
                        file << CLASS_COLOR << left << setw(day_width - 2) 
                             << entry;
                    }
                    file << " " << RESET;
                }
                file << "\n";
                current_time = end_time + break_duration;
            }
            file << "\n\n";
        }
    }
    print_legend(file);
}

void TimetableGenerator::generate_teacher_timetables() {
    ofstream file("teacher_timetables.txt");
    if (!file) {
        throw runtime_error("Could not open teacher timetable file for writing");
    }

    file << "TEACHER TIMETABLES\n";
    file << string(60, '=') << "\n\n";

    map<string, vector<tuple<string, string, string, string>>> teacher_schedules;

    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        for (int section = 0; section < sections_per_stream[stream]; section++) {
            const Timetable &tt = timetables[stream][section];
            char sec_char = 'A' + section;
            string stream_sec = stream_names[stream] + " Sec " + sec_char;

            for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                for (int slot = 0; slot < total_teaching_slots; slot++) {
                    if (slot == lunch_break_slot) continue;
                    
                    string teacher_name;
                    if (tt.subject_id[day][slot] == -3) {
                        teacher_name = tt.teacher[day][slot];
                    } else if (tt.subject_id[day][slot] >= 0) {
                        teacher_name = subjects[stream][tt.subject_id[day][slot]].teacher;
                    } else {
                        continue;
                    }

                    if (!teacher_name.empty()) {
                        int current_time = start_hour * 60;
                        for (int s = 0; s < slot; s++) {
                            if (s == lunch_break_slot) {
                                current_time = lunch_break_time + lunch_break_duration;
                            } else {
                                current_time += slot_durations[s] + break_duration;
                            }
                        }
                        int end_time = current_time + slot_durations[slot];
                        
                        teacher_schedules[teacher_name].push_back(
                            make_tuple(
                                DAY_NAMES[day],
                                format_time(current_time) + "-" + format_time(end_time),
                                tt.room[day][slot],
                                stream_sec
                            )
                        );
                    }
                }
            }
        }
    }

    for (const auto& [teacher, schedule] : teacher_schedules) {
        file << "TEACHER: " << teacher << "\n";
        file << string(60, '-') << "\n";
        file << left << setw(15) << "DAY" << setw(20) << "TIME" 
             << setw(15) << "ROOM" << "CLASS\n";
        file << string(60, '-') << "\n";

        for (const auto& [day, time, room, cls] : schedule) {
            file << left << setw(15) << day << setw(20) << time 
                 << setw(15) << room << cls << "\n";
        }
        file << "\n\n";
    }
}

// Update the format_time_range function to accept current_slot parameter
string TimetableGenerator::format_time_range(int start_time, int current_slot) {
    int end_time = start_time + slot_durations[current_slot];
    return format_time(start_time) + "-" + format_time(end_time);
}

// Update format_class_entry to use proper member access
string TimetableGenerator::format_class_entry(const Timetable &tt, int day, int slot, int stream) {
    if (tt.subject_id[day][slot] == -3) {
        return find_lab_name(stream, day) + " (LAB) - " + tt.room[day][slot];
    } else {
        int subj_idx = tt.subject_id[day][slot];
        return subjects[stream][subj_idx].name + " (" + 
               subjects[stream][subj_idx].teacher + ") - " + 
               tt.room[day][slot];
    }
}

string TimetableGenerator::center_text(const string& text, int width) {
    int padding = width - text.length();
    int left_pad = padding / 2;
    return string(left_pad, ' ') + text + 
           string(padding - left_pad, ' ');
}

void TimetableGenerator::print_legend(ofstream& file) {
    file << "\nLEGEND:\n";
    file << "-------\n";
    file << CLASS_COLOR << "Regular Classes" << RESET << "\n";
    file << FREE_COLOR << "Free Periods" << RESET << "\n";
    file << LUNCH_COLOR << "Lunch Break" << RESET << "\n";
    file << "\nFormat: Subject (Teacher) - Room\n\n";
}

void TimetableGenerator::print_summary(ofstream& file, const Timetable& tt) {
    map<string, int> teacher_hours;
    map<string, int> room_usage;
    
    for (size_t day = 0; day < DAY_NAMES.size(); day++) {
        for (int slot = 0; slot < total_teaching_slots; slot++) {
            if (tt.subject_id[day][slot] >= 0) {
                teacher_hours[tt.teacher[day][slot]]++;
                room_usage[tt.room[day][slot]]++;
            }
        }
    }

    file << "\nSUMMARY STATISTICS:\n";
    file << "-----------------\n";
    file << "Teaching Hours per Teacher:\n";
    for (const auto& [teacher, hours] : teacher_hours) {
        file << teacher << ": " << hours << " hours\n";
    }
    
    file << "\nRoom Utilization:\n";
    for (const auto& [room, usage] : room_usage) {
        file << room << ": " << usage << " slots\n";
    }
}
