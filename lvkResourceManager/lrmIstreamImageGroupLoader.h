#ifndef LRM_ISTREAM_IMAGE_GROUP_LOADER_H
#define LRM_ISTREAM_IMAGE_GROUP_LOADER_H


#include <cinttypes>

struct lrmImageMetadata
{
    enum lImageFormat
    {
        FORMAT_RGBAF32,
    };
    
    std::uint32_t Format;
    std::uint32_t Width;
    std::uint32_t Height;
    
    static constexpr std::uint32_t COMPONENT_COUNT[] = {
        4,
    };
    
    static constexpr std::uint32_t PIXEL_SIZES[] = {
        4*sizeof(float),
    };
    
    std::size_t GetTotalSize()
    {
        return Width * Height * PIXEL_SIZES[Format];
    }
};

#include <fstream>

struct lrmIstreamImageFile
{
    std::ifstream    In;
    lrmImageMetadata ImageMetadata;
    
    void ReadMetadata()
    {
        In.read((char*)&ImageMetadata.Format,sizeof(std::uint32_t));
        In.read((char*)&ImageMetadata.Width, sizeof(std::uint32_t));
        In.read((char*)&ImageMetadata.Height,sizeof(std::uint32_t));
    }
    
    void ReadPixelData(void *dest)
    {
        In.read((char*)dest,ImageMetadata.GetTotalSize());
    }
};

#include <algorithm>
#include <vector>

class lrmIstreamImageGroupLoader
{
private:
    std::vector<lrmIstreamImageFile> ImageFiles;
    
public:
    
    lrmImageMetadata::lImageFormat ImageFormat;
    std::size_t SingleImageSize;
    std::size_t TexelSize;
    
    std::size_t GetImageCount() const
    {
        return ImageFiles.size();
    }
    
    const std::vector<lrmIstreamImageFile> &GetImageFiles()
    {
        return ImageFiles;
    }
    
    void ReadMetadata()
    {
        for(auto &MeshFile : ImageFiles)
        {
            MeshFile.ReadMetadata();
            
            switch(MeshFile.ImageMetadata.Format)
            {
                case lrmImageMetadata::FORMAT_RGBAF32:
                    ImageFormat = lrmImageMetadata::FORMAT_RGBAF32;
                    break;
            }
            
            SingleImageSize = MeshFile.ImageMetadata.GetTotalSize();
            TexelSize       = lrmImageMetadata::PIXEL_SIZES[MeshFile.ImageMetadata.Format];
        }
    }
    
    void ReadPixelData(std::uint32_t first_image,std::uint32_t image_count,void *data)
    {
        for(std::size_t i=first_image;i < std::min<std::size_t>(ImageFiles.size(),first_image + image_count);i++)
        {
            ImageFiles[i].ReadPixelData((char *)data + (i-first_image)*SingleImageSize);
        }
    }
    
    void Close()
    {
        for(std::size_t i=0;i < ImageFiles.size();i++)
        {
            ImageFiles[i].In.close();
        }
    }
    
    lrmIstreamImageGroupLoader(const char * const file_names[],unsigned int file_count)
    {
        ImageFiles.resize(file_count);
        for(std::size_t i=0;i < ImageFiles.size();i++)
        {
            ImageFiles[i].In.open(file_names[i]);
        }
    }
};

#include <lvkResourceManager/lvkResourceGroupLoader.h>

struct lvkFileImageGroupLoader : public liImageGroupSrc
{
    lrmIstreamImageGroupLoader ImageGroupLoader;
    
    constexpr static std::uint32_t WIDTH  = 2;
    constexpr static std::uint32_t HEIGHT = 2;
    constexpr static std::uint32_t IMAGE_COMPONENT_COUNT = 4;
    
    constexpr static std::uint64_t SINGLE_IMAGE_SIZE = WIDTH * HEIGHT * IMAGE_COMPONENT_COUNT * sizeof(float);
    
    constexpr static std::uint32_t IMAGE_COUNT = 4;
    
    std::array<std::array<float,WIDTH*HEIGHT*IMAGE_COMPONENT_COUNT>,IMAGE_COUNT> Images;
    
    virtual void LoadMetadata() override
    {
        ImageGroupLoader.ReadMetadata();
    }
    
    virtual std::uint32_t GetTexelSize() const override
    {
        return ImageGroupLoader.TexelSize;
    }
    
    std::uint32_t GetSingleImageSize() const override
    {
        return ImageGroupLoader.SingleImageSize;
    }
    
    std::uint32_t GetImageCount() const override
    {
        return ImageGroupLoader.GetImageCount();
    }
    
    void ReadData(std::uint32_t first_image,std::uint32_t image_count,lMemoryView &dst_region_view) override
    {
        lMemoryView DstView = dst_region_view.GetView(
                first_image * GetSingleImageSize(),
                image_count * GetSingleImageSize()
            );
        
        ImageGroupLoader.ReadPixelData(first_image,image_count,DstView.GetPtr());
    }
    
    virtual void FinishLoading() override
    {
        ImageGroupLoader.Close();
    }
    
    lvkFileImageGroupLoader(const char * const file_names[],unsigned int file_count)
        :ImageGroupLoader(file_names,file_count)
    {}
};

#endif // LRM_ISTREAM_IMAGE_GROUP_LOADER_H
