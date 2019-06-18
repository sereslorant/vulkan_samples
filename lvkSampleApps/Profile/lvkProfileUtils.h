#ifndef LVK_PROFILE_UTILS_H
#define LVK_PROFILE_UTILS_H

#include <chrono>

#include <lvkRenderer/lvkResourceHierarchy.h>

//constexpr VkCommandPoolCreateFlags COMMAND_POOL_CREATE_FLAGS = 0x0;
constexpr VkCommandPoolCreateFlags LVK_PROF_COMMAND_POOL_CREATE_FLAGS = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

/*constexpr VkCommandBufferUsageFlags LVK_PROF_PRIMARY_COMMAND_BUFFER_USAGE_FLAGS =   VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT |
                                                                                    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // */
constexpr VkCommandBufferUsageFlags LVK_PROF_PRIMARY_COMMAND_BUFFER_USAGE_FLAGS = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // */

/*constexpr VkCommandBufferUsageFlags LVK_PROF_SECONDARY_COMMAND_BUFFER_USAGE_FLAGS = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT     |
                                                                                    VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
                                                                                    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // */
constexpr VkCommandBufferUsageFlags LVK_PROF_SECONDARY_COMMAND_BUFFER_USAGE_FLAGS = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
                                                                                    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // */

//constexpr static std::size_t LKV_PROF_SCENE_DIM = 20;
constexpr static std::size_t LKV_PROF_SCENE_DIM = 50;

constexpr bool LVK_PROF_TURN_ON_VALIDATION = false;

constexpr std::size_t NUM_ACTUAL_MODELS = 5;
constexpr std::size_t NUM_ACTUAL_IMAGES = 16;

#ifdef LVK_PROF_BIG_INPUT
constexpr std::size_t NUM_MODELS = 8*NUM_ACTUAL_MODELS;
constexpr std::size_t NUM_IMAGES = 8*NUM_ACTUAL_IMAGES;
#else
constexpr std::size_t NUM_MODELS = NUM_ACTUAL_MODELS;
constexpr std::size_t NUM_IMAGES = NUM_ACTUAL_IMAGES;
#endif

constexpr std::array<const char *,NUM_MODELS> MeshNames =
{
    "../ProfileContent/Models/ProfModels_Cone",
    "../ProfileContent/Models/ProfModels_Cylinder",
    "../ProfileContent/Models/ProfModels_Sphere",
    "../ProfileContent/Models/ProfModels_Suzanne",
    "../ProfileContent/Models/ProfModels_Torus",
    #ifdef LVK_PROF_BIG_INPUT
    "../ProfileContent/Models/ProfModels_Cone",
    "../ProfileContent/Models/ProfModels_Cylinder",
    "../ProfileContent/Models/ProfModels_Sphere",
    "../ProfileContent/Models/ProfModels_Suzanne",
    "../ProfileContent/Models/ProfModels_Torus",
    
    "../ProfileContent/Models/ProfModels_Cone",
    "../ProfileContent/Models/ProfModels_Cylinder",
    "../ProfileContent/Models/ProfModels_Sphere",
    "../ProfileContent/Models/ProfModels_Suzanne",
    "../ProfileContent/Models/ProfModels_Torus",
    
    "../ProfileContent/Models/ProfModels_Cone",
    "../ProfileContent/Models/ProfModels_Cylinder",
    "../ProfileContent/Models/ProfModels_Sphere",
    "../ProfileContent/Models/ProfModels_Suzanne",
    "../ProfileContent/Models/ProfModels_Torus",
    
    "../ProfileContent/Models/ProfModels_Cone",
    "../ProfileContent/Models/ProfModels_Cylinder",
    "../ProfileContent/Models/ProfModels_Sphere",
    "../ProfileContent/Models/ProfModels_Suzanne",
    "../ProfileContent/Models/ProfModels_Torus",
    
    "../ProfileContent/Models/ProfModels_Cone",
    "../ProfileContent/Models/ProfModels_Cylinder",
    "../ProfileContent/Models/ProfModels_Sphere",
    "../ProfileContent/Models/ProfModels_Suzanne",
    "../ProfileContent/Models/ProfModels_Torus",
    
    "../ProfileContent/Models/ProfModels_Cone",
    "../ProfileContent/Models/ProfModels_Cylinder",
    "../ProfileContent/Models/ProfModels_Sphere",
    "../ProfileContent/Models/ProfModels_Suzanne",
    "../ProfileContent/Models/ProfModels_Torus",
    
    "../ProfileContent/Models/ProfModels_Cone",
    "../ProfileContent/Models/ProfModels_Cylinder",
    "../ProfileContent/Models/ProfModels_Sphere",
    "../ProfileContent/Models/ProfModels_Suzanne",
    "../ProfileContent/Models/ProfModels_Torus",
    #endif
};

