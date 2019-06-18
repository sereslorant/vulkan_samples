#ifndef LVK_LOADING_THREAD_DATA_H
#define LVK_LOADING_THREAD_DATA_H

#include "lvkResourceGroupLoader.h"

/*
 * Resource loader tasks
 */

// Allocate

struct lvkResourceCategories
{
    lvkBufferPoolCategory        &VertexBufferCategory;
    lvkBufferPoolCategory        &IndexBufferCategory;
    lvkImagePoolCategory         &ImagePoolCategory;
    lvkPMappedBufferPoolCategory &StagingBufferCategory;
    
    lvkResourceCategories(
        lvkBufferPoolCategory        &vertex_buffer_category,
        lvkBufferPoolCategory        &index_buffer_category,
        lvkImagePoolCategory         &image_pool_category,
        lvkPMappedBufferPoolCategory &staging_buffer_category
    )
        :VertexBufferCategory(vertex_buffer_category),
         IndexBufferCategory(index_buffer_category),
         ImagePoolCategory(image_pool_category),
         StagingBufferCategory(staging_buffer_category)
    {}
};

struct lvkAllocateVertexBufferTask
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    std::size_t MeshGroupId;
    
    void Execute(lvkResourceCategories &resource_categories)
    {
        lvkMeshGroupLoader &MeshGroupLoader = ResourceTransferBatch.MeshGroupLoaders[MeshGroupId];
        
        resource_categories.VertexBufferCategory.CreateIfFailed(
            [&MeshGroupLoader](lvkBufferPool &vertex_buffer)
            {
                MeshGroupLoader.RequestVertexBuffer(vertex_buffer);
                
                return MeshGroupLoader.MeshGroupData.VertexBuffer != VK_NULL_HANDLE;
            }
        );
    }
    
    lvkAllocateVertexBufferTask(lvkResourceTransferBatch &resource_transfer_batch,std::size_t mesh_group_id)
        :ResourceTransferBatch(resource_transfer_batch),MeshGroupId(mesh_group_id)
    {}
};

struct lvkAllocateIndexBufferTask
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    std::size_t MeshGroupId;
    
    void Execute(lvkResourceCategories &resource_categories)
    {
        lvkMeshGroupLoader &MeshGroupLoader = ResourceTransferBatch.MeshGroupLoaders[MeshGroupId];
        
        resource_categories.IndexBufferCategory.CreateIfFailed(
            [&MeshGroupLoader](lvkBufferPool &index_buffer)
            {
                MeshGroupLoader.RequestIndexBuffer(index_buffer);
                
                return MeshGroupLoader.MeshGroupData.IndexBuffer != VK_NULL_HANDLE;
            }
        );
    }
    
    lvkAllocateIndexBufferTask(lvkResourceTransferBatch &resource_transfer_batch,std::size_t mesh_group_id)
        :ResourceTransferBatch(resource_transfer_batch),MeshGroupId(mesh_group_id)
    {}
};

struct lvkAllocateVertexStagingBufferTask
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    std::size_t MeshGroupId;
    
    void Execute(lvkResourceCategories &resource_categories)
    {
        lvkMeshGroupLoader &MeshGroupLoader = ResourceTransferBatch.MeshGroupLoaders[MeshGroupId];
        
        resource_categories.StagingBufferCategory.CreateIfFailed(
            [&MeshGroupLoader](lvkPMappedBufferPool &vertex_buffer)
            {
                MeshGroupLoader.RequestVertexStagingBuffer(vertex_buffer);
                
                return MeshGroupLoader.VertexStagingBuffer != nullptr;
            }
        );
    }
    
    lvkAllocateVertexStagingBufferTask(lvkResourceTransferBatch &resource_transfer_batch,std::size_t mesh_group_id)
        :ResourceTransferBatch(resource_transfer_batch),MeshGroupId(mesh_group_id)
    {}
};

