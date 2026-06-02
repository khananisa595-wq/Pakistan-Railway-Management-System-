#include <iostream>
#include <cstring>   // strcpy, strcmp, strlen
#include <fstream>   // file read/write
#include <sstream>   // parse comma-separated file lines
#include <cctype>    // isdigit()
using namespace std;

// global variables
const int MAX          = 50;
const int MANAGER_CODE = 1234;
const int CLASS_COUNT  = 4;

struct ClassRule
{
    char  name[15];
    float minFare;
    float maxFare;
};

const ClassRule classRules[CLASS_COUNT] = {
    {"Business",  7000.0, 15000.0},
    {"Parlour",   5000.0,  9000.0},
    {"Lower AC",  2500.0,  5500.0},
    {"Economy",    400.0,  1500.0}
};

struct TrainClass {
    int   seats;
    int   available;
    float fare;
};

struct Train {
    char       id[10];
    char       name[30];
    char       from[20];
    char       to[20];
    char       reportTime[10];
    char       departureTime[10];
    int        platform;
    TrainClass cls[CLASS_COUNT];
};

struct Passenger {
    char id[10];
    char name[30];
};

struct Booking {
    char id[10];
    char pid[10];
    char tid[10];
    int  classIdx; 
    int  seat;
};

struct Payment {
    char  bookingId[10];
    char  method[10];
    char  cardNumber[20];
    float amount;
    char  status[10];
};

Train     t[MAX];
Passenger p[MAX];
Booking   b[MAX];
Payment   pay[MAX];

int tCount   = 0;
int pCount   = 0;
int bCount   = 0;
int payCount = 0;

int getValidChoice(int min, int max) {
    int ch;
    while (true) {
        cin >> ch;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "  Invalid! Enter a number: ";
        }
        else {
            if (ch >= min && ch <= max) { return ch; }
            else { cout << "  Invalid! Try again (" << min << "-" << max << "): "; }
        }
    }
}

bool validTime(char time[]) {
    if (strlen(time) != 5)  { return false; }
    if (time[2] != ':')     { return false; }
    if (!isdigit(time[0]) || !isdigit(time[1]) ||
        !isdigit(time[3]) || !isdigit(time[4])) { return false; }
    int hh = (time[0]-'0') * 10 + (time[1]-'0');
    int mm = (time[3]-'0') * 10 + (time[4]-'0');
    return (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59);
}

bool isConflict(char newTime[], int newPlatform) {
    for (int i = 0; i < tCount; i++) {
        if (t[i].platform == newPlatform &&
            strcmp(t[i].departureTime, newTime) == 0) {
            return true;
        }
    }
    return false;
}

bool validateCardLength(const char card[]) {
    if (strlen(card) != 16) { return false; }
    for (int i = 0; i < 16; i++) {
        if (!isdigit(card[i])) { return false; }
    }
    return true;
}

bool luhnCheck(const char card[]) {
    int  sum = 0;
    bool alt = false;
    for (int i = 15; i >= 0; i--) {
        int n = card[i] - '0';
        if (alt) {
            n *= 2;
            if (n > 9) { n -= 9; }
        }
        sum += n;
        alt  = !alt;
    }
    return (sum % 10 == 0);
}

bool validateCVV(const char cvv[]) {
    int len = strlen(cvv);
    if (len < 3 || len > 4) { return false; }
    for (int i = 0; i < len; i++) {
        if (!isdigit(cvv[i])) { return false; }
    }
    return true;
}

int findTrain(char id[]) {
    for (int i = 0; i < tCount; i++) {
        if (strcmp(t[i].id, id) == 0) { return i; }
    }
    return -1;
}

int findPassenger(char id[]) {
    for (int i = 0; i < pCount; i++) {
        if (strcmp(p[i].id, id) == 0) { return i; }
    }
    return -1;
}

void savePayment(Payment px) {
    pay[payCount++] = px;
    ofstream fout;
    fout.open("payments.txt", ios::app);
    fout << px.bookingId << "," << px.method     << ","
         << px.cardNumber << ","
         << px.amount     << "," << px.status    << "\n";
}

