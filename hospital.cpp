/*
 * ============================================================
 *   HOSPITAL MANAGEMENT SYSTEM
 *   Programming Fundamentals - 
 *   Bisma Mamoon And Bisma Khan
 * ============================================================
 */
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cctype>
#include <iomanip>
using namespace std;
int findPatientIndex(int id);
int findDoctorIndex(int id);

// ─────────────────────────────────────────────
//  STRUCTURES
// ─────────────────────────────────────────────
struct Patient 
{
    int    patientId;
    string name;
    int    age;
    string gender;
    string contact;
    double balance;
};

struct Doctor 
{
    int    doc_id;
    string name;
    string specialty;
    int    experience;
};

struct Appointment 
{
    int  patientId;
    int  doctorId;
    char date[11];   // MM-DD-YYYY
    char time[10];   // hh:mm AM/PM
};

struct Treatment 
{
    int    patientId;
    string description;
    double cost;
    bool   paid;
};

// ─────────────────────────────────────────────
//  GLOBAL DYNAMIC ARRAYS
// ─────────────────────────────────────────────

Patient*     patients     = nullptr;
Doctor*      doctors      = nullptr;
Appointment* appointments = nullptr;
Treatment*   treatments   = nullptr;

int patientCount     = 0;
int doctorCount      = 0;
int appointmentCount = 0;
int treatmentCount   = 0;
 
// ─────────────────────────────────────────────
//  EMPLOYEE CREDENTIALS
// ─────────────────────────────────────────────
const string EMP_ID   = "123";
const string EMP_PASS = "pass123";

// ─────────────────────────────────────────────
//  UTILITY / VALIDATION HELPERS
// ─────────────────────────────────────────────

// Trim leading/trailing whitespace
string trim(string s)
{
    int start = 0;
    int end = s.length() - 1;
    // Find first non-space
   while (start < s.length() && s[start] == ' ')
    {
        start++;
    }
    // Find last non-space
    while (s[end]==' ')
    {
        end--;
    }
    string result = "";

    for (int i=start;i<=end;i++)
    {
        result+= s[i];
    }
    return result;
}

// Check if string contains only digits
bool allDigits(string s)
{
    if (s == "")
        return false;

    for (int i = 0; i < s.length(); i++)
    {
        if (s[i]< '0' || s[i] > '9')
            return false;
    }

    return true;
}

// Validate phone: must be 11 digits, all numeric
bool isValidPhone(const string& phone) 
{
    string p = trim(phone);
    if (p.length()!= 11) 
	  return false;
    return allDigits(p);
}
int safeStoi(const string& s, int def = 0) {
    string t = trim(s);
    if (t.empty()) return def;
    try { return stoi(t); } catch (...) { return def; }
}

double safeStod(const string& s, double def = 0.0) {
    string t = trim(s);
    if (t.empty()) return def;
    try { return stod(t); } catch (...) { return def; }
}

// Convert "M" -> "Male", "F" -> "Female"
string normalizeGender(const string& g) 
{
    string s = trim(g);
    if (s == "M" || s == "m") 
	 return "Male";
    if (s == "F" || s == "f") 
	 return "Female";
    return s;   // already "Male" / "Female"
}

// Convert 24h time string to 12h AM/PM  e.g. "13:25" -> "01:25 PM"
string convertTo12Hour(string time)
{
    int hour;
    string minutes;
    string period;
    hour = (time[0] - '0') * 10 + (time[1] - '0');

    minutes = "";
    minutes += time[3];
    minutes += time[4];

    if (hour == 0)
    {
        hour = 12;
        period="AM";
    }
    else if (hour < 12)
    {
        period ="AM";
    }
    else if (hour == 12)
    {
        period ="PM";
    }
    else
    {
        hour =hour - 12;
        period = "PM";
    }

    string result ="";

    if (hour < 10)
        result +='0';

    result += to_string(hour);
    result += ":";
    result += minutes;
    result += " ";
    result += period;

    return result;
}

// Convert date from YYYY-MM-DD to MM-DD-YYYY
string normalizeDateFormat(string date)
{
    string first = "";
    string second = "";
    string third = "";
    int i = 0;

    // First part
    while (date[i] != '-')
    {
        first += date[i];
        i++;
    }
    i++;
    // Second part
    while (date[i] != '-')
    {
        second += date[i];
        i++;
    }
    i++;
    // Third part
    while (i < date.length())
    {
        third += date[i];
        i++;
    }
    // If first part length is 4 → YYYY-MM-DD
    if (first.length() == 4)
    {
        return second + "-" + third + "-" + first;
    }
    // Otherwise assume DD-MM-YYYY
    else
    {
        return second + "-" + first + "-" + third;
    }
}

// Check a patient record has the minimum required fields
bool isValidPatientRecord(const Patient& p) 
{
    if (p.patientId <= 0) 
	 return false;
    if (trim(p.name).empty()) 
	return false;
    if (!isValidPhone(p.contact)) 
	return false;
    return true;
}

// ─────────────────────────────────────────────
//  LOAD / SAVE FILES
// ─────────────────────────────────────────────

