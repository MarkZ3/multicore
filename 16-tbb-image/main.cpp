#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QDebug>
#include <QImage>
#include <QDir>
#include <QRgb>
#include <QVector>
#include <thread>
#include <iostream>

using namespace std;

void processImage(QImage &img, int cpus)
{
    (void) cpus;

    // TODO: modify the image in parallel
    int dr = 20, dg = 0, db = 0;
    for (int y = 0; y < img.height(); y++) {
        QRgb *line = (QRgb *) img.scanLine(y);
        for (int x = 0; x < img.width(); x++) {
            if (!img.valid(x, y)) {
                qDebug() << QString("invalid (%1,%2)").arg(x).arg(y);
            }
            QRgb *rgb = line + x;

            int r = qBound(0, qRed(*rgb) + dr, 255);
            int g = qBound(0, qGreen(*rgb) + dg, 255);
            int b = qBound(0, qBlue(*rgb) + db, 255);

            *rgb = qRgb(r, g, b);
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.setApplicationDescription("multithread image modification");

    QCommandLineOption inOption("input", "input image", "input");
    parser.addOption(inOption);

    QCommandLineOption outOption("output", "output image", "output");
    parser.addOption(outOption);

    parser.process(app);

    if (!parser.isSet(inOption)) {
        parser.showHelp(1);
    }

    QString input(parser.value(inOption));
    QString output(parser.value(outOption));
    if (!parser.isSet(outOption)) {
        output = parser.value(inOption) + ".modz";
    }

    int n = thread::hardware_concurrency();
    qDebug() << "input:" << input;
    qDebug() << "output:" << output;
    qDebug() << "num cpus:" << n;

    QImage image;
    if (!image.load(input)) {
        qDebug() << "error loading image" << input;
        return false;
    }
    image = image.convertToFormat(QImage::Format_RGB32);

    // benchmark
    QMap<int, qint64> results;
    QElapsedTimer timer;
    for (int cpus = 1; cpus <= n; cpus *= 2) {
        qDebug() << "threads=" << cpus;
        timer.restart();
        processImage(image, cpus);
        results[cpus] = timer.nsecsElapsed();
        image.save(output + "." + QString::number(cpus) + ".png");
    }

    // report
    double baseline = results[1];
    for (const int key : results.keys()) {
        double speedup = baseline / results[key];
        cout << QString("%1;%2;%3")
                .arg(key)
                .arg(results[key])
                .arg(speedup)
                .toStdString() << endl;
    }

    return 0;
}
