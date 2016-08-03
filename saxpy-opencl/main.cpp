#include <QDebug>
#include <QVector>
#include <QMap>
#include <exception>
#include <functional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#ifdef __APPLE__
    #include "OpenCL/opencl.hpp"
#else
    #include "CL/cl.hpp"
#endif

struct OpenCL {
public:
    OpenCL() { }

    void init() {
        /*
         * Get available platforms
         */
        cl::Platform::get(&m_platforms);

        /*
         * Get device info for each platforms
         */
        for(const auto platform: m_platforms) {
            std::vector<cl::Device> devices;
            platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
            m_devices.insert(m_devices.end(), devices.begin(), devices.end());
        }

        qDebug() << "platforms" << m_platforms.size();
        qDebug() << "devices" << m_devices.size();

        if (m_devices.size() == 0)
            throw std::runtime_error("No OpenCL device");

    }

    /* Just return the first device for simplicity */
    cl::Device get_device() {
        return m_devices.at(0);
    }

    std::vector<cl::Platform> m_platforms;
    std::vector<cl::Device> m_devices;
};

#define MULTI_LINE_STRING(...) #__VA_ARGS__
class SaxpyCL {

    /*
     * Text of the program. Here, we use a special macro to avoid
     * quoting the string. The kernel code is not compiled when
     * this file is compiled. It is compiled at runtime.
     *
     * __kernel : Attribute indicating this function can be called from the host
     * __global : A buffer on the device created with cl::Buffer()
     */
    const char *kernel_text = MULTI_LINE_STRING(
        __kernel void SAXPY (__global float* x,
                             __global float* y,
                             __global float* z,
                             float a) {
            const int i = get_global_id (0);
            z[i] = a * x[i] + y[i];
        }
    );

public:
    SaxpyCL() { }
    ~SaxpyCL() { }

    void init(const cl::Device &dev) {
        /* Create context that will group all related operations on a device */
        m_ctx = cl::Context(dev);

        /* Load the program (does not compile it yet) */
        std::string text(kernel_text);
        m_prog = cl::Program(m_ctx, text);

        /* Compile the program */
        if (m_prog.build({dev}) != CL_SUCCESS) {
            throw std::runtime_error("Failed to compile the OpenCL program");
        }

        /* Obtain a reference on the main kernel (function annotated with __kernel attribute) */
        m_kernel = cl::Kernel(m_prog, "SAXPY");

        /* Create the queue for the commands. They are executed in FIFO order. */
        m_queue     = cl::CommandQueue(m_ctx, dev);
    }

    void compute(float a, QVector<float> &x, QVector<float> &y, QVector<float> &z) {

        /* la taille en octets des tampons, requis pour allouer les tampons sur le périphérique */
        size_t size = x.size() * sizeof(float);

        /*
         * Create buffers on the device. The operations CL_MEM_READ/CL_MEM_WRITE
         * are from the device point of view. The kernel reads cl_x and cl_y and write to cl_z
         */
        cl::Buffer cl_x(m_ctx, CL_MEM_READ_ONLY, size);
        cl::Buffer cl_y(m_ctx, CL_MEM_READ_ONLY, size);
        cl::Buffer cl_z(m_ctx, CL_MEM_WRITE_ONLY, size);

        /*
         * Copy data from the host to the device. The READ/WRITE operations are from the host
         * point of view. For instance, the input vector x is copied to the device in cl_x.
         */
        m_queue.enqueueWriteBuffer(cl_x, CL_TRUE, 0, size, x.data());
        m_queue.enqueueWriteBuffer(cl_y, CL_TRUE, 0, size, y.data());

        /* Set kernel arguments */
        m_kernel.setArg(0, cl_x);
        m_kernel.setArg(1, cl_y);
        m_kernel.setArg(2, cl_z);
        m_kernel.setArg(3, a);

        /*
         * Call the kernel. cl::NDRange() indicates the size for all dimensions. Here,
         * there is only one dimension and the kernel obtains it by using get_global_id(0)
         */
        m_queue.enqueueNDRangeKernel(m_kernel, cl::NullRange, cl::NDRange(x.size()));
        m_queue.finish();

        /* Fetch result (copy from the device to the host) */
        m_queue.enqueueReadBuffer(cl_z, CL_TRUE, 0, size, z.data());

        /* Free buffers on the device (Buffer::release() called in the destructor) */
    }

    OpenCL m_ocl;
    cl::Context m_ctx;
    cl::Program m_prog;
    cl::Kernel m_kernel;
    cl::CommandQueue m_queue;
};

void do_serial(QVector<float> &x, QVector<float> &y, QVector<float> &z, float a)
{
    for (int i = 0; i < x.size(); i++) {
        z[i] = a * x[i] + y[i];
    }
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    OpenCL ocl;
    ocl.init();
    SaxpyCL saxpy;
    saxpy.init(ocl.get_device());

    float a = 2.0;
    QVector<float> x = {1.0, 2.0, 3.0};
    QVector<float> y = {10.0, 20.0, 30.0};
    QVector<float> act = {0, 0, 0};
    QVector<float> exp = {0, 0, 0};

    do_serial(x, y, exp, a);
    saxpy.compute(a, x, y, act);

    qDebug() << exp;
    qDebug() << act;

    return 0;
}

#pragma GCC diagnostic pop