void loadPatients()
{
    ifstream fin("patients.txt");
    if (!fin)
    {
        return;
    }
    Patient temp[1000];
    int count = 0;

    while (!fin.eof())
    {
        Patient p;
        string data;
        // Patient ID
        getline(fin, data, '#');
        if (data == "")
        {
            break;
        }
        p.patientId = stoi(data);

        // Name
        getline(fin, p.name, '#');

        // Age
        getline(fin, data, '#');
        p.age = safeStoi(data);

        // Gender
        getline(fin, p.gender, '#');
        p.gender = normalizeGender(p.gender);

        // Contact
        getline(fin, p.contact, '#');

        // Balance
        getline(fin, data);
       p.balance = safeStod(data);
        // Validation
        if (p.patientId <= 0)
        {
            continue;
        }
        if (p.name == "")
        {
            continue;
        }
        if (!isValidPhone(p.contact))
        {
            continue;
        }
        // Duplicate check
        bool duplicate = false;
        for (int i = 0; i < count; i++)
        {
            if (temp[i].patientId == p.patientId)
            {
                duplicate = true;
                break;
            }
        }
        if (!duplicate)
        {
            temp[count] = p;
            count++;
        }
    }
    fin.close();
    delete[] patients;
    patients = new Patient[count];
    for (int i = 0; i < count; i++)
    {
        patients[i] = temp[i];
    }

    patientCount = count;
}

void savePatients() 
{
    ofstream fout("patients.txt");
    for (int i = 0; i < patientCount; i++) 
	{
        fout << patients[i].patientId << "#"
             << patients[i].name      << "#"
             << patients[i].age       << "#"
             << patients[i].gender    << "#"
             << patients[i].contact   << "#"
             << patients[i].balance   << endl;
    }
}

void loadDoctors()
{
    ifstream fin("doctors.txt");
    if (!fin)
    {
        return;
    }

    Doctor temp[500];
    int count = 0;

    while (!fin.eof())
    {
        Doctor d;
        string data;

        // Doctor ID
        getline(fin, data, '#');

        if (data == "")
        {
            break;
        }

        d.doc_id = stoi(data);

        // Name
        getline(fin, d.name, '#');

        // Specialty
        getline(fin, d.specialty, '#');

        // Experience
        getline(fin, data);
        d.experience = safeStoi(data);

        // Validation
        if (d.doc_id <= 0)
        {
            continue;
        }

        if (trim(d.name).empty())
        {
            continue;
        }

        temp[count] = d;
        count++;
    }

    fin.close();
    delete[] doctors;
    doctors = new Doctor[count];
    for (int i = 0; i < count; i++)
    {
        doctors[i] = temp[i];
    }
    doctorCount = count;
}

void saveDoctors() 
{
    ofstream fout("doctors.txt");
    for (int i = 0; i < doctorCount; i++)
        fout << doctors[i].doc_id   << "#"
             << doctors[i].name     << "#"
             << doctors[i].specialty << "#"
             << doctors[i].experience <<endl;
}

void loadAppointments()
{
    ifstream fin("appointments.txt");
    if (!fin)
    {
        return;
    }
    Appointment temp[1000];
    int count = 0;
    while (!fin.eof())
    {
        Appointment a;
        string data;

        // Patient ID
        getline(fin, data, '#');

        if (data == "")
        {
            break;
        }

        a.patientId = safeStoi(data);

        // Doctor ID
        getline(fin, data, '#');

        a.doctorId = safeStoi(data);

        // Date
        getline(fin, data, '#');

        data = normalizeDateFormat(trim(data));

        strncpy(a.date, data.c_str(), 10);
        a.date[10] = '\0';

        // Time
        getline(fin, data);

        data = convertTo12Hour(trim(data));

        strncpy(a.time, data.c_str(), 9);
        a.time[9] = '\0';

        // Validation
        if (a.patientId <= 0 || a.doctorId <= 0)
        {
            continue;
        }
        
        if (findPatientIndex(a.patientId) == -1)
        {
            continue;
         }
        if (findDoctorIndex(a.doctorId) == -1)
       {
         continue;
        }
        
        temp[count] = a;
        count++;
    }
    fin.close();

    delete[] appointments;
    appointments = new Appointment[count];
    for (int i = 0; i < count; i++)
    {
        appointments[i] = temp[i];
    }
    appointmentCount = count;
}
void saveAppointments() {
    ofstream fout("appointments.txt");
    for (int i = 0; i < appointmentCount; i++)
        fout << appointments[i].patientId << "#"
             << appointments[i].doctorId  << "#"
             << appointments[i].date      << "#"
             << appointments[i].time      << "\n";
}

void loadTreatments()
{
    ifstream fin("treatments.txt");
    if (!fin)
    {
        return;
    }
    Treatment temp[1000];
    int count = 0;

    while (!fin.eof())
    {
        Treatment t;
        string data;

        // Patient ID
        getline(fin, data, '#');

        if (data == "")
        {
            break;
        }

       t.patientId = safeStoi(data);

        // Description
        getline(fin, t.description, '#');

        // Cost
        getline(fin, data, '#');

        t.cost = safeStod(data);

        // Paid status
        getline(fin, data);

        data = trim(data);

        if (data == "true" || data == "1" || data == "Paid")
        {
            t.paid = true;
        }
        else
        {
            t.paid = false;
        }

        // Validation
        if (t.patientId <= 0)
        {
            continue;
        }

       if (findPatientIndex(t.patientId) == -1)
       {
       continue;
       }
        temp[count] = t;
        count++;
    }
    fin.close();
    
    delete[] treatments;
    treatments = new Treatment[count];
    for (int i = 0; i < count; i++)
    {
        treatments[i] = temp[i];
    }
    treatmentCount = count;
}
void saveTreatments() 
{
    ofstream fout("treatments.txt");
    for (int i = 0; i < treatmentCount; i++)
        fout << treatments[i].patientId   << "#"
             << treatments[i].description << "#"
             << treatments[i].cost        << "#"
             << (treatments[i].paid ? "true" : "false") <<endl;
}

