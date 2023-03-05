#include "vulkan_object_shader.h"

#include "core/logger.h"
#include "math/math_types.h"
#include "math/dmath.h"
#include "core/dmemory.h"

#include "renderer/vulkan/vulkan_pipeline.h"
#include "renderer/vulkan/vulkan_shader_utils.h"
#include "renderer/vulkan/vulkan_buffer.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 VulkanObjectShaderCreate(VulkanContext* context, VulkanObjectShader* out_shader) {
    char stage_type_strings[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stage_types[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT,
                                                                    VK_SHADER_STAGE_FRAGMENT_BIT};
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++) {
        if (!CreateShaderModule(context, BUILTIN_SHADER_NAME_OBJECT, stage_type_strings[i], stage_types[i], i, out_shader->stages)) {
            DERROR("Unable to create %s shader module for '%s'.", stage_type_strings[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    //Global descriptors
    VkDescriptorSetLayoutBinding global_ubo_layout_binding = {};
    global_ubo_layout_binding.binding = 0;
    global_ubo_layout_binding.descriptorCount = 1;
    global_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_ubo_layout_binding.pImmutableSamplers = 0;
    global_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo global_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    global_layout_info.bindingCount = 1;
    global_layout_info.pBindings = &global_ubo_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device.logical_device, &global_layout_info,
                                        context->allocator, &out_shader->global_descriptor_set_layout));
    
    //Global descriptor pool used for global items like view/projection matrix
    VkDescriptorPoolSize global_pool_size = {};
    global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_pool_size.descriptorCount = context->swapchain.image_count;

    VkDescriptorPoolCreateInfo global_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    global_pool_info.poolSizeCount = 1; //number of size objects passed in, not number of sets in the pool
    global_pool_info.pPoolSizes = &global_pool_size;
    global_pool_info.maxSets = context->swapchain.image_count;
    VK_CHECK(vkCreateDescriptorPool(context->device.logical_device, &global_pool_info,
                                    context->allocator, &out_shader->global_descriptor_pool));

    //Local/object descriptors
    const u32 local_sampler_count = 1;
    VkDescriptorType descriptor_types[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, //Binding 0: uniform buffer
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER //Binding 1: diffuse sampler layout
    };
    VkDescriptorSetLayoutBinding bindings[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT] = {};
    for (u32 i = 0; i < VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT; i++) {
        bindings[i].binding = i;        
        bindings[i].descriptorCount = 1;
        bindings[i].descriptorType = descriptor_types[i];
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layout_info.bindingCount = VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT;
    layout_info.pBindings = bindings;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device.logical_device, &layout_info, 0, &out_shader->object_descriptor_set_layout));

    //Local/object descriptor pool: used for object specific items like diffuse color
    VkDescriptorPoolSize object_pool_sizes[2];
    //The first section will be used for uniform buffers
    object_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    object_pool_sizes[0].descriptorCount = VULKAN_OBJECT_MAX_OBJECT_COUNT;
    //Second section used for image samplers
    object_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    object_pool_sizes[1].descriptorCount = local_sampler_count * VULKAN_OBJECT_MAX_OBJECT_COUNT;

    VkDescriptorPoolCreateInfo object_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    object_pool_info.poolSizeCount = 2;
    object_pool_info.pPoolSizes = object_pool_sizes;
    object_pool_info.maxSets = VULKAN_OBJECT_MAX_OBJECT_COUNT;

    //Create object descriptor pool
    VK_CHECK(vkCreateDescriptorPool(context->device.logical_device, &object_pool_info, context->allocator, &out_shader->object_descriptor_pool));

    //Pipeline generation
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = (f32)context->frame_buffer_height;    
    viewport.width = (f32)context->frame_buffer_width;
    viewport.height = -(f32)context->frame_buffer_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //Scissor
    VkRect2D scissor = {};
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context->frame_buffer_width;
    scissor.extent.height = context->frame_buffer_height;

    //Attributes
    u32 offset = 0;
#define ATTRIBUTE_COUNT 2
    VkVertexInputAttributeDescription attribute_descriptions[ATTRIBUTE_COUNT];
    //Position, texcoord
    VkFormat formats[ATTRIBUTE_COUNT] = {
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32_SFLOAT
    };
    u64 sizes[ATTRIBUTE_COUNT] = {
        sizeof(Vec3),
        sizeof(Vec2)
    };
    for (u32 i = 0; i < ATTRIBUTE_COUNT; i++) {
        attribute_descriptions[i].binding = 0; //should match binding description
        attribute_descriptions[i].location = i; //attrib location
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    //Descriptor set layouts
    const i32 descriptor_set_layout_count = 2;
    VkDescriptorSetLayout layouts[descriptor_set_layout_count] = {
        out_shader->global_descriptor_set_layout,
        out_shader->object_descriptor_set_layout
    };

    //Stages (should match the number of shader->stages)
    VkPipelineShaderStageCreateInfo stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
    DZeroMemory(stage_create_infos, sizeof(stage_create_infos));
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++){
        stage_create_infos[i].sType = out_shader->stages[i].shader_stage_create_info.sType;
        stage_create_infos[i] = out_shader->stages[i].shader_stage_create_info;
    }

    if (!VulkanGraphicsPipelineCreate(context, &context->main_render_pass, ATTRIBUTE_COUNT,
                                      attribute_descriptions, descriptor_set_layout_count, layouts,
                                      OBJECT_SHADER_STAGE_COUNT, stage_create_infos, viewport,
                                      scissor, false, &out_shader->pipeline)) {
        DERROR("Failed to load graphics pipeline for object shader.");
        return false;
    }

    //Create uniform buffer
    if (!VulkanBufferCreate(context, sizeof(GlobalUniformObject)*3,
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           true, &out_shader->global_uniform_buffer)) {
        DERROR("Vulkan buffer creation failed for object shader.");
        return false;
    }

    //Allocate global descriptor sets
    VkDescriptorSetLayout global_layouts[3] = {
        out_shader->global_descriptor_set_layout,
        out_shader->global_descriptor_set_layout,
        out_shader->global_descriptor_set_layout
    };

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = out_shader->global_descriptor_pool;
    alloc_info.descriptorSetCount = 3;
    alloc_info.pSetLayouts = global_layouts;
    VK_CHECK(vkAllocateDescriptorSets(context->device.logical_device, &alloc_info, out_shader->global_descriptor_sets));

    if (!VulkanBufferCreate(context, sizeof(ObjectUniformObject)*3,
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           true, &out_shader->object_uniform_buffer)) {
        DERROR("Material instance buffer creation failed for shader.");
        return false;
    }


    return true;
}

