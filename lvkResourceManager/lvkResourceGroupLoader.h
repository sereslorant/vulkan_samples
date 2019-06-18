#ifndef LVK_RESOURCE_GROUP_LOADER_H
#define LVK_RESOURCE_GROUP_LOADER_H

#include <atomic>

#include "lvkResourcePools.h"

#include <lvkRenderer/lvkResourceHierarchy.h>

/*
 * API independent image loader
 */

class liImageGroupSrc
{
public:
    virtual void LoadMetadata() = 0;
    
    virtual std::uint32_t GetTexelSize() const = 0;
    virtual std::uint32_t GetSingleImageSize() const = 0;
    virtual std::uint32_t GetImageCount() const = 0;
    
    virtual void ReadData(std::uint32_t first_image,std::uint32_t image_count,lMemoryView &dst_region_view) = 0;
    
    virtual void FinishLoading() = 0;
    
    virtual ~liImageGroupSrc()
    {}
};

struct lrmImageGroupData
{
    lrmImageSuballocGuard ImageSuballocs;
    
    bool Loaded = false;
};

struct lrmImageGroupLoader
{
    /*
     * Input
     */
    
    liImageGroupSrc &ImageGroupSrc;
    
    /*
     * Output
     */
    
    lrmImageGroupData &ImageGroupData;
    
    /*
     * Internal state
     */
    
    struct lrmImageAllocContiguousRegion
    {
        std::size_t            FirstImage;
        std::size_t            FirstImageIndex;
        std::size_t            ImageCount;
        lrmBufferSuballocGuard StagingRegion;
    };
    
    std::vector<lrmImageAllocContiguousRegion> ImageRegions;
    
    void RequestImageMemory(lrmImageSuballocator &image_suballocator)
    {
        ImageGroupData.ImageSuballocs = image_suballocator.Allocate(ImageGroupSrc.GetImageCount());
        
        if(ImageGroupData.ImageSuballocs.IsValid())
        {
            lrmImageAllocContiguousRegion CurrentRegion = {
                .FirstImage = ImageGroupData.ImageSuballocs.GetTextureAllocs()[0],
                .FirstImageIndex = 0,
                .ImageCount = 1,
                .StagingRegion = {}
            };
            
            for(std::size_t i=1;i < ImageGroupData.ImageSuballocs.GetTextureAllocs().size();i++)
            {
                if(ImageGroupData.ImageSuballocs.GetTextureAllocs()[i] != (CurrentRegion.FirstImage + CurrentRegion.ImageCount))
                {
                    ImageRegions.push_back(std::move(CurrentRegion));
                    
                    CurrentRegion = {
                        .FirstImage = ImageGroupData.ImageSuballocs.GetTextureAllocs()[i],
                        .FirstImageIndex = i,
                        .ImageCount = 1,
                        .StagingRegion = {}
                    };
                }
                else
                {
                    CurrentRegion.ImageCount++;
                }
            }
            
            ImageRegions.push_back(std::move(CurrentRegion));
        }
    }
    
    void RequestStagingMemory(std::size_t region_id,lrmBufferSuballocator &staging_buffer_allocator)
    {
        ImageRegions[region_id].StagingRegion = staging_buffer_allocator.Allocate(ImageGroupSrc.GetTexelSize(),ImageRegions[region_id].ImageCount * ImageGroupSrc.GetSingleImageSize());
    }
    
    void LoadRegion(std::size_t region_id,lMemoryView &staging_memory_view)
    {
        lMemoryView RegionView = staging_memory_view.GetView(
            ImageRegions[region_id].StagingRegion.GetOffset(),
            ImageRegions[region_id].ImageCount * ImageGroupSrc.GetSingleImageSize()
        );
        
        ImageGroupSrc.ReadData(ImageRegions[region_id].FirstImageIndex,ImageRegions[region_id].ImageCount,RegionView);
    }
    
