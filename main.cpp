#include <iostream>
#include <tss2/tss2_esys.h>
#include <tss2/tss2_tctildr.h>
#include <tss2/tss2_sys.h>

int main(int argc, char** argv)
{
    TSS2_RC r = TSS2_RC_SUCCESS;

    try {
        const char* device = "device:/dev/tpmrm0";

        // TCTI initialize
        TSS2_TCTI_CONTEXT* tcti = nullptr;
        r = Tss2_TctiLdr_Initialize(device, &tcti);

        if (r != TSS2_RC_SUCCESS) {
            std::cout << "Tss2_TctiLdr_Initialize() = " << r << std::endl;
            if (! tcti) std::cout << "  tcti is null" << std::endl;
            throw r;
        }

        // TCTI Information
        TSS2_TCTI_INFO* info = nullptr;
        r = Tss2_TctiLdr_GetInfo(device, &info);
        if (r != TSS2_RC_SUCCESS) {
            std::cout << "Tss2_TctiLdr_GetInfo() = " << r << std::endl;
            if (! info) std::cout << "  tcti info is null" << std::endl;
            throw r;
        }
        else {
            std::cout << "TCTI Information: " << std::endl;
            std::cout << "  version: " << info->version << std::endl;
            std::cout << "  name: " << info->name << std::endl;
            std::cout << "  description: " << info->description << std::endl;
            std::cout << "  config_help: " << info->config_help << std::endl;
            std::cout << "  has initialize func: " << std::boolalpha << info->init << std::endl;

            Tss2_TctiLdr_FreeInfo(&info);
        }

        // ESYS Initialize
        ESYS_CONTEXT* context = nullptr;
        r = Esys_Initialize(&context, tcti, nullptr);

        if (r != TSS2_RC_SUCCESS) {
            std::cout << "Esys_Initialize() = " << r << std::endl;
            if (! context) std::cout << "  esys context is null" << std::endl;
            throw r;
        }

        // Get SYS context
        TSS2_SYS_CONTEXT* sys = nullptr;
        r = Esys_GetSysContext(context, &sys);

        if (r != TSS2_RC_SUCCESS) {
            std::cout << "Esys_GetSysContext() = " << r << std::endl;
            if (! sys) std::cout << "  sys context is null" << std::endl;
            throw r;
        }

        // ESYS Finalize
        Esys_Finalize(&context);

        // TCTI Finalize
        Tss2_TctiLdr_Finalize(&tcti);
    }
    catch (...) {
        std::cout << "error occurred" << std::endl;
    }

    return r;
}