void VulkanObjectShaderDestroy(VulkanContext* context, VulkanObjectShader* shader) {
    VkDevice logical_device = context->device.logical_device;

    vkDestroyDescriptorPool(logical_device, shader->object_descriptor_pool, context->allocator);
    vkDestroyDescriptorSetLayout(logical_device, shader->object_descriptor_set_layout, context->allocator);

    VulkanBufferDestroy(context, &shader->global_uniform_buffer);
    VulkanBufferDestroy(context, &shader->object_uniform_buffer);

    VulkanPipelineDestroy(context, &shader->pipeline);

    vkDestroyDescriptorPool(logical_device, shader->global_descriptor_pool, context->allocator);

    vkDestroyDescriptorSetLayout(logical_device, shader->global_descriptor_set_layout, context->allocator);

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++) {
        vkDestroyShaderModule(context->device.logical_device, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = 0;
    }
}

void VulkanObjectShaderUse(VulkanContext* context, VulkanObjectShader* shader) {
    u32 image_index = context->image_index;
    VulkanPipelineBind(&context->graphics_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}

void VulkanObjectShaderUpdateGlobalState(VulkanContext* context, VulkanObjectShader* shader, f32 delta_time) {
    u32 image_index = context->image_index;
    VkCommandBuffer command_buffer = context->graphics_command_buffers[image_index].handle;
    VkDescriptorSet global_descriptor = shader->global_descriptor_sets[image_index];

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            shader->pipeline.pipeline_layout, 0, 1, &global_descriptor, 0, 0);
    
    /*
    TODO: The commented out pieces are for cards that cant "set after bind" on descriptors
          To roll back to that working Bind needs to go after the set
          I just dont know when to flip the descriptor_updated flag 
    */

    //if (!shader->descriptor_updated[image_index]) {
        //configure the descirptors for the given index
        u32 range = sizeof(GlobalUniformObject);
        u64 offset = sizeof(GlobalUniformObject) * image_index;
        //Copy data to buffer
        VulkanBufferLoadData(context, &shader->global_uniform_buffer, offset, range, 0, &shader->global_ubo);

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = shader->global_uniform_buffer.handle;
        buffer_info.offset = offset;
        buffer_info.range = range;

        //Update descriptor sets
        VkWriteDescriptorSet descriptor_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptor_write.dstSet = shader->global_descriptor_sets[image_index];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(context->device.logical_device, 1, &descriptor_write, 0, 0);
        //shader->descriptor_updated[image_index] = true;
    //}
    //bind global descriptor set to be updated
    //vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
    //                        shader->pipeline.pipeline_layout, 0, 1, &global_descriptor, 0, 0);
}

