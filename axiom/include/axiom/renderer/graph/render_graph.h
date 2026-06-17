#pragma once

namespace axiom {
    class RenderGraph {
    public:

        template<typename TPass>
        void add_pass();

        void compile();

        void execute();
    };
}