struct lvkAllocateIndexStagingBufferTask
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    std::size_t MeshGroupId;
    
    void Execute(lvkResourceCategories &resource_categories)
    {
        lvkMeshGroupLoader &MeshGroupLoader = ResourceTransferBatch.MeshGroupLoaders[MeshGroupId];
        
        resource_categories.StagingBufferCategory.CreateIfFailed(
            [&MeshGroupLoader](lvkPMappedBufferPool &index_buffer)
            {
                MeshGroupLoader.RequestIndexStagingBuffer(index_buffer);
                
                return MeshGroupLoader.IndexStagingBuffer != nullptr;
            }
        );
    }
    
    lvkAllocateIndexStagingBufferTask(lvkResourceTransferBatch &resource_transfer_batch,std::size_t mesh_group_id)
        :ResourceTransferBatch(resource_transfer_batch),MeshGroupId(mesh_group_id)
    {}
};

struct lvkAllocateImageMemoryTask
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    std::size_t ImageGroupId;
    
    void Execute(lvkResourceCategories &resource_categories)
    {
        lvkImageGroupLoader &ImageGroupLoader = ResourceTransferBatch.ImageGroupLoaders[ImageGroupId];
        
        resource_categories.ImagePoolCategory.CreateIfFailed(
            [&ImageGroupLoader](lvkImagePool &image_pool)
            {
                ImageGroupLoader.RequestImageMemory(image_pool);
                
                return ImageGroupLoader.ImageGroupData.Image != nullptr;
            }
        );
    }
    
    lvkAllocateImageMemoryTask(lvkResourceTransferBatch &resource_transfer_batch,std::size_t image_group_id)
        :ResourceTransferBatch(resource_transfer_batch),ImageGroupId(image_group_id)
    {}
};

struct lvkAllocateRegionStagingBufferTask
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    std::size_t ImageGroupId;
    
    void Execute(lvkResourceCategories &resource_categories)
    {
        lvkImageGroupLoader &ImageGroupLoader = ResourceTransferBatch.ImageGroupLoaders[ImageGroupId];
        
        for(std::size_t region = 0;region < ImageGroupLoader.ImageGroupLoader.ImageRegions.size();region++)
        {
            resource_categories.StagingBufferCategory.CreateIfFailed(
                [&ImageGroupLoader,region](lvkPMappedBufferPool &staging_buffer)
                {
                    ImageGroupLoader.RequestStagingMemory(region,staging_buffer);
                    
                    return ImageGroupLoader.RegionStagingBuffers[region] != nullptr;
                }
            );
        }
    }
    
    lvkAllocateRegionStagingBufferTask(lvkResourceTransferBatch &resource_transfer_batch,std::size_t image_group_id)
        :ResourceTransferBatch(resource_transfer_batch),ImageGroupId(image_group_id)
    {}
};

// Upload

struct lvkUploadMeshGroupTask
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    std::size_t MeshGroupId;
    
    void Execute()
    {
        ResourceTransferBatch.MeshGroupLoaders[MeshGroupId].LoadVertexData();
        ResourceTransferBatch.MeshGroupLoaders[MeshGroupId].LoadIndexData();
    }
    
    lvkUploadMeshGroupTask(lvkResourceTransferBatch &resource_transfer_batch,std::size_t mesh_group_id)
        :ResourceTransferBatch(resource_transfer_batch),MeshGroupId(mesh_group_id)
    {}
};

struct lvkUploadImageGroupTask
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    std::size_t ImageGroupId;
    
    void Execute()
    {
        ResourceTransferBatch.ImageGroupLoaders[ImageGroupId].LoadRegion();
    }
    
    lvkUploadImageGroupTask(lvkResourceTransferBatch &resource_transfer_batch,std::size_t image_group_id)
        :ResourceTransferBatch(resource_transfer_batch),ImageGroupId(image_group_id)
    {}
};

// Cmd buffer recording

