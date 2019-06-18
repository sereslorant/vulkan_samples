#ifndef LVK_APP_UTILS_H
#define LVK_APP_UTILS_H

#include <lvkUtils/lvkSmartPtrs/lvkInstance.h>
#include <lvkUtils/lvkSmartPtrs/lvkSurfaceKHR.h>
#include <lvkUtils/lvkSmartPtrs/lvkDevice.h>

#include <vector>

/*
 * Queue family utils
 */

class lvkQueueFamilies
{
private:
    std::vector<VkQueueFamilyProperties> &QueueFamilies;
    VkQueueFlags QueueFlags;
    
public:
    
    constexpr static VkQueueFlags GRAPHICS_QUEUE_FLAGS = VK_QUEUE_GRAPHICS_BIT;
    constexpr static VkQueueFlags COMPUTE_QUEUE_FLAGS  = VK_QUEUE_COMPUTE_BIT;
    constexpr static VkQueueFlags TRANSFER_QUEUE_FLAGS = VK_QUEUE_TRANSFER_BIT;
    constexpr static VkQueueFlags GENERAL_QUEUE_FLAGS  = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
    
    std::uint32_t GetFirst() const
    {
        return GetNext(0);
    }
    
    std::uint32_t GetNext(int start) const
    {
        for(int i=start;i < QueueFamilies.size();i++)
        {
            if(QueueFamilies[i].queueFlags & QueueFlags)
            {
                return i;
            }
        }
        
        return QueueFamilies.size();
    }
    
    lvkQueueFamilies(std::vector<VkQueueFamilyProperties> &queue_families,VkQueueFlags queue_flags)
        :QueueFamilies(queue_families),QueueFlags(queue_flags)
    {}
};

class lvkPresentQueueFamilies
{
private:
    VkPhysicalDevice PhysicalDevice;
    std::uint32_t QueueFamilyCount;
    
public:
    
    std::uint32_t GetFirst(VkSurfaceKHR surface)
    {
        return GetNext(surface,0);
    }

    std::uint32_t GetNext(VkSurfaceKHR surface,int start)
    {
        for(int i=start;i < QueueFamilyCount;i++)
        {
            VkBool32 Supported = 0;
            vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice,i,surface,&Supported);
            
            if(Supported)
            {
                return i;
            }
        }
        
        return QueueFamilyCount;
    }

    lvkPresentQueueFamilies(VkPhysicalDevice physical_device,std::uint32_t queue_family_count)
        :PhysicalDevice(physical_device),QueueFamilyCount(queue_family_count)
    {}
};

#include <lvkSampleApps/IVKSampleApp.h>

#include <lvkUtils/lvkSmartPtrs/lvkRenderPass.h>
#include <lvkUtils/lvkSmartPtrs/lvkPipelineLayout.h>

#include <lvkUtils/lvkSmartPtrs/lvkFence.h>
#include <lvkUtils/lvkSmartPtrs/lvkSemaphore.h>

#include <lvkUtils/lvkDefaultCreateInfos.h>

#include <lvkUtils/lvkCommandBufferUtils.h>

#include <vector>
#include <cstring>

#include <lvkRenderer/lvkRenderer.h>
#include <lvkResourceManager/lvkTransferRecorder.h>
#include <lvkResourceManager/lvkResourceManager.h>

struct lvkDemoFramework
{
    lvkInstance   Instance;
    lvkSurfaceKHR Surface;
    
    VkPhysicalDevice PhysicalDevice;
    uint32_t GraphicsQueueFamilyIndex = 0;
    uint32_t PresentQueueFamilyIndex  = 0;
    
    lvkDevice Device;
    VkQueue   GraphicsQueue = VK_NULL_HANDLE;
    VkQueue   PresentQueue  = VK_NULL_HANDLE;
    
    
    struct lvkChosenPhysicalDevice
    {
        VkPhysicalDevice PhysicalDevice   = VK_NULL_HANDLE;
        uint32_t GraphicsQueueFamilyIndex = 0;
        uint32_t PresentQueueFamilyIndex  = 0;
    };
    
