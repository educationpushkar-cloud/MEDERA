#include "function.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

typedef struct
{
    string city;
    string area;
    string name;
    float rating;
} hos;

hos h[3][4][3] =
{
    {
        {
            {"DEHRADUN", "CLEMENT TOWN", "VELMED", 3.4f},
            {"DEHRADUN", "CLEMENT TOWN", "ABC", 4.5f},
            {"DEHRADUN", "CLEMENT TOWN", "XYZ", 5.0f}
        },
        {
            {"DEHRADUN", "GHARI CANTT", "AIIMS", 3.4f},
            {"DEHRADUN", "GHARI CANTT", "NURAHH", 4.5f},
            {"DEHRADUN", "GHARI CANTT", "ZAFFAR", 5.0f}
        },
        {
            {"DEHRADUN", "MAJRA", "SHREE RAM", 3.4f},
            {"DEHRADUN", "MAJRA", "HAPPY", 4.5f},
            {"DEHRADUN", "MAJRA", "DENTAL", 5.0f}
        },
        {
            {"DEHRADUN", "RAJPUR", "CARE U", 3.4f},
            {"DEHRADUN", "RAJPUR", "MAINS", 4.5f},
            {"DEHRADUN", "RAJPUR", "DBS", 5.0f}
        }
    },
    {
        {
            {"DELHI", "RED FORT", "DELHI HOSPITAL 1", 4.1f},
            {"DELHI", "RED FORT", "DELHI HOSPITAL 2", 4.4f},
            {"DELHI", "RED FORT", "DELHI HOSPITAL 3", 4.7f}
        },
        {
            {"DELHI", "PATEL NAGAR", "PATEL HOSPITAL 1", 4.0f},
            {"DELHI", "PATEL NAGAR", "PATEL HOSPITAL 2", 4.3f},
            {"DELHI", "PATEL NAGAR", "PATEL HOSPITAL 3", 4.6f}
        },
        {
            {"DELHI", "SHASTRI NAGAR", "SHASTRI HOSPITAL 1", 4.2f},
            {"DELHI", "SHASTRI NAGAR", "SHASTRI HOSPITAL 2", 4.5f},
            {"DELHI", "SHASTRI NAGAR", "SHASTRI HOSPITAL 3", 4.8f}
        },
        {
            {"DELHI", "KAROL BAGH", "KAROL HOSPITAL 1", 4.1f},
            {"DELHI", "KAROL BAGH", "KAROL HOSPITAL 2", 4.4f},
            {"DELHI", "KAROL BAGH", "KAROL HOSPITAL 3", 4.7f}
        }
    },
    {
        {
            {"KANPUR", "BARRA", "BARRA HOSPITAL 1", 4.0f},
            {"KANPUR", "BARRA", "BARRA HOSPITAL 2", 4.3f},
            {"KANPUR", "BARRA", "BARRA HOSPITAL 3", 4.6f}
        },
        {
            {"KANPUR", "KIDWAI NAGAR", "KIDWAI HOSPITAL 1", 4.1f},
            {"KANPUR", "KIDWAI NAGAR", "KIDWAI HOSPITAL 2", 4.4f},
            {"KANPUR", "KIDWAI NAGAR", "KIDWAI HOSPITAL 3", 4.7f}
        },
        {
            {"KANPUR", "KAKADEO", "KAKADEO HOSPITAL 1", 4.0f},
            {"KANPUR", "KAKADEO", "KAKADEO HOSPITAL 2", 4.3f},
            {"KANPUR", "KAKADEO", "KAKADEO HOSPITAL 3", 4.6f}
        },
        {
            {"KANPUR", "BADA CHAURAHA", "CHAURAHA HOSPITAL 1", 4.2f},
            {"KANPUR", "BADA CHAURAHA", "CHAURAHA HOSPITAL 2", 4.5f},
            {"KANPUR", "BADA CHAURAHA", "CHAURAHA HOSPITAL 3", 4.8f}
        }
    }
};

void clearscreen()
{
    system("cls");
}

void welcomepage()
{
    cout << "+" << setfill('-') << setw(40) << "-" << "+" << endl;
    cout << "|" << setfill(' ') << setw(40) << "WELCOME TO" << "|" << endl;
    cout << "|" << setfill(' ') << setw(40) << "MED ERA" << "|" << endl;
    cout << "|" << setfill(' ') << setw(40) << "PRESS ENTER TO CONTINUE" << "|" << endl;
    cout << "+" << setfill('-') << setw(40) << "-" << "+" << endl;

    cin.get();
}

