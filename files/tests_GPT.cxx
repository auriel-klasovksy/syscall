#include "hw2_test.h"
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

using namespace std;

// ANSI color codes for terminal output
#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

// Test invalid arguments for set_sec
bool testSetSecInvalidArguments() {
    errno = 0;
    long result = set_sec(-1, 1, 1); // Negative value is invalid
    if (result != -1 || errno != EINVAL) {
        cerr << "testSetSecInvalidArguments failed: expected -1/EINVAL, got " << result << "/" << errno << endl;
        return false;
    }
    return true;
}

// Test invalid clearance argument for get_sec
bool testGetSecInvalidArgument() {
    errno = 0;
    long result = get_sec('x'); // 'x' is not a valid clearance character
    if (result != -1 || errno != EINVAL) {
        cerr << "testGetSecInvalidArgument failed: expected -1/EINVAL, got " << result << "/" << errno << endl;
        return false;
    }
    return true;
}

// Test clearance check for a nonexistent process
bool testCheckSecNonexistentProcess() {
    errno = 0;
    long result = check_sec(-1, 'm'); // Negative PID is invalid
    if (result != -1 || errno != ESRCH) {
        cerr << "testCheckSecNonexistentProcess failed: expected -1/ESRCH, got " << result << "/" << errno << endl;
        return false;
    }
    return true;
}

// Test setting clearance for ancestors with invalid arguments
bool testSetSecBranchInvalidArguments() {
    errno = 0;
    long result = set_sec_branch(-1, 's'); // Negative height is invalid
    if (result != -1 || errno != EINVAL) {
        cerr << "testSetSecBranchInvalidArguments failed: expected -1/EINVAL, got " << result << "/" << errno << endl;
        return false;
    }
    return true;
}

// Test setting clearance for parent with branch height = 1
bool testSetSecBranchHeightOne() {
    pid_t child_pid = fork();
    if (child_pid < 0) {
        cerr << "Fork failed for child process" << endl;
        return false;
    }

    if (child_pid == 0) {
        pid_t grandchild_pid = fork();
        if (grandchild_pid < 0) {
            cerr << "Fork failed for grandchild process" << endl;
            exit(1);
        }

        if (grandchild_pid == 0) {
            // Grandchild process: Set 'midnight' clearance and apply set_sec_branch
            errno = 0;
            long set_result = set_sec(0, 1, 0); // Give 'midnight' clearance
            if (set_result < 0) {
                cerr << "Grandchild: set_sec failed with " << set_result << "/" << errno << endl;
                exit(1);
            }

            errno = 0;
            long branch_result = set_sec_branch(1, 'm');
            if (branch_result < 0) {
                cerr << "Grandchild: set_sec_branch failed with " << branch_result << "/" << errno << endl;
                exit(1);
            }
            exit(0);
        }

        // Child process: Wait for grandchild and check clearance
        int grandchild_status;
        waitpid(grandchild_pid, &grandchild_status, 0);
        if (!WIFEXITED(grandchild_status) || WEXITSTATUS(grandchild_status) != 0) {
            cerr << "Child: Grandchild test failed" << endl;
            exit(1);
        }

        errno = 0;
        long child_clearance = get_sec('m');
        if (child_clearance != 1) {
            cerr << "Child: Expected 'midnight' clearance, got " << child_clearance << "/" << errno << endl;
            exit(1);
        }
        exit(0);
    }

    // Parent process
    int child_status;
    waitpid(child_pid, &child_status, 0);
    if (!WIFEXITED(child_status) || WEXITSTATUS(child_status) != 0) {
        cerr << "Parent: Child test failed" << endl;
        return false;
    }

    errno = 0;
    long parent_clearance = get_sec('m');
    if (parent_clearance != 0) {
        cerr << "Parent: Unexpected 'midnight' clearance, got " << parent_clearance << "/" << errno << endl;
        return false;
    }

    return true;
}

// Test setting clearance boundary values
bool testSetSecBoundaryValues() {
    errno = 0;
    long result = set_sec(0, 1, 1); // Valid boundaries
    if (result < 0) {
        cerr << "testSetSecBoundaryValues failed: expected success, got " << result << "/" << errno << endl;
        return false;
    }
    return true;
}

// Test check_sec with invalid clearance
bool testCheckSecInvalidClearance() {
    errno = 0;
    long result = check_sec(getpid(), 'z'); // Invalid clearance
    if (result != -1 || errno != EINVAL) {
        cerr << "testCheckSecInvalidClearance failed: expected -1/EINVAL, got " << result << "/" << errno << endl;
        return false;
    }
    return true;
}