    lvkChosenPhysicalDevice ChoosePhysicalDevice()
    {
        lvkChosenPhysicalDevice ChosenPhysicalDevice;
        
        std::cout << "CSEKKOLD LE, HOGY HOGYAN ÉRDEMES DEVICE-OT ÉS QUEUE-T VÁLASZTANI!" << std::endl;
        std::vector<VkPhysicalDevice> PhysicalDevices;
        {
            uint32_t PhysicalDeviceCount = 0;
            if(vkEnumeratePhysicalDevices(Instance.get(),&PhysicalDeviceCount,nullptr) == VK_SUCCESS)
            {
                PhysicalDevices.resize(PhysicalDeviceCount);
                vkEnumeratePhysicalDevices(Instance.get(),&PhysicalDeviceCount,PhysicalDevices.data());
            }
        }
        
        for(VkPhysicalDevice PhysicalDevice : PhysicalDevices)
        {
            std::vector<VkQueueFamilyProperties> QueueFamilies;
            {
                uint32_t QueueFamiliesCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice,&QueueFamiliesCount,nullptr);
                
                QueueFamilies.resize(QueueFamiliesCount);
                vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice,&QueueFamiliesCount,QueueFamilies.data());
            }
            
            lvkQueueFamilies GraphicsQueueFamilies(QueueFamilies,VK_QUEUE_GRAPHICS_BIT);
            int GraphicsQueueFamily = GraphicsQueueFamilies.GetFirst();
            
            lvkPresentQueueFamilies PresentQueueFamilies(PhysicalDevice,QueueFamilies.size());
            int PresentQueueFamily = PresentQueueFamilies.GetFirst(Surface.get());
            
            std::vector<VkExtensionProperties> PhysicalDeviceExtensions;
            {
                uint32_t DeviceExtensionCount = 0;
                vkEnumerateDeviceExtensionProperties(PhysicalDevice,nullptr,&DeviceExtensionCount,nullptr);
                
                PhysicalDeviceExtensions.resize(DeviceExtensionCount);
                vkEnumerateDeviceExtensionProperties(PhysicalDevice,nullptr,&DeviceExtensionCount,&PhysicalDeviceExtensions[0]);
            }
            
            bool HasSwapchainSupport = false;
            for(VkExtensionProperties ExtensionProperties : PhysicalDeviceExtensions)
            {
                if(std::strcmp(ExtensionProperties.extensionName,VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
                {
                    HasSwapchainSupport = true;
                    break;
                }
            }
            
            if(HasSwapchainSupport && GraphicsQueueFamily != QueueFamilies.size() && PresentQueueFamily != QueueFamilies.size())
            {
                ChosenPhysicalDevice.PhysicalDevice = PhysicalDevice;
                
                ChosenPhysicalDevice.GraphicsQueueFamilyIndex = GraphicsQueueFamily;
                ChosenPhysicalDevice.PresentQueueFamilyIndex  = PresentQueueFamily;
                break;
            }
        }
        
        return ChosenPhysicalDevice;
    }
    
    static lvkRenderPass CreateSimpleRenderPass(VkDevice device,VkFormat format,bool has_depth_buffer = false)
    {
        lvkRenderPassBuilder RenderPassBuilder(device);
        
        std::uint32_t ColorAttachmentId;
        {
            lvkAttachmentBuilder AttachmentBuilder = RenderPassBuilder.CreateAttachment();
            
            AttachmentBuilder.SetFormat(format);
            
            ColorAttachmentId = AttachmentBuilder.Build();
        }
        
        std::uint32_t DepthAttachmentId = 0;
        if(has_depth_buffer)
        {
            lvkAttachmentBuilder AttachmentBuilder = RenderPassBuilder.CreateAttachment();
            
            AttachmentBuilder.SetFormat(VK_FORMAT_D32_SFLOAT);
            
            DepthAttachmentId = AttachmentBuilder.Build();
        }
        
        std::uint32_t SubpassId;
        {
            lvkSubpassBuilder SubpassBuilder = RenderPassBuilder.CreateSubpass();
            
            SubpassBuilder.AddColorAttachment(ColorAttachmentId);
            
            if(has_depth_buffer)
            {
                SubpassBuilder.SetDepthAttachment(DepthAttachmentId);
            }
            
            SubpassBuilder.AddDependency(VK_SUBPASS_EXTERNAL);
            
            SubpassId = SubpassBuilder.Build();
        }
        
        return RenderPassBuilder.Build();
    }
    