constexpr std::array<const char *,NUM_IMAGES> ImageFileNames =
{
    "../ProfileContent/Textures/Img0",
    "../ProfileContent/Textures/Img1",
    "../ProfileContent/Textures/Img2",
    "../ProfileContent/Textures/Img3",
    "../ProfileContent/Textures/Img0Blu",
    "../ProfileContent/Textures/Img1Blu",
    "../ProfileContent/Textures/Img2Blu",
    "../ProfileContent/Textures/Img3Blu",
    "../ProfileContent/Textures/Img0Brn",
    "../ProfileContent/Textures/Img1Brn",
    "../ProfileContent/Textures/Img2Brn",
    "../ProfileContent/Textures/Img3Brn",
    "../ProfileContent/Textures/Img0Grn",
    "../ProfileContent/Textures/Img1Grn",
    "../ProfileContent/Textures/Img2Grn",
    "../ProfileContent/Textures/Img3Grn",
    #ifdef LVK_PROF_BIG_INPUT
    "../ProfileContent/Textures/Img0",
    "../ProfileContent/Textures/Img1",
    "../ProfileContent/Textures/Img2",
    "../ProfileContent/Textures/Img3",
    "../ProfileContent/Textures/Img0Blu",
    "../ProfileContent/Textures/Img1Blu",
    "../ProfileContent/Textures/Img2Blu",
    "../ProfileContent/Textures/Img3Blu",
    "../ProfileContent/Textures/Img0Brn",
    "../ProfileContent/Textures/Img1Brn",
    "../ProfileContent/Textures/Img2Brn",
    "../ProfileContent/Textures/Img3Brn",
    "../ProfileContent/Textures/Img0Grn",
    "../ProfileContent/Textures/Img1Grn",
    "../ProfileContent/Textures/Img2Grn",
    "../ProfileContent/Textures/Img3Grn",
    
    "../ProfileContent/Textures/Img0",
    "../ProfileContent/Textures/Img1",
    "../ProfileContent/Textures/Img2",
    "../ProfileContent/Textures/Img3",
    "../ProfileContent/Textures/Img0Blu",
    "../ProfileContent/Textures/Img1Blu",
    "../ProfileContent/Textures/Img2Blu",
    "../ProfileContent/Textures/Img3Blu",
    "../ProfileContent/Textures/Img0Brn",
    "../ProfileContent/Textures/Img1Brn",
    "../ProfileContent/Textures/Img2Brn",
    "../ProfileContent/Textures/Img3Brn",
    "../ProfileContent/Textures/Img0Grn",
    "../ProfileContent/Textures/Img1Grn",
    "../ProfileContent/Textures/Img2Grn",
    "../ProfileContent/Textures/Img3Grn",
    
    "../ProfileContent/Textures/Img0",
    "../ProfileContent/Textures/Img1",
    "../ProfileContent/Textures/Img2",
    "../ProfileContent/Textures/Img3",
    "../ProfileContent/Textures/Img0Blu",
    "../ProfileContent/Textures/Img1Blu",
    "../ProfileContent/Textures/Img2Blu",
    "../ProfileContent/Textures/Img3Blu",
    "../ProfileContent/Textures/Img0Brn",
    "../ProfileContent/Textures/Img1Brn",
    "../ProfileContent/Textures/Img2Brn",
    "../ProfileContent/Textures/Img3Brn",
    "../ProfileContent/Textures/Img0Grn",
    "../ProfileContent/Textures/Img1Grn",
    "../ProfileContent/Textures/Img2Grn",
    "../ProfileContent/Textures/Img3Grn",
    
    "../ProfileContent/Textures/Img0",
    "../ProfileContent/Textures/Img1",
    "../ProfileContent/Textures/Img2",
    "../ProfileContent/Textures/Img3",
    "../ProfileContent/Textures/Img0Blu",
    "../ProfileContent/Textures/Img1Blu",
    "../ProfileContent/Textures/Img2Blu",
    "../ProfileContent/Textures/Img3Blu",
    "../ProfileContent/Textures/Img0Brn",
    "../ProfileContent/Textures/Img1Brn",
    "../ProfileContent/Textures/Img2Brn",
    "../ProfileContent/Textures/Img3Brn",
    "../ProfileContent/Textures/Img0Grn",
    "../ProfileContent/Textures/Img1Grn",
    "../ProfileContent/Textures/Img2Grn",
    "../ProfileContent/Textures/Img3Grn",
    
    "../ProfileContent/Textures/Img0",
    "../ProfileContent/Textures/Img1",
    "../ProfileContent/Textures/Img2",
    "../ProfileContent/Textures/Img3",
    "../ProfileContent/Textures/Img0Blu",
    "../ProfileContent/Textures/Img1Blu",
    "../ProfileContent/Textures/Img2Blu",
    "../ProfileContent/Textures/Img3Blu",
    "../ProfileContent/Textures/Img0Brn",
    "../ProfileContent/Textures/Img1Brn",
    "../ProfileContent/Textures/Img2Brn",
    "../ProfileContent/Textures/Img3Brn",
    "../ProfileContent/Textures/Img0Grn",
    "../ProfileContent/Textures/Img1Grn",
    "../ProfileContent/Textures/Img2Grn",
    "../ProfileContent/Textures/Img3Grn",
    
    "../ProfileContent/Textures/Img0",
    "../ProfileContent/Textures/Img1",
    "../ProfileContent/Textures/Img2",
    "../ProfileContent/Textures/Img3",
    "../ProfileContent/Textures/Img0Blu",
    "../ProfileContent/Textures/Img1Blu",
    "../ProfileContent/Textures/Img2Blu",
    "../ProfileContent/Textures/Img3Blu",
    "../ProfileContent/Textures/Img0Brn",
    "../ProfileContent/Textures/Img1Brn",
    "../ProfileContent/Textures/Img2Brn",
    "../ProfileContent/Textures/Img3Brn",
    "../ProfileContent/Textures/Img0Grn",
    "../ProfileContent/Textures/Img1Grn",
    "../ProfileContent/Textures/Img2Grn",
    "../ProfileContent/Textures/Img3Grn",
    
    "../ProfileContent/Textures/Img0",
    "../ProfileContent/Textures/Img1",
    "../ProfileContent/Textures/Img2",
    "../ProfileContent/Textures/Img3",
    "../ProfileContent/Textures/Img0Blu",
    "../ProfileContent/Textures/Img1Blu",
    "../ProfileContent/Textures/Img2Blu",
    "../ProfileContent/Textures/Img3Blu",
    "../ProfileContent/Textures/Img0Brn",
    "../ProfileContent/Textures/Img1Brn",
    "../ProfileContent/Textures/Img2Brn",
    "../ProfileContent/Textures/Img3Brn",
    "../ProfileContent/Textures/Img0Grn",
    "../ProfileContent/Textures/Img1Grn",
    "../ProfileContent/Textures/Img2Grn",
    "../ProfileContent/Textures/Img3Grn",
    #endif
};

