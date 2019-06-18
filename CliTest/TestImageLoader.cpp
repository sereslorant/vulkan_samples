
#include <lvkResourceManager/lrmIstreamImageGroupLoader.h>

#include <iostream>

void Print(const lrmImageMetadata &image_metadata,void *data)
{
    float *Data = (float *)data;
    
    std::cout << "Format: " << image_metadata.Format << std::endl;
    std::cout << "Width:  " << image_metadata.Width  << std::endl;
    std::cout << "Height: " << image_metadata.Height << std::endl;
    
    for(std::size_t i=0;i < image_metadata.Width * image_metadata.Height * lrmImageMetadata::COMPONENT_COUNT[image_metadata.Format];i++)
    {
        std::cout << Data[i] << ";";
    }
    std::cout << std::endl;
}

int main(int argc,char *argv[])
{
    lrmIstreamImageFile ImageFile;
    ImageFile.In.open("TestImage/TestImage",std::ifstream::binary);
    
    ImageFile.ReadMetadata();
    
    float *Data = new float[ImageFile.ImageMetadata.Width * ImageFile.ImageMetadata.Height * lrmImageMetadata::COMPONENT_COUNT[ImageFile.ImageMetadata.Format]];
    
    ImageFile.ReadPixelData(Data);
    
    std::cout << "Format: " << ImageFile.ImageMetadata.Format << std::endl;
    std::cout << "Width:  " << ImageFile.ImageMetadata.Width  << std::endl;
    std::cout << "Height: " << ImageFile.ImageMetadata.Height << std::endl;
    
    for(std::size_t i=0;i < ImageFile.ImageMetadata.Width * ImageFile.ImageMetadata.Height * lrmImageMetadata::COMPONENT_COUNT[ImageFile.ImageMetadata.Format];i++)
    {
        std::cout << Data[i] << ";";
    }
    std::cout << std::endl;
    
    delete [] Data;
    
    
    
    std::int32_t ImageFileCount = 4;
    const char *ImageFileNames[4] =
    {
        "TestImageGroup/TestImage0",
        "TestImageGroup/TestImage1",
        "TestImageGroup/TestImage2",
        "TestImageGroup/TestImage3"
    };
    
    lrmIstreamImageGroupLoader ImageGroupLoader(ImageFileNames,ImageFileCount);
    
    ImageGroupLoader.ReadMetadata();
    
    char *Data1 = new char[ImageGroupLoader.SingleImageSize];
    char *Data2 = new char[ImageGroupLoader.SingleImageSize * (ImageGroupLoader.GetImageCount() - 1)];
    
    ImageGroupLoader.ReadPixelData(0,1,Data1);
    ImageGroupLoader.ReadPixelData(1,ImageGroupLoader.GetImageCount() - 1,Data2);
    
    Print(ImageGroupLoader.GetImageFiles()[0].ImageMetadata,Data1);
    
    for(std::size_t i=1;i < ImageGroupLoader.GetImageCount();i++)
        {Print(ImageGroupLoader.GetImageFiles()[i].ImageMetadata,&Data2[ImageGroupLoader.SingleImageSize*(i-1)]);}
    
    ImageGroupLoader.Close();
    
    delete [] Data1;
    delete [] Data2;
    
    return 0;
}