    lvkPipelineLayout CreateEmptyPipelineLayout()
    {
        VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = LVK_EMPTY_PIPELINE_LAYOUT_CREATE_INFO;
            
        std::cout << "Creating pipeline layout" << std::endl;
        VkPipelineLayout NewPipelineLayout;
        if(vkCreatePipelineLayout(Device.get(),&PipelineLayoutCreateInfo,nullptr,&NewPipelineLayout) != VK_SUCCESS)
        {
            std::cout << "Error while creating pipeline layout" << std::endl;
        }
        
        return lvkPipelineLayout{NewPipelineLayout,{Device.get()}};
    }
    
    lvkDemoFramework(IVKWindow &window,bool allow_validation = true)
    {
        std::vector<const char *> Extensions = window.GetVulkanExtensions();
        std::vector<const char *> Layers     = {"VK_LAYER_LUNARG_standard_validation","VK_LAYER_LUNARG_parameter_validation"};
        
        if(!allow_validation)
            {Layers.clear();}
        
        VkApplicationInfo ApplicationInfo = {};
        ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        ApplicationInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo InstanceCreateInfo = {};
        InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;

        InstanceCreateInfo.enabledExtensionCount   = Extensions.size();
        InstanceCreateInfo.ppEnabledExtensionNames = Extensions.data();
        
        InstanceCreateInfo.enabledLayerCount   = Layers.size();
        InstanceCreateInfo.ppEnabledLayerNames = Layers.data();
        
        {
            std::cout << "Creating instance" << std::endl;
            VkInstance NewInstance;
            if(vkCreateInstance(&InstanceCreateInfo,nullptr,&NewInstance) != VK_SUCCESS)
            {
                std::cout << "Couldn't create Vulkan instance" << std::endl;
            }
            Instance = lvkInstance{NewInstance};
        }
        
        Surface = window.CreateSurface(Instance.get());
        
        lvkChosenPhysicalDevice ChosenPhysicalDevice = ChoosePhysicalDevice();
        
        if(ChosenPhysicalDevice.PhysicalDevice == VK_NULL_HANDLE)
        {
            Surface  = nullptr;
            Instance = nullptr;
            return;
        }
        
        PhysicalDevice = ChosenPhysicalDevice.PhysicalDevice;
        GraphicsQueueFamilyIndex = ChosenPhysicalDevice.GraphicsQueueFamilyIndex;
        PresentQueueFamilyIndex  = ChosenPhysicalDevice.PresentQueueFamilyIndex;
        
        VkDeviceCreateInfo DeviceCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0x0,
            .queueCreateInfoCount = 0,
            .pQueueCreateInfos = nullptr,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = 0,
            .ppEnabledExtensionNames = nullptr,
            .pEnabledFeatures = nullptr,
        };
        
        const float QueuePriority[1] = {1.0f};
        std::array<VkDeviceQueueCreateInfo, 2> DeviceQueueCreateInfo = {};
        DeviceQueueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        DeviceQueueCreateInfo[0].queueFamilyIndex = ChosenPhysicalDevice.GraphicsQueueFamilyIndex;
        DeviceQueueCreateInfo[0].queueCount = 1;
        DeviceQueueCreateInfo[0].pQueuePriorities = QueuePriority;
        
        DeviceQueueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        DeviceQueueCreateInfo[1].queueFamilyIndex = ChosenPhysicalDevice.PresentQueueFamilyIndex;
        DeviceQueueCreateInfo[1].queueCount = 1;
        DeviceQueueCreateInfo[1].pQueuePriorities = QueuePriority;
        
        if(ChosenPhysicalDevice.GraphicsQueueFamilyIndex == ChosenPhysicalDevice.PresentQueueFamilyIndex)
            {DeviceCreateInfo.queueCreateInfoCount = 1;}
        else
            {DeviceCreateInfo.queueCreateInfoCount = 2;}
        
        DeviceCreateInfo.pQueueCreateInfos    = DeviceQueueCreateInfo.data();
        