constexpr static std::array<const char *,1> FileNamesArray[NUM_ACTUAL_MODELS] =
{
    {"../ProfileContent/Models/ProfModels_Cone"},
    {"../ProfileContent/Models/ProfModels_Cylinder"},
    {"../ProfileContent/Models/ProfModels_Sphere"},
    {"../ProfileContent/Models/ProfModels_Suzanne"},
    {"../ProfileContent/Models/ProfModels_Torus"},
};

constexpr static std::array<const char *,1> ImageFileNamesArray[NUM_ACTUAL_IMAGES] =
{
    {"../ProfileContent/Textures/Img0"},
    {"../ProfileContent/Textures/Img1"},
    {"../ProfileContent/Textures/Img2"},
    {"../ProfileContent/Textures/Img3"},
    {"../ProfileContent/Textures/Img0Blu"},
    {"../ProfileContent/Textures/Img1Blu"},
    {"../ProfileContent/Textures/Img2Blu"},
    {"../ProfileContent/Textures/Img3Blu"},
    {"../ProfileContent/Textures/Img0Brn"},
    {"../ProfileContent/Textures/Img1Brn"},
    {"../ProfileContent/Textures/Img2Brn"},
    {"../ProfileContent/Textures/Img3Brn"},
    {"../ProfileContent/Textures/Img0Grn"},
    {"../ProfileContent/Textures/Img1Grn"},
    {"../ProfileContent/Textures/Img2Grn"},
    {"../ProfileContent/Textures/Img3Grn"},
};

