#include "clamav_gui.h"
#include <QTranslator>
#include <QApplication>
#include <QtNetwork/QTcpServer>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include "schedulescanobject.h"
#include "setupfilehandler.h"

#define PORT_NUM 55000

int main(int argc, char *argv[])
{
QTranslator translator(0);
QTcpServer server;
QStringList parameters;
QString lang;
QString setLang;
QString rc;
bool translatorLoaded = false;

    QApplication a(argc, argv);
    lang = QLocale::system().name();

    if (argc > 1) rc=(QString)argv[1];
    if (argc > 2) setLang = (QString) argv[2];

    if (rc == "--language"){
        printf("\nDesktop Language Settings: %s\n",lang.toStdString().c_str());
    }

    QFile file;
    if (file.exists(QDir::homePath() + "/.clamav-gui/settings.ini") == true) {
        setupFileHandler * setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
        if (setupFile->keywordExists("Setup","language") == true) {
            lang = setupFile->getSectionValue("Setup","language").mid(1,5);
        } else {
            lang = "en_GB";
        }
    } else {
        lang = "en_GB";
    }

    if (rc == "--setlang") {
        lang = setLang;
    }

    if (rc == "--scan") {
        if (file.exists(QDir::homePath() + "/.clamav-gui/settings.ini") == true) {
            setupFileHandler * setupFile = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
            //QStringList parameters;
            QStringList selectedOptions = setupFile->getKeywords("SelectedOptions");
            QStringList directoryOptions = setupFile->getKeywords("Directories");
            QStringList scanLimitations = setupFile->getKeywords("ScanLimitations");
            QString option;
            QString checked;
            QString value;

            for (int i = 0; i < selectedOptions.count(); i++){
                parameters << selectedOptions.at(i).left(selectedOptions.indexOf("|")).replace("<equal>","=");
            }

            // Directory Options
            for (int i = 0; i < directoryOptions.count(); i++){
                option = directoryOptions.at(i);
                value = setupFile->getSectionValue("Directories",option);
                checked = value.left(value.indexOf("|"));
                value = value.mid(value.indexOf("|") + 1);
                if ((checked == "checked") && (value != "")) {
                    if (option == "LoadSupportedDBFiles") parameters << "--database=" + value;
                    if (option == "ScanReportToFile") {
                        parameters << "--log=" + value;
                        QFile file(value);
                        if (file.open(QIODevice::ReadWrite|QIODevice::Append|QIODevice::Text)){
                            QTextStream stream(&file);
                            stream << "\n<Scanning startet> " << QDateTime::currentDateTime().toString("yyyy/M/d - hh:mm");
                            file.close();
                        }
                    }
                    if (option == "TmpFile") parameters << "--tempdir=" + value;
                    if (option == "MoveInfectedFiles") parameters << "--move=" + value;
                    if (option == "CopyInfectedFiles") parameters << "--copy=" + value;
                    if (option == "SCanFileFromFiles") parameters << "--file-list=" + value;
                    if (option == "FollowDirectorySymLinks") parameters << "--follow-dir-symlinks=" + value;
                    if (option == "FollowFileSymLinks") parameters << "--follow-file-symlinks=" + value;
                }
            }

            // Scan Limitations
            for (int i = 0; i < scanLimitations.count(); i++){
                option = scanLimitations.at(i);
                value = setupFile->getSectionValue("ScanLimitations",option);
                checked = value.left(value.indexOf("|"));
                value = value.mid(value.indexOf("|") + 1);
                if (checked == "checked"){
                    if (option == "Files larger than this will be skipped and assumed clean") parameters << "--max-filesize=" + value;
                    if (option == "The maximum amount of data to scan for each container file") parameters << "--max-scansize=" + value;
                    if (option == "The maximum number of files to scan for each container file") parameters << "--max-files=" + value;
                    if (option == "Maximum archive recursion level for container file") parameters << "--max-recursion=" + value;
                    if (option == "Maximum directory recursion level") parameters << "--max-dir-recursion=" + value;
                    if (option == "Maximum size file to check for embedded PE") parameters << "--max-embeddedpe=" + value;
                    if (option == "Maximum size of HTML file to normalize") parameters << "--max-htmlnormalize=" + value;
                    if (option == "Maximum size of normalized HTML file to scan") parameters << "--max-htmlnotags=" + value;
                    if (option == "Maximum size of script file to normalize") parameters << "--max-scriptnormalize=" + value;
                    if (option == "Maximum size zip to type reanalyze") parameters << "--max-ziptypercg=" + value;
                    if (option == "Maximum number of partitions in disk image to be scanned") parameters << "--max-partitions=" + value;
                    if (option == "Maximum number of icons in PE file to be scanned") parameters << "--max-iconspe=" + value;
                    if (option == "Number of seconds to wait for waiting a response back from the stats server") parameters << "--stats-timeout=" + value;
                    if (option == "Bytecode timeout in milliseconds") parameters << "--bytecode-timeout=" + value;
                    if (option == "Collect and print execution statistics") parameters << "--statistics=" + value;
                    if (option == "Structured SSN Format") parameters << "--structured-ssn-format=" + value;
                    if (option == "Structured SSN Count") parameters << "--structured-ssn-count=" + value;
                    if (option == "Structured CC Count") parameters << "--structured-cc-count=" + value;
                    if (option == "Structured CC Mode") parameters << "--structured-cc-mode=" + value;
                    if (option == "Max Scan-Time") parameters << "--max-scantime=" + value;
                    if (option == "Max recursion to HWP3 parsing function") parameters << "--max-rechwp3=" + value;
                    if (option == "Max calls to PCRE match function") parameters << "--pcre-match-limit=" + value;
                    if (option == "Max recursion calls to the PCRE match function") parameters << "--pcre-recmatch-limit=" + value;
                    if (option == "Max PCRE file size") parameters << "--pcre-max-filesize=" + value;
                    if (option == "Database outdated if older than x days") parameters << "--fail-if-cvd-older-than=" + value;
                }
            }

            // REGEXP and Include Exclude Options
            value = setupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanFileNamesMatchingRegExp");
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);
            if (checked == "checked") parameters << "--exclude=" + value;

            value = setupFile->getSectionValue("REGEXP_and_IncludeExclude","DontScanDirectoriesMatchingRegExp");
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);
            if (checked == "checked") parameters << "--exclude-dir=" + value;

            value = setupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanFileNamesMatchingRegExp");
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);
            if (checked == "checked") parameters << "--include=" + value;

            value = setupFile->getSectionValue("REGEXP_and_IncludeExclude","OnlyScanDirectoriesMatchingRegExp");
            checked = value.left(value.indexOf("|"));
            value = value.mid(value.indexOf("|") + 1);
            if (checked == "checked") parameters << "--include-dir=" + value;

            if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","EnablePUAOptions") == true){
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAPacked") == true) parameters << "--include-pua=Packed";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAPWTool") == true) parameters << "--include-pua=PWTool";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUANetTool") == true) parameters << "--include-pua=NetTool";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAP2P") == true) parameters << "--include-pua=P2P";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAIRC") == true) parameters << "--include-pua=IRC";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUARAT") == true) parameters << "--include-pua=RAT";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUANetToolSpy") == true) parameters << "--include-pua=NetToolSpy";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAServer") == true) parameters << "--include-pua=Server";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAScript") == true) parameters << "--include-pua=Script";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAAndr") == true) parameters << "--include-pua=Andr";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAJava") == true) parameters << "--include-pua=Java";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAOsx") == true) parameters << "--include-pua=Osx";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUATool") == true) parameters << "--include-pua=Tool";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAUnix") == true) parameters << "--include-pua=Unix";
                if (setupFile->getSectionBoolValue("REGEXP_and_IncludeExclude","LoadPUAWin") == true) parameters << "--include-pua=Win";
            }
        }

        parameters << "-r";
        // if (databaseDir != "") parameters << "--database=" + databaseDir;

        for (int i = 2; i < argc; i++){
            parameters << (QString)argv[i];
        }

        scheduleScanObject * scanObject = new scheduleScanObject(0,"Direct Scan",parameters);

        scanObject->setWindowTitle("Direct Scan-Job");
        scanObject->setWindowIcon(QIcon(":/icons/icons/media.png"));
        scanObject->setModal(true);
        scanObject->exec();
        delete scanObject;
    } else {
        if (rc != "--force"){
            if( !server.listen( QHostAddress::LocalHost, PORT_NUM ) ) {
                qDebug() << "Application already running!";
                exit(0);
            }
        }

        QString filename = "clamav-gui-" + lang + ".qm";

        if (QFile::exists("/usr/share/clamav-gui/" + filename)){
            translatorLoaded = translator.load(filename,"/usr/share/clamav-gui");
        }

        if (translatorLoaded == true) a.installTranslator(&translator);

        clamav_gui w;
        w.show();

        return a.exec();
    }
}
