#ifndef PLAY_H
#define PLAY_H
#include<mutex>
#include<condition_variable>
std::mutex search;
std::mutex stop;
std::condition_variable cv;
bool isSearching = false;
#endif