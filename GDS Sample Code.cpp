#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cuda_runtime.h>
#include "cufile.h"
/*
Our code is majorly based on Nvidia's sample code
*/

using namespace std;

int main(void) {
        int fd;
        ssize_t rtrn;
        void *devPtr;
        off_t file_offset = 0x2000;
        off_t devPtr_offset = 0x1000;
        ssize_t IO_size = 1UL << 24;
        size_t buff_size = IO_size + 0x1000;
        CUfileError_t status;
        int cuda_result;
        CUfileDescr_t cf_descr;
        CUfileHandle_t cf_handle;
        char *file;
        file = getenv("TESTFILE");
        if (file==NULL) {
            std::cerr << "No testfile defined via TESTFILE.  Exiting." << std::endl;
            return -1;
        }
        cout << std::endl;

        cout << "Opening File " << file << std::endl;
        fd = open(file, O_CREAT|O_WRONLY|O_DIRECT, 0644);

        cout << "Opening cuFileDriver." << std::endl;
        status = cuFileDriverOpen();

        cout << "Registering cuFile handle to " << file << "." << std::endl;
        memset((void *)&cf_descr, 0, sizeof(CUfileDescr_t));
        cf_descr.handle.fd = fd;
        cf_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
        status = cuFileHandleRegister(&cf_handle, &cf_descr);
        cout << "Allocating CUDA buffer of " << buff_size << " bytes." << std::endl;
        cuda_result = cudaMalloc(&devPtr, buff_size);

        cout << "Registering Buffer of " << buff_size << " bytes." << std::endl;
        status = cuFileBufRegister(devPtr, buff_size, 0);
        cout << "Filling memory." << std::endl;
        cudaMemset((void *) devPtr, 0xab, buff_size);

        cout << "Writing buffer to file." << std::endl;
        rtrn = cuFileWrite(cf_handle, devPtr, IO_size, file_offset, devPtr_offset);

        cout << "Releasing cuFile buffer." << std::endl;
        status = cuFileBufDeregister(devPtr);

        cout << "Freeing CUDA buffer." << std::endl;
        cudaFree(devPtr);

        cout << "Releasing file handle. " << std::endl;
        (void) cuFileHandleDeregister(cf_handle);
        close(fd);

        cout << "Closing File Driver." << std::endl;
        (void) cuFileDriverClose();

        cout << std::endl;

        return 0;
}
