#include "vulkan_shader_utils.h"

#include "core/dstring.h"
#include "core/logger.h"
#include "core/dmemory.h"

#include "platform/filesystem.h"

b8 CreateShaderModule(VulkanContext* context, char* name, char* type_str, 
                      VkShaderStageFlagBits stage_flag, u32 stage_index, VulkanShaderStage* shader_stages){
    char file_name[512];
    StringFormat(file_name, "assets/shaders/%s.%s.spv", name, type_str);

    DZeroMemory(&shader_stages[stage_index].create_info, sizeof(VkShaderModuleCreateInfo));
    shader_stages[stage_index].create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    FileHandle handle = {};
    if (!FileSystemOpen(file_name, FILE_MODE_READ, true, &handle)) {
        DERROR("Unable to open read shader module: %s.", file_name);
        return false;
    }

    u64 size = 0;
    u8* file_buffer = 0;
    if (!FileSystemReadAllBytes(&handle, &file_buffer, &size)) {
        DERROR("Unable to binary read shader module: %s.", file_name);
        return false;
    }

    shader_stages[stage_index].create_info.codeSize = size;
    shader_stages[stage_index].create_info.pCode = (u32*)file_buffer;
    FileSystemClose(&handle);

    VK_CHECK(vkCreateShaderModule(context->device.logical_device, 
                                  &shader_stages[stage_index].create_info,
                                  context->allocator,
                                  &shader_stages[stage_index].handle));
    
    DZeroMemory(&shader_stages[stage_index].shader_stage_create_info, sizeof(VkPipelineShaderStageCreateInfo));
    shader_stages[stage_index].shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[stage_index].shader_stage_create_info.stage = stage_flag;
    shader_stages[stage_index].shader_stage_create_info.module = shader_stages[stage_index].handle;
    //below is the entry point into the shader so just has to match whatever you have as the entry point in the shader
    shader_stages[stage_index].shader_stage_create_info.pName = "main";    

    if(file_buffer){
        DFree(file_buffer, sizeof(u8) * size, MEMORY_TAG_STRING);
        file_buffer = 0;
    }

    return true;
}