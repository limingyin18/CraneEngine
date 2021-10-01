#include "Sky.hpp"

using namespace std;
using namespace Crane;

void Sky::init(Render *ctx)
{
	Actor::init(ctx);

	LOGI("创建网格")
	{
		context->loadMeshs["Sky"] = make_shared<Plane>(2, 2);
		mesh = context->loadMeshs["Sky"];
		mesh->setVertices([](uint32_t i, Vertex& v) {v.position.y() = v.position.z(); v.position.z() = 1.0f; });
	}

	LOGI("创建管线")
	{
		pipelinePass.device = context->device.get();
		pipelinePass.renderPass = context->renderPass.get();

		auto shaderCodeVertSky = Loader::readFile("shaders/sky.vert.spv");
		pipelinePass.addShader(shaderCodeVertSky, vk::ShaderStageFlagBits::eVertex);
		auto shaderCodeFragSky = Loader::readFile("shaders/sky.frag.spv");
		pipelinePass.addShader(shaderCodeFragSky, vk::ShaderStageFlagBits::eFragment);
		pipelinePass.bindings[0][0].descriptorType = vk::DescriptorType::eUniformBufferDynamic;

		pipelinePass.buildDescriptorSetLayout();

		pipelinePass.buildPipelineLayout();
		pipelinePass.buildPipeline(context->pipelineBuilder);
	}

	LOGI("创建材质工厂")
	{
		materialBuilder.descriptorPool = context->descriptorPool.get();
		materialBuilder.pipelinePass = &pipelinePass;

		materialBuilder.descriptorInfos[0][0].first = &context->sceneParameterBufferDescriptorInfo;
	}

	LOGI("创建材质")
	{
		context->materials["Sky"] = materialBuilder.build();
		material = &context->materials["Sky"];
	}
}