        std::array<const char *, 1> DeviceExtensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,};
        
        DeviceCreateInfo.enabledExtensionCount   = DeviceExtensionNames.size();
        DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensionNames.data();
        {
            std::cout << "Creating device" << std::endl;
            VkDevice NewDevice;
            if(vkCreateDevice(ChosenPhysicalDevice.PhysicalDevice,&DeviceCreateInfo,nullptr,&NewDevice) != VK_SUCCESS)
            {
                std::cout << "Couldn't create device" << std::endl;
                NewDevice = VK_NULL_HANDLE;
            }
            Device = lvkDevice{NewDevice};
            vkGetDeviceQueue(Device.get(),ChosenPhysicalDevice.GraphicsQueueFamilyIndex,0,&GraphicsQueue);
            vkGetDeviceQueue(Device.get(),ChosenPhysicalDevice.PresentQueueFamilyIndex,0,&PresentQueue);
        }
    }
    
    ~lvkDemoFramework()
    {}
};

#include <lvkUtils/lvkSwapchain.h>

struct lvkDemoTaskGraph
{
    enum lvkDemoSemaphoreId
    {
        LVK_DEMO_IMG_AVAILABLE      = 0,
        LVK_DEMO_RENDERING_FINISHED = 1,
    };
    lvkSemaphore Semaphores[2];
    lvkFence FrameFence;
    
    std::unique_ptr<lvkGpuCommandSubmitTask> GraphicsSubmitTask;
    std::unique_ptr<lvkGpuPresentSubmitTask> PresentSubmitTask;
    
    void Submit(VkDevice device,lvkSwapchain &swapchain,VkCommandBuffer *command_buffers)
    {
        std::array<VkFence,1> Fences = {FrameFence.get()};
        vkWaitForFences(device,1,Fences.data(),VK_TRUE,std::numeric_limits<uint64_t>::max());
        vkResetFences(device,1,Fences.data());
        
        std::uint32_t NextImageIndex;
        
        VkResult Result = vkAcquireNextImageKHR(device,swapchain.GetSwapchain(),std::numeric_limits<uint64_t>::max(),Semaphores[LVK_DEMO_IMG_AVAILABLE].get(),VK_NULL_HANDLE,&NextImageIndex);
        
        if(Result == VK_SUBOPTIMAL_KHR || Result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            swapchain.Recreate();
            
            vkAcquireNextImageKHR(device,swapchain.GetSwapchain(),std::numeric_limits<uint64_t>::max(),Semaphores[LVK_DEMO_IMG_AVAILABLE].get(),VK_NULL_HANDLE,&NextImageIndex);
        }
        
        GraphicsSubmitTask->Submit(1,&command_buffers[NextImageIndex],FrameFence.get());
        
        VkSwapchainKHR Swapchains[] = {swapchain.GetSwapchain()};
        std::uint32_t  ImgIndices[] = {NextImageIndex};
        PresentSubmitTask->Submit(1,Swapchains,ImgIndices);
    }
    
    void Initialize(VkDevice device,VkQueue graphics_queue,VkQueue present_queue)
    {
        Semaphores[LVK_DEMO_IMG_AVAILABLE] = lvkSemaphore{lvkGpuSubmission::CreateSemaphore(device),{device}};
        Semaphores[LVK_DEMO_RENDERING_FINISHED] = lvkSemaphore{lvkGpuSubmission::CreateSemaphore(device),{device}};
        
        GraphicsSubmitTask = std::unique_ptr<lvkGpuCommandSubmitTask>(new lvkGpuCommandSubmitTask(graphics_queue,1,1));
        PresentSubmitTask  = std::unique_ptr<lvkGpuPresentSubmitTask>(new lvkGpuPresentSubmitTask(present_queue,1));
        
        GraphicsSubmitTask->AddDependency(Semaphores[LVK_DEMO_IMG_AVAILABLE].get(),VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        GraphicsSubmitTask->AddSignalSemaphore(Semaphores[LVK_DEMO_RENDERING_FINISHED].get());
        
        PresentSubmitTask->AddDependency(Semaphores[LVK_DEMO_RENDERING_FINISHED].get());
        
        FrameFence = lvkGpuSubmission::CreateFence(device,VK_FENCE_CREATE_SIGNALED_BIT);
    }
    
    ~lvkDemoTaskGraph()
    {}
};

#include <lvkUtils/lvkSmartPtrs/lvkImageView.h>
#include <lvkUtils/lvkSmartPtrs/lvkFramebuffer.h>

class lvkDemoSwapchainFramebuffers : public lvkSwapchain::liObserver
{
private:
    VkDevice     Device;
    VkRenderPass RenderPass;
    