void saveAllTrains() {
    ofstream fout;
    fout.open("trains.txt");
    for (int i = 0; i < tCount; i++) {
        fout << t[i].id   << "," << t[i].name << ","
             << t[i].from << "," << t[i].to   << ","
             << t[i].reportTime    << ","
             << t[i].departureTime << ","
             << t[i].platform;
        for (int c = 0; c < CLASS_COUNT; c++) {
            fout << "," << t[i].cls[c].seats
                 << "," << t[i].cls[c].available
                 << "," << t[i].cls[c].fare;
        }
        fout << "\n";
    }
}

void savePassenger(Passenger x) {
    ofstream fout;
    fout.open("passengers.txt", ios::app);
    fout << x.id << "," << x.name << "\n";
}

void saveBooking(Booking x) {
    ofstream fout;
    fout.open("bookings.txt", ios::app);
    fout << x.id  << "," << x.pid      << ","
         << x.tid << "," << x.classIdx << ","
         << x.seat << "\n";
}

void loadTrains() {
    tCount = 0;
    ifstream fin;
    fin.open("trains.txt");
    if (!fin) { return; }
    string line;
    while (getline(fin, line)) {
        if (line.empty()) { continue; }
        stringstream ss(line);
        string temp;
        getline(ss, temp, ','); strcpy(t[tCount].id,            temp.c_str());
        getline(ss, temp, ','); strcpy(t[tCount].name,          temp.c_str());
        getline(ss, temp, ','); strcpy(t[tCount].from,          temp.c_str());
        getline(ss, temp, ','); strcpy(t[tCount].to,            temp.c_str());
        getline(ss, temp, ','); strcpy(t[tCount].reportTime,    temp.c_str());
        getline(ss, temp, ','); strcpy(t[tCount].departureTime, temp.c_str());
        getline(ss, temp, ','); t[tCount].platform = stoi(temp);
        for (int c = 0; c < CLASS_COUNT; c++) {
            getline(ss, temp, ','); t[tCount].cls[c].seats     = stoi(temp);
            getline(ss, temp, ','); t[tCount].cls[c].available = stoi(temp);
            getline(ss, temp, ','); t[tCount].cls[c].fare      = stof(temp);
        }
        tCount++;
    }
}

void loadPassengers() {
    pCount = 0;
    ifstream fin;
    fin.open("passengers.txt");
    if (!fin) { return; }
    string line;
    while (getline(fin, line)) {
        if (line.empty()) { continue; }
        stringstream ss(line);
        string temp;
        getline(ss, temp, ','); strcpy(p[pCount].id,   temp.c_str());
        getline(ss, temp, ','); strcpy(p[pCount].name, temp.c_str());
        pCount++;
    }
}

void loadBookings() {
    bCount = 0;
    ifstream fin;
    fin.open("bookings.txt");
    if (!fin) { return; }
    string line;
    while (getline(fin, line)) {
        if (line.empty()) { continue; }
        stringstream ss(line);
        string temp;
        getline(ss, temp, ','); strcpy(b[bCount].id,  temp.c_str());
        getline(ss, temp, ','); strcpy(b[bCount].pid, temp.c_str());
        getline(ss, temp, ','); strcpy(b[bCount].tid, temp.c_str());
        getline(ss, temp, ','); b[bCount].classIdx = stoi(temp);
        getline(ss, temp, ','); b[bCount].seat     = stoi(temp);
        bCount++;
    }
}

void loadPayments() {
    payCount = 0;
    ifstream fin;
    fin.open("payments.txt");
    if (!fin) { return; }
    string line;
    while (getline(fin, line)) {
        if (line.empty()) { continue; }
        stringstream ss(line);
        string temp;
        getline(ss, temp, ','); strcpy(pay[payCount].bookingId,  temp.c_str());
        getline(ss, temp, ','); strcpy(pay[payCount].method,     temp.c_str());
        getline(ss, temp, ','); strcpy(pay[payCount].cardNumber, temp.c_str());
        getline(ss, temp, ','); pay[payCount].amount = stof(temp);
        getline(ss, temp, ','); strcpy(pay[payCount].status,     temp.c_str());
        payCount++;
    }
}

