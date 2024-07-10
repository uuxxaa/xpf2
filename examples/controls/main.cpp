#include <xpf/windows/Application.h>
#include <xpf/ui/ThemeEngine.h>
#include "MainWindow.h"

using namespace xpf;

class MyApp : public xpf::Application
{
public:
    MyApp() = default;

    virtual bool OnStartup(std::vector<xpf::string_viewex>&& args) override {
        xpf::stringex title = "Controls";
        RendererType rendererType = RendererType::OpenGL;
        if (args.size() == 1)
        {
            if (args[0].equals("metal", string_comparison::ordinal_ignoreCase))
            {
                rendererType = RendererType::Metal;
            }
            else if (args[0].equals("directx", string_comparison::ordinal_ignoreCase) ||
                     args[0].equals("dx", string_comparison::ordinal_ignoreCase))
            {
                rendererType = RendererType::DirectX;
            }
            else if (args[0].equals("null", string_comparison::ordinal_ignoreCase))
            {
                rendererType = RendererType::Null;
            }
        }

        switch (rendererType)
        {
            case RendererType::DirectX:  title += " [DirectX]"; break;
            case RendererType::Metal:  title += " [Metal]"; break;
            case RendererType::OpenGL:  title += " [OpenGl]"; break;
            case RendererType::Null:  title += " [Null]"; break;
        }

        ThemeEngine::Initialize();

        WindowOptions options;
        options.renderer = rendererType;
        options.title = title;
        options.width = 800;
        options.height = 480;
        options.is_projection_ortho2d = true;
        options.show_stats = 1;
        // options.metal_transactional_rendering = true;
        // options.enable_vsync = false;
        options.sample_count = 0;
        options.background_color = xpf::Colors::White;

        MainWindow window(std::move(options));
        window.Show();
        return true;
    }
};

int main(int argc, char* argv[])
{
   MyApp app;
   return app.Start(argc, argv);
}