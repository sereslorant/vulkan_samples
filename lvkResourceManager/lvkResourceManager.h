#ifndef LVK_RESOURCE_MANAGER_H
#define LVK_RESOURCE_MANAGER_H

#include <lvkRenderer/lvkRenderer.h>

struct lvkDemoSingleQueueTransferGraph
{
    VkDevice Device;
    std::unique_ptr<lvkGpuCommandSubmitTask> TransferSubmitTask;
    lvkFence       TransferFence;
    
    void Submit(std::uint32_t buffer_count,VkCommandBuffer command_buffers[])
    {
        std::array<VkFence,1> Fences = {TransferFence.get()};
        vkResetFences(Device,1,Fences.data());
        TransferSubmitTask->Submit(buffer_count,command_buffers,TransferFence.get());
    }
    
    void Initialize(VkDevice device,VkQueue graphics_queue)
    {
        Device = device;
        
        TransferFence = lvkGpuSubmission::CreateFence(device);
        
        TransferSubmitTask = std::unique_ptr<lvkGpuCommandSubmitTask>(
            new lvkGpuCommandSubmitTask(graphics_queue)
        );
    }
};

#include <lvkResourceManager/lvkLoadingThreadData.h>

#include <thread>

class lvkResourceManager
{
private:
    lvkLoadingThreadData LoadingThreadData;
    
    bool BgThreadRunning = true;
    std::thread BgThread;
    
    bool TaskGraphBusy = false;
    lvkLoadingThreadData::lvkRecordedResources RecordedBuffers;
    
    VkDevice Device;
    lvkDemoSingleQueueTransferGraph TransferTaskGraph;
    
public:
    
    void AddTask(
        const std::vector<std::pair<liMeshGroupSrc*,lvkMeshGroupData *> > &mesh_groups,
        const std::vector<std::pair<liImageGroupSrc*,lvkImageGroupData *> > &image_groups
        )
    {
        LoadingThreadData.AddTask(mesh_groups,image_groups);
    }
    
    void Run()
    {
        if(!TaskGraphBusy)
        {
            RecordedBuffers = LoadingThreadData.TakeFinishedCmdsAndRotate();
            
            auto CommandBuffers = RecordedBuffers.GetCommandBuffers();
            if(CommandBuffers.size() != 0)
            {
                TransferTaskGraph.Submit(CommandBuffers.size(),CommandBuffers.data());
                TaskGraphBusy = true;
            }
        }
        
        if(vkGetFenceStatus(Device,TransferTaskGraph.TransferFence.get()) == VK_SUCCESS)
        {
            TaskGraphBusy = false;
            RecordedBuffers = {};
        }
    }
    
    lvkResourceManager(VkDevice device,VkQueue transfer_queue,std::uint32_t transfer_queue_family,
        
        lvkBufferPoolCategory        &vertex_buffer_category,
        lvkBufferPoolCategory        &index_buffer_category,
        lvkImagePoolCategory         &image_pool_category,
        lvkPMappedBufferPoolCategory &staging_buffer_category
    )
        :LoadingThreadData(device,transfer_queue_family,
            
            vertex_buffer_category,
            index_buffer_category,
            image_pool_category,
            staging_buffer_category
        ),
        Device(device)
    {
        TransferTaskGraph.Initialize(device,transfer_queue);
        
        BgThread = std::thread(
            [this]()
            {
                while(BgThreadRunning)
                {
                    LoadingThreadData.Execute();
                }
            }
        );
    }
    
    ~lvkResourceManager()
    {
        BgThreadRunning = false;
        BgThread.join();
    }
};

#endif // LVK_RESOURCE_MANAGER_H