bool processPayment(const char bookingId[], float amount, const char className[]) {
    cout << "\n  ========================================\n";
    cout << "             PAYMENT PORTAL              \n";
    cout << "  ========================================\n";
    cout << "  Class      : " << className << "\n";
    cout << "  Amount Due : PKR " << amount << "\n";
    cout << "  ----------------------------------------\n";
    cout << "  1. Pay by Card\n";
    cout << "  2. Cancel Booking\n";
    cout << "  Enter choice: ";

    if (getValidChoice(1, 2) == 2) {
        cout << "  Booking cancelled.\n";
        return false;
    }

    Payment px;
    strcpy(px.bookingId, bookingId);
    px.amount = amount;
    strcpy(px.method, "CARD");

    char cardRaw[20], cvv[6];
    int  attempts = 3;

    cout << "\n  Test card: 4111111111111111  CVV: 123\n\n";

    while (attempts > 0) {
        cout << "  --- Attempt " << (3 - attempts + 1) << " of 3 ---\n";

        cout << "  Card Number (16 digits): ";
        cin >> cardRaw;
        if (!validateCardLength(cardRaw)) {
            cout << "  ERROR: Must be 16 numeric digits.\n";
            cout << "  Attempts left: " << --attempts << "\n\n";
            continue;
        }
        if (!luhnCheck(cardRaw)) {
            cout << "  ERROR: Invalid card number. Check for typos.\n";
            cout << "  Attempts left: " << --attempts << "\n\n";
            continue;
        }

        cout << "  CVV: ";
        cin >> cvv;
        if (!validateCVV(cvv)) {
            cout << "  ERROR: CVV must be 3 or 4 digits.\n";
            cout << "  Attempts left: " << --attempts << "\n\n";
            continue;
        }

        strcpy(px.cardNumber, cardRaw);
        strcpy(px.status,     "SUCCESS");
        savePayment(px);

        cout << "\n  ========================================\n";
        cout << "           PAYMENT SUCCESSFUL            \n";
        cout << "  ========================================\n";
        cout << "  Card   : " << cardRaw   << "\n";
        cout << "  Class  : " << className << "\n";
        cout << "  Amount : PKR " << amount << "\n";
        cout << "  Status : APPROVED\n";
        cout << "  ========================================\n";
        return true;
    }

    strcpy(px.cardNumber, "FAILED");
    strcpy(px.status,     "FAILED");
    savePayment(px);
    cout << "\n  Too many failed attempts. Payment DECLINED.\n";
    return false;
}

void viewTrains() {
    if (tCount == 0) { cout << "  No trains in schedule.\n"; return; }

    cout << "\n================================================================\n";
    cout << "               PAKISTAN RAILWAY SCHEDULE                       \n";
    cout << "================================================================\n";

    for (int i = 0; i < tCount; i++) {
        cout << "\n  [" << i+1 << "] " << t[i].name << "  (" << t[i].id << ")\n";
        cout << "      Route   : " << t[i].from << "  -->  " << t[i].to << "\n";
        cout << "      Departs : " << t[i].departureTime
             << "  Report: "       << t[i].reportTime
             << "  Platform: "     << t[i].platform << "\n";
        cout << "      +-----------+----------------+----------+\n";
        cout << "      | Class     | Seats Avail    | Fare PKR |\n";
        cout << "      +-----------+----------------+----------+\n";
        for (int c = 0; c < CLASS_COUNT; c++) {
            if (t[i].cls[c].seats == 0) { continue; }
            cout << "      | " << classRules[c].name;
            int pad = 9 - strlen(classRules[c].name);
            for (int j = 0; j < pad; j++) { cout << " "; }
            cout << " | " << t[i].cls[c].available << "/" << t[i].cls[c].seats;
            int np = 10 - (int)to_string(t[i].cls[c].available).length()
                       - (int)to_string(t[i].cls[c].seats).length();
            for (int j = 0; j < np; j++) { cout << " "; }
            cout << " | " << t[i].cls[c].fare << "    |\n";
        }
        cout << "      +-----------+----------------+----------+\n";
    }

    cout << "\n================================================================\n";
}