    lrmImageGroupLoader(liImageGroupSrc &image_group_src,lrmImageGroupData &image_group_data)
        :ImageGroupSrc(image_group_src),ImageGroupData(image_group_data)
    {}
};

/*
 * API independent model loader
 */

class liMeshGroupSrc
{
public:
    virtual void LoadMetadata() = 0;
    
    virtual std::size_t GetVertexDataSize() const = 0;
    virtual std::size_t GetIndexDataSize() const = 0;
    virtual void ReadVertexOffsets(std::uint32_t &binding_count,std::uint32_t binding_offsets[],const std::uint32_t max_bindings) const = 0;
    
    virtual void LoadVertexData(lMemoryView &vertex_data_region) = 0;
    virtual void LoadIndexData(lMemoryView &index_data_region) = 0;
    
    virtual void FinishLoading(lrMeshGroup &mesh_group) = 0;
    
    virtual ~liMeshGroupSrc()
    {}
};

struct lrmMeshGroupData
{
    constexpr static std::size_t MAX_BINDINGS = 16;
    
    std::uint32_t BindingCount = 0;
    std::array<std::uint32_t,MAX_BINDINGS> BindingsOffsets = {};
    
    lrmBufferSuballocGuard VertexDataRegion;
    lrmBufferSuballocGuard IndexDataRegion;
    
    lrMeshGroup MeshGroup;
    
    bool Loaded = false;
};

struct lrmMeshGroupLoader
{
    /*
     * Input
     */
    
    liMeshGroupSrc &MeshGroupSrc;
    
    /*
     * Output
     */
    
    lrmMeshGroupData &MeshGroupData;
    
    /*
     * Internal state
     */
    
    lrmBufferSuballocGuard VertexDataStagingRegion;
    lrmBufferSuballocGuard IndexDataStagingRegion;
    
    void RequestVertexBuffer(lrmBufferSuballocator &vertex_buffer_allocator)
    {
        MeshGroupData.VertexDataRegion = vertex_buffer_allocator.Allocate(16,MeshGroupSrc.GetVertexDataSize());
        
        if(MeshGroupData.VertexDataRegion.IsValid())
        {
            MeshGroupSrc.ReadVertexOffsets(MeshGroupData.BindingCount,MeshGroupData.BindingsOffsets.data(),lrmMeshGroupData::MAX_BINDINGS);
            
            for(std::size_t i=0;i < MeshGroupData.BindingCount;i++)
                {MeshGroupData.BindingsOffsets[i] += MeshGroupData.VertexDataRegion.GetOffset();}
        }
    }
    
    void RequestIndexBuffer(lrmBufferSuballocator &index_buffer_allocator)
    {
        MeshGroupData.IndexDataRegion = index_buffer_allocator.Allocate(16,MeshGroupSrc.GetIndexDataSize());
    }
    
    void RequestVertexStagingBuffer(lrmBufferSuballocator &staging_buffer_allocator)
    {
        VertexDataStagingRegion = staging_buffer_allocator.Allocate(16,MeshGroupSrc.GetVertexDataSize());
    }
    
    void RequestIndexStagingBuffer(lrmBufferSuballocator &staging_buffer_allocator)
    {
        IndexDataStagingRegion  = staging_buffer_allocator.Allocate(16,MeshGroupSrc.GetIndexDataSize());
    }
    
    void LoadVertexData(lMemoryView &staging_memory_view)
    {
        lMemoryView VertexDataView = staging_memory_view.GetView(VertexDataStagingRegion.GetOffset(),VertexDataStagingRegion.GetSize());
        
        MeshGroupSrc.LoadVertexData(VertexDataView);
    }
    
    void LoadIndexData(lMemoryView &staging_memory_view)
    {
        lMemoryView IndexMemoryView = staging_memory_view.GetView(IndexDataStagingRegion.GetOffset(),IndexDataStagingRegion.GetSize());
        
        MeshGroupSrc.LoadIndexData(IndexMemoryView);
    }
    
