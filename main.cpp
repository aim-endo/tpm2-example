#include <iostream>
#include <tss2/tss2_esys.h>
#include <tss2/tss2_tctildr.h>
#include <tss2/tss2_sys.h>

/**
 *
 */
#define THROW_IF_ERR(r, f)    \
    if (r != TSS2_RC_SUCCESS) { \
        std::cout << "[" << __FILE__ << ": " << __LINE__ << "] " << f << "() = " << r << std::endl; \
        throw r; \
    }

/**
 *
 */
int main(int argc, char** argv)
{
    TSS2_RC r = TSS2_RC_SUCCESS;

    try {
        const char* device = "device:/dev/tpmrm0";

        // TCTI initialize
        TSS2_TCTI_CONTEXT* tcti = nullptr;
        r = Tss2_TctiLdr_Initialize(device, &tcti);
        THROW_IF_ERR(r, "Tss2_TctiLdr_Initialize");

        // TCTI Information
        TSS2_TCTI_INFO* info = nullptr;
        r = Tss2_TctiLdr_GetInfo(device, &info);
        THROW_IF_ERR(r, "Tss2_TctiLdr_GetInfo");

        std::cout << "TCTI Information: " << std::endl;
        std::cout << "  version: " << info->version << std::endl;
        std::cout << "  name: " << info->name << std::endl;
        std::cout << "  description: " << info->description << std::endl;
        std::cout << "  config_help: " << info->config_help << std::endl;
        std::cout << "  has initialize func: " << std::boolalpha << info->init << std::endl;

        Tss2_TctiLdr_FreeInfo(&info);

        // ESYS Initialize
        ESYS_CONTEXT* context = nullptr;
        r = Esys_Initialize(&context, tcti, nullptr);
        THROW_IF_ERR(r, "Esys_Initialize");

        // Get SYS context
        TSS2_SYS_CONTEXT* sys = nullptr;
        r = Esys_GetSysContext(context, &sys);
        THROW_IF_ERR(r, "Esys_GetSysContext");

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

