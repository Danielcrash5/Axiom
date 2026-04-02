#include "axiom/renderer/SlangCompiler.h"

#include <slang.h>
#include <slang-com-ptr.h>
#include <vector>
#include <stdexcept>

using namespace Slang;

static ShaderStage GetStage(const std::string& name) {
    if (name == "vsMain") return ShaderStage::Vertex;
    if (name == "psMain") return ShaderStage::Fragment;
    if (name == "gsMain") return ShaderStage::Geometry;
    if (name == "csMain") return ShaderStage::Compute;
    if (name == "hsMain") return ShaderStage::TessControl;
    if (name == "dsMain") return ShaderStage::TessEvaluation;

    throw std::runtime_error("Unknown entry point");
}

ShaderSource CompileSlang(const std::string& path) {
    ShaderSource result;

    ComPtr<IGlobalSession> globalSession;
    if (SLANG_FAILED(createGlobalSession(globalSession.writeRef())))
        throw std::runtime_error("Slang: failed to create global session");

    TargetDesc target = {};
    target.format = SLANG_GLSL;
    target.profile = globalSession->findProfile("glsl_460");

    SessionDesc desc = {};
    desc.targets = &target;
    desc.targetCount = 1;

    ComPtr<ISession> session;
    if (SLANG_FAILED(globalSession->createSession(desc, session.writeRef())))
        throw std::runtime_error("Slang: failed to create session");

    ComPtr<IModule> module;
    if (SLANG_FAILED(session->loadModule(path.c_str(), module.writeRef())))
        throw std::runtime_error("Slang: failed to load module");

    std::vector<const char*> entryPoints =
    {
        "vsMain",
        "psMain",
        "gsMain",
        "csMain",
        "hsMain",
        "dsMain"
    };

    for (auto name : entryPoints) {
        ComPtr<IEntryPoint> entry;
        module->findEntryPointByName(name, entry.writeRef());

        if (!entry)
            continue;

        IComponentType* components[] = { module, entry };

        ComPtr<IComponentType> program;
        if (SLANG_FAILED(session->createCompositeComponentType(
            components, 2, program.writeRef())))
            continue;

        ComPtr<IComponentType> linked;
        if (SLANG_FAILED(program->link(linked.writeRef())))
            continue;

        ComPtr<IBlob> code;
        if (SLANG_FAILED(linked->getEntryPointCode(0, 0, code.writeRef())))
            continue;

        std::string glsl(
            (const char*)code->getBufferPointer(),
            code->getBufferSize()
        );

        result.Sources[GetStage(name)] = glsl;
    }

    return result;
}