    lrmMeshGroupLoader(liMeshGroupSrc &mesh_group,lrmMeshGroupData &mesh_group_data)
        :MeshGroupSrc(mesh_group),MeshGroupData(mesh_group_data)
    {}
};

/*
 * Vulkan specific image and mesh loader
 */

#include <lvkResourceManager/lvkResourcePools.h>

/*
 * Image loader
 */

struct lvkImageGroupData : public lrmImageGroupData
{
    lvkImagePool *Image = nullptr;
};

struct lvkImageGroupLoader
{
    /*
     * Input
     */
    
    liImageGroupSrc &ImageGroupSrc;
    
    /*
     * Output
     */
    
    lvkImageGroupData &ImageGroupData;
    
    /*
     * Internal state
     */
    
    lrmImageGroupLoader   ImageGroupLoader;
    std::vector<lvkPMappedBufferPool *> RegionStagingBuffers;
    
    void LoadMetadata()
    {
        ImageGroupSrc.LoadMetadata();
    }
    
    void RequestImageMemory(lvkImagePool &image_pool)
    {
        ImageGroupLoader.RequestImageMemory(image_pool.Suballocator);
        
        if(ImageGroupData.ImageSuballocs.IsValid())
        {
            ImageGroupData.Image = &image_pool;
            RegionStagingBuffers.resize(ImageGroupLoader.ImageRegions.size(),VK_NULL_HANDLE);
        }
    }
    
    void RequestStagingMemory(std::size_t region_id,lvkPMappedBufferPool &staging_buffer_pool)
    {
        ImageGroupLoader.RequestStagingMemory(region_id,staging_buffer_pool.Suballocator);
        
        if(ImageGroupLoader.ImageRegions[region_id].StagingRegion.IsValid())
        {
            RegionStagingBuffers[region_id] = &staging_buffer_pool;
        }
    }
    
    void LoadRegion()
    {
        for(int i=0;i < ImageGroupLoader.ImageRegions.size();i++)
        {
            lMemoryView FullMemoryView = RegionStagingBuffers[i]->MappedMemory.GetView();
            ImageGroupLoader.LoadRegion(i,FullMemoryView);
        }
    }
    
    void FinishLoading()
    {
        ImageGroupSrc.FinishLoading();
    }
    
    lvkImageGroupLoader(liImageGroupSrc &image_group_src,lvkImageGroupData &image_group_data)
        :ImageGroupSrc(image_group_src),ImageGroupData(image_group_data),ImageGroupLoader(image_group_src,image_group_data)
    {}
};

/*
 * Mesh loader
 */

struct lvkMeshGroupData : public lrmMeshGroupData
{
    VkBuffer VertexBuffer = VK_NULL_HANDLE;
    VkBuffer IndexBuffer  = VK_NULL_HANDLE;
};

struct lvkMeshGroupLoader
{
    /*
     * Input
     */
    
    liMeshGroupSrc &MeshGroupSrc;
    
    /*
     * Output
     */
    
    lvkMeshGroupData &MeshGroupData;
    
    /*
     * Internal state
     */
    
    lrmMeshGroupLoader MeshGroupLoader;
    lvkPMappedBufferPool *VertexStagingBuffer = nullptr;
    lvkPMappedBufferPool *IndexStagingBuffer  = nullptr;
    
    void LoadMetadata()
    {
        MeshGroupSrc.LoadMetadata();
    }
    
    void RequestVertexBuffer(lvkBufferPool &vertex_buffer_pool)
    {
        MeshGroupLoader.RequestVertexBuffer(vertex_buffer_pool.Suballocator);
        
        if(MeshGroupData.VertexDataRegion.IsValid())
            {MeshGroupData.VertexBuffer = vertex_buffer_pool.BufferPoolResource.Buffer.get();}
    }
    
