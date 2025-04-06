#include "../include/timetable_generator.h"
#include <fstream>
#include <iostream>  // Add this line
#include <iomanip>
#include <sstream>

using namespace std;

void TimetableGenerator::save_to_csv_column_wise() {
    ofstream file("timetables_column_wise.csv");
    if (!file) {
        throw runtime_error("Could not open CSV file for writing");
    }

    // Basic header information
    file << "COLLEGE TIMETABLE\n\n";
    file << "Basic Information,\n";
    file << "Start Time," << format_time(start_hour * 60) << "\n";
    file << "Lunch Time," << format_time(lunch_break_time) << "\n";
    file << "Lunch Duration," << lunch_break_duration << " minutes\n\n";

    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        for (int section = 0; section < sections_per_stream[stream]; section++) {
            const Timetable &tt = timetables[stream][section];
            char sec_char = 'A' + section;

            // Stream and section header
            file << stream_names[stream] << " - Section " << sec_char << "\n\n";

            // Simple column headers
            file << "Time,";
            for (const string& day : DAY_NAMES) {
                file << day << ",";
            }
            file << "\n";

            // Timetable content
            int current_time = start_hour * 60;
            for (int slot = 0; slot < total_teaching_slots; slot++) {
                // Handle lunch break
                if (slot == lunch_break_slot) {
                    file << format_time(lunch_break_time) << "-" 
                         << format_time(lunch_break_time + lunch_break_duration) << ",";
                    for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                        file << "LUNCH BREAK,";
                    }
                    file << "\n";
                    current_time = lunch_break_time + lunch_break_duration;
                    continue;
                }

                // Regular time slots
                int end_time = current_time + slot_durations[slot];
                file << format_time(current_time) << "-" << format_time(end_time) << ",";

                // Print each day's classes
                for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                    if (tt.subject_id[day][slot] == -3) {
                        // Lab session
                        file << find_lab_name(stream, day) << " [" 
                             << tt.teacher[day][slot] << "] " 
                             << tt.room[day][slot] << ",";
                    } else if (tt.subject_id[day][slot] == -1) {
                        file << "FREE,";
                    } else {
                        // Regular class
                        int subj_idx = tt.subject_id[day][slot];
                        file << subjects[stream][subj_idx].name << " ["
                             << subjects[stream][subj_idx].teacher << "] "
                             << tt.room[day][slot] << ",";
                    }
                }
                file << "\n";
                current_time = end_time + break_duration;
            }
            file << "\n\n";
        }
        file << "\n";
    }

    // Simple legend
    file << "Notes:\n";
    file << "- Format: Subject [Teacher] Room\n";
    file << "- FREE: No class scheduled\n";
    file << "- LUNCH BREAK: Lunch period\n";
}

void TimetableGenerator::save_to_txt_column_wise() {
    ofstream file("timetables_column_wise.txt");
    if (!file) {
        throw runtime_error("Could not open TXT file for writing");
    }

    file << "COLLEGE TIMETABLE\n";
    file << "================\n\n";
    file << "Start Time: " << format_time(start_hour * 60) << "\n";
    file << "Lunch Time: " << format_time(lunch_break_time) << " (" << lunch_break_duration << " mins)\n\n";

    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        for (int section = 0; section < sections_per_stream[stream]; section++) {
            const Timetable &tt = timetables[stream][section];
            char sec_char = 'A' + section;

            file << "STREAM: " << stream_names[stream];
            file << " - SECTION " << sec_char << "\n";
            file << string(60, '=') << "\n\n";

            const int time_width = 14;
            const int day_width = 30;

            // Column headers
            file << left << setw(time_width) << "TIME";
            for (const string& day : DAY_NAMES) {
                file << "| " << left << setw(day_width - 2) << day;
            }
            file << "\n" << string(time_width + (day_width * DAY_NAMES.size()), '-') << "\n";

            int current_time = start_hour * 60;
            for (int slot = 0; slot < total_teaching_slots; slot++) {
                if (slot == lunch_break_slot) {
                    string lunch_time = format_time(lunch_break_time) + "-" + 
                                      format_time(lunch_break_time + lunch_break_duration);
                    file << left << setw(time_width) << lunch_time;
                    for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                        file << "| " << left << setw(day_width - 2) << "LUNCH BREAK";
                    }
                    file << "\n";
                    current_time = lunch_break_time + lunch_break_duration;
                    continue;
                }

                int end_time = current_time + slot_durations[slot];
                file << left << setw(time_width) << format_time_range(current_time, slot);

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
                    file << "| " << left << setw(day_width - 2) << entry;
                }
                file << "\n";
                current_time = end_time + break_duration;
            }
            file << "\n\n";
        }
    }

    file << "\nLEGEND:\n";
    file << "-------\n";
    file << "- Regular Classes: Subject (Teacher) - Room\n";
    file << "- Free Periods\n";
    file << "- Lunch Break\n\n";
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