struct lvkRecordTransferBufferTask
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    
    bool Finished()
        {return ResourceTransferBatch.TransferBuffer != VK_NULL_HANDLE;}
    
    void Execute(VkCommandBuffer transfer_buffer)
    {
        lvkTransferCommandRecorder TransferCommandRecorder;
        
        for(std::size_t i=0;i < ResourceTransferBatch.MeshGroupLoaders.size();i++)
            {TransferCommandRecorder.AddMeshGroup(ResourceTransferBatch.MeshGroupLoaders[i]);}
        
        for(std::size_t i=0;i < ResourceTransferBatch.ImageGroupLoaders.size();i++)
            {TransferCommandRecorder.AddImageGroup(ResourceTransferBatch.ImageGroupLoaders[i]);}
        
        ResourceTransferBatch.TransferBuffer = transfer_buffer;
        TransferCommandRecorder.RecordTransferBuffer(ResourceTransferBatch.TransferBuffer);
    }
    
    lvkRecordTransferBufferTask(lvkResourceTransferBatch &resource_transfer_batch)
        :ResourceTransferBatch(resource_transfer_batch)
    {}
};

struct lvkResourceLoadingTasks
{
    lvkResourceTransferBatch &ResourceTransferBatch;
    
    std::vector<lvkAllocateVertexBufferTask> AllocVertexBufferTasks;
    std::vector<lvkAllocateIndexBufferTask> AllocIndexBufferTasks;
    
    std::vector<lvkAllocateVertexStagingBufferTask> AllocVertexStagingBufferTasks;
    std::vector<lvkAllocateIndexStagingBufferTask> AllocIndexStagingBufferTasks;
    
    std::vector<lvkAllocateImageMemoryTask> AllocImageMemoryTasks;
    std::vector<lvkAllocateRegionStagingBufferTask> AllocRegionStagingBufferTasks;
    
    std::vector<lvkUploadMeshGroupTask>  UploadMeshGroupTasks;
    std::vector<lvkUploadImageGroupTask> UploadImageGroupTasks;
    
    lvkRecordTransferBufferTask RecordCommandBufferTask;
    
    bool FinishedAllocs  = false;
    bool FinishedLoading = false;
    
    void ExecuteAllocs(lvkResourceCategories &resource_categories)
    {
        for(auto &MeshGroupLoader : ResourceTransferBatch.MeshGroupLoaders)
            {MeshGroupLoader.LoadMetadata();}
        
        for(auto &ImageGroupLoader : ResourceTransferBatch.ImageGroupLoaders)
            {ImageGroupLoader.LoadMetadata();}
        
        for(auto &AllocVertexBufferTask : AllocVertexBufferTasks)
            {AllocVertexBufferTask.Execute(resource_categories);}
        
        for(auto &AllocIndexBufferTask : AllocIndexBufferTasks)
            {AllocIndexBufferTask.Execute(resource_categories);}
        
        for(auto &AllocVertexStagingBufferTask : AllocVertexStagingBufferTasks)
            {AllocVertexStagingBufferTask.Execute(resource_categories);}
        
        for(auto &AllocIndexStagingBufferTask : AllocIndexStagingBufferTasks)
            {AllocIndexStagingBufferTask.Execute(resource_categories);}
        
        for(auto &AllocImageMemoryTask : AllocImageMemoryTasks)
            {AllocImageMemoryTask.Execute(resource_categories);}
        
        for(auto &AllocRegionStagingBufferTask : AllocRegionStagingBufferTasks)
            {AllocRegionStagingBufferTask.Execute(resource_categories);}
        
        FinishedAllocs = true;
    }
    
    void ExecuteLoading()
    {
        for(auto &UploadMeshGroupTask : UploadMeshGroupTasks)
            {UploadMeshGroupTask.Execute();}
        
        for(auto &UploadImageGroupTask : UploadImageGroupTasks)
            {UploadImageGroupTask.Execute();}
            
        for(auto &MeshGroupLoader : ResourceTransferBatch.MeshGroupLoaders)
            {MeshGroupLoader.FinishLoading();}
        
        for(auto &ImageGroupLoader : ResourceTransferBatch.ImageGroupLoaders)
            {ImageGroupLoader.FinishLoading();}
        
        FinishedLoading = true;
    }
    
