#ifndef LVK_SHADER_MODULE_LIBRARY_H
#define LVK_SHADER_MODULE_LIBRARY_H

#include <lvkUtils/lvkSmartPtrs/lvkShaderModule.h>

#include <map>

#include <lFileUtils.h>

class lvkShaderModuleLibrary
{
private:
    VkDevice Device;
    std::map<std::string,lvkShaderModule> ShaderModules;
    
public:
    
    VkShaderModule GetShaderModule(const std::string &filename)
    {
        auto I = ShaderModules.find(filename);
        if(I == ShaderModules.end())
        {
            std::vector<char> ShaderBytecode = lLoadBinaryFile(filename);
            if(ShaderBytecode.size() == 0)
            {
                return nullptr;
            }
            
            VkShaderModuleCreateInfo CreateInfo = {};
            CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

            CreateInfo.codeSize = ShaderBytecode.size();
            CreateInfo.pCode = reinterpret_cast<const uint32_t *>(ShaderBytecode.data());
            
            VkShaderModule ShaderModule;
            std::cout << "Creating shader module" << std::endl;
            if(vkCreateShaderModule(Device,&CreateInfo,nullptr,&ShaderModule) != VK_SUCCESS)
            {
                std::cout << "Shader creation error" << std::endl;
                return VK_NULL_HANDLE;
            }
            
            ShaderModules[filename] = lvkShaderModule{ShaderModule,{Device}};
            return ShaderModule;
        }
        else
        {
            return I->second.get();
        }
    }
    
    lvkShaderModuleLibrary(VkDevice device = VK_NULL_HANDLE)
        :Device(device)
    {}
};

#endif // LVK_SHADER_MODULE_LIBRARY_H