void VulkanObjectShaderUpdateObject(VulkanContext* context, VulkanObjectShader* shader, GeometryRenderData data) {
    u32 image_index = context->image_index;
    VkCommandBuffer command_buffer = context->graphics_command_buffers[image_index].handle;

    vkCmdPushConstants(command_buffer, shader->pipeline.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &data.model);

    //Obtain material data
    VulkanObjectShaderObjectState* object_state = &shader->object_states[data.object_id];
    VkDescriptorSet object_descriptor_set = object_state->descriptor_sets[image_index];

    //TODO: check if update is needed
    VkWriteDescriptorSet descriptor_writes[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT] = {};
    u32 descriptor_count = 0;
    u32 descriptor_index = 0;

    //descriptor 0 is uniform buffer
    u32 range = sizeof(ObjectUniformObject);
    u64 offset = sizeof(ObjectUniformObject) * data.object_id; //also the index into the array
    ObjectUniformObject obo = {};

    //TODO: get diffuse color from a material
    static f32 accumulator = 0.0f;
    accumulator += context->frame_delta_time;
    f32 s = (dsin(accumulator) + 1.0f) / 2.0f; //scale from [-1,1] to [0,1]
    obo.diffuse_color = {s, s, s, 1.0f};

    //Load the data into the buffer
    VulkanBufferLoadData(context, &shader->object_uniform_buffer, offset, range, 0, &obo);

    //Only do this if the descriptor set has not been updated yet
    if (object_state->descriptor_states[descriptor_index].generations[image_index] == INVALID_ID) {
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = shader->object_uniform_buffer.handle;
        buffer_info.offset = offset;
        buffer_info.range = range;

        VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptor.dstSet = object_descriptor_set;
        descriptor.dstBinding = descriptor_index;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor.descriptorCount = 1;
        descriptor.pBufferInfo = &buffer_info;

        descriptor_writes[descriptor_count] = descriptor;
        descriptor_count++;

        //Update the frame generation. In this case it is only needed once since this is a buffer
        object_state->descriptor_states[descriptor_index].generations[image_index] = 1;
    }
    descriptor_index++;

    //TODO: samplers
    const u32 sampler_count = 1;
    VkDescriptorImageInfo image_infos[1];
    for (u32 sampler_index = 0; sampler_index < sampler_count; sampler_index++) {
        Texture* t = data.textures[sampler_index];
        u32* descriptor_generation = &object_state->descriptor_states[descriptor_index].generations[image_index];

        //Check if descriptor needs updating first
        if (t && (*descriptor_generation != t->generation || *descriptor_generation == INVALID_ID)) {
            VulkanTextureData* internal_data = (VulkanTextureData*)t->internal_data;
            //Assign view and sampler
            image_infos[sampler_index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_infos[sampler_index].imageView = internal_data->image.view;
            image_infos[sampler_index].sampler = internal_data->sampler;

            VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            descriptor.dstSet = object_descriptor_set;
            descriptor.dstBinding = descriptor_index;
            descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor.descriptorCount = 1;
            descriptor.pImageInfo = &image_infos[sampler_index];

            descriptor_writes[descriptor_count] = descriptor;
            descriptor_count++;
            //Sync frame generation if not using a default texture
            if (t->generation != INVALID_ID) {
                *descriptor_generation = t->generation;
            }
            descriptor_index++;
        }
    }

    if (descriptor_count > 0) {
        vkUpdateDescriptorSets(context->device.logical_device, descriptor_count, descriptor_writes, 0, 0);
    }

    //Bind the descriptor set to be updated or in case the shader changed
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipeline_layout, 1, 1, &object_descriptor_set, 0, 0);
}


b8 VulkanObjectShaderAcquireResources(VulkanContext* context, VulkanObjectShader* shader, u32* out_object_id) {
    //TODO: free list
    *out_object_id = shader->object_uniform_buffer_index;
    shader->object_uniform_buffer_index++;

    u32 object_id = *out_object_id;
    VulkanObjectShaderObjectState* object_state = &shader->object_states[object_id];
    for (u32 i = 0; i < VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT; i++) {
        for (u32 j = 0; j < 3; j++) {
            object_state->descriptor_states[i].generations[j] = INVALID_ID;
        }
    }
    //Allocate descriptor sets
    VkDescriptorSetLayout layouts[3] {
        shader->object_descriptor_set_layout,
        shader->object_descriptor_set_layout,
        shader->object_descriptor_set_layout
    };

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = shader->object_descriptor_pool;
    alloc_info.descriptorSetCount = 3; //one per frame
    alloc_info.pSetLayouts = layouts;
    VkResult result = vkAllocateDescriptorSets(context->device.logical_device, &alloc_info, object_state->descriptor_sets);
    if (result != VK_SUCCESS) {
        DERROR("Error allocating descriptor sets in shader");
        return false;
    }

    return true;
}

void VulkanObjectShaderReleaseResources(VulkanContext* context, VulkanObjectShader* shader, u32 object_id) {
    VulkanObjectShaderObjectState* object_state = &shader->object_states[object_id];

    const u32 descriptor_set_count = 3;
    //Release object descriptor sets
    VkResult result = vkFreeDescriptorSets(context->device.logical_device, shader->object_descriptor_pool, descriptor_set_count, object_state->descriptor_sets);
    if (result != VK_SUCCESS) {
        DERROR("Error freeing object shader descriptor sets");
    }

    for (u32 i = 0; i < VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT; i++) {
        for (u32 j = 0; j < 3; j++) {
            object_state->descriptor_states[i].generations[j] = INVALID_ID;
        }
    }
    //TODO: add object_id to free list
}