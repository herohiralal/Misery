#include <__init.h>
#include <MeshOpt.h>

#if MSR_BUILD_DEPS
MSR_SUPPRESS_WARN
#include <ExtDeps/vertexfilter.cpp>
#include <ExtDeps/meshletutils.cpp>
#include <ExtDeps/vertexcodec.cpp>
#include <ExtDeps/vcacheoptimizer.cpp>
#include <ExtDeps/overdrawoptimizer.cpp>
#include <ExtDeps/stripifier.cpp>
#include <ExtDeps/opacitymap.cpp>
#include <ExtDeps/tangentspace.cpp>
#include <ExtDeps/quantization.cpp>
#include <ExtDeps/indexcodec.cpp>
#include <ExtDeps/indexanalyzer.cpp>
#include <ExtDeps/rasterizer.cpp>
#include <ExtDeps/partition.cpp>
#include <ExtDeps/vfetchoptimizer.cpp>
#include <ExtDeps/allocator.cpp>
#include <ExtDeps/spatialorder.cpp>
#include <ExtDeps/meshletcodec.cpp>
#include <ExtDeps/indexgenerator.cpp>
#include <ExtDeps/clusterizer.cpp>
#include <ExtDeps/simplifier.cpp>
MSR_UNSUPPRESS_WARN
#endif
