// -*- coding: utf-8 -*-
#ifndef MUJINCONTROLLER_TESTUTILS_H
#define MUJINCONTROLLER_TESTUTILS_H

#include <mujinwebstackclient/mujinwebstackclient.h>
#include <iostream>

mujinwebstackclient::WebstackClientPtr CreateControllerFromCommandLine(int argc, char ** argv)
{
    mujinwebstackclient::WebstackClientPtr controller;
    if( argc >= 5 ) {
        controller = mujinwebstackclient::CreateWebstackClient(argv[1], argv[2], argv[3], argv[4]);
    }
    if( argc == 4 ) {
        controller = mujinwebstackclient::CreateWebstackClient(argv[1], argv[2], argv[3]);
    }
    else if( argc == 3 ) {
        controller = mujinwebstackclient::CreateWebstackClient(argv[1], argv[2]);
    }
    else {
        controller = mujinwebstackclient::CreateWebstackClient(argv[1]);
    }
    std::cout << "connected to controller v" << controller->GetVersion() << std::endl;
    return controller;
}

#endif
