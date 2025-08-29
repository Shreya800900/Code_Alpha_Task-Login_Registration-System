#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <limits>
#include <cctype>

using namespace std;

struct User {
    string passwordHash;
    int failedAttempts;
    bool isLocked;
};

// Simple hashing function (demo only)
string betterHash(const string &password) {
    unsigned long hash = 1469598103934665603ULL; // FNV-1a hash
    for (char c : password) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    return to_string(hash);
}

// Load all users from file
unordered_map<string, User> loadUsers(const string &filename) {
    unordered_map<string, User> users;
    ifstream file(filename);
    if (!file) return users;

    string username, hash;
    int attempts;
    bool locked;
    while (file >> username >> hash >> attempts >> locked) {
        users[username] = {hash, attempts, locked};
    }
    file.close();
    return users;
}

// Save all users to file
void saveUsers(const string &filename, const unordered_map<string, User> &users) {
    ofstream file(filename, ios::trunc);
    for (auto &pair : users) {
        file << pair.first << " " 
             << pair.second.passwordHash << " " 
             << pair.second.failedAttempts << " " 
             << pair.second.isLocked << "\n";
    }
    file.close();
}

// Masked password input
string getPasswordMasked() {
    string password;
    char ch;
    cout << "Enter password: ";
    while (true) {
        ch = getchar();
        if (ch == '\n') break; 
        if (ch == '\b' && !password.empty()) { 
            cout << "\b \b";
            password.pop_back();
        } else if (isprint(ch)) {
            password.push_back(ch);
            cout << '*';
        }
    }
    cout << endl;
    return password;
}

// Password strength check
string passwordStrength(const string &pass) {
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSymbol = false;
    for (char c : pass) {
        if (isupper(c)) hasUpper = true;
        else if (islower(c)) hasLower = true;
        else if (isdigit(c)) hasDigit = true;
        else hasSymbol = true;
    }

    if (pass.size() >= 8 && hasUpper && hasLower && hasDigit && hasSymbol)
        return "Strong";
    else if (pass.size() >= 6 && ((hasUpper && hasLower) || (hasLower && hasDigit)))
        return "Medium";
    else
        return "Weak";
}

// Registration
void registerUser(const string &filename) {
    auto users = loadUsers(filename);
    string username;
    cout << "Enter username: ";
    cin >> username;

    if (username.size() < 3) {
        cout << " Username must be at least 3 characters.\n";
        return;
    }
    if (users.find(username) != users.end()) {
        cout << " Username already exists.\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string password = getPasswordMasked();
    cout << "Password Strength: " << passwordStrength(password) << "\n";
    if (password.size() < 6) {
        cout << " Password must be at least 6 characters.\n";
        return;
    }

    users[username] = {betterHash(password), 0, false};
    saveUsers(filename, users);
    cout << " Registration successful!\n";
}

// Login
void loginUser(const string &filename) {
    auto users = loadUsers(filename);
    string username;
    cout << "Enter username: ";
    cin >> username;

    if (users.find(username) == users.end()) {
        cout << " No such user found.\n";
        return;
    }

    if (users[username].isLocked) {
        cout << " Account is locked due to too many failed attempts.\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string password = getPasswordMasked();

    if (users[username].passwordHash == betterHash(password)) {
        cout << " Login successful! Welcome, " << username << ".\n";
        users[username].failedAttempts = 0; // reset attempts
    } else {
        cout << " Incorrect password.\n";
        users[username].failedAttempts++;
        if (users[username].failedAttempts >= 3) {
            users[username].isLocked = true;
            cout << " Account locked after 3 failed attempts.\n";
        }
    }

    saveUsers(filename, users);
}

// Admin unlock account
void unlockAccount(const string &filename) {
    auto users = loadUsers(filename);
    string username;
    cout << "Enter username to unlock: ";
    cin >> username;

    if (users.find(username) == users.end()) {
        cout << " No such user found.\n";
        return;
    }

    users[username].failedAttempts = 0;
    users[username].isLocked = false;
    saveUsers(filename, users);
    cout << " Account unlocked for user: " << username << "\n";
}

// Main menu
int main() {
    const string filename = "users.txt";
    int choice;
    while (true) {
        cout << "\n===== LOGIN & REGISTRATION SYSTEM =====\n";
        cout << "1. Register\n2. Login\n3. Unlock Account (Admin)\n4. Exit\nEnter choice: ";
        cin >> choice;
        switch (choice) {
            case 1: registerUser(filename); break;
            case 2: loginUser(filename); break;
            case 3: unlockAccount(filename); break;
            case 4: cout << "Exiting...\n"; return 0;
            default: cout << " Invalid choice.\n";
        }
    }
}