// Test clearance inheritance across fork
bool testInheritanceOnFork() {
    long result = set_sec(1, 0, 0); // Give "sword" clearance
    if (result < 0) {
        cerr << "testInheritanceOnFork: set_sec failed" << endl;
        return false;
    }

    pid_t child_pid = fork();
    if (child_pid < 0) {
        cerr << "Fork failed" << endl;
        return false;
    }

    if (child_pid == 0) {
        errno = 0;
        long clearance = get_sec('s'); // Check "sword" clearance
        if (clearance != 1) {
            cerr << "Child: Expected clearance, got " << clearance << "/" << errno << endl;
            exit(1);
        }
        exit(0);
    }

    int status;
    waitpid(child_pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        cerr << "Parent: Child test failed" << endl;
        return false;
    }

    return true;
}

// Test set_sec_branch with height = 0
bool testSetSecBranchWithHeightZero() {
    errno = 0;
    long result = set_sec_branch(0, 'm'); // Height = 0
   if (result != -1 || errno != EINVAL) {
        cerr << "testSetSecBranchWithHeightZero failed: expected -1/EINVAL, got " << result << "/" << errno << endl;
        return false;
    }
    return true;
}

// Test setting multiple clearances
bool testSetSecMultipleClearances() {
    errno = 0;
    long result = set_sec(1, 1, 1); // Set all clearances
    if (result < 0) {
        cerr << "testSetSecMultipleClearances failed: expected success, got " << result << "/" << errno << endl;
        return false;
    }

    if (get_sec('s') != 1 || get_sec('m') != 1 || get_sec('c') != 1) {
        cerr << "testSetSecMultipleClearances failed: clearances not properly set" << endl;
        return false;
    }
    return true;
}

int main() {
    int passedTests = 0;
    int totalTests = 10;

    cout << "Running tests..." << endl;

    if (testSetSecInvalidArguments()) {
        cout << GREEN << "testSetSecInvalidArguments: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testSetSecInvalidArguments: Failure" << RESET << endl;
    }
	std::cout<<std::endl;
    if (testGetSecInvalidArgument()) {
        cout << GREEN << "testGetSecInvalidArgument: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testGetSecInvalidArgument: Failure" << RESET << endl;
    }
	std::cout<<std::endl;
    if (testCheckSecNonexistentProcess()) {
        cout << GREEN << "testCheckSecNonexistentProcess: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testCheckSecNonexistentProcess: Failure" << RESET << endl;
    }
std::cout<<std::endl;
    if (testSetSecBranchInvalidArguments()) {
        cout << GREEN << "testSetSecBranchInvalidArguments: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testSetSecBranchInvalidArguments: Failure" << RESET << endl;
    }
std::cout<<std::endl;
    if (testSetSecBranchHeightOne()) {
        cout << GREEN << "testSetSecBranchHeightOne: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testSetSecBranchHeightOne: Failure" << RESET << endl;
    }
std::cout<<std::endl;
    if (testSetSecBoundaryValues()) {
        cout << GREEN << "testSetSecBoundaryValues: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testSetSecBoundaryValues: Failure" << RESET << endl;
    }
std::cout<<std::endl;
    if (testCheckSecInvalidClearance()) {
        cout << GREEN << "testCheckSecInvalidClearance: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testCheckSecInvalidClearance: Failure" << RESET << endl;
    }
std::cout<<std::endl;
    if (testInheritanceOnFork()) {
        cout << GREEN << "testInheritanceOnFork: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testInheritanceOnFork: Failure" << RESET << endl;
    }
std::cout<<std::endl;
    if (testSetSecBranchWithHeightZero()) {
        cout << GREEN << "testSetSecBranchWithHeightZero: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testSetSecBranchWithHeightZero: Failure" << RESET << endl;
    }
std::cout<<std::endl;
    if (testSetSecMultipleClearances()) {
        cout << GREEN << "testSetSecMultipleClearances: Success" << RESET << endl;
        passedTests++;
    } else {
        cout << RED << "testSetSecMultipleClearances: Failure" << RESET << endl;
    }
std::cout<<std::endl;
    cout << "\nTests Passed: " << GREEN << passedTests << RESET << "/" << totalTests << endl;

    return 0;
}