// ─────────────────────────────────────────────
//  FILE CLEANING (runs at startup)
// ─────────────────────────────────────────────
void cleanFile() 
{
    cout << "\n[System] Running file cleaning & validation...\n";

    // Load each file, apply all cleaning rules, then write back ONLY if
    // records were actually loaded. This prevents overwriting a valid file
    // with empty data if the file path is wrong or the file is unreadable.

    loadPatients();
    if (patientCount > 0)
        savePatients();
    else
        cout << "  [Warning] patients.txt: no valid records found or file missing. Original file NOT overwritten.\n";

    loadDoctors();
    if (doctorCount > 0)
        saveDoctors();
    else
        cout << "  [Warning] doctors.txt: no valid records found or file missing. Original file NOT overwritten.\n";

    loadAppointments();
    if (appointmentCount > 0)
        saveAppointments();
    else
        cout << "  [Warning] appointments.txt: no valid records found or file missing. Original file NOT overwritten.\n";

    loadTreatments();
    if (treatmentCount > 0)
        saveTreatments();
    else
        cout << "  [Warning] treatments.txt: no valid records found or file missing. Original file NOT overwritten.\n";

    cout << "[System] File cleaning complete.\n";
    cout << "  Loaded: " << patientCount     << " patients,  "
                         << doctorCount      << " doctors,  "
                         << appointmentCount << " appointments,  "
                         << treatmentCount   << " treatments.\n";
}

// ─────────────────────────────────────────────
//  LOOKUP HELPERS
// ─────────────────────────────────────────────
int findPatientIndex(int id) 
{
    for (int i = 0; i < patientCount; i++)
        if (patients[i].patientId == id) 
		return i;
    return -1;
}

int findDoctorIndex(int id) 
{
    for (int i = 0; i < doctorCount; i++)
        if (doctors[i].doc_id == id) 
		return i;
    return -1;
}

// Print a single patient row
void printPatient(const Patient& p) 
{
    cout << left
         << setw(6)  << p.patientId
         << setw(22) << p.name
         << setw(5)  << p.age
         << setw(9)  << p.gender
         << setw(14) << p.contact
         << "PKR " << fixed << setprecision(0) << p.balance << "\n";
}

void printPatientHeader() 
{
    cout << string(70, '-') <<endl;
    cout << left
         << setw(6)  << "ID"
         << setw(22) << "Name"
         << setw(5)  << "Age"
         << setw(9)  << "Gender"
         << setw(14) << "Contact"
         << "Balance"<<endl;
    cout << string(70, '-') <<endl;
}
void printDoctor(const Doctor& d) 
{
    cout << left
         << setw(6)  << d.doc_id
         << setw(22) << d.name
         << setw(22) << d.specialty
         << d.experience << " yrs"<<endl;
}
void printDoctorHeader() 
{
    cout << string(60, '-') <<endl;
    cout << left
         << setw(6)  << "ID"
         << setw(22) << "Name"
         << setw(22) << "Specialty"
         << "Exp"<<endl;
    cout << string(60, '-') <<endl;
}

// ─────────────────────────────────────────────
//  PATIENT MANAGEMENT
// ─────────────────────────────────────────────
void viewPatients() 
{
    if (patientCount == 0) 
	{ 
	  cout << "No patient records found."<<endl; 
	  return; 
	}
    printPatientHeader();
    for (int i = 0; i < patientCount; i++) 
	printPatient(patients[i]);
    cout << "Total: " << patientCount << " patient(s)."<<endl;
}

void addPatient() 
{
    Patient p;
    cout << "\n--- Add New Patient ---\n";
    cout << "Enter Patient ID: "; 
	cin >> p.patientId;
    if (p.patientId <= 0) 
	{ 
	  cout << "Invalid ID."<<endl; 
	  return; 
	}
    if (findPatientIndex(p.patientId) != -1) 
	{ 
	  cout << "Patient ID already exists."<<endl; 
	  return; 
	}
    cin.ignore();
    cout << "Name: "; 
	getline(cin, p.name); 
	p.name = trim(p.name);
    if (p.name.empty()) 
	{ 
	  cout << "Name cannot be empty."<<endl; 
	  return; 
	}
    cout << "Age: "; 
	cin >> p.age;
    cin.ignore();
    cout << "Gender (Male/Female): "; 
	getline(cin, p.gender); 
	p.gender = normalizeGender(p.gender);
    cout << "Contact (11 digits): "; 
	getline(cin, p.contact); 
	p.contact = trim(p.contact);
    if (!isValidPhone(p.contact)) 
	{ 
	  cout << "Invalid phone number. Patient not added."<<endl; 
	  return; 
	}
    cout << "Balance (PKR): "; 
	cin >> p.balance; 
	cin.ignore();

    // Resize array
    Patient* newArr = new Patient[patientCount + 1];
    for (int i = 0; i < patientCount; i++) 
	newArr[i] = patients[i];
    newArr[patientCount] = p;
    delete[] patients;
    patients = newArr;
    patientCount++;
    savePatients();
    cout << "Patient added successfully."<<endl;
}

