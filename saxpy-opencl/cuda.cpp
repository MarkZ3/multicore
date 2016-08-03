/*
 * Summing vectors C = A + B
 *
 * Source: CUDA by example, An Introduction to
 * General-Purpose GPU Programming
 */

#define N 10

__global__ void add(int *a, int *b, int *c)
{
    // built-in kernel variable
    int tid = blockIdx.x;
    if (tid < N) {
        c[tid] = a[tid] + b[tid];
    }
}

int main()
{
    int a[N], b[N], c[N];

    int *dev_a, *dev_b, *dev_c;
    int buf_size = N * sizeof(int);

    // Allocate memory on the device
    cudaMalloc(&dev_a, buf_size);
    cudaMalloc(&dev_b, buf_size);
    cudaMalloc(&dev_c, buf_size);

    // Init input data
    for (int i = 0; i < N; i++) {
        a[i] = -i;
        b[i] = i * i;
    }

    // Copy input data to the device
    cudaMemcpy(dev_a, a, buf_size, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_b, b, buf_size, cudaMemcpyHostToDevice);

    // call the kernel (nvcc compiler specific)
    add<<<N,1>>>(dev_a, dev_b, dev_c);

    // Copy the result to the host
    cudaMemcpy(c, dev_c, buf_size, cudaMemcpyDeviceToHost);

    return 0;
}
