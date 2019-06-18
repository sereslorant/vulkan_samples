#ifndef IVK_SAMPLE_APP_H
#define IVK_SAMPLE_APP_H

#include <vector>
#include <lvkUtils/lvkSmartPtrs/lvkSurfaceKHR.h>

class IVKSampleApp
{
public:
    virtual void Loop() = 0;
    
    virtual ~IVKSampleApp()
    {}
};

class IVKWindow
{
public:
    virtual std::vector<const char *> GetVulkanExtensions() = 0;
    virtual lvkSurfaceKHR CreateSurface(VkInstance instance) = 0;
    
    virtual ~IVKWindow()
    {}
};

#include <memory>

extern const bool IsResizable;

std::unique_ptr<IVKSampleApp> CreateSampleApp(IVKWindow &window);

#endif // IVK_SAMPLE_APP_H