void updatePatient() 
{
    int id;
    cout << "\nEnter Patient ID to update: "; 
	cin >> id; 
	cin.ignore();
    int idx = findPatientIndex(id);
    if (idx == -1) 
	{ 
	cout << "Patient not found."<<endl;
	 return;
     }
    Patient& p = patients[idx];
    string inp; 
    cout << "Current Name: " << p.name << "  New Name (Enter to keep): "; 
	getline(cin, inp);
    if (!trim(inp).empty()) p.name = trim(inp);
    int a;
    cout << "Current Age: " << p.age << "  New Age (0 to keep): "; 
	cin >> a; 
	cin.ignore();
    if (a > 0) 
	p.age = a;
    cout << "Current Gender: " << p.gender << "  New Gender (Enter to keep): ";
    getline(cin, inp);
    if (!trim(inp).empty()) p.gender = normalizeGender(inp);
    cout << "Current Contact: " << p.contact << "  New Contact (Enter to keep): ";
    getline(cin, inp); inp = trim(inp);
    if (!inp.empty()) 
	{
        if (!isValidPhone(inp)) 
		{ 
		cout << "Invalid phone. Contact not updated."<<endl; 
		}
        else p.contact = inp;
    }
    double b;
    cout << "Current Balance: " << p.balance << "  New Balance: "; 
	cin >> b; 
	cin.ignore();
    if (b >= 0) 
	p.balance = b;
    savePatients();
    cout << "Patient updated successfully.\n";
}

void deletePatient() 
{
    int id;
    cout << "\nEnter Patient ID to delete: "; 
	cin >> id;
    cin.ignore();
    int idx = findPatientIndex(id);
    if (idx == -1) 
	{ 
	  cout << "Patient not found."<<endl; 
	  return; 
	}
    Patient* newArr = new Patient[patientCount - 1];
    int j = 0;
    for (int i = 0; i < patientCount; i++) 
	if (i != idx) 
	newArr[j++] = patients[i];
    delete[] patients;
    patients = newArr;
    patientCount--;
    savePatients();
    cout << "Patient deleted.\n";
}

// ─────────────────────────────────────────────
//  DOCTOR MANAGEMENT
// ─────────────────────────────────────────────
void viewDoctors() 
{
    if (doctorCount == 0) 
	{ 
	cout << "No doctor records found."<<endl; 
	return; 
	}
    printDoctorHeader();
    for (int i = 0; i < doctorCount; i++) 
	printDoctor(doctors[i]);
    cout << "Total: " << doctorCount << " doctor(s)."<<endl;
}

void addDoctor() 
{
    Doctor d;
    cout << "\n--- Add New Doctor ---\n";
    cout << "Doctor ID: "; 
	cin >> d.doc_id; 
	cin.ignore();
    if (d.doc_id <= 0) 
	{ 
	cout << "Invalid ID."<<endl; 
	return; 
	}
    if (findDoctorIndex(d.doc_id) != -1) 
	{ 
	cout << "Doctor ID already exists."<<endl; 
	return; 
	}
    cout << "Name: "; 
	getline(cin, d.name); 
	d.name = trim(d.name);
    if (d.name.empty()) 
	{ 
	cout << "Name cannot be empty."<<endl; 
	return; 
	}
    cout << "Specialty: "; 
	getline(cin, d.specialty);
    cout << "Years of Experience: "; 
	cin >> d.experience; 
	cin.ignore();

    Doctor* newArr = new Doctor[doctorCount + 1];
    for (int i = 0; i < doctorCount; i++) newArr[i] = doctors[i];
    newArr[doctorCount] = d;
    delete[] doctors;
    doctors = newArr;
    doctorCount++;
    saveDoctors();
    cout << "Doctor added successfully."<<endl;
}

void updateDoctor() 
{
    int id;
    cout << "\nEnter Doctor ID to update: "; 
	cin >> id; 
	cin.ignore();
    int idx = findDoctorIndex(id);
    if (idx == -1) 
	{ 
	cout << "Doctor not found."<<endl; 
	return; 
	}
    Doctor& d = doctors[idx];
    cout << "Current Name: " << d.name << " New Name (Enter to keep): ";
    string inp; 
	getline(cin, inp);
    if (!trim(inp).empty()) 
	d.name = trim(inp);
    cout << "Current Specialty: " << d.specialty << "  New Specialty (Enter to keep): ";
    getline(cin, inp);
    if (!trim(inp).empty()) 
	d.specialty = trim(inp);
    cout << "Current Experience: " << d.experience << " yrs  New Experience (0 to keep): ";
    int e; cin >> e; cin.ignore();
    if (e > 0) 
	d.experience = e;
    saveDoctors();
    cout << "Doctor updated successfully."<<endl;
}

void deleteDoctor() 
{
    int id;
    cout << "\nEnter Doctor ID to delete: "; 
	cin >> id; 
	cin.ignore();
    int idx = findDoctorIndex(id);
    if (idx == -1) 
	{ 
	cout << "Doctor not found."<<endl; 
	return; 
	}
    Doctor* newArr = new Doctor[doctorCount - 1];
    int j = 0;
    for (int i = 0; i < doctorCount; i++) if (i != idx) newArr[j++] = doctors[i];
    delete[] doctors;
    doctors = newArr;
    doctorCount--;
    saveDoctors();
    cout << "Doctor deleted.\n";
}