    lvkSwapchain &Swapchain;
    
    std::vector<lvkImageView>   ImageViews;
    std::vector<lvkFramebuffer> Framebuffers;
    
    bool HasDepthBuffer;
    std::unique_ptr<lvkMemoryAllocProcElement> DepthAllocator;
    std::vector<lvkImage>            DepthBuffers;
    std::vector<lvkMemoryAllocation> DepthMemory;
    std::vector<lvkImageView>        DepthViews;
    
public:
    
    VkFramebuffer GetFramebuffer(std::uint32_t index)
    {
        return Framebuffers[index].get();
    }
    
    virtual void Destroyed() override
    {
        for(int i=0;i < ImageViews.size();i++)
        {
            ImageViews[i] = nullptr;
        }
        for(int i=0;i < Framebuffers.size();i++)
        {
            Framebuffers[i] = nullptr;
        }
        
        for(int i=0;i < DepthBuffers.size();i++)
        {
            DepthBuffers[i] = nullptr;
        }
        for(int i=0;i < DepthMemory.size();i++)
        {
            DepthMemory[i] = {};
        }
        for(int i=0;i < DepthViews.size();i++)
        {
            DepthViews[i] = nullptr;
        }
    }
    
    virtual void Recreated() override
    {
        for(int i=0;i < Swapchain.GetImageCount();i++)
        {
            {
                VkImageViewCreateInfo ImageViewCreateInfo = LVK_2D_IMG_VIEW_CREATE_INFO;
                ImageViewCreateInfo.format = Swapchain.GetFormat();
                ImageViewCreateInfo.image  = Swapchain.GetImage(i);
                
                std::cout << "Creating image view" << std::endl;
                VkImageView NewImageView;
                if(vkCreateImageView(Device,&ImageViewCreateInfo,nullptr,&NewImageView) != VK_SUCCESS)
                {
                    std::cout << "Couldn't create image view " << std::endl;
                }
                
                ImageViews[i] = lvkImageView{NewImageView,{Device}};
            }
            
            if(HasDepthBuffer)
            {
                VkImageCreateInfo ImageCreateInfo = LVK_EMPTY_2D_IMAGE_CREATE_INFO;
                ImageCreateInfo.extent = {
                    .width  = Swapchain.GetExtent().width,
                    .height = Swapchain.GetExtent().height,
                    .depth  = 1
                };
                ImageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
                ImageCreateInfo.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                
                std::cout << "Creating image view" << std::endl;
                VkImage NewImage;
                if(vkCreateImage(Device,&ImageCreateInfo,nullptr,&NewImage) != VK_SUCCESS)
                {
                    std::cout << "Couldn't create depth image " << std::endl;
                }
                
                VkMemoryRequirements MemoryRequirements;
                vkGetImageMemoryRequirements(Device,NewImage,&MemoryRequirements);
                
                DepthMemory[i] = DepthAllocator->RequestAllocation(MemoryRequirements.size,MemoryRequirements.memoryTypeBits);
                
                vkBindImageMemory(Device,NewImage,DepthMemory[i].GetMemory(),0);
                
                DepthBuffers[i] = lvkImage{NewImage,{Device}};
                
                VkImageViewCreateInfo ImageViewCreateInfo = LVK_2D_IMG_VIEW_CREATE_INFO;
                ImageViewCreateInfo.format = VK_FORMAT_D32_SFLOAT;
                ImageViewCreateInfo.image  = DepthBuffers[i].get();
                ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                
                std::cout << "Creating image view" << std::endl;
                VkImageView NewImageView;
                if(vkCreateImageView(Device,&ImageViewCreateInfo,nullptr,&NewImageView) != VK_SUCCESS)
                {
                    std::cout << "Couldn't create depth image view " << std::endl;
                }
                
                DepthViews[i] = lvkImageView{NewImageView,{Device}};
            }
            
            {
                std::size_t AttachmentCount = 1;
                std::array<VkImageView,2> Attachments = {ImageViews[i].get()};
                VkFramebufferCreateInfo   FramebufferCreateInfo  = LVK_EMPTY_FRAMEBUFFER_CREATE_INFO;
                
                if(HasDepthBuffer)
                {
                    Attachments[1]  = DepthViews[i].get();
                    AttachmentCount = 2;
                }
                
                FramebufferCreateInfo.pAttachments    = Attachments.data();
                FramebufferCreateInfo.attachmentCount = AttachmentCount;
                
                FramebufferCreateInfo.width  = Swapchain.GetExtent().width;
                FramebufferCreateInfo.height = Swapchain.GetExtent().height;
                
                FramebufferCreateInfo.renderPass = RenderPass;
                
                std::cout << "Creating framebuffer" << std::endl;
                VkFramebuffer NewFramebuffer;
                if(vkCreateFramebuffer(Device,&FramebufferCreateInfo,nullptr,&NewFramebuffer) != VK_SUCCESS)
                {
                    std::cout << "Failed to create framebuffer! " << std::endl;
                }
                
                Framebuffers[i] = lvkFramebuffer{NewFramebuffer,{Device}};
            }
        }
    }
    
