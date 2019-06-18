
#include <lvkSampleApps/lvkDemoUtils.h>

class lvkInstanceApp : public IVKSampleApp
{
private:
    lvkDemoFramework DemoFramework;
    
public:
    
    virtual void Loop() override
    {}
    
    lvkInstanceApp(IVKWindow &window)
        :DemoFramework(window)
    {}
    
    ~lvkInstanceApp()
    {}
};

const bool IsResizable = false;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window)
{
    return std::unique_ptr<IVKSampleApp>(new lvkInstanceApp(window));
}