    void ExecuteCmdRecording(VkCommandBuffer transfer_buffer)
    {
        RecordCommandBufferTask.Execute(transfer_buffer);
    }
    
    lvkResourceLoadingTasks(lvkResourceTransferBatch &resource_transfer_batch)
        :ResourceTransferBatch(resource_transfer_batch),RecordCommandBufferTask(resource_transfer_batch)
    {
        AllocVertexBufferTasks.reserve(resource_transfer_batch.MeshGroupLoaders.size());
        for(std::size_t i=0;i < resource_transfer_batch.MeshGroupLoaders.size();i++)
            {AllocVertexBufferTasks.emplace_back(resource_transfer_batch,i);}
        
        AllocIndexBufferTasks.reserve(resource_transfer_batch.MeshGroupLoaders.size());
        for(std::size_t i=0;i < resource_transfer_batch.MeshGroupLoaders.size();i++)
            {AllocIndexBufferTasks.emplace_back(resource_transfer_batch,i);}
        
        AllocVertexStagingBufferTasks.reserve(resource_transfer_batch.MeshGroupLoaders.size());
        for(std::size_t i=0;i < resource_transfer_batch.MeshGroupLoaders.size();i++)
            {AllocVertexStagingBufferTasks.emplace_back(resource_transfer_batch,i);}
        
        AllocIndexStagingBufferTasks.reserve(resource_transfer_batch.MeshGroupLoaders.size());
        for(std::size_t i=0;i < resource_transfer_batch.MeshGroupLoaders.size();i++)
            {AllocIndexStagingBufferTasks.emplace_back(resource_transfer_batch,i);}
        
        AllocImageMemoryTasks.reserve(resource_transfer_batch.ImageGroupLoaders.size());
        for(std::size_t i=0;i < resource_transfer_batch.ImageGroupLoaders.size();i++)
            {AllocImageMemoryTasks.emplace_back(resource_transfer_batch,i);}
        
        AllocRegionStagingBufferTasks.reserve(resource_transfer_batch.ImageGroupLoaders.size());
        for(std::size_t i=0;i < resource_transfer_batch.ImageGroupLoaders.size();i++)
            {AllocRegionStagingBufferTasks.emplace_back(resource_transfer_batch,i);}
        
        UploadMeshGroupTasks.reserve(resource_transfer_batch.MeshGroupLoaders.size());
        for(std::size_t i=0;i < resource_transfer_batch.MeshGroupLoaders.size();i++)
            {UploadMeshGroupTasks.emplace_back(resource_transfer_batch,i);}
        
        UploadImageGroupTasks.reserve(resource_transfer_batch.ImageGroupLoaders.size());
        for(std::size_t i=0;i < resource_transfer_batch.ImageGroupLoaders.size();i++)
            {UploadImageGroupTasks.emplace_back(resource_transfer_batch,i);}
    }
};

#include <atomic>

#include <lvkUtils/lvkCommandPoolUtils.h>

struct lvkLoadingThreadData
{
    struct lvkResourceLoadingTaskPair
    {
        std::unique_ptr<lvkResourceTransferBatch> TransferBatch;
        lvkResourceLoadingTasks Tasks;
        
        lvkResourceLoadingTaskPair(lvkResourceTransferBatch *transfer_batch)
            :TransferBatch(transfer_batch),Tasks(*transfer_batch)
        {}
    };
    
    struct lvkAllocatedCmdBuffers
    {
        lvkCommandPool     CommandPool;
        lvkCommandPoolUtil CommandPoolUtil;
        
        std::size_t UsedBuffers = 0;
        std::vector<VkCommandBuffer> AllocatedCommandBuffers;
        
        std::atomic<bool> IsAvailable;
        