    void RequestIndexBuffer(lvkBufferPool &index_buffer_pool)
    {
        MeshGroupLoader.RequestIndexBuffer(index_buffer_pool.Suballocator);
        
        if(MeshGroupData.IndexDataRegion.IsValid())
            {MeshGroupData.IndexBuffer = index_buffer_pool.BufferPoolResource.Buffer.get();}
    }
    
    void RequestVertexStagingBuffer(lvkPMappedBufferPool &staging_buffer_allocator)
    {
        MeshGroupLoader.RequestVertexStagingBuffer(staging_buffer_allocator.Suballocator);
        
        if(MeshGroupLoader.VertexDataStagingRegion.IsValid())
            {VertexStagingBuffer = &staging_buffer_allocator;}
    }
    
    void RequestIndexStagingBuffer(lvkPMappedBufferPool &staging_buffer_allocator)
    {
        MeshGroupLoader.RequestIndexStagingBuffer(staging_buffer_allocator.Suballocator);
        
        if(MeshGroupLoader.IndexDataStagingRegion.IsValid())
            {IndexStagingBuffer = &staging_buffer_allocator;}
    }
    
    void LoadVertexData()
    {
        lMemoryView FullMemoryView = VertexStagingBuffer->MappedMemory.GetView();
        MeshGroupLoader.LoadVertexData(FullMemoryView);
    }
    
    void LoadIndexData()
    {
        lMemoryView FullMemoryView = IndexStagingBuffer->MappedMemory.GetView();
        MeshGroupLoader.LoadIndexData(FullMemoryView);
    }
    
    void FinishLoading()
    {
        MeshGroupSrc.FinishLoading(MeshGroupData.MeshGroup);
    }
    
    lvkMeshGroupLoader(liMeshGroupSrc &mesh_group,lvkMeshGroupData &mesh_group_data)
        :MeshGroupSrc(mesh_group),MeshGroupData(mesh_group_data),MeshGroupLoader(mesh_group,mesh_group_data)
    {}
};

#include <lvkResourceManager/lvkTransferRecorder.h>
#include <lvkUtils/lvkCommandBufferUtils.h>

class lvkTransferCommandRecorder
{
private:
    lvkBufferToBufferTransferRecorder BufferToBufferTransferRecorder;
    lvkBufferToImageTransferRecorder  BufferToImageTransferRecorder;
    
    void AddBufferTransfer(VkBuffer src_buffer,VkBuffer dst_buffer,VkDeviceSize src_offset,VkDeviceSize dst_offset,VkDeviceSize size)
    {
        lvkBufferToBufferTransferBatch *TransferBatch = nullptr;
        for(auto &BufferTransfer : BufferToBufferTransferRecorder.BufferTransfers)
        {
            if( BufferTransfer.SrcBuffer == src_buffer &&
                BufferTransfer.DstBuffer == dst_buffer
            )
            {
                TransferBatch = &BufferTransfer;
            }
        }
        
        if(TransferBatch == nullptr)
        {
            BufferToBufferTransferRecorder.BufferTransfers.push_back({
                .SrcBuffer = src_buffer,
                .DstBuffer = dst_buffer
            });
            
            TransferBatch = &BufferToBufferTransferRecorder.BufferTransfers.back();
        }
        
        TransferBatch->CopyRegions.push_back({
            .srcOffset = src_offset,
            .dstOffset = dst_offset,
            .size      = size,
        });
    }
    
public:
    
