//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Draven Chen
// Course      : CS 300 Analysis and Design
// Description : ABCU advising assistance program. Reads course data from a
//               CSV file into a self-balancing (AVL) binary search tree
//               keyed on course number,
//               prints an alphanumeric course list (in-order traversal), and
//               looks up a single course with its prerequisites.
//============================================================================

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

//----------------------------------------------------------------------------
// Course: one row of the input file
//----------------------------------------------------------------------------
struct Course {
    string courseNumber;           // e.g. "CSCI200"
    string courseTitle;            // e.g. "Data Structures"
    vector<string> prerequisites;  // prerequisite course numbers (0..N)
};

//----------------------------------------------------------------------------
// Node: one node of the binary search tree
//----------------------------------------------------------------------------
struct Node {
    Course course;
    Node* left;
    Node* right;
    int height;  // height of the subtree rooted here (leaf = 1)

    explicit Node(const Course& newCourse)
        : course(newCourse), left(nullptr), right(nullptr), height(1) {}
};

//----------------------------------------------------------------------------
// CourseTree: self-balancing (AVL) binary search tree ordered by course
// number. In-order traversal visits courses in alphanumeric order, so the
// course list never needs a separate sorting step. Balancing keeps search
// at O(log n) even when the input file is already close to sorted, which
// Project One identified as the worst case for a plain BST.
//----------------------------------------------------------------------------
class CourseTree {
public:
    CourseTree() : root(nullptr), courseCount(0) {}
    ~CourseTree() { destroySubtree(root); }

    // The tree owns raw pointers, so copying is disabled to avoid double frees
    CourseTree(const CourseTree&) = delete;
    CourseTree& operator=(const CourseTree&) = delete;

    // Removes every course so the file can be reloaded cleanly
    void clear() {
        destroySubtree(root);
        root = nullptr;
        courseCount = 0;
    }

    bool isEmpty() const { return root == nullptr; }
    int size() const { return courseCount; }
    int height() const { return nodeHeight(root); }

    // Inserts a course. If the course number already exists, the newer
    // record replaces the old one and the function returns false.
    bool insert(const Course& course) {
        bool added = false;
        root = addNode(root, course, added);
        if (added) {
            ++courseCount;
        }
        return added;
    }

    // Returns a pointer to the matching course, or nullptr if not found
    const Course* search(const string& courseNumber) const {
        Node* current = root;
        while (current != nullptr) {
            if (courseNumber == current->course.courseNumber) {
                return &current->course;
            }
            current = (courseNumber < current->course.courseNumber)
                          ? current->left
                          : current->right;
        }
        return nullptr;
    }

    // Collects every course in alphanumeric order (in-order traversal)
    void inOrder(vector<const Course*>& output) const {
        inOrder(root, output);
    }

private:
    Node* root;
    int courseCount;

    static int nodeHeight(const Node* node) {
        return node == nullptr ? 0 : node->height;
    }

    static void updateHeight(Node* node) {
        node->height = 1 + max(nodeHeight(node->left), nodeHeight(node->right));
    }

    // Right rotation: y's left child x becomes the subtree root,
    // y becomes x's right child, and x's old right subtree moves to y's left
    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        x->right = y;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    // Mirror image of rotateRight
    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    // Restores the AVL property (subtree heights differ by at most 1)
    static Node* rebalance(Node* node) {
        updateHeight(node);
        int balance = nodeHeight(node->left) - nodeHeight(node->right);

        if (balance > 1) {  // left-heavy
            if (nodeHeight(node->left->left) < nodeHeight(node->left->right)) {
                node->left = rotateLeft(node->left);  // left-right case
            }
            return rotateRight(node);
        }
        if (balance < -1) {  // right-heavy
            if (nodeHeight(node->right->right) < nodeHeight(node->right->left)) {
                node->right = rotateRight(node->right);  // right-left case
            }
            return rotateLeft(node);
        }
        return node;
    }

    // Recursive insert from the Project One pseudocode, plus rebalancing
    static Node* addNode(Node* node, const Course& course, bool& added) {
        if (node == nullptr) {
            added = true;
            return new Node(course);
        }

        if (course.courseNumber < node->course.courseNumber) {
            node->left = addNode(node->left, course, added);
        } else if (course.courseNumber > node->course.courseNumber) {
            node->right = addNode(node->right, course, added);
        } else {
            node->course = course;  // duplicate: keep the newer record
            return node;
        }
        return rebalance(node);
    }

