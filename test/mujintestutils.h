// -*- coding: utf-8 -*-
#ifndef MUJINCONTROLLER_TESTUTILS_H
#define MUJINCONTROLLER_TESTUTILS_H

#include <mujinwebstackclientcpp/webstackclient.h>
#include <iostream>

mujinclient::WebstackClientPtr CreateControllerFromCommandLine(int argc, char ** argv)
{
    mujinclient::WebstackClientPtr controller;
    if( argc >= 5 ) {
        controller = mujinclient::CreateWebstackClient(argv[1], argv[2], argv[3], argv[4]);
    }
    if( argc == 4 ) {
        controller = mujinclient::CreateWebstackClient(argv[1], argv[2], argv[3]);
    }
    else if( argc == 3 ) {
        controller = mujinclient::CreateWebstackClient(argv[1], argv[2]);
    }
    else {
        controller = mujinclient::CreateWebstackClient(argv[1]);
    }
    std::cout << "connected to controller v" << controller->GetVersion() << std::endl;
    return controller;
}

#endif
