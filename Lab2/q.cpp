#include <iostream>
using namespace std;

int main(){
    string date = "HTTP/1.1 200 OK\nAccept\n-Ranges: \nbytes";
    cout << date.find_first_of("\n",16);
    return 0;
}