void addTrain() {
    Train &x = t[tCount];

    cout << "\n-- Add New Train --\n";
    cout << "  Train ID        : ";
    cin >> x.id;
    cin.ignore();

    for (int i = 0; i < tCount; i++) {
        if (strcmp(t[i].id, x.id) == 0) {
            cout << "  ERROR: Train ID '" << x.id << "' already exists!\n";
            return;
        }
    }

    cout << "  Train Name      : "; cin.getline(x.name, 30);
    cout << "  From            : "; cin.getline(x.from, 20);
    cout << "  To              : "; cin.getline(x.to,   20);

    cout << "  Report Time  (HH:MM): "; cin >> x.reportTime;
    while (!validTime(x.reportTime)) {
        cout << "  Invalid! Enter (HH:MM): "; cin >> x.reportTime;
    }

    cout << "  Departure (HH:MM): "; cin >> x.departureTime;
    while (!validTime(x.departureTime)) {
        cout << "  Invalid! Enter (HH:MM): "; cin >> x.departureTime;
    }

    cout << "  Platform No     : "; cin >> x.platform;
    if (isConflict(x.departureTime, x.platform)) {
        cout << "  ERROR: Platform " << x.platform
             << " already used at "  << x.departureTime << "!\n";
        return;
    }

    cout << "\n  Enter seats and fare for each class:\n";
    cout << "  +---+-----------+---------------------+\n";
    cout << "  |   | Class     | Allowed Fare (PKR)  |\n";
    cout << "  +---+-----------+---------------------+\n";
    for (int c = 0; c < CLASS_COUNT; c++) {
        cout << "  | " << c+1 << " | " << classRules[c].name;
        int pad = 9 - strlen(classRules[c].name);
        for (int j = 0; j < pad; j++) { cout << " "; }
        cout << " | " << classRules[c].minFare
             << " - " << classRules[c].maxFare << "         |\n";
    }
    cout << "  +---+-----------+---------------------+\n\n";

    for (int c = 0; c < CLASS_COUNT; c++) {
        cout << "  --- " << classRules[c].name << " ---\n";
        cout << "  Seats (0 to skip): ";
        while (true) {
            cin >> x.cls[c].seats;
            if (!cin.fail() && x.cls[c].seats >= 0) { break; }
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "  Invalid! Enter seats: ";
        }
        x.cls[c].available = x.cls[c].seats;

        if (x.cls[c].seats == 0) {
            x.cls[c].fare = 0;
            cout << "  (Skipped)\n\n";
            continue;
        }

        cout << "  Fare PKR (" << classRules[c].minFare
             << " - " << classRules[c].maxFare << "): ";
        while (true) {
            cin >> x.cls[c].fare;
            if (cin.fail()) {
                cin.clear(); cin.ignore(10000, '\n');
                cout << "  Invalid: "; continue;
            }
            if (x.cls[c].fare < classRules[c].minFare) {
                cout << "  Min PKR " << classRules[c].minFare << ": "; continue;
            }
            if (x.cls[c].fare > classRules[c].maxFare) {
                cout << "  Max PKR " << classRules[c].maxFare << ": "; continue;
            }
            break;
        }
        cout << "\n";
    }

    tCount++;
    saveAllTrains();

    cout << "  Train added successfully!\n";
    cout << "  +-----------+-------+----------+\n";
    cout << "  | Class     | Seats | Fare PKR |\n";
    cout << "  +-----------+-------+----------+\n";
    for (int c = 0; c < CLASS_COUNT; c++) {
        if (x.cls[c].seats == 0) { continue; }
        cout << "  | " << classRules[c].name;
        int pad = 9 - strlen(classRules[c].name);
        for (int j = 0; j < pad; j++) { cout << " "; }
        cout << " | " << x.cls[c].seats << "     | " << x.cls[c].fare << " |\n";
    }
    cout << "  +-----------+-------+----------+\n";
}

void removeTrain() {
    char id[10];
    cout << "  Enter Train ID to remove: "; cin >> id;

    int idx = -1;
    for (int i = 0; i < tCount; i++) {
        if (strcmp(t[i].id, id) == 0) { idx = i; break; }
    }

    if (idx == -1) { cout << "  Train not found!\n"; return; }

    for (int i = 0; i < bCount; i++) {
        if (strcmp(b[i].tid, id) == 0) {
            cout << "  Cannot remove — active bookings exist for this train.\n";
            return;
        }
    }

    for (int i = idx; i < tCount - 1; i++) {
        t[i] = t[i+1];
    }

    tCount--;
    saveAllTrains();
    cout << "  Train removed.\n";
}

void addPassenger() {
    Passenger &x = p[pCount];
    cout << "  Passenger ID: ";
    cin >> x.id;

    for (int i = 0; i < pCount; i++) {
        if (strcmp(p[i].id, x.id) == 0) {
            cout << "  ERROR: ID '" << x.id << "' already exists!\n";
            return;
        }
    }

    cin.ignore();
    cout << "  Name        : ";
    cin.getline(x.name, 30);
    pCount++;
    savePassenger(x);
    cout << "  Passenger saved!\n";
}