        VkCommandBuffer GetNext()
        {
            if(UsedBuffers >= AllocatedCommandBuffers.size())
            {
                AllocatedCommandBuffers.push_back(CommandPoolUtil.Allocate());
            }
            
            std::size_t CurrentBuffer = UsedBuffers;
            UsedBuffers++;
            
            return AllocatedCommandBuffers[CurrentBuffer];
        }
    };
    
    lvkResourceCategories ResourceCategories;
    
    std::mutex ResourceListLock;
    std::list<lvkResourceLoadingTaskPair> LoadingTasks;
    
    constexpr static std::size_t MAX_CMD_POOL_COUNT = 3;
    
    std::mutex LoadedResourceLock;
    
    std::size_t CurrentPool = 0;
    std::array<lvkAllocatedCmdBuffers,MAX_CMD_POOL_COUNT> TransferPools;
    std::vector<std::unique_ptr<lvkResourceTransferBatch> > LoadedResources;
    
    class lvkRecordedResources
    {
    private:
        lvkLoadingThreadData *LoadingThreadData = nullptr;
        std::size_t PoolId = 0;
        std::vector<std::unique_ptr<lvkResourceTransferBatch> > LoadedResources;
        
    public:
        
        std::vector<VkCommandBuffer> GetCommandBuffers()
        {
            std::vector<VkCommandBuffer> Result(LoadedResources.size());
            
            for(std::size_t i=0;i < Result.size();i++)
                {Result[i] = LoadedResources[i]->TransferBuffer;}
            
            return Result;
        }
        
        void operator=(lvkRecordedResources &&other)
        {
            if(LoadingThreadData != nullptr)
            {
                for(auto &LoadedResource : LoadedResources)
                {
                    for(auto &MeshGroupLoader : LoadedResource->MeshGroupLoaders)
                    {
                        MeshGroupLoader.MeshGroupData.Loaded = true;
                    }
                    
                    for(auto &ImageGroupLoader : LoadedResource->ImageGroupLoaders)
                    {
                        ImageGroupLoader.ImageGroupData.Loaded = true;
                    }
                }
                
                LoadingThreadData->TransferPools[PoolId].CommandPoolUtil.Reset();
                LoadingThreadData->TransferPools[PoolId].UsedBuffers = 0;
                LoadingThreadData->TransferPools[PoolId].IsAvailable.store(true);
            }
            
            LoadingThreadData = other.LoadingThreadData;
            PoolId            = other.PoolId;
            LoadedResources   = std::move(other.LoadedResources);
            
            other.LoadingThreadData = nullptr;
        }
        
        lvkRecordedResources()
        {}
        
        lvkRecordedResources(lvkLoadingThreadData *loading_thread_data,std::size_t pool_id,std::vector<std::unique_ptr<lvkResourceTransferBatch> > &&loaded_resources)
            :LoadingThreadData(loading_thread_data),PoolId(pool_id),LoadedResources(std::move(loaded_resources))
        {}
        
        lvkRecordedResources(lvkRecordedResources &&other)
        {
            LoadingThreadData = other.LoadingThreadData;
            PoolId            = other.PoolId;
            LoadedResources   = std::move(other.LoadedResources);
            
            other.LoadingThreadData = nullptr;
        }
        
        ~lvkRecordedResources()
        {
            if(LoadingThreadData != nullptr)
            {
                for(auto &LoadedResource : LoadedResources)
                {
                    for(auto &MeshGroupLoader : LoadedResource->MeshGroupLoaders)
                    {
                        MeshGroupLoader.MeshGroupData.Loaded = true;
                    }
                    
                    for(auto &ImageGroupLoader : LoadedResource->ImageGroupLoaders)
                    {
                        ImageGroupLoader.ImageGroupData.Loaded = true;
                    }
                }
                
                LoadingThreadData->TransferPools[PoolId].CommandPoolUtil.Reset();
                LoadingThreadData->TransferPools[PoolId].UsedBuffers = 0;
                LoadingThreadData->TransferPools[PoolId].IsAvailable.store(true);
            }
        }
    };
    
