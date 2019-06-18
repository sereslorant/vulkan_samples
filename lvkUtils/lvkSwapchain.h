#ifndef LVK_SWAPCHAIN_H
#define LVK_SWAPCHAIN_H

#include <vulkan/vulkan.h>

#include <memory>

//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <iostream>
//TMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "lvkSmartPtrs/lvkSwapchainKHR.h"
#include "lvkDefaultCreateInfos.h"

#include <limits>



#include <list>

struct lvkSwapchain
{
public:
    class liObserver
    {
    public:
        virtual void Destroyed() = 0;
        virtual void Recreated() = 0;
        
        virtual ~liObserver()
        {}
    };
    
private:
    VkDevice Device;
    VkPhysicalDevice PhysDevice;
    VkSwapchainCreateInfoKHR CreateInfo = LVK_DEFAULT_SWAPCHAIN_CREATE_INFO;

    lvkSwapchainKHR Swapchain;
    std::vector<VkImage> SwapchainImages;

    std::list<liObserver *> Observers;

    void GetSwapchainImages()
    {
        uint32_t SwapchainImageCount;
        vkGetSwapchainImagesKHR(Device,Swapchain.get(),&SwapchainImageCount,nullptr);
        
        SwapchainImages.resize(SwapchainImageCount);
        vkGetSwapchainImagesKHR(Device,Swapchain.get(),&SwapchainImageCount,SwapchainImages.data());
    }
    
public:

    void AddObserver(liObserver *observer)
    {
        Observers.push_back(observer);
    }

    void RemoveObserver(liObserver *observer)
    {
        Observers.remove(observer);
    }

    VkFormat GetFormat() const
    {
        return CreateInfo.imageFormat;
    }

    const VkExtent2D &GetExtent() const
    {
        return CreateInfo.imageExtent;
    }

    VkSwapchainKHR GetSwapchain()
    {
        return Swapchain.get();
    }

    unsigned int GetImageCount() const
    {
        return SwapchainImages.size();
    }

    VkImage GetImage(std::int32_t index)
    {
        return SwapchainImages[index];
    }

    void Recreate()
    {
        VkSurfaceCapabilitiesKHR SurfaceCapabilities = {};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysDevice,CreateInfo.surface,&SurfaceCapabilities);
        
        VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        
        uint32_t PresentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDevice,CreateInfo.surface,&PresentModeCount,nullptr);

        std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDevice,CreateInfo.surface,&PresentModeCount,PresentModes.data());
        
        for(auto &NewPresentMode : PresentModes)
        {
            if(NewPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                PresentMode = NewPresentMode;
            }
        }
        
        /*
        for(auto &NewPresentMode : PresentModes)
        {
            if(NewPresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                PresentMode = NewPresentMode;
            }
        }
        */
        VkSurfaceFormatKHR SurfaceFormat = {};
        {
            VkSurfaceFormatKHR PreferredFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
            
            uint32_t FormatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDevice,CreateInfo.surface,&FormatCount,nullptr);

            std::vector<VkSurfaceFormatKHR> SurfaceFormats(FormatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDevice,CreateInfo.surface,&FormatCount,SurfaceFormats.data());
            
            SurfaceFormat = SurfaceFormats[0];
            
            if(SurfaceFormats.size() == 1 && SurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
            {
                SurfaceFormat = PreferredFormat;
            }
            else
            {
                for(auto &Format : SurfaceFormats)
                {
                    //std::cout << "Format: " << (Format.format == VK_FORMAT_B8G8R8A8_UNORM) << ";" << Format.format << std::endl;
                    //std::cout << "ColorSpace: " << (Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) << std::endl;
                    
                    if(Format.format == PreferredFormat.format && Format.colorSpace == PreferredFormat.colorSpace)
                    {
                        SurfaceFormat = Format;
                    }
                }
            }
        }
        
        CreateInfo.minImageCount = SurfaceCapabilities.minImageCount;
        CreateInfo.preTransform  = SurfaceCapabilities.currentTransform;
        
        CreateInfo.presentMode   = PresentMode;
        
        CreateInfo.imageFormat      = SurfaceFormat.format;
        CreateInfo.imageColorSpace  = SurfaceFormat.colorSpace;
        
        CreateInfo.imageExtent  = SurfaceCapabilities.currentExtent;
        CreateInfo.oldSwapchain = Swapchain.get();
        
        for(liObserver *Observer : Observers)
            {Observer->Destroyed();}
        
        {
            std::cout << "Creating swapchain" << std::endl;
            VkSwapchainKHR NewSwapchain;
            if(vkCreateSwapchainKHR(Device,&CreateInfo,nullptr,&NewSwapchain) != VK_SUCCESS)
            {
                std::cout << "Error while creating swapchain!" << std::endl;
            }
            
            Swapchain = lvkSwapchainKHR{NewSwapchain,{Device}};
        }
        
        GetSwapchainImages();
        
        for(liObserver *Observer : Observers)
            {Observer->Recreated();}
    }

    lvkSwapchain(VkDevice device,VkPhysicalDevice phys_device,VkSurfaceKHR surface)
        :Device(device),PhysDevice(phys_device)
    {
        CreateInfo.surface = surface;
        
        Recreate();
    }

    ~lvkSwapchain()
    {}
};

#endif // LVK_SWAPCHAIN_H