void printTicketForPassenger() {
    char pid[10], bid[10];

    cout << "  Enter your Passenger ID: ";
    cin >> pid;

    int pi = findPassenger(pid);
    if (pi == -1) {
        cout << "  Passenger ID not found.\n";
        return;
    }

    cout << "  Enter your Booking ID  : ";
    cin >> bid;

    for (int i = 0; i < bCount; i++) {
        if (strcmp(b[i].id,  bid) == 0 &&
            strcmp(b[i].pid, pid) == 0) {

            int ti = findTrain(b[i].tid);
            int ci = b[i].classIdx;

            cout << "\n  ============================================\n";
            cout << "          PAKISTAN RAILWAY TICKET            \n";
            cout << "  ============================================\n";
            cout << "  Booking ID : " << b[i].id    << "\n";
            cout << "  Passenger  : " << p[pi].name << "\n";
            if (ti != -1) {
                cout << "  Train      : " << t[ti].name << "\n";
                cout << "  Class      : " << classRules[ci].name << "\n";
                cout << "  Route      : " << t[ti].from << " --> " << t[ti].to << "\n";
                cout << "  Departure  : " << t[ti].departureTime << "\n";
                cout << "  Platform   : " << t[ti].platform      << "\n";
                cout << "  Fare Paid  : PKR " << t[ti].cls[ci].fare << "\n";
            }
            cout << "  Seat No    : " << b[i].seat << "\n";
            cout << "  ============================================\n";
            return;
        }
    }

    cout << "  No booking found with this ID under your account.\n";
}

void printTicketForManager() {
    char bid[10];
    cout << "  Enter Booking ID: "; cin >> bid;

    for (int i = 0; i < bCount; i++) {
        if (strcmp(b[i].id, bid) == 0) {
            int pi = findPassenger(b[i].pid);
            int ti = findTrain(b[i].tid);
            int ci = b[i].classIdx;

            cout << "\n  ============================================\n";
            cout << "          PAKISTAN RAILWAY TICKET            \n";
            cout << "  ============================================\n";
            cout << "  Booking ID : " << b[i].id << "\n";
            cout << "  Passenger  : ";
            if (pi != -1) { cout << p[pi].name << " (ID: " << b[i].pid << ")"; }
            else          { cout << b[i].pid; }
            cout << "\n";
            if (ti != -1) {
                cout << "  Train      : " << t[ti].name << "\n";
                cout << "  Class      : " << classRules[ci].name << "\n";
                cout << "  Route      : " << t[ti].from << " --> " << t[ti].to << "\n";
                cout << "  Departure  : " << t[ti].departureTime << "\n";
                cout << "  Platform   : " << t[ti].platform      << "\n";
                cout << "  Fare Paid  : PKR " << t[ti].cls[ci].fare << "\n";
            }
            cout << "  Seat No    : " << b[i].seat << "\n";
            cout << "  ============================================\n";
            return;
        }
    }
    cout << "  Booking not found!\n";
}

void bookTicketManager() {
    char pid[10], tid[10];
    cout << "  Passenger ID: "; cin >> pid;
    cout << "  Train ID    : "; cin >> tid;

    int pi = findPassenger(pid);
    int ti = findTrain(tid);
    if (pi == -1 || ti == -1) { cout << "  Invalid ID!\n"; return; }

    cout << "\n  Available classes:\n";
    bool any = false;
    for (int c = 0; c < CLASS_COUNT; c++) {
        if (t[ti].cls[c].seats == 0) { continue; }
        cout << "  " << c+1 << ". " << classRules[c].name
             << "  Seats: " << t[ti].cls[c].available
             << "/" << t[ti].cls[c].seats
             << "  PKR " << t[ti].cls[c].fare << "\n";
        any = true;
    }
    if (!any) { cout << "  No seats available!\n"; return; }

    cout << "  Select class (1-4): ";
    int ci = getValidChoice(1, 4) - 1;

    if (t[ti].cls[ci].seats     == 0) { cout << "  Class not on this train!\n"; return; }
    if (t[ti].cls[ci].available <= 0) { cout << "  No seats left!\n"; return; }

    Booking &x = b[bCount];
    cout << "  Booking ID: "; cin >> x.id;

    for (int i = 0; i < bCount; i++) {
        if (strcmp(b[i].id, x.id) == 0) {
            cout << "  ERROR: Booking ID already exists!\n"; return;
        }
    }

    int seatNo;
    while (true) {
        cout << "  Seat No (1-" << t[ti].cls[ci].seats << "): ";
        cin >> seatNo;
        if (seatNo < 1 || seatNo > t[ti].cls[ci].seats) {
            cout << "  Invalid seat!\n"; continue;
        }
        bool taken = false;
        for (int i = 0; i < bCount; i++) {
            if (strcmp(b[i].tid, tid) == 0 &&
                b[i].classIdx == ci        &&
                b[i].seat     == seatNo) { taken = true; break; }
        }
        if (taken) {
            cout << "  Seat " << seatNo << " already booked! Choose another.\n";
            continue;
        }
        break;
    }
    x.seat = seatNo;
    strcpy(x.pid, pid);
    strcpy(x.tid, tid);
    x.classIdx = ci;
    t[ti].cls[ci].available--;
    bCount++;
    saveBooking(x);
    saveAllTrains();

    cout << "  Booking Done! Class: " << classRules[ci].name
         << "  Fare: PKR " << t[ti].cls[ci].fare << "\n";
}