    lvkDemoSwapchainFramebuffers(VkDevice device,VkRenderPass render_pass,lvkSwapchain &swapchain,std::unique_ptr<lvkMemoryAllocProcElement> &&depth_allocator = {},bool has_depth_buffer = false)
        :Device(device),RenderPass(render_pass),Swapchain(swapchain),HasDepthBuffer(has_depth_buffer)
    {
        ImageViews.resize(Swapchain.GetImageCount());
        Framebuffers.resize(Swapchain.GetImageCount());
        
        if(HasDepthBuffer)
        {
            DepthAllocator = std::move(depth_allocator);
            DepthBuffers.resize(Swapchain.GetImageCount());
            DepthMemory.resize(Swapchain.GetImageCount());
            DepthViews.resize(Swapchain.GetImageCount());
        }
        
        Recreated();
        
        Swapchain.AddObserver(this);
    }
    
    ~lvkDemoSwapchainFramebuffers()
    {
        Swapchain.RemoveObserver(this);
    }
};

#include <lvkUtils/lvkDefaultCreateInfos.h>

struct lvkDemoColorBlendAttachments
{
    VkPipelineColorBlendAttachmentState ColorBlendAttachmentStateCreateInfo = LVK_DEFAULT_COLOR_BLEND_ATTACHMENT_STATE;
    VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo           = LVK_DEFAULT_COLOR_BLEND_CREATE_INFO;
    
    void AssignToPipelineCreateInfo(VkGraphicsPipelineCreateInfo &pipeline_create_info)
    {
        ColorBlendStateCreateInfo.attachmentCount = 1;
        ColorBlendStateCreateInfo.pAttachments = &ColorBlendAttachmentStateCreateInfo;
        
        pipeline_create_info.pColorBlendState = &ColorBlendStateCreateInfo;
    }
};

struct lvkDemoShaderStages
{
    std::array<VkPipelineShaderStageCreateInfo,2> ShaderStages =
    {
        LVK_VERTEX_SHADER_STAGE_CREATE_INFO,
        LVK_FRAGMENT_SHADER_STAGE_CREATE_INFO,
    };
    
    void SetShaders(VkShaderModule vertex_shader,VkShaderModule fragment_shader)
    {
        ShaderStages[0].module = vertex_shader;
        ShaderStages[0].pName = "main";
        
        ShaderStages[1].module = fragment_shader;
        ShaderStages[1].pName = "main";
    }
    
    void AssignToPipelineCreateInfo(VkGraphicsPipelineCreateInfo &pipeline_create_info)
    {
        pipeline_create_info.stageCount = ShaderStages.size();
        pipeline_create_info.pStages    = ShaderStages.data();
    }
};

class lvkDemoRecordSimpleRenderPassGuard
{
private:
    VkCommandBuffer CommandBuffer;
    
public:
    
