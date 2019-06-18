
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "IVKSampleApp.h"

#include <iostream>

class lSDL2_VulkanWindow : public IVKWindow
{
private:
    SDL_Window *Window;
    
public:
    
    virtual lvkSurfaceKHR CreateSurface(VkInstance instance) override
    {
        std::cout << "Creating surface" << std::endl;
        VkSurfaceKHR Surface = VK_NULL_HANDLE;
        SDL_Vulkan_CreateSurface(Window,instance,&Surface);
        
        return lvkSurfaceKHR{Surface,{instance}};
    }
    
    virtual std::vector<const char *> GetVulkanExtensions() override
    {
        unsigned int ExtensionCount = 0;
        SDL_Vulkan_GetInstanceExtensions(Window,&ExtensionCount,nullptr);
        
        std::vector<const char *> Extensions(ExtensionCount);
        SDL_Vulkan_GetInstanceExtensions(Window,&ExtensionCount,Extensions.data());
        
        return Extensions;
    }
    
    void SetResizable(bool resizable)
    {
        SDL_bool Resizable = SDL_FALSE;
        if(resizable)
            {Resizable = SDL_TRUE;}
        
        SDL_SetWindowResizable(Window,Resizable);
    }
    
    lSDL2_VulkanWindow(const std::string &window_title,unsigned int width,unsigned int height,bool resizable)
    {
        std::cout << "Creating window" << std::endl;
        Uint32 Flags = SDL_WINDOW_VULKAN;
        if(resizable)
            {Flags |= SDL_WINDOW_RESIZABLE;}
        Window = SDL_CreateWindow(window_title.c_str(),SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,width,height,Flags);
        SDL_ShowWindow(Window);
    }
    
    ~lSDL2_VulkanWindow()
    {
        std::cout << "Deleting window" << std::endl;
        SDL_DestroyWindow(Window);
    }
};

int main(int argc,char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Vulkan_LoadLibrary(nullptr);

    {
        std::unique_ptr<lSDL2_VulkanWindow> Window = std::unique_ptr<lSDL2_VulkanWindow>(new lSDL2_VulkanWindow("Renderer test",600,600,IsResizable));
        std::unique_ptr<IVKSampleApp> SampleApp = CreateSampleApp(*Window.get());
        
        bool IsRunning = true;
        while(IsRunning)
        {
            int BeginTime = SDL_GetTicks();
            
            SDL_Event Event;
            while(SDL_PollEvent(&Event))
            {
                if(Event.type == SDL_QUIT)
                {
                    IsRunning = false;
                }
            }
            
            SampleApp->Loop();
            
            int EndTime = SDL_GetTicks();
            //std::cout << "Frame time: " << (EndTime-BeginTime) << "ms\nFps: " << 1000.0/(EndTime-BeginTime) << std::endl;
        }
    }
    
    SDL_Vulkan_UnloadLibrary();
        SDL_Quit();

    return 0;
}