// ─────────────────────────────────────────────
//  APPOINTMENTS
// ─────────────────────────────────────────────
void viewAppointments() 
{
    if (appointmentCount == 0) 
	{ 
	cout << "No appointments found."<<endl; 
	return; 
	}
    cout << string(65, '-') <<endl;
    cout << left << setw(8) << "Pat ID" << setw(8) << "Doc ID"
         << setw(14) << "Date" << setw(12) << "Time" <<endl;
    cout << string(65, '-') <<endl;
    for (int i = 0; i < appointmentCount; i++) 
	{
        cout << left << setw(8) << appointments[i].patientId
             << setw(8) << appointments[i].doctorId
             << setw(14) << appointments[i].date
             << setw(12) << appointments[i].time <<endl;
    }
    cout << "Total: " << appointmentCount << " appointment(s)."<<endl;
}


// Predefined slots for the weekly schedule
const char* WEEKLY_SLOTS[] = 
{
    "09:00 AM","10:00 AM","11:00 AM","12:00 PM",
    "01:00 PM","02:00 PM","03:00 PM","04:00 PM"
};
const int SLOT_COUNT = 8;

// Days in a simple week (offset from base date — for bonus display)
const char* WEEK_DAYS[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};

bool isSlotTaken(int docId, const char* date, const char* time)
{
    for (int i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].doctorId == docId &&
            strcmp(appointments[i].date, date) == 0 &&
            strcmp(appointments[i].time, time) == 0)
        {
            return true;
        }
    }

    return false;
}

int findNextFreeSlot(int docId, const char* date, int startSlot)
{
    for (int i = startSlot; i < SLOT_COUNT; i++)
    {
        // If slot is free
        if (!isSlotTaken(docId, date, WEEKLY_SLOTS[i]))
        {
            return i;
        }
    }

    // No free slot found
    return -1;
}


void scheduleAppointment() 
{
    int patId, docId;
    cout << "\n--- Schedule Appointment ---\n";
    cout << "Patient ID: "; 
	cin >> patId; 
	cin.ignore();
    int pIdx = findPatientIndex(patId);
    if (pIdx == -1) 
	{ 
	cout << "Patient not found."<<endl; 
	return; 
	}

    cout << "Doctor ID: "; 
	cin >> docId; 
	cin.ignore();
    int dIdx = findDoctorIndex(docId);
    if (dIdx == -1) 
	{ 
	cout << "Doctor not found."<<endl; 
	return; 
	}

    string dateStr, timeStr;
    cout << "Date (MM-DD-YYYY): "; 
	getline(cin, dateStr);
    dateStr = normalizeDateFormat(trim(dateStr));
    cout << "Time (e.g. 10:00 AM): "; 
	getline(cin, timeStr);
    timeStr = convertTo12Hour(trim(timeStr));

    // Check balance against cheapest related treatment cost (use 0 if none yet)
    // For simplicity: check slot availability
    char date[11], time[10];
    strncpy(date, dateStr.c_str(), 10); 
	date[10] = '\0';
    strncpy(time, timeStr.c_str(), 9); 
	time[9] = '\0';

    if (isSlotTaken(docId, date, time)) 
	{
        cout << "Slot already booked! Searching for next available slot (same day)...\n";
        // Find slot index of requested time
        int reqSlot = -1;
        for (int i = 0; i < SLOT_COUNT; i++)
            if (strcmp(WEEKLY_SLOTS[i], time) == 0) 
			{ 
			reqSlot = i; 
			break; 
			}
        int nextSlot = findNextFreeSlot(docId, date, (reqSlot >= 0 ? reqSlot + 1 : 0));
        if (nextSlot == -1) 
		{
            cout << "No available slot for this doctor on this day."<<endl; 
			return;
        }
        cout << "Next available slot: " << WEEKLY_SLOTS[nextSlot] <<endl;
        cout << "Book this slot? (y/n): ";
        char ch; 
		cin >> ch; 
		cin.ignore();
        if (ch != 'y' && ch != 'Y') 
		{ 
		cout << "Appointment not scheduled."<<endl; 
		return; 
		}
        strncpy(time, WEEKLY_SLOTS[nextSlot], 9); 
		time[9] = '\0';
    }

    Appointment a;
    a.patientId = patId; 
	a.doctorId = docId;
    strncpy(a.date, date, 10); 
	a.date[10] = '\0';
    strncpy(a.time, time, 9); 
	a.time[9] = '\0';

    Appointment* newArr = new Appointment[appointmentCount + 1];
    for (int i = 0; i < appointmentCount; i++) newArr[i] = appointments[i];
    newArr[appointmentCount] = a;
    delete[] appointments;
    appointments = newArr;
    appointmentCount++;
    saveAppointments();
    cout << "Appointment scheduled: Patient " << patId
         << " with Doctor " << docId << " on " << date << " at " << time << "\n";
}

void cancelAppointment() 
{
    int patId, docId;
    cout << "\nCancel Appointment\n";
    cout << "Patient ID: "; 
	cin >> patId;
    cout << "Doctor ID:  "; 
	cin >> docId; 
	cin.ignore();
    string dateStr;
    cout << "Date (MM-DD-YYYY): "; 
	getline(cin, dateStr);
    dateStr = normalizeDateFormat(trim(dateStr));

    int found = -1;
    for (int i = 0; i < appointmentCount; i++)
        if (appointments[i].patientId == patId &&
            appointments[i].doctorId == docId &&
            string(appointments[i].date) == dateStr) 
			{ 
			found = i; 
			break; 
			}

    if (found == -1) 
	{ 
	cout << "Appointment not found."<<endl;
	 return; 
	 }

    Appointment* newArr = new Appointment[appointmentCount - 1];
    int j = 0;
    for (int i = 0; i < appointmentCount; i++) if (i != found) newArr[j++] = appointments[i];
    delete[] appointments;
    appointments = newArr;
    appointmentCount--;
    saveAppointments();
    cout << "Appointment cancelled."<<endl;
}

