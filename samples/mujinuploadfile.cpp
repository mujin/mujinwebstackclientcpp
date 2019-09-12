// -*- coding: utf-8 -*-
/** \example mujinuploadfile.cpp

    Shows how to upload file
    example1: mujinuploadfile --controller_hostname=yourhost --filename=sample.mujin.dae
 */

#include <mujincontrollerclient/mujincontrollerclient.h>

#include <boost/program_options.hpp>
#include <signal.h>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#undef GetUserName // clashes with ControllerClient::GetUserName
#define NOMINMAX
#include <windows.h>
const char s_filesep = '\\';
const wchar_t s_wfilesep = L'\\';
#else
const char s_filesep = '/';
const wchar_t s_wfilesep = L'/';
#endif // defined(_WIN32) || defined(_WIN64)

using namespace mujinclient;
namespace bpo = boost::program_options;
using namespace std;

/// \brief parse command line options and store in a map
/// \param argc number of arguments
/// \param argv arguments
/// \param opts map where parsed options are stored
/// \return true if non-help options are parsed succesfully.
bool ParseOptions(int argc, char ** argv, bpo::variables_map& opts)
{
    // parse command line arguments
    bpo::options_description desc("Options");

    desc.add_options()
        ("help,h", "produce help message")
        ("controller_hostname", bpo::value<string>()->required(), "hostname or ip of the mujin controller, e.g. controllerXX or 192.168.0.1")
        ("controller_port", bpo::value<unsigned int>()->default_value(80), "port of the mujin controller")
        ("controller_username_password", bpo::value<string>()->default_value("testuser:pass"), "username and password to the mujin controller, e.g. username:password")
        ("filename", bpo::value<string>()->required(), "file name to upload")
        ;

    try {
        bpo::store(bpo::parse_command_line(argc, argv, desc, bpo::command_line_style::unix_style ^ bpo::command_line_style::allow_short), opts);
    }
    catch (const exception& ex) {
        stringstream errss;
        errss << "Caught exception " << ex.what();
        cerr << errss.str() << endl;
        return false;
    }

    bool badargs = false;
    try {
        bpo::notify(opts);
    }
    catch(const exception& ex) {
        stringstream errss;
        errss << "Caught exception " << ex.what();
        cerr << errss.str() << endl;
        badargs = true;
    }

    if(opts.count("help") || badargs) {
        cout << "Usage: " << argv[0] << " [OPTS]" << endl;
        cout << endl;
        cout << desc << endl;
        return false;
    }
    return true;
}

int main(int argc, char ** argv)
{
    // parsing options
    bpo::variables_map opts;
    if (!ParseOptions(argc, argv, opts)) {
        // parsing option failed
        return 1;
    }

    const string controllerUsernamePass = opts["controller_username_password"].as<string>();
    const string hostname = opts["controller_hostname"].as<string>();
    const unsigned int controllerPort = opts["controller_port"].as<unsigned int>();
    const string filename = opts["filename"].as<string>();
    stringstream urlss;
    urlss << "http://" << hostname << ":" << controllerPort;

    // connect to mujin controller
    ControllerClientPtr controllerclient = CreateControllerClient(controllerUsernamePass, urlss.str());
    cerr << "connected to mujin controller at " << urlss.str() << endl;

    size_t nBaseFilenameStartIndex = filename.find_last_of(s_filesep);
    std::string basename = nBaseFilenameStartIndex == std::string::npos ? filename : filename.substr(nBaseFilenameStartIndex+1);
    controllerclient->UploadFileToController_UTF8(filename, std::string("mujin:/")+basename);

    return 0;
}
