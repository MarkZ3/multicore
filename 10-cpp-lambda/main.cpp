#include <QDebug>
#include <QVector>
#include <functional>

using namespace std;

QVector<int> filter(const QVector<int> &v, std::function<bool (const int&)> func)
{
    QVector<int> filtered;
    for (int x: v) {
        if (func(x)) {
            filtered.append(x);
        }
    }
    return filtered;
}

void execute(std::function<void ()> func)
{
    func();
}

std::function<void ()> make_func()
{
    QString msg("local variable invalid outside of make_func");
    return [&] () { qDebug() << "lambda invalid" << msg; };
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    /* Source: http://en.cppreference.com/w/cpp/language/lambda
     *
     * lambda syntax: [capture](parameters) -> ret { body };
     *
     * [a,&b] where a is captured by value and b is captured by reference.
     * [this] captures the this pointer by value
     * [&] captures all automatic variables odr-used in the body of the lambda by reference
     * [=] captures all automatic variables odr-used in the body of the lambda by value
     * [] captures nothing
     *
     * if ret is not preset, implied from the return statement
     *
     */

    // basic example: no capture, no arguments, no return value
    std::function<void ()> f1 = [] () { qDebug() << "lambda"; };
    f1();           // explicit call
    execute(f1);    // pass as parameter

    // let the compiler figure the type
    auto f2 = [] () { qDebug() << "lambda"; };
    f2();

    // you can access variables from the parent scope...
    QString msg("variable in parent scope by reference");
    auto f3 = [&] () { qDebug() << "lambda" << msg; };
    f3();

    // ...but this variable must be valid at the time the lambda is called
    // don't do the following:
    auto f4 = make_func();
    f4(); // SIGSEGV

    // example of filter using lambda with argument and return
    QVector<int> data({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

    auto even = [] (const int &x) { return (x % 2) == 0; };
    auto odd = [] (const int &x) { return ((x + 1) % 2) == 0; };
    auto lucky = [] (const int &x) { return x == 7; };

    qDebug() << "even " << filter(data, even);
    qDebug() << "odd  " << filter(data, odd);
    qDebug() << "lucky" << filter(data, lucky);
    qDebug() << "mod3 " << filter(data, [] (const int &x) { return (x % 3) == 0; } );

    return 0;
}
