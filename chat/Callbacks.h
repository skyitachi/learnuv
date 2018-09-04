//
// Created by skyitachi on 2018/9/4.
//

#ifndef LEARNUV_CALLBACKS_H
#define LEARNUV_CALLBACKS_H
#include <functional>

class Connection;

typedef std::function<void(const char *msg)> MessageCallback;


#endif //LEARNUV_CALLBACKS_H