// Weekly schedule table
void viewWeeklySchedule() 
{
    int docId;
    cout << "\nEnter Doctor ID for weekly schedule: "; 
	cin >> docId; 
	cin.ignore();
    if (findDoctorIndex(docId) == -1) 
	{ cout << "Doctor not found.\n"; 
	return; 
	}

    cout << "\n--- Weekly Schedule for Doctor " << docId << " ---\n";
    cout << left << setw(12) << "Time";
    cout << "Appointments"<<endl;
    cout << string(60, '-') <<endl;
    for (int s = 0; s < SLOT_COUNT; s++) 
	{
        cout << setw(12) << WEEKLY_SLOTS[s];
        bool any = false;
        for (int i = 0; i < appointmentCount; i++) 
		{
            if (appointments[i].doctorId == docId &&
                strcmp(appointments[i].time, WEEKLY_SLOTS[s]) == 0) 
			{
                cout << "Pat#" << appointments[i].patientId << " on " << appointments[i].date << "  ";
                any = true;
            }
        }
        if (!any) 
		cout << "Free";
		cout<<endl;
		
    }
}

// ─────────────────────────────────────────────
//  TREATMENTS & BILLING
// ─────────────────────────────────────────────
void viewTreatments() 
{
    if (treatmentCount == 0) 
	{
	 cout << "No treatment records.\n";
	 return; 
	}
    int id;
    cout << "Enter Patient ID (0 for all): "; 
	cin >> id; 
	cin.ignore();
    bool found = false;
    cout << string(70, '-') <<endl;
    cout << left << setw(8) << "Pat ID" << setw(30) << "Description"
         << setw(12) << "Cost (PKR)" << "Status"<<endl;
    cout << string(70, '-') <<endl;
    for (int i = 0; i < treatmentCount; i++) 
	{
        if (id == 0 || treatments[i].patientId == id) 
		{
            cout << left << setw(8) << treatments[i].patientId
                 << setw(30) << treatments[i].description
                 << setw(12) << fixed << setprecision(0) << treatments[i].cost
                 << (treatments[i].paid ? "Paid" : "Unpaid") <<endl;
            found = true;
        }
    }
    if (!found) 
	cout << "No records found."<<endl;
}

void addTreatment() 
{
    Treatment t;
    cout << "\n--- Add Treatment ---\n";
    cout << "Patient ID: "; 
	cin >> t.patientId; 
	cin.ignore();
    if (findPatientIndex(t.patientId) == -1) 
	{ 
	cout << "Patient not found."<<endl; 
	return; 
	}
    cout << "Treatment Description: "; 
	getline(cin, t.description);
    cout << "Cost (PKR): "; 
	cin >> t.cost; 
	cin.ignore();
    cout << "Is bill paid? (y/n): "; 
	char ch; 
	cin >> ch; 
	cin.ignore();
    t.paid = (ch == 'y' || ch == 'Y');

    // Check patient balance if paying now
    if (t.paid) 
	{
        int pIdx = findPatientIndex(t.patientId);
        if (patients[pIdx].balance < t.cost) 
		{
            cout << "Insufficient balance (Balance: PKR " << patients[pIdx].balance
                 << ", Cost: PKR " << t.cost << "). Cannot mark as paid."<<endl;
            t.paid = false;
        } 
		else 
		{
            patients[pIdx].balance -= t.cost;
            savePatients();
            cout << "Amount deducted. New balance: PKR " << patients[pIdx].balance <<endl;
        }
    }

    Treatment* newArr = new Treatment[treatmentCount + 1];
    for (int i = 0; i < treatmentCount; i++) newArr[i] = treatments[i];
    newArr[treatmentCount] = t;
    delete[] treatments;
    treatments = newArr;
    treatmentCount++;
    saveTreatments();
    cout << "Treatment record added.\n";
}

void updatePayment() 
{
    int patId;
    cout << "\nUpdate Payment Status\n";
    cout << "Patient ID: "; 
	cin >> patId; 
	cin.ignore();
    string desc;
    cout << "Treatment description: "; 
	getline(cin, desc);
    bool found = false;
    for (int i = 0; i < treatmentCount; i++) 
	{
        if (treatments[i].patientId == patId &&
            treatments[i].description.find(desc) != string::npos) 
			{
            if (treatments[i].paid) 
			{ cout << "Already marked Paid."<<endl; 
			continue; 
			}
            int pIdx = findPatientIndex(patId);
            if (pIdx != -1 && patients[pIdx].balance < treatments[i].cost) 
			{
                cout << "Insufficient balance (PKR " << patients[pIdx].balance
                     << "). Cannot pay PKR " << treatments[i].cost <<endl;
                continue;
            }
            treatments[i].paid = true;
            if (pIdx != -1) 
			{
                patients[pIdx].balance -= treatments[i].cost;
                savePatients();
            }
            found = true;
            cout << "Payment updateld for: " << treatments[i].description <<endl;
        }
    }
    if (!found) cout << "No matching unpaid treatment found."<<endl;
    saveTreatments();
}