    lvkDemoRecordSimpleRenderPassGuard(
        VkCommandBuffer command_buffer,
        VkRenderPass render_pass,
        VkFramebuffer framebuffer,
        const VkExtent2D &extent,
        bool has_depth_buffer = false,
        VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE
    )
        :CommandBuffer(command_buffer)
    {
        VkRenderPassBeginInfo RenderPassBeginInfo = {};
        RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        RenderPassBeginInfo.renderPass  = render_pass;
        RenderPassBeginInfo.framebuffer = framebuffer;
        
        RenderPassBeginInfo.renderArea.offset = {0, 0};
        RenderPassBeginInfo.renderArea.extent = extent;
        
        std::size_t  ClearColorCount = 1;
        VkClearValue ClearColors[2]  = {};
        ClearColors[0].color.float32[0] = 0.0f;
        ClearColors[0].color.float32[1] = 0.0f;
        ClearColors[0].color.float32[2] = 0.0f;
        ClearColors[0].color.float32[3] = 1.0f;
        if(has_depth_buffer)
        {
            ClearColorCount = 2;
            ClearColors[1].depthStencil = {1.0f, 0};
        }
        
        RenderPassBeginInfo.clearValueCount = ClearColorCount;
        RenderPassBeginInfo.pClearValues    = ClearColors;
        
        vkCmdBeginRenderPass(CommandBuffer,&RenderPassBeginInfo,subpass_contents);
    }
    
    ~lvkDemoRecordSimpleRenderPassGuard()
    {
        vkCmdEndRenderPass(CommandBuffer);
    }
};

#include <lvkUtils/lvkCommandPoolUtils.h>

class lvkDemoCommandPool : public lvkSwapchain::liObserver
{
private:
    VkDevice       Device;
    lvkCommandPool CommandPool;
    
    lvkSwapchain *Swapchain;
    
public:
    
    VkCommandBuffer Allocate()
    {
        lvkCommandPoolUtil CommandPoolUtil(Device,CommandPool.get());
        return CommandPoolUtil.Allocate();
    }
    
    std::vector<VkCommandBuffer> Allocate(std::uint32_t cmd_buffer_count)
    {
        lvkCommandPoolUtil CommandPoolUtil(Device,CommandPool.get());
        return CommandPoolUtil.Allocate(cmd_buffer_count);
    }
    
    virtual void Destroyed() override
    {
        lvkCommandPoolUtil CommandPoolUtil(Device,CommandPool.get());
        return CommandPoolUtil.Reset();
    }
    
    virtual void Recreated() override
        {}
    
    lvkDemoCommandPool(VkDevice device,std::uint32_t queue_family,VkCommandPoolCreateFlags flags = 0x0,lvkSwapchain *swapchain = nullptr)
        :Device(device),Swapchain(swapchain)
    {
        CommandPool = lvkCommandPoolUtil::CreateCommandPool(Device,queue_family,flags);
        
        if(Swapchain != nullptr)
            {Swapchain->AddObserver(this);}
    }
    
    ~lvkDemoCommandPool()
    {
        if(Swapchain != nullptr)
            {Swapchain->RemoveObserver(this);}
    }
};

#include <functional>

class lvkDemoBufferRecorder : public lvkSwapchain::liObserver
{
private:
    std::function<void ()> BufferRecorder;
    lvkSwapchain &Swapchain;
    
public:
    
    virtual void Destroyed() override
        {}
    
    virtual void Recreated() override
        {BufferRecorder();}
    
    lvkDemoBufferRecorder(std::function<void ()> buffer_recorder,lvkSwapchain &swapchain)
        :BufferRecorder(buffer_recorder),Swapchain(swapchain)
    {
        Recreated();
        Swapchain.AddObserver(this);
    }
    
    ~lvkDemoBufferRecorder()
    {
        Swapchain.RemoveObserver(this);
    }
};

#include <lvkUtils/lvkSmartPtrs/lvkPipeline.h>
#include <lvkUtils/lvkPipelineUtils.h>

#endif // LVK_APP_UTILS_H
