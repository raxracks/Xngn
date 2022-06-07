#include "pch.h"
#include "XngnMain.h"
#include "Common\DirectXHelper.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"

using namespace Xngn;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

bool showDemoWindow = true;
bool lockedFPS = false;

static int item_current_idx = 0; // Here we store our selection data as an index.

// Loads and initializes application assets when the application is loaded.
XngnMain::XngnMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<SceneRenderer>(new SceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	m_timer.SetTargetElapsedSeconds(1.0 / 60);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());

	Transform t = Transform{ 0, 0, -2.0f, 0.8f, 2.3f, 0, 1, 1, 1 };
	m_sceneRenderer->AddTransform(t);
	item_current_idx = m_sceneRenderer->TransformCount() - 1;
}

XngnMain::~XngnMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void XngnMain::CreateWindowSizeDependentResources()
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Updates the application state once per frame.
void XngnMain::Update()
{
	// Update scene objects.
	m_timer.Tick([&]()
		{
			// TODO: Replace this with your app's content update functions.
			m_sceneRenderer->Update(m_timer);
			m_fpsTextRenderer->Update(m_timer);
		});
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool XngnMain::Render()
{
	ImGuiIO& io = ImGui::GetIO();

	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	m_timer.SetFixedTimeStep(lockedFPS);

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	io.DisplaySize = ImVec2(viewport.Width, viewport.Height);

	// Reset render targets to the screen.
	ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_sceneRenderer->Render();
	//m_sceneRenderer->Render(0, 0, -2, m_timer.GetTotalSeconds(), m_timer.GetTotalSeconds(), m_timer.GetTotalSeconds());
	
	m_fpsTextRenderer->Render();

	//// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow(&showDemoWindow);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Scene");                          // Create a window called "Hello, world!" and append into it.

		if (ImGui::Button("Add"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		{
			Transform t = Transform{ 0, 0, 0, 0, 0, 0, 1, 1, 1 };
			m_sceneRenderer->AddTransform(t);
			item_current_idx = m_sceneRenderer->TransformCount() - 1;
		}
		
		ImGui::SameLine();

		if (m_sceneRenderer->TransformCount() == 0) ImGui::BeginDisabled();
		if (ImGui::Button("Remove"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		{
			m_sceneRenderer->GetTransforms().erase(m_sceneRenderer->GetTransforms().begin() + item_current_idx);
			if (item_current_idx > m_sceneRenderer->TransformCount() - 1) item_current_idx--;
		}
		if (m_sceneRenderer->TransformCount() == 0) ImGui::EndDisabled();


		ImGui::Text("GameObjects");
		if (ImGui::BeginListBox("##GameObjects"))
		{
			for (int n = 0; n < m_sceneRenderer->TransformCount(); n++)
			{
				const bool is_selected = (item_current_idx == n);
				if (ImGui::Selectable(("GameObject##" + std::to_string(n)).c_str(), is_selected))
					item_current_idx = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}

		if (m_sceneRenderer->TransformCount() > 0) {
			Transform& t = m_sceneRenderer->GetTransform(item_current_idx);

			ImGui::Begin("Inspector");

			ImGui::Text("Transform");

			ImGui::Text("Position");
			ImGui::SliderFloat("X##pos", &t.posX, -5.0f, 5.0f);
			ImGui::SliderFloat("Y##pos", &t.posY, -5.0f, 5.0f);
			ImGui::SliderFloat("Z##pos", &t.posZ, -5.0f, 5.0f);

			ImGui::Text("Rotation");
			ImGui::SliderFloat("X##rot", &t.rotX, 0.0f, 3.14f * 2);
			ImGui::SliderFloat("Y##rot", &t.rotY, 0.0f, 3.14f * 2);
			ImGui::SliderFloat("Z##rot", &t.rotZ, 0.0f, 3.14f * 2);

			ImGui::Text("Scale");
			ImGui::SliderFloat("X##scale", &t.scaleX, 0.01f, 10.0f);
			ImGui::SliderFloat("Y##scale", &t.scaleY, 0.01f, 10.0f);
			ImGui::SliderFloat("Z##scale", &t.scaleZ, 0.01f, 10.0f);

			ImGui::End();
		}


		ImGui::End();

		ImGui::Begin("Debug");
		ImGui::Text(("FPS: " + std::to_string(m_timer.GetFramesPerSecond())).c_str());
		ImGui::Text(("Frame time: " + std::to_string(m_timer.GetElapsedSeconds())).c_str());
		ImGui::Checkbox("Locked FPS", &lockedFPS);
		ImGui::End();
	}

	//ImGui::Begin("Another Window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	//ImGui::Text("Hello from another window!");
	//ImGui::End();

	//// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return true;
}

// Notifies renderers that device resources need to be released.
void XngnMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void XngnMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