struct lvkProfFileLoaderArray
{
    lvkFileMeshGroupLoader MeshGroupLoader[NUM_MODELS] = {
        {FileNamesArray[0].data(),FileNamesArray[0].size()},
        {FileNamesArray[1].data(),FileNamesArray[1].size()},
        {FileNamesArray[2].data(),FileNamesArray[2].size()},
        {FileNamesArray[3].data(),FileNamesArray[3].size()},
        {FileNamesArray[4].data(),FileNamesArray[4].size()},
        #ifdef LVK_PROF_BIG_INPUT
        {FileNamesArray[0].data(),FileNamesArray[0].size()},
        {FileNamesArray[1].data(),FileNamesArray[1].size()},
        {FileNamesArray[2].data(),FileNamesArray[2].size()},
        {FileNamesArray[3].data(),FileNamesArray[3].size()},
        {FileNamesArray[4].data(),FileNamesArray[4].size()},
        
        {FileNamesArray[0].data(),FileNamesArray[0].size()},
        {FileNamesArray[1].data(),FileNamesArray[1].size()},
        {FileNamesArray[2].data(),FileNamesArray[2].size()},
        {FileNamesArray[3].data(),FileNamesArray[3].size()},
        {FileNamesArray[4].data(),FileNamesArray[4].size()},
        
        {FileNamesArray[0].data(),FileNamesArray[0].size()},
        {FileNamesArray[1].data(),FileNamesArray[1].size()},
        {FileNamesArray[2].data(),FileNamesArray[2].size()},
        {FileNamesArray[3].data(),FileNamesArray[3].size()},
        {FileNamesArray[4].data(),FileNamesArray[4].size()},
        
        {FileNamesArray[0].data(),FileNamesArray[0].size()},
        {FileNamesArray[1].data(),FileNamesArray[1].size()},
        {FileNamesArray[2].data(),FileNamesArray[2].size()},
        {FileNamesArray[3].data(),FileNamesArray[3].size()},
        {FileNamesArray[4].data(),FileNamesArray[4].size()},
        
        {FileNamesArray[0].data(),FileNamesArray[0].size()},
        {FileNamesArray[1].data(),FileNamesArray[1].size()},
        {FileNamesArray[2].data(),FileNamesArray[2].size()},
        {FileNamesArray[3].data(),FileNamesArray[3].size()},
        {FileNamesArray[4].data(),FileNamesArray[4].size()},
        
        {FileNamesArray[0].data(),FileNamesArray[0].size()},
        {FileNamesArray[1].data(),FileNamesArray[1].size()},
        {FileNamesArray[2].data(),FileNamesArray[2].size()},
        {FileNamesArray[3].data(),FileNamesArray[3].size()},
        {FileNamesArray[4].data(),FileNamesArray[4].size()},
        
        {FileNamesArray[0].data(),FileNamesArray[0].size()},
        {FileNamesArray[1].data(),FileNamesArray[1].size()},
        {FileNamesArray[2].data(),FileNamesArray[2].size()},
        {FileNamesArray[3].data(),FileNamesArray[3].size()},
        {FileNamesArray[4].data(),FileNamesArray[4].size()},
        #endif
    };
    