    void AddImageGroup(lvkImageGroupLoader &ImageGroupLoader)
    {
        for(std::size_t i=0;i < ImageGroupLoader.ImageGroupLoader.ImageRegions.size();i++)
        {
            lvkBufferToImageTransferBatch *TransferBatch = nullptr;
            for(auto &ImageTransfer : BufferToImageTransferRecorder.ImageTransfers)
            {
                if( ImageTransfer.Image  == ImageGroupLoader.ImageGroupData.Image->ImagePoolResource.Image.get() &&
                    ImageTransfer.Buffer == ImageGroupLoader.RegionStagingBuffers[i]->BufferPoolResource.Buffer.get()
                )
                {
                    TransferBatch = &ImageTransfer;
                }
            }
            
            if(TransferBatch == nullptr)
            {
                BufferToImageTransferRecorder.ImageTransfers.push_back({
                    .Buffer         = ImageGroupLoader.RegionStagingBuffers[i]->BufferPoolResource.Buffer.get(),
                    .Image          = ImageGroupLoader.ImageGroupData.Image->ImagePoolResource.Image.get(),
                    .Width          = ImageGroupLoader.ImageGroupData.Image->ImagePoolResource.Width,
                    .Height         = ImageGroupLoader.ImageGroupData.Image->ImagePoolResource.Height,
                });
                
                TransferBatch = &BufferToImageTransferRecorder.ImageTransfers.back();
            }
            
            TransferBatch->AddTransfer(
                ImageGroupLoader.ImageGroupLoader.ImageRegions[i].StagingRegion.GetOffset(),
                ImageGroupLoader.ImageGroupLoader.ImageRegions[i].FirstImage,
                ImageGroupLoader.ImageGroupLoader.ImageRegions[i].ImageCount
            );
        }
    }
    
    void AddMeshGroup(lvkMeshGroupLoader &MeshGroupLoader)
    {
        AddBufferTransfer(
            MeshGroupLoader.VertexStagingBuffer->BufferPoolResource.Buffer.get(),
            MeshGroupLoader.MeshGroupData.VertexBuffer,
            
            MeshGroupLoader.MeshGroupLoader.VertexDataStagingRegion.GetOffset(),
            MeshGroupLoader.MeshGroupData.VertexDataRegion.GetOffset(),
            MeshGroupLoader.MeshGroupLoader.VertexDataStagingRegion.GetSize()
        );
        
        AddBufferTransfer(
            MeshGroupLoader.IndexStagingBuffer->BufferPoolResource.Buffer.get(),
            MeshGroupLoader.MeshGroupData.IndexBuffer,
            
            MeshGroupLoader.MeshGroupLoader.IndexDataStagingRegion.GetOffset(),
            MeshGroupLoader.MeshGroupData.IndexDataRegion.GetOffset(),
            MeshGroupLoader.MeshGroupLoader.IndexDataStagingRegion.GetSize()
        );
    }
    
    void RecordTransferBuffer(VkCommandBuffer transfer_buffer)
    {
        std::vector<VkImageMemoryBarrier> ImageBarriers;
        BufferToImageTransferRecorder.Resize(ImageBarriers);
        
        std::vector<VkBufferMemoryBarrier> BufferBarriers;
        BufferToBufferTransferRecorder.Resize(BufferBarriers);
        
        {
            lvkRecordCmdBufferGuard TransferGuard(transfer_buffer,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            
            BufferToImageTransferRecorder.RecordTransferBarrier(ImageBarriers,transfer_buffer);
            //BufferToBufferTransferRecorder.RecordTransferBarrier(BufferBarriers,transfer_buffer);
            
            BufferToImageTransferRecorder.RecordTransferOps(transfer_buffer);
            BufferToBufferTransferRecorder.RecordTransferOps(transfer_buffer);
            
            BufferToImageTransferRecorder.RecordShaderAccessBarrier(ImageBarriers,transfer_buffer);
            BufferToBufferTransferRecorder.RecordShaderAccessBarrier(BufferBarriers,transfer_buffer);
        }
    }
};

struct lvkResourceTransferBatch
{
    std::vector<lvkImageGroupLoader> ImageGroupLoaders;
    std::vector<lvkMeshGroupLoader> MeshGroupLoaders;
    
    VkCommandBuffer TransferBuffer = VK_NULL_HANDLE;
};

#endif // LVK_RESOURCE_GROUP_LOADER_H
