#pragma once
#include <xpf/windows/Window.h>
#include <xpf/ui/UIElement.h>
#include <xpf/ui/GridPanel.h>
#include <xpf/ui/StackPanel.h>
#include <xpf/ui/Button.h>
#include <xpf/ui/Border.h>
#include <xpf/core/Rectangle.h>

class MainWindow : public xpf::Window
{
protected:
    xpf::GridPanel m_root;
    std::shared_ptr<xpf::UIElement> m_spToolbarPanel;
    std::shared_ptr<xpf::StackPanel> m_spControlsPanel;
    std::shared_ptr<xpf::Border> m_spDemoPanel;
    bool m_captureScreen = false;

public:
    MainWindow(xpf::WindowOptions&& options) : xpf::Window(std::move(options))
    { }

    virtual bool OnSetup(xpf::IRenderer& r) override;
    void OnDraw(xpf::IRenderer& r) override;
    virtual void OnResize(int32_t width, int32_t height) override;
    virtual void Finalize() override { }

private:
    std::shared_ptr<xpf::UIElement> CreateToolBar(xpf::IRenderer& r);
    std::shared_ptr<xpf::StackPanel> CreateListOfControls();
    void LoadControls(xpf::IRenderer& r);
    std::shared_ptr<xpf::Button> CreateResetButton(xpf::IRenderer& r);
    std::shared_ptr<xpf::Button> CreateThemeSwitchButton();
    std::shared_ptr<xpf::Button> CreateScreenCaptureButton();
    std::shared_ptr<xpf::UIElement>  GetTestListItem(const std::shared_ptr<xpf::UIElement>& spTest);
    void OnScreenCapture(xpf::IRenderer& r, xpf::Image&& image);
};