    lvkFileImageGroupLoader ImageGroupLoader[NUM_IMAGES] = {
        {ImageFileNamesArray[0].data(),ImageFileNamesArray[0].size()},
        {ImageFileNamesArray[1].data(),ImageFileNamesArray[1].size()},
        {ImageFileNamesArray[2].data(),ImageFileNamesArray[2].size()},
        {ImageFileNamesArray[3].data(),ImageFileNamesArray[3].size()},
        {ImageFileNamesArray[4].data(),ImageFileNamesArray[4].size()},
        {ImageFileNamesArray[5].data(),ImageFileNamesArray[5].size()},
        {ImageFileNamesArray[6].data(),ImageFileNamesArray[6].size()},
        {ImageFileNamesArray[7].data(),ImageFileNamesArray[7].size()},
        {ImageFileNamesArray[8].data(),ImageFileNamesArray[8].size()},
        {ImageFileNamesArray[9].data(),ImageFileNamesArray[9].size()},
        {ImageFileNamesArray[10].data(),ImageFileNamesArray[10].size()},
        {ImageFileNamesArray[11].data(),ImageFileNamesArray[11].size()},
        {ImageFileNamesArray[12].data(),ImageFileNamesArray[12].size()},
        {ImageFileNamesArray[13].data(),ImageFileNamesArray[13].size()},
        {ImageFileNamesArray[14].data(),ImageFileNamesArray[14].size()},
        {ImageFileNamesArray[15].data(),ImageFileNamesArray[15].size()},
        #ifdef LVK_PROF_BIG_INPUT
        {ImageFileNamesArray[0].data(),ImageFileNamesArray[0].size()},
        {ImageFileNamesArray[1].data(),ImageFileNamesArray[1].size()},
        {ImageFileNamesArray[2].data(),ImageFileNamesArray[2].size()},
        {ImageFileNamesArray[3].data(),ImageFileNamesArray[3].size()},
        {ImageFileNamesArray[4].data(),ImageFileNamesArray[4].size()},
        {ImageFileNamesArray[5].data(),ImageFileNamesArray[5].size()},
        {ImageFileNamesArray[6].data(),ImageFileNamesArray[6].size()},
        {ImageFileNamesArray[7].data(),ImageFileNamesArray[7].size()},
        {ImageFileNamesArray[8].data(),ImageFileNamesArray[8].size()},
        {ImageFileNamesArray[9].data(),ImageFileNamesArray[9].size()},
        {ImageFileNamesArray[10].data(),ImageFileNamesArray[10].size()},
        {ImageFileNamesArray[11].data(),ImageFileNamesArray[11].size()},
        {ImageFileNamesArray[12].data(),ImageFileNamesArray[12].size()},
        {ImageFileNamesArray[13].data(),ImageFileNamesArray[13].size()},
        {ImageFileNamesArray[14].data(),ImageFileNamesArray[14].size()},
        {ImageFileNamesArray[15].data(),ImageFileNamesArray[15].size()},
        
        {ImageFileNamesArray[0].data(),ImageFileNamesArray[0].size()},
        {ImageFileNamesArray[1].data(),ImageFileNamesArray[1].size()},
        {ImageFileNamesArray[2].data(),ImageFileNamesArray[2].size()},
        {ImageFileNamesArray[3].data(),ImageFileNamesArray[3].size()},
        {ImageFileNamesArray[4].data(),ImageFileNamesArray[4].size()},
        {ImageFileNamesArray[5].data(),ImageFileNamesArray[5].size()},
        {ImageFileNamesArray[6].data(),ImageFileNamesArray[6].size()},
        {ImageFileNamesArray[7].data(),ImageFileNamesArray[7].size()},
        {ImageFileNamesArray[8].data(),ImageFileNamesArray[8].size()},
        {ImageFileNamesArray[9].data(),ImageFileNamesArray[9].size()},
        {ImageFileNamesArray[10].data(),ImageFileNamesArray[10].size()},
        {ImageFileNamesArray[11].data(),ImageFileNamesArray[11].size()},
        {ImageFileNamesArray[12].data(),ImageFileNamesArray[12].size()},
        {ImageFileNamesArray[13].data(),ImageFileNamesArray[13].size()},
        {ImageFileNamesArray[14].data(),ImageFileNamesArray[14].size()},
        {ImageFileNamesArray[15].data(),ImageFileNamesArray[15].size()},
        
        {ImageFileNamesArray[0].data(),ImageFileNamesArray[0].size()},
        {ImageFileNamesArray[1].data(),ImageFileNamesArray[1].size()},
        {ImageFileNamesArray[2].data(),ImageFileNamesArray[2].size()},
        {ImageFileNamesArray[3].data(),ImageFileNamesArray[3].size()},
        {ImageFileNamesArray[4].data(),ImageFileNamesArray[4].size()},
        {ImageFileNamesArray[5].data(),ImageFileNamesArray[5].size()},
        {ImageFileNamesArray[6].data(),ImageFileNamesArray[6].size()},
        {ImageFileNamesArray[7].data(),ImageFileNamesArray[7].size()},
        {ImageFileNamesArray[8].data(),ImageFileNamesArray[8].size()},
        {ImageFileNamesArray[9].data(),ImageFileNamesArray[9].size()},
        {ImageFileNamesArray[10].data(),ImageFileNamesArray[10].size()},
        {ImageFileNamesArray[11].data(),ImageFileNamesArray[11].size()},
        {ImageFileNamesArray[12].data(),ImageFileNamesArray[12].size()},
        {ImageFileNamesArray[13].data(),ImageFileNamesArray[13].size()},
        {ImageFileNamesArray[14].data(),ImageFileNamesArray[14].size()},
        {ImageFileNamesArray[15].data(),ImageFileNamesArray[15].size()},
        
        {ImageFileNamesArray[0].data(),ImageFileNamesArray[0].size()},
        {ImageFileNamesArray[1].data(),ImageFileNamesArray[1].size()},
        {ImageFileNamesArray[2].data(),ImageFileNamesArray[2].size()},
        {ImageFileNamesArray[3].data(),ImageFileNamesArray[3].size()},
        {ImageFileNamesArray[4].data(),ImageFileNamesArray[4].size()},
        {ImageFileNamesArray[5].data(),ImageFileNamesArray[5].size()},
        {ImageFileNamesArray[6].data(),ImageFileNamesArray[6].size()},
        {ImageFileNamesArray[7].data(),ImageFileNamesArray[7].size()},
        {ImageFileNamesArray[8].data(),ImageFileNamesArray[8].size()},
        {ImageFileNamesArray[9].data(),ImageFileNamesArray[9].size()},
        {ImageFileNamesArray[10].data(),ImageFileNamesArray[10].size()},
        {ImageFileNamesArray[11].data(),ImageFileNamesArray[11].size()},
        {ImageFileNamesArray[12].data(),ImageFileNamesArray[12].size()},
        {ImageFileNamesArray[13].data(),ImageFileNamesArray[13].size()},
        {ImageFileNamesArray[14].data(),ImageFileNamesArray[14].size()},
        {ImageFileNamesArray[15].data(),ImageFileNamesArray[15].size()},
        
        {ImageFileNamesArray[0].data(),ImageFileNamesArray[0].size()},
        {ImageFileNamesArray[1].data(),ImageFileNamesArray[1].size()},
        {ImageFileNamesArray[2].data(),ImageFileNamesArray[2].size()},
        {ImageFileNamesArray[3].data(),ImageFileNamesArray[3].size()},
        {ImageFileNamesArray[4].data(),ImageFileNamesArray[4].size()},
        {ImageFileNamesArray[5].data(),ImageFileNamesArray[5].size()},
        {ImageFileNamesArray[6].data(),ImageFileNamesArray[6].size()},
        {ImageFileNamesArray[7].data(),ImageFileNamesArray[7].size()},
        {ImageFileNamesArray[8].data(),ImageFileNamesArray[8].size()},
        {ImageFileNamesArray[9].data(),ImageFileNamesArray[9].size()},
        {ImageFileNamesArray[10].data(),ImageFileNamesArray[10].size()},
        {ImageFileNamesArray[11].data(),ImageFileNamesArray[11].size()},
        {ImageFileNamesArray[12].data(),ImageFileNamesArray[12].size()},
        {ImageFileNamesArray[13].data(),ImageFileNamesArray[13].size()},
        {ImageFileNamesArray[14].data(),ImageFileNamesArray[14].size()},
        {ImageFileNamesArray[15].data(),ImageFileNamesArray[15].size()},
        
        {ImageFileNamesArray[0].data(),ImageFileNamesArray[0].size()},
        {ImageFileNamesArray[1].data(),ImageFileNamesArray[1].size()},
        {ImageFileNamesArray[2].data(),ImageFileNamesArray[2].size()},
        {ImageFileNamesArray[3].data(),ImageFileNamesArray[3].size()},
        {ImageFileNamesArray[4].data(),ImageFileNamesArray[4].size()},
        {ImageFileNamesArray[5].data(),ImageFileNamesArray[5].size()},
        {ImageFileNamesArray[6].data(),ImageFileNamesArray[6].size()},
        {ImageFileNamesArray[7].data(),ImageFileNamesArray[7].size()},
        {ImageFileNamesArray[8].data(),ImageFileNamesArray[8].size()},
        {ImageFileNamesArray[9].data(),ImageFileNamesArray[9].size()},
        {ImageFileNamesArray[10].data(),ImageFileNamesArray[10].size()},
        {ImageFileNamesArray[11].data(),ImageFileNamesArray[11].size()},
        {ImageFileNamesArray[12].data(),ImageFileNamesArray[12].size()},
        {ImageFileNamesArray[13].data(),ImageFileNamesArray[13].size()},
        {ImageFileNamesArray[14].data(),ImageFileNamesArray[14].size()},
        {ImageFileNamesArray[15].data(),ImageFileNamesArray[15].size()},
        
        {ImageFileNamesArray[0].data(),ImageFileNamesArray[0].size()},
        {ImageFileNamesArray[1].data(),ImageFileNamesArray[1].size()},
        {ImageFileNamesArray[2].data(),ImageFileNamesArray[2].size()},
        {ImageFileNamesArray[3].data(),ImageFileNamesArray[3].size()},
        {ImageFileNamesArray[4].data(),ImageFileNamesArray[4].size()},
        {ImageFileNamesArray[5].data(),ImageFileNamesArray[5].size()},
        {ImageFileNamesArray[6].data(),ImageFileNamesArray[6].size()},
        {ImageFileNamesArray[7].data(),ImageFileNamesArray[7].size()},
        {ImageFileNamesArray[8].data(),ImageFileNamesArray[8].size()},
        {ImageFileNamesArray[9].data(),ImageFileNamesArray[9].size()},
        {ImageFileNamesArray[10].data(),ImageFileNamesArray[10].size()},
        {ImageFileNamesArray[11].data(),ImageFileNamesArray[11].size()},
        {ImageFileNamesArray[12].data(),ImageFileNamesArray[12].size()},
        {ImageFileNamesArray[13].data(),ImageFileNamesArray[13].size()},
        {ImageFileNamesArray[14].data(),ImageFileNamesArray[14].size()},
        {ImageFileNamesArray[15].data(),ImageFileNamesArray[15].size()},
        #endif
    };
};

