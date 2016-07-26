#include <QCommandLineParser>
#include <QDebug>
#include <QImage>
#include <QDir>
#include <QRgb>
#include <QVector>
#include <thread>

using namespace std;

class work {
public:
    work() : n(1), rank(1), image(nullptr) {}
    work(int _n, int _rank, QImage *_img) :
        n(_n), rank(_rank), image(_img) { }
    int n;
    int rank;
    QImage *image;
};

void *effect(void *arg)
{
    work *wk = static_cast<work*>(arg);
    QImage *img = wk->image;

    ulong size = img->height();
    int h0 = size * (wk->rank)     / wk->n;
    int h1 = size * (wk->rank + 1) / wk->n;

    qDebug() << "rank" << wk->rank << "h0" << h0 << "h1" << h1;

    int dr = (255 / wk->n * wk->rank) % 255;
    int dg = 0; int db = 0;
    int r, g, b;
    for (int y = h0; y < h1; y++) {
        //QRgb line = img->scanLine(y);
        for (int x = 0; x < img->width(); x++) {
            if (!img->valid(x, y)) {
                qDebug() << QString("invalid (%1,%2)").arg(x).arg(y);
            }
            QRgb rgb = img->pixel(x, y);

            r = qBound(0, qRed(rgb) + dr, 255);
            g = qBound(0, qGreen(rgb) + dg, 255);
            b = qBound(0, qBlue(rgb) + db, 255);

            img->setPixel(x, y, qRgb(r, g, b));
        }
    }

    return 0;
}

bool processImage(const QString &input, const QString &output)
{
    /*
     * Get the number of cores on the system
     */
    int n = thread::hardware_concurrency();
    QImage image;
    thread threads[n];
    work items[n];

    if (!image.load(input)) {
        qDebug() << "error loading image" << input;
        return false;
    }
    image = image.convertToFormat(QImage::Format_RGB32);

    for (int i = 0; i < n; i++) {
        items[i] = work{n, i, &image};
        threads[i] = thread(effect, &items[i]);
    }

    for (int i = 0; i < n; i++) {
        threads[i].join();
    }

    return image.save(output);
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
        output = parser.value(inOption) + ".modz.png";
    }

    qDebug() << "input:" << input;
    qDebug() << "output:" << output;

    return processImage(input, output);
}