bool managerLogin() {
    int code;
    cout << "\n  *** MANAGER LOGIN ***\n";
    cout << "  Enter Manager Code: ";
    cin >> code;
    if (code == MANAGER_CODE) { cout << "  Access Granted!\n"; return true; }
    cout << "  Wrong code! Access Denied.\n";
    return false;
}

void trainMenu() {
    int ch;
    do {
        cout << "\n  --- Train Management ---\n";
        cout << "  1. Add Train\n";
        cout << "  2. View Schedule\n";
        cout << "  3. Remove Train\n";
        cout << "  4. Back\n";
        cout << "  Enter choice: ";
        ch = getValidChoice(1, 4);
        if      (ch == 1) { addTrain(); }
        else if (ch == 2) { viewTrains(); }
        else if (ch == 3) { removeTrain(); }
    } while (ch != 4);
}

void managerMenu() {
    if (!managerLogin()) { return; }
    int ch;
    do {
        cout << "\n  ===== MANAGER PORTAL =====\n";
        cout << "  1. Train Management\n";
        cout << "  2. Add Passenger\n";
        cout << "  3. Book Ticket (Counter)\n";
        cout << "  4. Print Ticket\n";
        cout << "  5. Logout\n";
        cout << "  Enter choice: ";
        ch = getValidChoice(1, 5);
        if      (ch == 1) { trainMenu(); }
        else if (ch == 2) { addPassenger(); }
        else if (ch == 3) { bookTicketManager(); }
        else if (ch == 4) { printTicketForManager(); }
    } while (ch != 5);
    cout << "  Logged out.\n";
}

