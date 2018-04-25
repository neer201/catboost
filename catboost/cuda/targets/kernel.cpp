#include "kernel.h"

using namespace NKernelHost;

namespace NCudaLib {
    REGISTER_KERNEL(0xA112200, TCrossEntropyTargetKernel);
    REGISTER_KERNEL(0xA112201, TMseTargetKernel);
    REGISTER_KERNEL(0xA112202, TPointwiseTargetImplKernel);
    REGISTER_KERNEL(0xA112203, TQueryRmseKernel);
    REGISTER_KERNEL(0xA112204, TYetiRankKernel);
    REGISTER_KERNEL(0xA112205, TQuerySoftMaxKernel);
    REGISTER_KERNEL(0xA112206, TMakeFinalTargetKernel);
    REGISTER_KERNEL(0xA112207, TPFoundFGradientKernel);
    REGISTER_KERNEL(0xA112208, TComputeMatrixSizesKernel);
    REGISTER_KERNEL(0xA112209, TMakePairsKernel);
    REGISTER_KERNEL(0xA112210, TPairLogitPairwiseKernel);
}