    void AddTask(
        const std::vector<std::pair<liMeshGroupSrc*,lvkMeshGroupData *> > &mesh_groups,
        const std::vector<std::pair<liImageGroupSrc*,lvkImageGroupData *> > &image_groups
        )
    {
        lvkResourceTransferBatch *ResourceTransferBatch = new lvkResourceTransferBatch;
        
        ResourceTransferBatch->MeshGroupLoaders.reserve(mesh_groups.size());
        ResourceTransferBatch->ImageGroupLoaders.reserve(image_groups.size());
        
        for(auto &MeshGroup : mesh_groups)
        {
            auto [MeshGroupSrc,MeshGroupData] = MeshGroup;
            
            ResourceTransferBatch->MeshGroupLoaders.emplace_back(*MeshGroupSrc,*MeshGroupData);
        }
        
        for(auto &ImageGroup : image_groups)
        {
            auto [ImageGroupSrc,ImageGroupData] = ImageGroup;
            
            ResourceTransferBatch->ImageGroupLoaders.emplace_back(*ImageGroupSrc,*ImageGroupData);
        }
        
        {
            std::lock_guard<std::mutex> guard(ResourceListLock);
            LoadingTasks.emplace_back(ResourceTransferBatch);
        }
    }
    
    void Execute()
    {
        std::lock_guard<std::mutex> guard(ResourceListLock);
        
        for(auto LoadingTask = LoadingTasks.begin();LoadingTask != LoadingTasks.end();)
        {
            auto CurrentTask = LoadingTask;
            LoadingTask++;
            
            if(!CurrentTask->Tasks.FinishedAllocs)
                {CurrentTask->Tasks.ExecuteAllocs(ResourceCategories);}
            
            if(!CurrentTask->Tasks.FinishedLoading && CurrentTask->Tasks.FinishedAllocs)
                {CurrentTask->Tasks.ExecuteLoading();}
            
            if(!CurrentTask->Tasks.RecordCommandBufferTask.Finished() && CurrentTask->Tasks.FinishedLoading)
            {
                if(TransferPools[CurrentPool].IsAvailable.load())
                {
                    std::lock_guard<std::mutex> guard(LoadedResourceLock);
                    if(TransferPools[CurrentPool].IsAvailable.load())
                    {
                        CurrentTask->Tasks.ExecuteCmdRecording(TransferPools[CurrentPool].GetNext());
                        LoadedResources.push_back(std::move(CurrentTask->TransferBatch));
                        LoadingTasks.erase(CurrentTask);
                    }
                }
            }
        }
    }
    
    lvkRecordedResources TakeFinishedCmdsAndRotate()
    {
        std::lock_guard<std::mutex> guard(LoadedResourceLock);
        
        if(LoadedResources.size() > 0)
        {
            lvkRecordedResources Result = {this,CurrentPool,std::move(LoadedResources)};
            
            TransferPools[CurrentPool].IsAvailable.store(false);
            
            CurrentPool = (CurrentPool + 1) % TransferPools.size();
            
            return Result;
        }
        else
        {
            return {};
        }
    }
    
    lvkLoadingThreadData(VkDevice device,std::uint32_t transfer_queue_family,
        
        lvkBufferPoolCategory        &vertex_buffer_category,
        lvkBufferPoolCategory        &index_buffer_category,
        lvkImagePoolCategory         &image_pool_category,
        lvkPMappedBufferPoolCategory &staging_buffer_category
    )
        :ResourceCategories(vertex_buffer_category,index_buffer_category,image_pool_category,staging_buffer_category)
    {
        for(std::size_t i=0;i < TransferPools.size();i++)
        {
            TransferPools[i].CommandPool = lvkCommandPoolUtil::CreateCommandPool(
                device,
                transfer_queue_family,
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
            );
            TransferPools[i].CommandPoolUtil = {device,TransferPools[i].CommandPool.get()};
            TransferPools[i].IsAvailable.store(true);
        }
    }
};

#endif // LVK_LOADING_THREAD_DATA_H