    void inOrder(const Node* node, vector<const Course*>& output) const {
        if (node == nullptr) {
            return;
        }
        inOrder(node->left, output);
        output.push_back(&node->course);
        inOrder(node->right, output);
    }

    // Post-order delete so children are freed before their parent
    void destroySubtree(Node* node) {
        if (node == nullptr) {
            return;
        }
        destroySubtree(node->left);
        destroySubtree(node->right);
        delete node;
    }
};

//----------------------------------------------------------------------------
// String helpers
//----------------------------------------------------------------------------

// Strips spaces, tabs, Windows carriage returns, a UTF-8 byte-order mark,
// and surrounding quotes (some terminals add quotes to dragged-in paths)
string trim(const string& text) {
    string result = text;

    // UTF-8 BOM that Excel sometimes writes at the start of a CSV file
    const string bom = "\xEF\xBB\xBF";
    if (result.compare(0, bom.size(), bom) == 0) {
        result.erase(0, bom.size());
    }

    const string whitespace = " \t\r\n";
    size_t first = result.find_first_not_of(whitespace);
    if (first == string::npos) {
        return "";
    }
    size_t last = result.find_last_not_of(whitespace);
    result = result.substr(first, last - first + 1);

    if (result.size() >= 2 && result.front() == '"' && result.back() == '"') {
        result = trim(result.substr(1, result.size() - 2));
    }
    return result;
}

// Returns an uppercase copy so "csci400" and "CSCI400" match
string toUpperCase(const string& text) {
    string result = text;
    for (char& ch : result) {
        ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
    }
    return result;
}

// Splits one CSV line on commas and trims each field.
// The course file has no quoted fields, so a plain split is enough.
vector<string> splitLine(const string& line) {
    vector<string> fields;
    stringstream lineStream(line);
    string field;
    while (getline(lineStream, field, ',')) {
        fields.push_back(trim(field));
    }
    return fields;
}

//----------------------------------------------------------------------------
// loadCourses
// Pass 1 reads and parses each line, skipping malformed lines.
// Pass 2 checks that every prerequisite is itself a course in the file.
// Loading is all-or-nothing: if any prerequisite is unknown, the tree is
// cleared so the program never works from partial data.
//----------------------------------------------------------------------------
bool loadCourses(const string& fileName, CourseTree& courseTree) {
    ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        cout << "Error: could not open file \"" << fileName << "\"." << endl;
        return false;
    }

    vector<Course> parsedCourses;
    string line;
    int lineNumber = 0;
    int skippedLines = 0;

    while (getline(inputFile, line)) {
        ++lineNumber;
        if (trim(line).empty()) {
            continue;  // ignore blank lines
        }

        vector<string> fields = splitLine(line);

        // Every valid line needs at least a course number and a title
        if (fields.size() < 2 || fields[0].empty() || fields[1].empty()) {
            cout << "Warning: line " << lineNumber
                 << " is missing a course number or title and was skipped."
                 << endl;
            ++skippedLines;
            continue;
        }

        Course course;
        course.courseNumber = toUpperCase(fields[0]);
        course.courseTitle = fields[1];

        // Any remaining non-empty fields are prerequisites
        // (trailing commas in the file produce empty fields, which we skip)
        for (size_t i = 2; i < fields.size(); ++i) {
            if (!fields[i].empty()) {
                course.prerequisites.push_back(toUpperCase(fields[i]));
            }
        }
        parsedCourses.push_back(course);
    }
    inputFile.close();

    if (parsedCourses.empty()) {
        cout << "Error: no valid course records were found in \"" << fileName
             << "\"." << endl;
        return false;
    }

    // Build the new tree contents (replacing anything loaded earlier)
    courseTree.clear();
    for (const Course& course : parsedCourses) {
        if (!courseTree.insert(course)) {
            cout << "Warning: " << course.courseNumber
                 << " appears more than once; the last entry was kept."
                 << endl;
        }
    }

    // Pass 2: every prerequisite must also exist as a course
    vector<const Course*> allCourses;
    courseTree.inOrder(allCourses);
    for (const Course* course : allCourses) {
        for (const string& prereq : course->prerequisites) {
            if (courseTree.search(prereq) == nullptr) {
                cout << "Error: " << course->courseNumber
                     << " lists prerequisite " << prereq
                     << ", which is not in the file. Nothing was loaded."
                     << endl;
                courseTree.clear();  // no partial data
                return false;
            }
        }
    }

    cout << courseTree.size()
         << (courseTree.size() == 1 ? " course loaded" : " courses loaded");
    if (skippedLines > 0) {
        cout << " (" << skippedLines << " line(s) skipped)";
    }
    cout << "." << endl;
    return true;
}