void generateBill() 
{
    int patId;
    cout << "\nGenerate Bill for Patient ID: "; 
	cin >> patId; 
	cin.ignore();
    int pIdx=findPatientIndex(patId);
    if (pIdx == -1) 
	{ 
	cout << "Patient not found."<<endl; 
	return; 
	}

    cout << "\n========================================\n";
    cout << "       HOSPITAL BILL SUMMARY\n";
    cout << "========================================\n";
    cout << "Patient: " << patients[pIdx].name
         << "  (ID: " << patId << ")\n";
    cout << "Contact: " << patients[pIdx].contact << "\n";
    cout << string(60, '-') << "\n";
    cout << left << setw(28) << "Treatment" << setw(12) << "Cost" << "Status\n";
    cout << string(60, '-') << "\n";

    double totalCost = 0, paidAmt = 0;
    bool anyRecord = false;
    for (int i = 0; i < treatmentCount; i++) 
	{
        if (treatments[i].patientId == patId) 
		{
            cout << left << setw(28) << treatments[i].description
                 << setw(12) << fixed << setprecision(0) << treatments[i].cost
                 << (treatments[i].paid ? "Paid" : "Unpaid") <<endl;
            totalCost += treatments[i].cost;
            if (treatments[i].paid) paidAmt += treatments[i].cost;
            anyRecord = true;
        }
    }
    if (!anyRecord) 
	{ cout << "No treatments on record."<<endl; 
	return; 
	}
    cout << string(60, '-') <<endl;
    cout << right << setw(28) << "Total Cost: " << "PKR " << totalCost <<endl;
    cout << right << setw(28) << "Amount Paid: " << "PKR " << paidAmt <<endl;
    cout << right << setw(28) << "Outstanding: " << "PKR " << (totalCost - paidAmt) <<endl;
    cout << "========================================\n";
}

// ─────────────────────────────────────────────
//  SEARCH / REPORTS
// ─────────────────────────────────────────────
void searchPatientsBydoc_ID() 
{
    int id;
    string name;
    cout << "\nSearch Patient by:\n 1. ID\n 2. Name\nChoice: ";
    int ch; 
	cin >> ch; 
	cin.ignore();
    if (ch == 1) 
	{
        cout << "Enter Patient ID: "; 
		cin >> id; 
		cin.ignore();
        int idx = findPatientIndex(id);
        if (idx == -1) 
		{ cout << "Patient not found.\n"; 
		return; 
		}
        printPatientHeader(); 
		printPatient(patients[idx]);
    } 
	else 
	{
        cout << "Enter name: "; 
		getline(cin, name);
        printPatientHeader();
        bool found = false;
        for (int i = 0; i <patientCount; i++) 
		{
            if (patients[i].name.find(name) != string::npos) 
			{
                printPatient(patients[i]); 
				found = true;
            }
        }
        if (!found) 
	    cout << "No patients found."<<endl;
    }
}
void searchDoctorBySpecialty() 
{
    int ch;
    cout << "\nSearch Doctor by:\n 1. ID\n 2. Specialty\nChoice: "; 
	cin >> ch; 
	cin.ignore();
    printDoctorHeader();
    if (ch == 1) 
	{
        int id; 
		cout << "Doctor ID: "; 
		cin >> id; 
		cin.ignore();
        int idx = findDoctorIndex(id);
        if (idx == -1) 
		cout << "Doctor not found."<<endl;
        else printDoctor(doctors[idx]);
    } 
	else 
	{
        string spec; 
		cout << "Specialty : "; 
		getline(cin, spec);
        bool found = false;
        for (int i = 0; i < doctorCount; i++) 
		{
            if (doctors[i].specialty.find(spec) != string::npos) 
			{
                printDoctor(doctors[i]); found = true;
            }
        }
        if (!found) 
		cout << "No doctors found for that specialty."<<endl;
    }
}

void viewTreatmentsByDoctor() 
{
    int docId;

    cout << "\nSearch treatments by Doctor ID: ";
    cin >> docId;
    cin.ignore();
    
    int dIdx = findDoctorIndex(docId);

    if (dIdx == -1)
    {
      cout << "Doctor not found." << endl;
      return;
    }


    cout << "----------------------------------------------------------------------" << endl;
    cout << "Pat ID\tDescription\t\tCost\tStatus" << endl;
    cout << "----------------------------------------------------------------------" << endl;

    bool found = false;

    // Check every treatment
    for (int t = 0; t < treatmentCount; t++)
    {
        int pid = treatments[t].patientId;

        bool belongsToDoctor = false;

        // Check if patient had appointment with this doctor
        for (int a = 0; a < appointmentCount; a++)
        {
            if (appointments[a].doctorId == docId &&
                appointments[a].patientId == pid)
            {
                belongsToDoctor = true;
                break;
            }
        }

        // Print treatment only once
        if (belongsToDoctor)
        {
            cout << treatments[t].patientId << "\t"
                 << treatments[t].description << "\t\t"
                 << treatments[t].cost << "\t";

            if (treatments[t].paid)
            {
                cout << "Paid" << endl;
            }
            else
            {
                cout << "Unpaid" << endl;
            }

            found = true;
        }
    }

    if (!found)
    {
        cout << "No treatments found for this doctor." << endl;
    }
}
// Sort doctors by experience (bubble sort)
void sortDoctorsByExperience() 
{
    for (int i = 0; i<doctorCount - 1;i++)
        for (int j = 0; j<doctorCount - i - 1; j++)
            if (doctors[j].experience>doctors[j + 1].experience) 
			{
                Doctor tmp = doctors[j]; 
				doctors[j] = doctors[j + 1]; 
				doctors[j + 1] = tmp;
            }
    cout << "\nDoctors sorted by experience (ascending):\n";
    printDoctorHeader();
    for (int i=0;i<doctorCount;i++) 
	printDoctor(doctors[i]);
    saveDoctors();
}
void removeDuplicatePatients() 
{
    loadPatients();
    cout<<"Duplicate check complete. Active records: " << patientCount <<endl;
}

