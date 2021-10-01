#include "Sky.hpp"

using namespace std;
using namespace Crane;

void Sky::init(Render *ctx)
{
	Actor::init(ctx);

	LOGI("��������")
	{
		context->loadMeshs["Sky"] = make_shared<Plane>(2, 2);
		mesh = context->loadMeshs["Sky"];
		mesh->setVertices([](uint32_t i, Vertex& v) {v.position.y() = v.position.z(); v.position.z() = 1.0f; });
	}

	LOGI("��������")
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

	LOGI("�������ʹ���")
	{
		materialBuilder.descriptorPool = context->descriptorPool.get();
		materialBuilder.pipelinePass = &pipelinePass;

		materialBuilder.descriptorInfos[0][0].first = &context->sceneParameterBufferDescriptorInfo;
	}

	LOGI("��������")
	{
		context->materials["Sky"] = materialBuilder.build();
		material = &context->materials["Sky"];
	}
}