void passengerMenu() {
    int ch;
    do {
        cout << "\n  ===== PASSENGER PORTAL =====\n";
        cout << "  1. View Train Schedule\n";
        cout << "  2. Book Ticket\n";
        cout << "  3. Print My Ticket\n";
        cout << "  4. Back\n";
        cout << "  Enter choice: ";
        ch = getValidChoice(1, 4);

        if (ch == 1) {
            viewTrains();

        } else if (ch == 2) {

            char pid[10];
            cout << "\n  Enter your Passenger ID (e.g. P001): ";
            cin >> pid;

            if (findPassenger(pid) == -1) {
                cout << "  Not found. Register? (1=Yes, 2=No): ";
                if (getValidChoice(1, 2) == 1) {
                    Passenger &x = p[pCount];
                    strcpy(x.id, pid);
                    cin.ignore();
                    cout << "  Your Name: "; cin.getline(x.name, 30);
                    pCount++;
                    savePassenger(x);
                    cout << "\n  *** Save your details ***\n";
                    cout << "  Passenger ID : " << pid    << "\n";
                    cout << "  Name         : " << x.name << "\n";
                    cout << "  *************************\n\n";
                } else { continue; }
            } else {
                cout << "  Welcome back, " << p[findPassenger(pid)].name << "!\n";
            }

            viewTrains();
            char tid[10];
            cout << "  Enter Train ID: ";
            cin >> tid;
            int ti = findTrain(tid);
            if (ti == -1) { cout << "  Train not found!\n"; continue; }

            cout << "\n  Available classes:\n";
            bool any = false;
            for (int c = 0; c < CLASS_COUNT; c++) {
                if (t[ti].cls[c].seats == 0) { continue; }
                cout << "  " << c+1 << ". " << classRules[c].name
                     << "  Seats: " << t[ti].cls[c].available
                     << "  Fare: PKR " << t[ti].cls[c].fare << "\n";
                any = true;
            }
            if (!any) { cout << "  No seats available!\n"; continue; }

            cout << "  Select class (1-4): ";
            int ci = getValidChoice(1, 4) - 1;
            if (t[ti].cls[ci].seats     == 0) { cout << "  Not on this train!\n"; continue; }
            if (t[ti].cls[ci].available <= 0) { cout << "  No seats left!\n"; continue; }

            int seatNo;
            while (true) {
                
                cout << "  Seat No (1-" << t[ti].cls[ci].seats << "): ";
                cin >> seatNo;
                if (seatNo < 1 || seatNo > t[ti].cls[ci].seats) {
                    cout << "  Invalid seat!\n"; continue;
                }
                bool taken = false;
                for (int i = 0; i < bCount; i++) {
                    if (strcmp(b[i].tid, tid) == 0 &&
                        b[i].classIdx == ci        &&
                        b[i].seat     == seatNo) { taken = true; break; }
                }
                if (taken) {
                    
                    cout << "  Seat " << seatNo << " already booked! Choose another.\n";
                    continue;
                }
                break;
            }

            char bkId[10];
            while (true) {
                cout << "  Booking ID (e.g. B001): ";
                cin >> bkId;
                bool exists = false;
                for (int i = 0; i < bCount; i++) {
                    if (strcmp(b[i].id, bkId) == 0) { exists = true; break; }
                }
                if (exists) { cout << "  ID taken! Enter another.\n"; continue; }
                break;
            }

            int pi = findPassenger(pid);
            cout << "\n  ======== Booking Summary ========\n";
            cout << "  Passenger : ";
            if (pi != -1) { cout << p[pi].name; }
            else { cout << pid; }
            cout << "\n";
            cout << "  Train     : " << t[ti].name          << "\n";
            cout << "  Class     : " << classRules[ci].name << "\n";
            cout << "  Route     : " << t[ti].from << " --> " << t[ti].to << "\n";
            cout << "  Departure : " << t[ti].departureTime << "\n";
            cout << "  Platform  : " << t[ti].platform      << "\n";
            cout << "  Seat      : " << seatNo               << "\n";
            cout << "  Fare      : PKR " << t[ti].cls[ci].fare << "\n";
            cout << "  =================================\n";
            cout << "  Confirm? (1=Yes, 2=No): ";
            if (getValidChoice(1, 2) == 2) { cout << "  Cancelled.\n"; continue; }

            bool paid = processPayment(bkId, t[ti].cls[ci].fare, classRules[ci].name);
            if (!paid) { cout << "  Booking NOT saved.\n"; continue; }

            Booking &bk = b[bCount];
            strcpy(bk.id,  bkId);
            strcpy(bk.pid, pid);
            strcpy(bk.tid, tid);
            bk.classIdx = ci;
            bk.seat     = seatNo;
            t[ti].cls[ci].available--;
            bCount++;
            saveBooking(bk);
            saveAllTrains();

            cout << "\n  *** Booking Confirmed! ***\n";
            cout << "  Passenger ID : " << pid   << "\n";
            cout << "  Booking ID   : " << bkId  << "\n";
            cout << "  Class        : " << classRules[ci].name    << "\n";
            cout << "  Seat         : " << seatNo                  << "\n";
            cout << "  Fare Paid    : PKR " << t[ti].cls[ci].fare << "\n";
            cout << "  Save both IDs to print your ticket later.\n";

        } else if (ch == 3) {
            printTicketForPassenger();
        }

    } while (ch != 4);
}

int main() {
    loadTrains();
    loadPassengers();
    loadBookings();
    loadPayments();

    int ch;
    do {
        cout << "\n  ==============================\n";
        cout << "    PAKISTAN RAILWAY SYSTEM    \n";
        cout << "  ==============================\n";
        cout << "  1. Train Manager\n";
        cout << "  2. Passenger\n";
        cout << "  3. Exit\n";
        cout << "  Enter choice: ";
        ch = getValidChoice(1, 3);
        if      (ch == 1) { managerMenu(); }
        else if (ch == 2) { passengerMenu(); }
    } while (ch != 3);

    cout << "\n  Thank you for using Pakistan Railway System!\n";
    return 0;
}