class lvkProfRenderCounter
{
private:
    constexpr static unsigned int FRAME_CNT = 40;
    unsigned int CurrentFrame = 0;
    
    float CmdLastAverage  = 0;
    float CmdAccumulator  = 0;
    float ExecLastAverage = 0;
    float ExecAccumulator = 0;
    
    std::chrono::steady_clock::time_point BeforeRecording;
    std::chrono::steady_clock::time_point AfterRecording;
    std::chrono::steady_clock::time_point AfterSubmission;
    
public:
    
    float GetCmdRecord() {return std::chrono::duration<float,std::milli>(AfterRecording  - BeforeRecording).count();}
    float GetCmdAverage() {return CmdLastAverage;}
    float GetExec() {return std::chrono::duration<float,std::milli>(AfterSubmission - AfterRecording ).count();}
    float GetExecAverage() {return ExecLastAverage;}
    
    void BeginRecording()
    {
        BeforeRecording = std::chrono::steady_clock::now();
    }
    
    void FinishRecording()
    {
        AfterRecording = std::chrono::steady_clock::now();
    }
    
    void FinishSubmission()
    {
        AfterSubmission = std::chrono::steady_clock::now();
        
        CmdAccumulator  += std::chrono::duration<float,std::milli>(AfterRecording  - BeforeRecording).count();
        ExecAccumulator += std::chrono::duration<float,std::milli>(AfterSubmission - AfterRecording ).count();
        
        CurrentFrame++;
        if(CurrentFrame == FRAME_CNT)
        {
            CmdLastAverage  = CmdAccumulator  / FRAME_CNT;
            CmdAccumulator  = 0;
            
            ExecLastAverage = ExecAccumulator / FRAME_CNT;
            ExecAccumulator = 0;
            
            CurrentFrame = 0;
        }
    }
    
