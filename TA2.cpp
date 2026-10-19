#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <chrono>
#include <cstdlib>
#include <ctime>
using namespace std;
using Clock = chrono::high_resolution_clock;

const int UI_WIDTH = 74;
const int BENCH_WIDTH = 100;

struct Student {
    int id;
    string name, service;
    int priority;
    double gpa;
    long long timestamp;

    Student() : id(0), priority(0), gpa(0), timestamp(0) {}

    Student(int i, string n, string s, int p, double g)
        : id(i), name(n), service(s), priority(p), gpa(g),
          timestamp(time(NULL)) {}
};

struct Node {
    Student value;
    Node* next;

    Node(const Student& s) : value(s), next(NULL) {}
};

struct Action {
    int type, id;
    Student saved;
    double oldGpa;
    Action* next;

    Action() : type(0), id(0), oldGpa(0), next(NULL) {}
};

struct QNode {
    Student value;
    QNode* next;

    QNode(const Student& s) : value(s), next(NULL) {}
};

int getInt(const string& prompt, int lo, int hi) {
    int x;

    for (;;) {
        cout << prompt;

        if (cin >> x && x >= lo && x <= hi) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return x;
        }

        cout << "  Invalid input. Enter a value from "
             << lo << " to " << hi << ".\n";

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double getDouble(const string& prompt, double lo, double hi) {
    double x;

    for (;;) {
        cout << prompt;

        if (cin >> x && x >= lo && x <= hi) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return x;
        }

        cout << "  Invalid number. Enter a value from "
             << lo << " to " << hi << ".\n";

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string getText(const string& prompt) {
    string s;

    do {
        cout << prompt;
        getline(cin, s);

        if (s.empty())
            cout << "  This field cannot be empty.\n";

    } while (s.empty());

    return s;
}

void show(const Student& s) {
    cout << left
         << setw(12) << s.id
         << setw(24) << s.name.substr(0, 23)
         << setw(22) << s.service.substr(0, 21)
         << setw(12) << (s.priority ? "Priority" : "Regular")
         << right << setw(4) << fixed << setprecision(2)
         << s.gpa << '\n';
}

void showStudentTableHeader() {
    cout << left
         << setw(12) << "ID"
         << setw(24) << "Name"
         << setw(22) << "Service"
         << setw(12) << "Lane"
         << right << setw(4) << "GPA" << '\n';

    for (int i = 0; i < UI_WIDTH; ++i)
        cout << '-';

    cout << '\n';
}

class Registry {
    Node* head;

public:
    Registry() : head(NULL) {}
    ~Registry() { clear(); }

    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;

    bool exists(int id) const {
        for (Node* p = head; p; p = p->next) {
            if (p->value.id == id)
                return true;
        }

        return false;
    }

    bool add(const Student& s) {
        if (exists(s.id))
            return false;

        Node* n = new Node(s);

        if (!head) {
            head = n;
            return true;
        }

        Node* p = head;

        while (p->next)
            p = p->next;

        p->next = n;
        return true;
    }

    Student* find(int id) {
        for (Node* p = head; p; p = p->next) {
            if (p->value.id == id)
                return &p->value;
        }

        return NULL;
    }

    bool remove(int id, Student& out) {
        Node* p = head;
        Node* prev = NULL;

        while (p && p->value.id != id) {
            prev = p;
            p = p->next;
        }

        if (!p)
            return false;

        out = p->value;

        if (prev)
            prev->next = p->next;
        else
            head = p->next;

        delete p;
        return true;
    }

    int count() const {
        int n = 0;

        for (Node* p = head; p; p = p->next)
            ++n;

        return n;
    }

    void display() const {
        if (!head) {
            cout << "  Registry is empty.\n";
            return;
        }

        showStudentTableHeader();

        for (Node* p = head; p; p = p->next)
            show(p->value);

        for (int i = 0; i < UI_WIDTH; ++i)
            cout << '-';

        cout << '\n';
    }

    void copyIds(int* output) const {
        int i = 0;

        for (Node* p = head; p; p = p->next)
            output[i++] = p->value.id;
    }

    void clear() {
        while (head) {
            Node* t = head;
            head = head->next;
            delete t;
        }
    }
};

class UndoStack {
    Action* top;

public:
    UndoStack() : top(NULL) {}
    ~UndoStack() { clear(); }

    void deleted(const Student& s) {
        Action* a = new Action();
        a->type = 1;
        a->saved = s;
        a->next = top;
        top = a;
    }

    void gpaChanged(int id, double old) {
        Action* a = new Action();
        a->type = 2;
        a->id = id;
        a->oldGpa = old;
        a->next = top;
        top = a;
    }

    bool pop(Action& out) {
        if (!top)
            return false;

        Action* t = top;
        out = *t;
        top = top->next;

        delete t;
        out.next = NULL;

        return true;
    }

    void latest() const {
        if (!top) {
            cout << "  Undo history is empty.\n";
        } else if (top->type == 1) {
            cout << "  Latest action : Record deletion\n";
            cout << "  Student ID    : " << top->saved.id << '\n';
            cout << "  Student name  : " << top->saved.name << '\n';
            cout << "  Status        : Can be restored\n";
        } else {
            cout << "  Latest action : GPA update\n";
            cout << "  Student ID    : " << top->id << '\n';
            cout << "  Previous GPA  : " << fixed
                 << setprecision(2) << top->oldGpa << '\n';
        }
    }

    void display() const {
        if (!top) {
            cout << "  Undo/audit trail is empty.\n";
            return;
        }

        cout << '\n';
        cout << left
             << setw(7) << "No."
             << setw(22) << "Action"
             << setw(14) << "Student ID"
             << setw(31) << "Details" << '\n';

        for (int i = 0; i < UI_WIDTH; ++i)
            cout << '-';

        cout << '\n';

        int i = 1;

        for (Action* p = top; p; p = p->next) {
            cout << left << setw(7) << i++;

            if (p->type == 1) {
                cout << setw(22) << "Record deletion"
                     << setw(14) << p->saved.id
                     << p->saved.name.substr(0, 30) << '\n';
            } else {
                cout << setw(22) << "GPA update"
                     << setw(14) << p->id
                     << "Previous GPA: "
                     << fixed << setprecision(2)
                     << p->oldGpa << '\n';
            }
        }

        for (int i = 0; i < UI_WIDTH; ++i)
            cout << '-';

        cout << '\n';
    }

    void clear() {
        while (top) {
            Action* t = top;
            top = top->next;
            delete t;
        }
    }
};

class Queue {
    QNode* front;
    QNode* rear;
    int n;
    bool priorityMode;

public:
    Queue(bool pr = false)
        : front(NULL), rear(NULL), n(0), priorityMode(pr) {}

    ~Queue() { clear(); }

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void enqueue(const Student& s) {
        QNode* x = new QNode(s);

        if (priorityMode) {
            if (!front || s.priority > front->value.priority) {
                x->next = front;
                front = x;

                if (!rear)
                    rear = x;
            } else {
                QNode* p = front;

                while (p->next &&
                       p->next->value.priority >= s.priority)
                    p = p->next;

                x->next = p->next;
                p->next = x;

                if (!x->next)
                    rear = x;
            }
        } else {
            if (!rear)
                front = rear = x;
            else {
                rear->next = x;
                rear = x;
            }
        }

        ++n;
    }

    bool peek(Student& out) const {
        if (!front)
            return false;

        out = front->value;
        return true;
    }

    bool dequeue(Student& out) {
        if (!front)
            return false;

        QNode* t = front;
        out = t->value;
        front = front->next;

        if (!front)
            rear = NULL;

        delete t;
        --n;

        return true;
    }

    void display(const string& title) const {
        cout << '\n';
        cout << title << " (" << n << " student";

        if (n != 1)
            cout << 's';

        cout << ")\n";

        if (!front) {
            cout << "  Queue is empty.\n";
            return;
        }

        showStudentTableHeader();

        for (QNode* p = front; p; p = p->next)
            show(p->value);

        for (int i = 0; i < UI_WIDTH; ++i)
            cout << '-';

        cout << '\n';
    }

    void clear() {
        while (front) {
            QNode* t = front;
            front = front->next;
            delete t;
        }

        rear = NULL;
        n = 0;
    }
};

struct Metrics {
    long long comparisons = 0;
    long long swaps = 0;
    double ms = 0;
};

class CLIUI {
public:
    static void clearScreen() {
        cout << "\033[2J\033[H";
    }

    static void line(char symbol = '=', int width = UI_WIDTH) {
        for (int i = 0; i < width; ++i)
            cout << symbol;

        cout << '\n';
    }

    static void header(const string& title) {
        line('=');

        int padding = (UI_WIDTH - static_cast<int>(title.length())) / 2;

        if (padding < 0)
            padding = 0;

        cout << string(padding, ' ') << title << '\n';

        line('=');
    }

    static void section(const string& title) {
        line('-');
        cout << "  " << title << '\n';
        line('-');
    }

    static void menuOption(int number, const string& label) {
        cout << "  [" << right << setw(2) << number << "]  "
             << left << label << '\n';
    }

    static void success(const string& message) {
        cout << "\n  [SUCCESS] " << message << '\n';
    }

    static void error(const string& message) {
        cout << "\n  [ERROR]   " << message << '\n';
    }

    static void info(const string& message) {
        cout << "\n  [INFO]    " << message << '\n';
    }

    static void pause() {
        cout << "\n";
        line('-');
        cout << "  Press ENTER to return to the main menu...";
        cin.get();
    }

    static void showMainMenu() {
        header("UNIVERSITY STUDENT SERVICE & QUEUE SYSTEM");

        section("STUDENT RECORD MANAGEMENT");
        menuOption(1, "Add Student Record");
        menuOption(2, "Display Student Registry");
        menuOption(3, "Search Student by ID");
        menuOption(4, "Delete Student Record");
        menuOption(5, "Update Student GPA");
        menuOption(6, "Undo Last Change");

        section("QUEUE MANAGEMENT");
        menuOption(7, "Display Service Queues");
        menuOption(8, "Serve Next Student");
        menuOption(9, "Count Student Records");
        menuOption(10, "Peek Next Student in Queue");

        section("SEARCH, SORTING & PERFORMANCE");
        menuOption(11, "Search and Sort Demo");
        menuOption(12, "Benchmarking Suite");
        menuOption(13, "Multi-Field Sorting");

        section("UNDO & AUDIT HISTORY");
        menuOption(14, "Latest Undo Action");
        menuOption(15, "Display Undo/Audit Trail");

        line('=');
        menuOption(0, "Exit Application");
        line('=');
    }

    static void showExitScreen() {
        clearScreen();
        header("APPLICATION TERMINATED");
        cout << "  Thank you for using the system.\n";
        cout << "  Dynamic memory will be released on exit.\n";
        line('=');
    }
};

int linearSearch(const int* a, int n, int target, Metrics& m) {
    m = Metrics();
    auto t = Clock::now();

    for (int i = 0; i < n; ++i) {
        ++m.comparisons;

        if (a[i] == target) {
            m.ms = chrono::duration<double, milli>(
                Clock::now() - t).count();
            return i;
        }
    }

    m.ms = chrono::duration<double, milli>(
        Clock::now() - t).count();

    return -1;
}

int binarySearch(const int* a, int n, int target, Metrics& m) {
    m = Metrics();
    auto t = Clock::now();

    int l = 0;
    int r = n - 1;

    while (l <= r) {
        int mid = l + (r - l) / 2;
        ++m.comparisons;

        if (a[mid] == target) {
            m.ms = chrono::duration<double, milli>(
                Clock::now() - t).count();
            return mid;
        }

        ++m.comparisons;

        if (a[mid] < target)
            l = mid + 1;
        else
            r = mid - 1;
    }

    m.ms = chrono::duration<double, milli>(
        Clock::now() - t).count();

    return -1;
}

void bubble(int* a, int n, Metrics& m) {
    m = Metrics();
    auto t = Clock::now();

    for (int i = 0; i < n - 1; ++i) {
        bool changed = false;

        for (int j = 0; j < n - i - 1; ++j) {
            ++m.comparisons;

            if (a[j] > a[j + 1]) {
                int x = a[j];
                a[j] = a[j + 1];
                a[j + 1] = x;
                ++m.swaps;
                changed = true;
            }
        }

        if (!changed)
            break;
    }

    m.ms = chrono::duration<double, milli>(
        Clock::now() - t).count();
}

void selection(int* a, int n, Metrics& m) {
    m = Metrics();
    auto t = Clock::now();

    for (int i = 0; i < n - 1; ++i) {
        int k = i;

        for (int j = i + 1; j < n; ++j) {
            ++m.comparisons;

            if (a[j] < a[k])
                k = j;
        }

        if (k != i) {
            int x = a[i];
            a[i] = a[k];
            a[k] = x;
            ++m.swaps;
        }
    }

    m.ms = chrono::duration<double, milli>(
        Clock::now() - t).count();
}

void insertion(int* a, int n, Metrics& m) {
    m = Metrics();
    auto t = Clock::now();

    for (int i = 1; i < n; ++i) {
        int key = a[i];
        int j = i - 1;

        while (j >= 0) {
            ++m.comparisons;

            if (a[j] > key) {
                a[j + 1] = a[j];
                ++m.swaps;
                --j;
            } else {
                break;
            }
        }

        a[j + 1] = key;
    }

    m.ms = chrono::duration<double, milli>(
        Clock::now() - t).count();
}

void metric(const char* name, int n, const Metrics& m,
            const char* bigO) {
    cout << left
         << setw(18) << name
         << setw(9) << n
         << setw(15) << m.comparisons
         << setw(12) << m.swaps
         << setw(12) << fixed << setprecision(3) << m.ms
         << setw(8) << bigO << '\n';
}

void benchmarkMetric(const char* name, const char* profile,
                     int n, const Metrics& m, const char* bigO) {
    cout << left
         << setw(18) << name
         << setw(20) << profile
         << setw(10) << n
         << setw(16) << m.comparisons
         << setw(14) << m.swaps
         << setw(14) << fixed << setprecision(3) << m.ms
         << setw(8) << bigO << '\n';
}

void benchmark() {
    int sizes[] = {100, 1000, 10000, 50000};
    int totalSizes = 4;

    cout << '\n';
    CLIUI::line('=', BENCH_WIDTH);

    string title = "BENCHMARK SUITE PERFORMANCE METRICS REPORT";
    int padding = (BENCH_WIDTH - static_cast<int>(title.length())) / 2;
    cout << string(padding, ' ') << title << '\n';

    CLIUI::line('=', BENCH_WIDTH);

    cout << left
         << setw(18) << "Algorithm"
         << setw(20) << "Data Profile"
         << setw(10) << "N"
         << setw(16) << "Comparisons"
         << setw(14) << "Swaps/Shifts"
         << setw(14) << "Time(ms)"
         << setw(8) << "Big-O" << '\n';

    CLIUI::line('-', BENCH_WIDTH);

    for (int sizeIndex = 0; sizeIndex < totalSizes; ++sizeIndex) {
        int n = sizes[sizeIndex];

        if (n >= 10000) {
            cout << "\n  The next benchmark uses N = " << n << ".\n";
            cout << "  Quadratic sorting algorithms may take a long time.\n";

            int proceed = getInt(
                "  Run this dataset size? (1 = Yes, 0 = Skip): ", 0, 1);

            if (proceed == 0) {
                cout << "  Skipped N = " << n << ".\n";
                continue;
            }
        }

        int* randomData = new int[n];
        int* sortedData = new int[n];
        int* reverseData = new int[n];
        int* work = new int[n];

        for (int i = 0; i < n; ++i) {
            randomData[i] = rand() % (n * 10 + 1);
            sortedData[i] = i;
            reverseData[i] = n - i;
        }

        Metrics m;

        linearSearch(randomData, n, randomData[n - 1], m);
        benchmarkMetric("Linear Search", "Random Unsorted",
                        n, m, "O(n)");

        binarySearch(sortedData, n, n / 2, m);
        benchmarkMetric("Binary Search", "Already Sorted",
                        n, m, "O(log n)");

        int* profiles[] = {
            randomData,
            sortedData,
            reverseData
        };

        const char* profileNames[] = {
            "Random Unsorted",
            "Already Sorted",
            "Reverse Sorted"
        };

        for (int profile = 0; profile < 3; ++profile) {
            for (int i = 0; i < n; ++i)
                work[i] = profiles[profile][i];

            bubble(work, n, m);
            benchmarkMetric("Bubble Sort", profileNames[profile],
                            n, m, "O(n^2)");

            for (int i = 0; i < n; ++i)
                work[i] = profiles[profile][i];

            selection(work, n, m);
            benchmarkMetric("Selection Sort", profileNames[profile],
                            n, m, "O(n^2)");

            for (int i = 0; i < n; ++i)
                work[i] = profiles[profile][i];

            insertion(work, n, m);
            benchmarkMetric("Insertion Sort", profileNames[profile],
                            n, m, "O(n^2)");
        }

        CLIUI::line('-', BENCH_WIDTH);

        delete[] randomData;
        delete[] sortedData;
        delete[] reverseData;
        delete[] work;
    }

    cout << "\n  Benchmarking completed.\n";
    cout << "  Insertion Sort's swaps/shifts metric counts shifts.\n";
    cout << "  Search timing excludes preparation and sorting time.\n";
    cout << "  Big-O for sorting shows worst-case time complexity.\n";

    CLIUI::line('=', BENCH_WIDTH);
}

bool studentGreater(const Student& a, const Student& b,
                    int field, Metrics& m) {
    ++m.comparisons;

    if (field == 1)
        return a.id > b.id;

    if (field == 2)
        return a.name > b.name;

    if (field == 3)
        return a.gpa > b.gpa;

    return a.priority > b.priority;
}

void bubbleStudents(Student* a, int n, int field, Metrics& m) {
    m = Metrics();
    auto t = Clock::now();

    for (int i = 0; i < n - 1; ++i) {
        bool changed = false;

        for (int j = 0; j < n - i - 1; ++j) {
            if (studentGreater(a[j], a[j + 1], field, m)) {
                Student temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
                ++m.swaps;
                changed = true;
            }
        }

        if (!changed)
            break;
    }

    m.ms = chrono::duration<double, milli>(
        Clock::now() - t).count();
}

void selectionStudents(Student* a, int n, int field, Metrics& m) {
    m = Metrics();
    auto t = Clock::now();

    for (int i = 0; i < n - 1; ++i) {
        int k = i;

        for (int j = i + 1; j < n; ++j) {
            if (studentGreater(a[k], a[j], field, m))
                k = j;
        }

        if (k != i) {
            Student temp = a[i];
            a[i] = a[k];
            a[k] = temp;
            ++m.swaps;
        }
    }

    m.ms = chrono::duration<double, milli>(
        Clock::now() - t).count();
}

void insertionStudents(Student* a, int n, int field, Metrics& m) {
    m = Metrics();
    auto t = Clock::now();

    for (int i = 1; i < n; ++i) {
        Student key = a[i];
        int j = i - 1;

        while (j >= 0) {
            if (studentGreater(a[j], key, field, m)) {
                a[j + 1] = a[j];
                ++m.swaps;
                --j;
            } else {
                break;
            }
        }

        a[j + 1] = key;
    }

    m.ms = chrono::duration<double, milli>(
        Clock::now() - t).count();
}

class App {
    Registry registry;
    UndoStack undo;
    Queue regular, priority;

    Student inputStudent() {
        int id = getInt("  Student ID: ", 1, 999999999);
        string name = getText("  Full name: ");

        cout << "\n";
        cout << "  Available services\n";
        cout << "  [1] Assessing\n";
        cout << "  [2] Clearance\n";
        cout << "  [3] Transcript Request\n";

        int s = getInt("  Select service: ", 1, 3);

        string service = s == 1 ? "Assessing" :
                         s == 2 ? "Clearance" : "Transcript Request";

        int p = getInt("  Lane (0-Regular, 1-Priority): ", 0, 1);
        double g = getDouble("  GPA (0-4): ", 0, 4);

        return Student(id, name, service, p, g);
    }

    void add() {
        Student s = inputStudent();

        if (!registry.add(s)) {
            CLIUI::error("Duplicate Student ID rejected.");
            return;
        }

        if (s.priority)
            priority.enqueue(s);
        else
            regular.enqueue(s);

        CLIUI::success("Student added to registry and service queue.");
    }

    void search() {
        int id = getInt("  Student ID: ", 1, 999999999);
        Student* s = registry.find(id);

        if (!s) {
            CLIUI::error("Student not found.");
        } else {
            cout << '\n';
            showStudentTableHeader();
            show(*s);
            CLIUI::line('-');
        }
    }

    void remove() {
        int id = getInt("  Student ID to delete: ", 1, 999999999);
        Student s;

        if (!registry.remove(id, s)) {
            CLIUI::error("Student not found.");
        } else {
            undo.deleted(s);
            CLIUI::success("Record deleted. Undo can restore it.");
            CLIUI::info("The existing queue ticket remains unchanged.");
        }
    }

    void update() {
        int id = getInt("  Student ID to update: ", 1, 999999999);
        Student* s = registry.find(id);

        if (!s) {
            CLIUI::error("Student not found.");
            return;
        }

        undo.gpaChanged(id, s->gpa);
        s->gpa = getDouble("  New GPA (0-4): ", 0, 4);

        CLIUI::success("GPA updated.");
    }

    void doUndo() {
        Action a;

        if (!undo.pop(a)) {
            CLIUI::info("Nothing to undo.");
            return;
        }

        if (a.type == 1) {
            if (registry.add(a.saved))
                CLIUI::success("Deleted record restored.");
            else
                CLIUI::error("Cannot restore: Student ID already exists.");
        } else {
            Student* s = registry.find(a.id);

            if (s) {
                s->gpa = a.oldGpa;
                CLIUI::success("Previous GPA restored.");
            } else {
                CLIUI::error("Student no longer exists.");
            }
        }
    }

    void serve() {
        cout << "\n";
        cout << "  [1] Priority Queue\n";
        cout << "  [2] Regular Queue\n";

        int c = getInt("  Select queue: ", 1, 2);
        Student s;

        if (!(c == 1 ? priority : regular).dequeue(s)) {
            CLIUI::error("Selected queue is empty.");
        } else {
            CLIUI::success("Now serving Student ID " +
                           to_string(s.id) + " - " + s.name);
        }
    }

    void peekQueue() {
        cout << "\n";
        cout << "  [1] Priority Queue\n";
        cout << "  [2] Regular Queue\n";

        int c = getInt("  Select queue to inspect: ", 1, 2);
        Student s;

        if (!(c == 1 ? priority : regular).peek(s)) {
            CLIUI::error("Selected queue is empty.");
        } else {
            cout << "\n  Next student to be served\n";
            CLIUI::line('-');
            showStudentTableHeader();
            show(s);
            CLIUI::line('-');
        }
    }

    void searchSort() {
        int n = registry.count();

        if (!n) {
            CLIUI::info("Registry is empty.");
            return;
        }

        int* a = new int[n];
        registry.copyIds(a);

        cout << "\n";
        cout << "  [1] Linear Search\n";
        cout << "  [2] Binary Search\n";
        cout << "  [3] Bubble Sort\n";
        cout << "  [4] Selection Sort\n";
        cout << "  [5] Insertion Sort\n";

        int c = getInt("  Choose operation: ", 1, 5);
        Metrics m;

        if (c == 1 || c == 2) {
            int target = getInt("  Target Student ID: ", 1, 999999999);

            if (c == 2) {
                for (int i = 1; i < n; ++i) {
                    int x = a[i];
                    int j = i - 1;

                    while (j >= 0 && a[j] > x) {
                        a[j + 1] = a[j];
                        --j;
                    }

                    a[j + 1] = x;
                }
            }

            int pos = c == 1
                ? linearSearch(a, n, target, m)
                : binarySearch(a, n, target, m);

            CLIUI::line('-');

            if (pos < 0)
                cout << "  Result       : Not found\n";
            else
                cout << "  Result       : Found\n";

            cout << "  Index        : " << pos << '\n';
            cout << "  Comparisons  : " << m.comparisons << '\n';
            cout << "  Time         : " << fixed
                 << setprecision(3) << m.ms << " ms\n";
        } else {
            if (c == 3)
                bubble(a, n, m);

            if (c == 4)
                selection(a, n, m);

            if (c == 5)
                insertion(a, n, m);

            cout << "\n  Sorted Student IDs\n";
            CLIUI::line('-');

            for (int i = 0; i < n; ++i) {
                cout << "  " << setw(12) << left << a[i];

                if ((i + 1) % 5 == 0 || i == n - 1)
                    cout << '\n';
            }

            CLIUI::line('-');
            cout << "  Comparisons  : " << m.comparisons << '\n';
            cout << "  Swaps/Shifts : " << m.swaps << '\n';
            cout << "  Time         : " << fixed
                 << setprecision(3) << m.ms << " ms\n";
        }

        delete[] a;
    }

    void multiFieldSort() {
        int n = registry.count();

        if (!n) {
            CLIUI::info("Registry is empty.");
            return;
        }

        int* ids = new int[n];
        Student* students = new Student[n];

        registry.copyIds(ids);

        for (int i = 0; i < n; ++i)
            students[i] = *registry.find(ids[i]);

        cout << "\n";
        cout << "  Sorting algorithms\n";
        cout << "  [1] Bubble Sort\n";
        cout << "  [2] Selection Sort\n";
        cout << "  [3] Insertion Sort\n";

        int algorithm = getInt("  Choose algorithm: ", 1, 3);

        cout << "\n";
        cout << "  Sorting fields\n";
        cout << "  [1] Student ID\n";
        cout << "  [2] Student Name\n";
        cout << "  [3] GPA\n";
        cout << "  [4] Priority\n";

        int field = getInt("  Choose field: ", 1, 4);
        Metrics m;

        if (algorithm == 1)
            bubbleStudents(students, n, field, m);

        if (algorithm == 2)
            selectionStudents(students, n, field, m);

        if (algorithm == 3)
            insertionStudents(students, n, field, m);

        cout << "\n";
        CLIUI::header("SORTED STUDENT RECORDS");
        showStudentTableHeader();

        for (int i = 0; i < n; ++i)
            show(students[i]);

        CLIUI::line('-');
        cout << "  Comparisons  : " << m.comparisons << '\n';
        cout << "  Swaps/Shifts : " << m.swaps << '\n';
        cout << "  Time         : " << fixed
             << setprecision(3) << m.ms << " ms\n";

        delete[] ids;
        delete[] students;
    }

public:
    App() : regular(false), priority(true) {}

    void run() {
        for (;;) {
            CLIUI::clearScreen();
            CLIUI::showMainMenu();

            int c = getInt("  Enter your choice: ", 0, 15);

            if (c == 0) {
                CLIUI::showExitScreen();
                return;
            }

            CLIUI::clearScreen();

            switch (c) {
                case 1:
                    CLIUI::header("ADD STUDENT RECORD");
                    add();
                    break;

                case 2:
                    CLIUI::header("STUDENT REGISTRY");
                    registry.display();
                    break;

                case 3:
                    CLIUI::header("SEARCH STUDENT");
                    search();
                    break;

                case 4:
                    CLIUI::header("DELETE STUDENT RECORD");
                    remove();
                    break;

                case 5:
                    CLIUI::header("UPDATE STUDENT GPA");
                    update();
                    break;

                case 6:
                    CLIUI::header("UNDO LAST CHANGE");
                    doUndo();
                    break;

                case 7:
                    CLIUI::header("SERVICE QUEUE STATUS");
                    priority.display("PRIORITY QUEUE");
                    regular.display("REGULAR FIFO QUEUE");
                    break;

                case 8:
                    CLIUI::header("SERVE NEXT STUDENT");
                    serve();
                    break;

                case 9:
                    CLIUI::header("REGISTRY STATISTICS");
                    CLIUI::line('-');
                    cout << "  Total student records: "
                         << registry.count() << '\n';
                    CLIUI::line('-');
                    break;

                case 10:
                    CLIUI::header("NEXT STUDENT IN QUEUE");
                    peekQueue();
                    break;

                case 11:
                    CLIUI::header("SEARCH AND SORT DEMO");
                    searchSort();
                    break;

                case 12:
                    CLIUI::header("BENCHMARKING SUITE");
                    benchmark();
                    break;

                case 13:
                    CLIUI::header("MULTI-FIELD SORTING");
                    multiFieldSort();
                    break;

                case 14:
                    CLIUI::header("LATEST UNDO ACTION");
                    undo.latest();
                    break;

                case 15:
                    CLIUI::header("UNDO / AUDIT HISTORY");
                    undo.display();
                    break;

                default:
                    CLIUI::error("Invalid menu selection.");
                    break;
            }

            CLIUI::pause();
        }
    }
};

int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    App app;
    app.run();

    return 0;
}