// ─────────────────────────────────────────────
//  MENUS
// ─────────────────────────────────────────────
void patientMenu() 
{
    int ch;
    do {
        cout << "\n--- Patient Management ---\n";
        cout << "1. Add Patient\n2. Update Patient\n3. Delete Patient\n4. View All Patients\n0. Back\n";
        cout << "Choice: "; cin >> ch; cin.ignore();
        switch (ch) {
            case 1: addPatient(); break;
            case 2: updatePatient(); break;
            case 3: deletePatient(); break;
            case 4: viewPatients(); break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }
    } while (ch != 0);
}

void doctorMenu() 
{
    int ch;
    do {
        cout << "\n--- Doctor Management ---\n";
        cout << "1. Add Doctor\n2. Update Doctor\n3. Delete Doctor\n4. View All Doctors\n0. Back\n";
        cout << "Choice: "; 
		cin >> ch; 
		cin.ignore();
        switch (ch) 
		{
            case 1: addDoctor(); 
			break;
            case 2: updateDoctor(); 
			break;
            case 3: deleteDoctor(); 
			break;
            case 4: viewDoctors(); 
			break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }
    } while (ch != 0);
}

void appointmentMenu() {
    int ch;
    do {
        cout << "\n--- Appointments ---\n";
        cout << "1. Schedule Appointment\n2. Cancel Appointment\n3. View All Appointments\n";
        cout << "4. Weekly Schedule \n0. Back\n";
        cout << "Choice: "; 
		cin >> ch; 
		cin.ignore();
        switch (ch) 
		{
            case 1: scheduleAppointment(); break;
            case 2: cancelAppointment(); break;
            case 3: viewAppointments(); break;
            case 4: viewWeeklySchedule(); break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }
    } while (ch != 0);
}

void treatmentMenu() 
{
    int ch;
    do {
        cout << "\n--- Treatments & Billing ---\n";
        cout << "1. Add Treatment Record\n2. View Treatments\n";
        cout << "3. Update Payment Status\n4. Generate Bill\n0. Back\n";
        cout << "Choice: "; cin >> ch; cin.ignore();
        switch (ch) {
            case 1: addTreatment(); 
			break;
            case 2: viewTreatments(); 
			break;
            case 3: updatePayment(); 
			break;
            case 4: generateBill(); 
			break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }
    } while (ch != 0);
}

void searchMenu() 
{
    int ch;
    do {
        cout << "\n--- Search / Reports ---\n";
        cout << "1. Search Patient (by ID or Name)\n";
        cout << "2. Search Doctor (by ID or Specialty)\n";
        cout << "3. View Treatments by Doctor\n";
        cout << "4. Sort Doctors by Experience\n";
        cout << "5. Remove Duplicate Patients (re-scan)\n";
        cout << "0. Back\n";
        cout << "Choice: "; 
		cin >> ch; 
		cin.ignore();
        switch (ch) 
		{
            case 1: searchPatientsBydoc_ID(); 
			break;
            case 2: searchDoctorBySpecialty();
			break;
            case 3: viewTreatmentsByDoctor(); 
			break;
            case 4: sortDoctorsByExperience(); 
			break;
            case 5: removeDuplicatePatients(); 
			break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }
    } while (ch != 0);
}

// ─────────────────────────────────────────────
//  LOGIN
// ─────────────────────────────────────────────
bool login() 
{
    string empId, pass;
    cout << "\n=========================================\n";
    cout << "       HOSPITAL MANAGEMENT SYSTEM\n";
    cout << "=========================================\n";
    cout << "Please login as Employee\n";
    cout << "Enter Employee ID: "; 
	getline(cin, empId);
    cout << "Enter Password: ";    
	getline(cin, pass);
    if (trim(empId) == EMP_ID && trim(pass) == EMP_PASS) 
	{
        cout << "Login Successful!\n";
        return true;
    }
    cout << "Invalid credentials. Access denied.\n";
    return false;
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────
int main() 
{
    cleanFile();  // Always run file cleaning first

    if (!login()) 
	return 0;

    int choice;
    do 
	{
        cout << "\n================ Main Menu ================\n";
        cout << "1. Patient Management\n";
        cout << "2. Doctor Management\n";
        cout << "3. Appointments\n";
        cout << "4. Treatments & Billing\n";
        cout << "5. Search / Reports\n";
        cout << "6. Exit\n";
        cout << "===========================================\n";
        cout << "Enter your choice: "; 
		cin >>choice;	
	    cin.ignore();
        switch (choice) {
            case 1: patientMenu(); 
			break;
            case 2: doctorMenu(); 
			break;
            case 3: appointmentMenu(); 
			break;
            case 4: treatmentMenu(); 
			break;
            case 5: searchMenu(); 
			break;
            case 6: cout << "Exiting. Goodbye!\n"; 
			break;
            default: cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 6);

    // Free dynamic memory
    delete[] patients;
    delete[] doctors;
    delete[] appointments;
    delete[] treatments;
    system("pause");
    return 0;
}