// Add new function to save individual section timetables
void TimetableGenerator::save_individual_section_timetables() {
    for (size_t stream = 0; stream < stream_names.size(); stream++) {
        for (int section = 0; section < sections_per_stream[stream]; section++) {
            // Create filename like "cse_sectionA.txt"
            string filename = stream_names[stream] + "_section" + 
                            static_cast<char>('A' + section) + ".txt";
            
            ofstream file(filename);
            if (!file) {
                throw runtime_error("Could not open file: " + filename);
            }

            file << "COLLEGE TIMETABLE\n";
            file << "================\n\n";
            file << "Stream: " << stream_names[stream] << "\n";
            file << "Section: " << static_cast<char>('A' + section) << "\n\n";
            file << "Start Time: " << format_time(start_hour * 60) << "\n";
            file << "Lunch Time: " << format_time(lunch_break_time) 
                 << " (" << lunch_break_duration << " mins)\n\n";

            print_section_timetable(file, stream, section);
            
            file << "\nLEGEND:\n";
            file << "-------\n";
            file << "- Regular Classes: Subject (Teacher) - Room\n";
            file << "- Free Periods\n";
            file << "- Lunch Break\n";
            
            cout << "Created timetable for " << stream_names[stream] 
                 << " Section " << static_cast<char>('A' + section) 
                 << " in " << filename << "\n";
        }
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
    file << "Regular Classes\n";
    file << "Free Periods\n";
    file << "Lunch Break\n";
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

void TimetableGenerator::print_section_timetable(ofstream& file, size_t stream, int section) {
    const Timetable &tt = timetables[stream][section];
    
    const int time_width = 14;
    const int day_width = 30;

    // Print column headers
    file << left << setw(time_width) << "TIME";
    for (const string& day : DAY_NAMES) {
        file << "| " << left << setw(day_width - 2) << day;
    }
    file << "\n" << string(time_width + (day_width * DAY_NAMES.size()), '-') << "\n";

    // Print timetable content
    int current_time = start_hour * 60;
    for (int slot = 0; slot < total_teaching_slots; slot++) {
        if (slot == lunch_break_slot) {
            string lunch_time = format_time(lunch_break_time) + "-" + 
                              format_time(lunch_break_time + lunch_break_duration);
            file << left << setw(time_width) << lunch_time;
            for (size_t day = 0; day < DAY_NAMES.size(); day++) {
                file << "| " << left << setw(day_width - 2) << "LUNCH BREAK";
            }
            file << "\n";
            current_time = lunch_break_time + lunch_break_duration;
            continue;
        }

        // Print regular time slots
        int end_time = current_time + slot_durations[slot];
        file << left << setw(time_width) << format_time_range(current_time, slot);

        // Print each day's classes
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
            file << "| " << left << setw(day_width - 2) << entry;
        }
        file << "\n";
        current_time = end_time + break_duration;
    }
}