    void Print()
    {
        std::cout << "Cmd buffer recording time: " << std::chrono::duration<float,std::milli>(AfterRecording - BeforeRecording).count() << "ms" << std::endl;
        std::cout << "Last average recording time: " << CmdLastAverage << "ms" << std::endl;
        
        std::cout << "Execution time: " << std::chrono::duration<float,std::milli>(AfterSubmission - AfterRecording).count() << "ms" << std::endl;
        std::cout << "Last average execution time: " << ExecLastAverage << "ms" << std::endl;
    }
};

struct lvkProfOptMeshDrawNode : public lrResourceNode
{
    struct lvkSingleDraw
    {
        lvkDynamicDescriptorSet  MeshDescriptors;
        lrDynamicUniformBindings MeshUbos;
        lrMeshDrawCall           MeshDrawCalls;
    };
    
    std::size_t DrawCount = 0;
    std::vector<lvkSingleDraw> Draws;
    
    virtual void Accept(liResourceHierarchyVisitor &visitor) const override
    {
        std::size_t Counter = 0;
        for(const auto &Draw : Draws)
        {
            if(Counter == DrawCount)
                {break;}
            Counter++;
            
            visitor.Visit(Draw.MeshUbos);
            visitor.Visit(Draw.MeshDrawCalls);
        }
    }
};

#endif // LVK_PROFILE_UTILS_H