//----------------------------------------------------------------------------
// printCourseList: option 2
//----------------------------------------------------------------------------
void printCourseList(const CourseTree& courseTree) {
    vector<const Course*> sortedCourses;
    courseTree.inOrder(sortedCourses);

    cout << "Here is a sample schedule:" << endl << endl;
    for (const Course* course : sortedCourses) {
        cout << course->courseNumber << ", " << course->courseTitle << endl;
    }
}

//----------------------------------------------------------------------------
// printCourseInformation: option 3
// Prints the title, the prerequisite numbers on one line (matching the
// sample output), then each prerequisite's title underneath.
//----------------------------------------------------------------------------
void printCourseInformation(const CourseTree& courseTree,
                            const string& courseNumber) {
    const Course* course = courseTree.search(courseNumber);
    if (course == nullptr) {
        cout << courseNumber << " was not found in the course list." << endl;
        return;
    }

    cout << course->courseNumber << ", " << course->courseTitle << endl;

    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
        return;
    }

    cout << "Prerequisites: ";
    for (size_t i = 0; i < course->prerequisites.size(); ++i) {
        if (i > 0) {
            cout << ", ";
        }
        cout << course->prerequisites[i];
    }
    cout << endl;

    // Title of each prerequisite, looked up in the same tree
    for (const string& prereqNumber : course->prerequisites) {
        const Course* prereq = courseTree.search(prereqNumber);
        cout << "    " << prereqNumber << ", "
             << (prereq != nullptr ? prereq->courseTitle
                                   : "(title not found in file)")
             << endl;
    }
}

//----------------------------------------------------------------------------
// Menu helpers
//----------------------------------------------------------------------------
void displayMenu() {
    cout << endl;
    cout << "   1. Load Data Structure." << endl;
    cout << "   2. Print Course List." << endl;
    cout << "   3. Print Course." << endl;
    cout << "   9. Exit" << endl;
    cout << endl;
    cout << "What would you like to do? ";
}

// Converts the whole input to an int; returns false for "abc", "2x", "", etc.
bool parseMenuChoice(const string& input, int& choice) {
    if (input.empty()) {
        return false;
    }
    for (char ch : input) {
        if (!isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    if (input.size() > 3) {
        return false;  // guards against overflow on very long digit strings
    }
    choice = stoi(input);
    return true;
}

//----------------------------------------------------------------------------
// main
//----------------------------------------------------------------------------
int main() {
    CourseTree courseTree;
    string input;
    int choice = 0;

    cout << "Welcome to the course planner." << endl;

    while (choice != 9) {
        displayMenu();

        // Read the full line so bad input never leaves cin in a failed state
        if (!getline(cin, input)) {
            break;  // end of input stream
        }
        input = trim(input);

        if (!parseMenuChoice(input, choice)) {
            cout << (input.empty() ? "(blank)" : input)
                 << " is not a valid option." << endl;
            choice = 0;
            continue;
        }

        switch (choice) {
        case 1: {
            cout << "Enter the name of the course data file: ";
            string fileName;
            if (!getline(cin, fileName)) {
                choice = 9;
                break;
            }
            fileName = trim(fileName);
            if (fileName.empty()) {
                cout << "Error: no file name was entered." << endl;
                break;
            }
            loadCourses(fileName, courseTree);
            break;
        }

        case 2:
            if (courseTree.isEmpty()) {
                cout << "No courses loaded. Please choose option 1 first."
                     << endl;
                break;
            }
            printCourseList(courseTree);
            break;

        case 3: {
            if (courseTree.isEmpty()) {
                cout << "No courses loaded. Please choose option 1 first."
                     << endl;
                break;
            }
            cout << "What course do you want to know about? ";
            string courseNumber;
            if (!getline(cin, courseNumber)) {
                choice = 9;
                break;
            }
            courseNumber = toUpperCase(trim(courseNumber));
            if (courseNumber.empty()) {
                cout << "Error: no course number was entered." << endl;
                break;
            }
            printCourseInformation(courseTree, courseNumber);
            break;
        }

        case 9:
            break;

        default:
            cout << choice << " is not a valid option." << endl;
            break;
        }
    }

    cout << "Thank you for using the course planner!" << endl;
    return 0;
}
