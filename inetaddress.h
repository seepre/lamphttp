//
// Created by HEADS on 2021/2/21.
//

#ifndef INETADDRESS_H
#define INETADDRESS_H

struct inet_address {
    int port;
};

struct inet_address *init_inet_address(int port);

#endif //INETADDRESS_H
