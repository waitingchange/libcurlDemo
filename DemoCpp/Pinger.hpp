//
//  Pinger.hpp
//  DemoCpp
//
//  Created by MacBook Pro on 4/8/21.
//

#ifndef Pinger_hpp
#define Pinger_hpp

#include <stdio.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <istream>
#include <iostream>
#include <ostream>

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

void MyPing(std::string url);

#endif /* Pinger_hpp */
