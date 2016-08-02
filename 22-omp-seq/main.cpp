#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <math.h>
#include <iostream>

QString gnuplot(
"set terminal png\n"
"set output '/tmp/seq.png'\n"
"set grid\n"
"set title 'Signal according to time'\n"
"set xlabel 'Amplitude'\n"
"set ylabel 'Time (s)'\n"
"plot '/tmp/seq.data' u 1 w lp, '/tmp/seq.data' u 2 w lp\n");

void write_data(QVector<float> &orig, QVector<float> &norm)
{
    QFile data;
    data.setFileName("/tmp/seq.data");
    data.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    QTextStream out(&data);
    for (int i = 0; i < orig.length(); i++) {
        out << QString("%1 %2\n").arg(orig[i]).arg(norm[i]);
    }
    data.close();

    QString plotpath = "/tmp/seq.gnuplot";
    QFile plot;
    plot.setFileName(plotpath);
    plot.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    QTextStream out2(&plot);
    out2 << gnuplot;
    plot.close();

    QProcess process;
    process.start("gnuplot", QStringList() << plotpath);
    process.waitForFinished();
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    // variables
    int n = 1000;
    float amplitude = 0.0;
    int i = 0;
    float p = 0.1;
    float t = 0.0;
    QVector<float> orig(n);
    QVector<float> norm(n);

    // init sound-like data
    #pragma omp parallel for private(t)
    for (i = 0; i < n; i++) {
        t = i * p;
        orig[i] = std::cos((t + 0.1) * 0.1) +
                  std::cos((t + 0.2) * 0.2) +
                  std::cos((t + 0.3) * 0.3);
    }

    // detect the max value
    #pragma omp parallel for reduction(max:amplitude)
    for (i = 0; i < n; i++) {
        amplitude = std::max(amplitude, std::abs(orig[i]));
    }

    qDebug() << "amplitude=" << amplitude;

    // normalize
    float inv_amplitude = 1 / amplitude;
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        norm[i] = orig[i] * inv_amplitude;
    }

    // verify the max value
    amplitude = 0.0;
    #pragma omp parallel for reduction(max:amplitude)
    for (i = 0; i < n; i++) {
        amplitude = std::max(amplitude, std::abs(norm[i]));
    }

    qDebug() << "normalized max=" << amplitude;

    // find the max value (manual reduction)
    float temp = 0.0;
    float amplitude2 = 0.0;
    #pragma omp parallel firstprivate(temp)
    {
        #pragma omp for
        for (i = 0; i < n; i++) {
            temp = std::max(temp, std::abs(norm[i]));
        }
        #pragma omp critical
        {
            amplitude2 = std::max(temp, amplitude2);
        }
    }

    qDebug() << "check amplitude2=" << amplitude2;

    write_data(orig, norm);

    return 0;
}
