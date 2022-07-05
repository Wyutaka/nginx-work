//
// Created by miwa on 2019/11/11.
//

#ifndef TRANSACTION_DEBUG_HPP
#define TRANSACTION_DEBUG_HPP

#include <iostream>

#ifdef RELEASE
#define DebugPrint(x) /* nothing to do */
#else
#define DebugPrint(x) std::clog<<__FILE__<<":"<<__LINE__<<": "<<__func__<<": "<< x << std::endl
#endif

#endif // TRANSACTION_DEBUG_HPP