void loginpage()
{
    int choice = 0;

    cout << "+" << setfill('-') << setw(40) << "-" << "+" << endl;
    cout << "|" << setfill(' ') << left << setw(40) << "1. APPOINTMENT" << "|" << endl;
    cout << "|" << setfill(' ') << left << setw(40) << "2. HOSPITAL STAFF LOGIN" << "|" << endl;
    cout << "|" << setfill(' ') << left << setw(40) << "3. EMERGENCY" << "|" << endl;
    cout << "+" << setfill('-') << setw(40) << "-" << "+" << endl;

    cout << "ENTER YOUR CHOICE: ";
    cin >> choice;
    clearscreen();

    switch(choice)
    {
        case 1:
            appointment();
            break;

        case 2:
            staff();
            break;

        case 3:
            emergency();
            break;

        default:
            cout << "INVALID CHOICE" << endl;
            break;
    }
}

void appointment()
{
    string name;
    string phone;

    cout << "ENTER YOUR NAME: ";
    getline(cin >> ws, name);

    cout << "ENTER PHONE NUMBER: ";
    getline(cin, phone);

    cout << "\nAPPOINTMENT DETAILS RECEIVED" << endl;
    cout << "NAME: " << name << endl;
    cout << "PHONE NUMBER: " << phone << endl;
    cout << "\nNOW SELECT YOUR CITY FOR APPOINTMENT" << endl;

    cities();
}

void staff()
{
    string name;
    string id;
    string phone;
    string password;

    cout << "ENTER STAFF NAME: ";
    getline(cin >> ws, name);

    cout << "ENTER STAFF ID: ";
    getline(cin, id);

    cout << "ENTER PHONE NUMBER: ";
    getline(cin, phone);

    cout << "ENTER PASSWORD: ";
    getline(cin, password);

    cout << "\nSTAFF LOGIN DETAILS RECEIVED" << endl;
    cout << "NAME: " << name << endl;
    cout << "STAFF ID: " << id << endl;
}

void emergency()
{
    string phone;

    cout << "EMERGENCY MODE" << endl;
    cout << "LOCATION CAPTURED (PROTOTYPE MESSAGE)" << endl;
    cout << "ENTER PHONE NUMBER: ";
    getline(cin >> ws, phone);

    cout << "\nEMERGENCY REQUEST RECEIVED" << endl;
    cout << "WE WILL CONTACT: " << phone << endl;
}

void cities()
{
    int choice = 0;

    cout << "\n";
    cout << "+" << setfill('-') << setw(40) << "-" << "+" << endl;
    cout << "|" << setfill(' ') << left << setw(40) << "{+- SELECT LOCATION -+}" << "|" << endl;
    cout << "|" << setfill(' ') << left << setw(40) << "1. DEHRADUN" << "|" << endl;
    cout << "|" << setfill(' ') << left << setw(40) << "2. DELHI" << "|" << endl;
    cout << "|" << setfill(' ') << left << setw(40) << "3. KANPUR" << "|" << endl;
    cout << "+" << setfill('-') << setw(40) << "-" << "+" << endl;

    cin >> choice;
    clearscreen();

    switch(choice)
    {
        case 1:
            cout << "SELECTED LOCATION IS DEHRADUN" << endl;
            area(1);
            break;

        case 2:
            cout << "SELECTED LOCATION IS DELHI" << endl;
            area(2);
            break;

        case 3:
            cout << "SELECTED LOCATION IS KANPUR" << endl;
            area(3);
            break;

        default:
            cout << "INVALID CHOICE" << endl;
            break;
    }
}

void area(int choice)
{
    int choose = 0;

    switch(choice)
    {
        case 1:
            cout << "1. CLEMENT TOWN" << endl;
            cout << "2. GHARI CANTT" << endl;
            cout << "3. MAJRA" << endl;
            cout << "4. RAJPUR" << endl;
            break;

        case 2:
            cout << "1. RED FORT" << endl;
            cout << "2. PATEL NAGAR" << endl;
            cout << "3. SHASTRI NAGAR" << endl;
            cout << "4. KAROL BAGH" << endl;
            break;

        case 3:
            cout << "1. BARRA" << endl;
            cout << "2. KIDWAI NAGAR" << endl;
            cout << "3. KAKADEO" << endl;
            cout << "4. BADA CHAURAHA" << endl;
            break;
    }

    cin >> choose;
    clearscreen();
    hospitals(choice, choose);
}

void hospitals(int a, int choose)
{
    if (a < 1 || a > 3 || choose < 1 || choose > 4)
    {
        cout << "INVALID CHOICE" << endl;
        return;
    }

    cout << "HOSPITAL AVAILABLE IN "
         << h[a - 1][choose - 1][0].city
         << " AREA "
         << h[a - 1][choose - 1][0].area
         << " ARE:" << endl;

    for (int i = 0; i < 3; i++)
    {
        cout << i + 1 << ". "
             << h[a - 1][choose - 1][i].name
             << " "
             << fixed << setprecision(1)
             << h[a - 1][choose - 1][i].rating
             << "*" << endl;
    }
}
