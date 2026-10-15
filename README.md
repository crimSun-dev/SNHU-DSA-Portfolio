# CS 300 Portfolio: Data Structures and Algorithms

**Draven Chen**
Southern New Hampshire University
CS-300 — Data Structures and Algorithms

This repository is my CS 300 portfolio submission. It holds one part of each course project: the Project One analysis of run time and memory, and the Project Two program that prints the Computer Science courses in alphanumeric order.

## What is in this repository

| Path | Artifact |
| --- | --- |
| [Project One/CS 300 Project_One Pseudocode and Analysis.docx](Project%20One/CS%20300%20Project_One%20Pseudocode%20and%20Analysis.docx) | Project One analysis of run time and memory for a vector, a hash table, and a binary search tree |
| [Project Two/ProjectTwo.cpp](Project%20Two/ProjectTwo.cpp) | Working C++ program. Menu option 2 prints the course list in alphanumeric order |
| [Project Two/ABCU Advising Program Input.csv](Project%20Two/ABCU%20Advising%20Program%20Input.csv) | Course file the program reads |

The analysis compares the three structures on loading with prerequisite checks, finding one course, and printing a sorted list. The recommendation was a binary search tree, with a self-balancing tree so a nearly sorted file does not turn the tree into a linked list. Project Two implements that recommendation as an AVL tree. An in-order traversal prints the courses in alphanumeric order without a separate sort.

## Running the course list

From the `Project Two` folder, with g++ installed:

```text
g++ -std=c++17 -Wall -o ProjectTwo ProjectTwo.cpp
```

Then run `ProjectTwo`, choose `1`, and enter `ABCU Advising Program Input.csv`. Choose `2` to print the sample schedule. The eight courses print in this order:

```text
CSCI100, Introduction to Computer Science
CSCI101, Introduction to Programming in C++
CSCI200, Data Structures
CSCI300, Introduction to Algorithms
CSCI301, Advanced Programming in C++
CSCI350, Operating Systems
CSCI400, Large Software Development
MATH201, Discrete Mathematics
```

Choose `3` and a course number such as `CSCI300` to see that course and its prerequisites. Choose `9` to exit.

## Reflection

### What was the problem you were solving in the projects for this course?

ABCU’s advising department needed a course planner for the Computer Science program. The program has to read a course file, print every course in alphanumeric order, and look up one course together with its prerequisites. Project One was the design step. I wrote one shared plan for loading the file, the menu, and printing a single course, then compared a vector, a hash table, and a binary search tree on insert, search, and printing the sorted list, including how much memory each structure uses. Project Two was the build: a working C++ program that does those jobs with the structure that comparison recommended.

### How did you approach the problem? Consider why data structures are important to understand.

I started from the operations an advisor repeats, not from a favorite structure. Loading checks every prerequisite, so search runs once for each prerequisite. The schedule has to come out sorted. A single-course lookup has to be reliable. Those operations do not cost the same amount in every structure. A vector keeps courses in one contiguous block and uses the least memory, but it is unordered, so every search walks the whole list and loading stays O(n²) even on average. A hash table finds a course in O(1) on average, but it has no order, so the schedule still has to be collected and sorted. A binary search tree searches in O(log n) on average, and an in-order traversal prints the sorted list in O(n) with no separate sort. Data structures matter because the same feature can be fast or unusable depending on which structure holds the data. The choice has to match the operations the program actually repeats.

### How did you overcome any roadblocks you encountered while going through the activities or project?

The sample file is almost already sorted: CSCI100, CSCI101, CSCI200, and then MATH201. Inserting that file in order into a plain binary search tree builds a tree that is nearly a linked list, so search falls back to O(n) and loading becomes O(n²). I treated that as a design constraint. The Project One recommendation was to use a self-balancing tree, and Project Two implements an AVL tree so rotations keep the height balanced. The alphanumeric list is still just an in-order walk. A second problem showed up in the hash table analysis. A sum-of-ASCII hash sends CSCI101 and CSCI110 to the same bucket, because those course numbers use the same characters, so a chain can grow and the average case is not guaranteed. File handling was the practical block in the code. The CSV can start with a byte-order mark, include blank lines and trailing commas, and mix the case of course numbers. The loader trims those, uppercases course numbers, skips a bad line, and refuses to keep a partial load if any prerequisite is missing from the file.

### How has your work on this project expanded your approach to designing software and developing programs?

I now write down the operations and their worst and average cases before I commit to an implementation. This program needed a sorted list and a single-course lookup in one structure, and prerequisite checking adds a search for every prerequisite, so the binary search tree fit better than the vector or the hash table. I also look at the real input, not only the textbook case. A balanced tree is O(log n) only while it stays balanced, and this file would not have kept a plain tree balanced. Putting that fix in the analysis, then coding the AVL tree in Project Two, is the process I want to repeat: name the operations, notice what the data will actually look like, then choose.

### How has your work on this project evolved the way you write programs that are maintainable, readable, and adaptable?

The three structures in Project One shared one course record, one loader, and one menu. Only insert, search, and the sorted print changed. That split made the comparison readable, and it made Project Two easier to build because the menu and the file format did not have to be reinvented. The C++ program keeps the same split: a `Course` struct, a `CourseTree` class, `loadCourses`, `printCourseList`, and the menu in `main`. `printCourseList` does not sort on its own. It walks the tree in order, so a later change to how a node is stored does not need a new sorting routine as long as the tree stays ordered by course number. The checks that make the program safer to change are in the same places: reload clears the old tree, a duplicate course number keeps the newer record, and a missing prerequisite wipes the load instead of advising from half a file. Comments record why the tree is balanced and why copying the tree is disabled, so the next change does not have to reverse-engineer those decisions.

Shapiro Library guidance used for this acknowledgment: [How do I cite AI (like ChatGPT) in APA Style?](https://libanswers.snhu.edu/faq/386764)
