#include <functional>
#include <iostream>
#include <string>
#include <vector>

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

typedef struct capability_option {
    std::string name;
    TPM2_CAP    capability;
    UINT32      property;
    UINT32      count;
    std::function<UINT32(const TPMS_CAPABILITY_DATA*)> fetch_count;
} capability_option;

/**
 *
 */
void print_esys_capabilities(const ESYS_CONTEXT* context)
{
    const std::vector<capability_option> capability_options = {
        { "algorithms", TPM2_CAP_ALGS, TPM2_ALG_FIRST, TPM2_MAX_CAP_ALGS, [](const TPMS_CAPABILITY_DATA* d){ return d->data.algorithms.count; } },
        { "commands", TPM2_CAP_COMMANDS, TPM2_CC_FIRST, TPM2_MAX_CAP_CC, [](const TPMS_CAPABILITY_DATA* d){ return d->data.command.count; } },
        { "pcrs", TPM2_CAP_PCRS, 0, TPM2_MAX_TPM_PROPERTIES, [](const auto d){ return d->data.assignedPCR.count; } },
        { "properties-fixed", TPM2_CAP_TPM_PROPERTIES, TPM2_PT_FIXED, TPM2_MAX_TPM_PROPERTIES, [](const auto d){ return d->data.tpmProperties.count; } },
        { "properties-variable", TPM2_CAP_TPM_PROPERTIES, TPM2_PT_VAR, TPM2_MAX_TPM_PROPERTIES, [](const auto d){ return d->data.tpmProperties.count; } },
        { "ecc-curves", TPM2_CAP_ECC_CURVES, TPM2_ECC_NIST_P192, TPM2_MAX_ECC_CURVES, [](const auto d){ return d->data.eccCurves.count; } },
        { "handles-transient", TPM2_CAP_HANDLES, TPM2_TRANSIENT_FIRST, TPM2_MAX_CAP_HANDLES, [](const auto d){ return d->data.handles.count; } },
        { "handles-persistent", TPM2_CAP_HANDLES, TPM2_PERSISTENT_FIRST, TPM2_MAX_CAP_HANDLES, [](const auto d){ return d->data.handles.count; } },
        { "handles-permanent", TPM2_CAP_HANDLES, TPM2_PERMANENT_FIRST, TPM2_MAX_CAP_HANDLES, [](const auto d){ return d->data.handles.count; } },
        { "handles-pcr", TPM2_CAP_HANDLES, TPM2_PCR_FIRST, TPM2_MAX_CAP_HANDLES, [](const auto d){ return d->data.handles.count; } },
        { "handles-nv-index", TPM2_CAP_HANDLES, TPM2_NV_INDEX_FIRST, TPM2_MAX_CAP_HANDLES, [](const auto d){ return d->data.handles.count; } },
        { "handles-loaded-session", TPM2_CAP_HANDLES, TPM2_LOADED_SESSION_FIRST, TPM2_MAX_CAP_HANDLES, [](const auto d){ return d->data.handles.count; } },
        { "handles-saved-session", TPM2_CAP_HANDLES, TPM2_ACTIVE_SESSION_FIRST, TPM2_MAX_CAP_HANDLES, [](const auto d){ return d->data.handles.count; } },
    };
    for (const auto& option : capability_options) {
        [[maybe_unused]] TPMS_CAPABILITY_DATA* data = nullptr;
        UINT32 property_count = 0;
        TPMI_YES_NO has_more_data = TPM2_NO;
        do {
            std::cout << "capability: " << option.name << std::endl;
            TPMS_CAPABILITY_DATA* fetched = nullptr;
            TSS2_RC r = Esys_GetCapability(const_cast<ESYS_CONTEXT*>(context), ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE, option.capability, option.property, option.count - property_count, &has_more_data, &fetched);
            THROW_IF_ERR(r, "Esys_GetCapability");

            if (! fetched)
                break;
            if (fetched->capability != option.capability)
                std::cout << "  capability unmatched" << std::endl;

            std::cout << "    algorithms:" << fetched->data.algorithms.count << std::endl;
            std::cout << "    handles:" << fetched->data.handles.count << std::endl;
            std::cout << "    command:" << fetched->data.command.count << std::endl;
            std::cout << "    ppCommands:" << fetched->data.ppCommands.count << std::endl;
            std::cout << "    auditCommands:" << fetched->data.auditCommands.count << std::endl;
            std::cout << "    assignedPCR:" << fetched->data.assignedPCR.count << std::endl;
            std::cout << "    tpmProperties:" << fetched->data.tpmProperties.count << std::endl;
            std::cout << "    pcrProperties:" << fetched->data.pcrProperties.count << std::endl;
            std::cout << "    eccCurves:" << fetched->data.eccCurves.count << std::endl;
            std::cout << "    authPolicies:" << fetched->data.authPolicies.count << std::endl;
            std::cout << "    actData:" << fetched->data.actData.count << std::endl;
            std::cout << "    intelPttProperty:" << fetched->data.intelPttProperty.count << std::endl;

            UINT32 fetched_count = option.fetch_count(fetched);
            if (fetched_count == 0) {
                std::cout << "  target field unknown" << std::endl;
                break;
            }

            property_count += fetched_count;
        } while (has_more_data);
    }
}

/**
 *
 */
void print_tcti_info(const char* device)
{
    TSS2_TCTI_INFO* info = nullptr;
    TSS2_RC r = Tss2_TctiLdr_GetInfo(device, &info);
    THROW_IF_ERR(r, "Tss2_TctiLdr_GetInfo");

    if (info) {
        std::cout << "TCTI Information: " << std::endl;
        std::cout << "  version: " << info->version << std::endl;
        std::cout << "  name: " << info->name << std::endl;
        std::cout << "  description: " << info->description << std::endl;
        std::cout << "  config_help: " << info->config_help << std::endl;
        std::cout << "  has initialize func: " << std::boolalpha << info->init << std::endl;

        Tss2_TctiLdr_FreeInfo(&info);
    }
}

/**
 *
 */
int main(int argc, char** argv)
{
    TSS2_RC r = TSS2_RC_SUCCESS;

    try {
        const char* device = "device:/dev/tpmrm0";

        // TCTI Information
        print_tcti_info(device);

        // TCTI initialize
        TSS2_TCTI_CONTEXT* tcti = nullptr;
        r = Tss2_TctiLdr_Initialize(device, &tcti);
        THROW_IF_ERR(r, "Tss2_TctiLdr_Initialize");

        // ESYS Initialize
        ESYS_CONTEXT* context = nullptr;
        r = Esys_Initialize(&context, tcti, nullptr);
        THROW_IF_ERR(r, "Esys_Initialize");

        // ESYS Capability
        //print_esys_capabilities(context);

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

