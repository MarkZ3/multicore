#include <BPatch.h>
#include <BPatch_object.h>
#include <BPatch_point.h>
#include <BPatch_function.h>
#include <vector>
#include <cstdlib>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <cstring>
#include <vector>

int main (int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.setApplicationDescription("dynamic instrumentation");
    parser.addPositionalArgument("binary", "path to binary to execute");

    QCommandLineOption inOption("function", "function to instrument", "function");
    parser.addOption(inOption);
    parser.process(app);

    QStringList lst = parser.positionalArguments();

    if (!parser.isSet(inOption) || lst.size() == 0) {
        parser.showHelp(1);
    }

    setenv("DYNINSTAPI_RT_LIB", "/usr/local/lib/libdyninstAPI_RT.so", 0);

    // convert QStringList to array of char
    const char *args[lst.size() + 1];
    for (int i = 0; i < lst.size(); i++) {
        args[i] = lst.at(i).toStdString().c_str();
    }
    args[lst.size()] = NULL;

    BPatch bpatch;
    BPatch_process *proc = bpatch.processCreate(args[0], args + 1);
    BPatch_image *image = proc->getImage();
    std::vector<BPatch_function*> funcs;
    std::vector<BPatch_function*> probes;
    image->findFunction("_ZN3Foo3fooEv", funcs);
    image->findFunction("probe", probes);

    if (!funcs.empty() && !probes.empty()) {
        for (BPatch_function *func: funcs) {
            qDebug() << "func" << func->getBaseAddr();
        }
        for (BPatch_function *func: probes) {
            qDebug() << "probe" << func->getBaseAddr();
        }
        std::vector<BPatch_snippet*> probe_args;
        BPatch_funcCallExpr call_probe(*probes[0], probe_args);
        proc->insertSnippet(call_probe, (funcs[0]->findPoint(BPatch_entry))[0]);
    }


    proc->continueExecution();
    while (!proc->isTerminated()) {
        bpatch.waitForStatusChange();
    }

}
