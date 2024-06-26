#include "skeletal_animation_scene.h"

SkeletalAnimationScene::SkeletalAnimationScene()
	: Scene(), renderModelShader(nullptr), model(nullptr)
{
}

void SkeletalAnimationScene::setup()
{
	uint32_t modelLoaderFlags = aiProcess_Triangulate;

	renderModelShader = new ShaderProgram("sources/shaders/1_render_model_vs.glsl", "sources/shaders/1_render_model_fs.glsl");
	model = new Model("resources/models/vampire/dancing_vampire.dae", modelLoaderFlags);
}

void SkeletalAnimationScene::clean()
{
	renderModelShader->clean();
	model->clean();

	delete renderModelShader;
	delete model;
}

void SkeletalAnimationScene::update(float deltaTime)
{
	model->animator.update(deltaTime);
}

void SkeletalAnimationScene::render(const Camera& camera, float deltaTime)
{
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 modelMatrix(1.0f);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.0001f));

	renderModelShader->bind();

	renderModelShader->setUniformMatrix4fv("uProjectionMatrix", camera.getProjectionMatrix());
	renderModelShader->setUniformMatrix4fv("uViewMatrix", camera.getViewMatrix());
	renderModelShader->setUniformMatrix4fv("uModelMatrix", modelMatrix);

	const std::vector<glm::mat4>& transforms = model->animator.getBonesMatrices();

	for (uint32_t index = 0; index < transforms.size(); index++)
	{
		renderModelShader->setUniformMatrix4fv(("uBonesMatrices[" + std::to_string(index) + "]").c_str(), transforms[index]);
	}

	model->render(renderModelShader);

	renderModelShader->unbind();
}

void SkeletalAnimationScene::processGUI()
{
	bool dialogOpen = true;
	ImGui::Begin("Skeletal Animation Dialog", &dialogOpen, ImGuiWindowFlags_MenuBar);

	ImGui::SeparatorText("Model");

	const std::vector<Animation>& animations = model->animator.getAnimations();

	const char** comboItems = new const char*[animations.size()];
	static int comboSelectedItem = model->animator.getCurrAnimation();

	for (uint32_t i = 0; i < animations.size(); i++)
	{
		comboItems[i] = animations[i].getName().c_str();
	}

	ImGui::Combo("Animations", &comboSelectedItem, comboItems, animations.size());
	{
		if (comboSelectedItem != model->animator.getCurrAnimation())
		{
			model->animator.execAnimation(comboSelectedItem);
		}
	}

	ImGui::End();
}
