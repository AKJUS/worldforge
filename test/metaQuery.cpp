#include <Eris/Metaserver.h>
#include <Eris/Log.h>
#include <Eris/PollDefault.h>
#include <Eris/ServerInfo.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <sigc++/slot.h>

using std::endl;
using std::cout;
using std::cerr;

bool queryDone = false,
    failure = false,
	exactTime = false;

void erisLog(Eris::LogLevel, const std::string& msg)
{
    cerr << "ERIS: " << msg << endl;
}

void gotServerList(int count)
{
    cerr << "metaserver knows about " << count << " servers." << endl;
}

void gotServer(const Eris::ServerInfo& info)
{
    cerr << "got info for server: " << info.getServername() << '/'
        << info.getHostname() << endl;
}

void queriesDone()
{
    cerr << "query complete" << endl;
    queryDone = true;
}

void queryFailed(const std::string& msg)
{
    cerr << "got query failure: " << msg << endl;
    failure = true;
}

std::string timeFormat(double time)
{
	std::string times[] = { "secs", "mins", "hours", "days", "weeks" };
	int precs[] = { 0, 1, 1, 2, 2, 2 };
	int divi = 0;
	std::stringstream result;
	
	if(exactTime == false)
	{
		int divs[] = { 60, 60, 24, 7, 0 };
		
		while((divs[divi] > 0) && (time > divs[divi]))
		{
			time /= divs[divi++];
		}
	}
	result << std::fixed << std::setprecision(precs[divi]) << time << ' ' << times[divi];
	
	return result.str();
}

void dumpToScreen(const Eris::Meta& meta)
{
    for (unsigned int S=0; S < meta.getGameServerCount(); ++S)
    {
        const Eris::ServerInfo& sv = meta.getInfoForServer(S);
        cout << S << ": " << sv.getServername() << '/'<< sv.getHostname() << endl;
        
        switch (sv.getStatus())
        {
        case Eris::ServerInfo::VALID:
            cout << "\tserver: " << sv.getServer() << " " << sv.getVersion() << " (builddate " << sv.getBuildDate() << ")" << endl;
            cout << "\truleset:" << sv.getRuleset() << endl;
            cout << "\tuptime:" << timeFormat(sv.getUptime()) << endl;
            cout << "\tping:" << sv.getPing() << endl;
            cout << "\tconnected clients:" << sv.getNumClients() << endl;
            break;
            
        case Eris::ServerInfo::TIMEOUT:
            cout << "Timed out." << endl;
            break;
            
        case Eris::ServerInfo::QUERYING:
            cout << "Something is broken, all queries should be done" << endl;
            break;
            
        default:
            cout << "Query failed" << endl;
        }
    } // of server iteration
}

void dumpToXML(const Eris::Meta & meta)
{
    cout << "<metaquery>" << endl;
    for(unsigned int S=0; S < meta.getGameServerCount(); ++S)
    {
        const Eris::ServerInfo& sv = meta.getInfoForServer(S);
        
        cout << "<server status=\"";
        switch (sv.getStatus())
        {
        case Eris::ServerInfo::VALID: cout << "valid"; break;
        case Eris::ServerInfo::TIMEOUT: cout << "timeout"; break;
        case Eris::ServerInfo::QUERYING: cout << "querying"; break;
        default: cout << "failed";
        }
        cout << "\">" << endl;
        cout << "<address>" << sv.getHostname() << "</address>" << endl;
        if(sv.getStatus() == Eris::ServerInfo::VALID)
        {
            cout << "<status>valid</status>" << endl;
            cout << "<name>" << sv.getServername() << "</name>" << endl;
            cout << "<servertype>" << sv.getServer() << "</servertype>" << endl;
            cout << "<ruleset>" << sv.getRuleset() << "</ruleset>" << endl;
            cout << "<uptime>" << timeFormat(sv.getUptime()) << "</uptime>" << endl;
            cout << "<ping>" << sv.getPing() << "</ping>" << endl;
            cout << "<clients>" << sv.getNumClients() << "</clients>" << endl;
			cout << "<builddate>" << sv.getBuildDate() << "</builddate>" << endl;
			cout << "<version>" << sv.getVersion() << "</version>" << endl;
        }
        cout << "</server>" << endl;
    } // of server iteration
    cout << "</metaquery>" << endl;
}

void dumpToHTML(const Eris::Meta& meta)
{
    cout << "<div class=\"metaserver\">" << endl;
    cout << "  <dl>" << endl;

    for (unsigned int S=0; S < meta.getGameServerCount(); ++S)
    {
        const Eris::ServerInfo& sv = meta.getInfoForServer(S);

        cout << "    <dt>" << sv.getHostname() << " :: " << sv.getServername() << "</dt>" << endl;
        cout << "    <dd>" << endl;
        
        switch (sv.getStatus())
        {
        case Eris::ServerInfo::VALID:
            cout << "Server: " << sv.getServer() << " " << sv.getVersion() << " (builddate " << sv.getBuildDate() << ")<br/>" << endl;
            cout << "Ruleset: " << sv.getRuleset() << "<br/>" << endl;
            cout << "Up: " << timeFormat(sv.getUptime()) << " ("  << sv.getPing() << " ping)<br/>" << endl;
            cout << "Clients: " << sv.getNumClients() << endl;
            break;
            
        case Eris::ServerInfo::TIMEOUT:
            cout << "Timed out." << endl;
            break;
            
        case Eris::ServerInfo::QUERYING:
            cout << "Something is broken, all queries should be done" << endl;
            break;
            
        default:
            cout << "Query failed" << endl;
        }

        cout << "    </dd>" << endl;
    } // of server iteration

    cout << "  </dl>" << endl;
    cout << "</div>"  << endl;
}

int main(int argc, char* argv[])
{
    std::string metaServer = "metaserver.worldforge.org";
    std::vector< std::string > args(argv, argv + argc);
    void (* dumper)(const Eris::Meta &) = dumpToScreen;
    
    Eris::setLogLevel(Eris::LOG_DEBUG);
    Eris::Logged.connect(SigC::slot(&erisLog));
    
    if(args.size() > 1)
    {
        if(args[1].substr(0, 2) != "--")
        {
            metaServer = argv[1];
        }
        if(find(args.begin(), args.end(), "--html") != args.end())
        {
            dumper = dumpToHTML;
        }
        else if(find(args.begin(), args.end(), "--xml") != args.end())
        {
            dumper = dumpToXML;
        }
		if(find(args.begin(), args.end(), "--exact") != args.end())
		{
			exactTime = true;
		}
    }
    
    // maximum of 5 simultaneous queries
    Eris::Meta meta(metaServer, 5);
    meta.CompletedServerList.connect(SigC::slot(&gotServerList));
    meta.AllQueriesDone.connect(SigC::slot(&queriesDone));
    meta.ReceivedServerInfo.connect(SigC::slot(&gotServer));
    meta.Failure.connect(SigC::slot(&queryFailed));
    
    cerr << "querying " << metaServer << endl;
    meta.refresh();
    
    while (!queryDone && !failure)
    {
        Eris::PollDefault::poll(10);
    }
    
    if (failure) {
        cerr << "querying meta server at " << metaServer << " failed" << endl;
        return EXIT_FAILURE;
    }
    
    cerr << "final list contains " << meta.getGameServerCount() << " servers." << endl;
  
    dumper(meta);
    
    return EXIT_SUCCESS;
}
