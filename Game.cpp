#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

namespace
{
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct ConstantBuffer
	{
        float time;
        float padding[3];
	};
}

Game::Game() noexcept(false)
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
}

void Game::Initialize(HWND window, int width, int height)
{
    m_keyboard = std::make_unique<Keyboard>();

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();
}

void Game::OnDeviceLost()
{
    m_spConstantBuffer.Reset();
    m_spTextureView.Reset();
    m_spInputLayout.Reset();
    m_spVertexBuffer.Reset();
    m_spVertexShader.Reset();
    m_spPixelShader.Reset();
    m_states.reset();
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}

void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void Game::GetDefaultSize(int& width, int& height) const noexcept
{
	width = 800;
	height = 600;
}

void Game::Update(DX::StepTimer const& timer)
{
    auto kb = m_keyboard->GetState();
    if (kb.Escape)
    {
        ExitGame();
    }
}

void Game::Render()
{
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");

    //
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->IASetInputLayout(m_spInputLayout.Get());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->IASetVertexBuffers(0, 1, m_spVertexBuffer.GetAddressOf(), &stride, &offset);
    context->VSSetShader(m_spVertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_spConstantBuffer.GetAddressOf());
    context->PSSetShader(m_spPixelShader.Get(), nullptr, 0);
    context->PSSetShaderResources(0, 1, m_spTextureView.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_spConstantBuffer.GetAddressOf());
    auto blend = m_states->NonPremultiplied();
    context->OMSetBlendState(blend, nullptr, 0xFFFFFFFF);
    
    ConstantBuffer cb = {};
    cb.time = static_cast<float>(m_timer.GetTotalSeconds());
    context->UpdateSubresource(m_spConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);
    
    context->Draw(4, 0);
    //

    m_deviceResources->PIXEndEvent();

    m_deviceResources->Present();
}

void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(
        depthStencil,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0
    );
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void Game::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);

	auto vsBlob = DX::ReadData(L"VertexShader.cso");
	DX::ThrowIfFailed(
		device->CreateVertexShader(
            vsBlob.data(),
            vsBlob.size(),
			nullptr,
			m_spVertexShader.ReleaseAndGetAddressOf()
		)
	);

	auto psBlob = DX::ReadData(L"PixelShader.cso");
	DX::ThrowIfFailed(
		device->CreatePixelShader(
            psBlob.data(),
            psBlob.size(),
			nullptr,
			m_spPixelShader.ReleaseAndGetAddressOf()
		)
	);

    static const D3D11_INPUT_ELEMENT_DESC s_inputElementDesc[2] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    DX::ThrowIfFailed(
		device->CreateInputLayout(
			s_inputElementDesc,
			_countof(s_inputElementDesc),
			vsBlob.data(),
			vsBlob.size(),
			m_spInputLayout.ReleaseAndGetAddressOf()
		)
	);

    static const Vertex s_vertexData[4] =
    {
        { {-0.5f,  0.5f,  0.0f },{ 0.0f, 0.0f } },
        { { 0.5f,  0.5f,  0.0f },{ 1.0f, 0.0f } },
        { {-0.5f, -0.5f,  0.0f },{ 0.0f, 1.0f } },
        { { 0.5f, -0.5f,  0.0f },{ 1.0f, 1.0f } }
    };

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = s_vertexData;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(s_vertexData);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.StructureByteStride = sizeof(Vertex);

    DX::ThrowIfFailed(
        device->CreateBuffer(&bufferDesc, &initialData, m_spVertexBuffer.ReleaseAndGetAddressOf()));

    DX::ThrowIfFailed(
        CreateWICTextureFromFile(device, L"textures/shop.png", nullptr, m_spTextureView.ReleaseAndGetAddressOf()));

    auto bd = CD3D11_BUFFER_DESC(sizeof(ConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
    DX::ThrowIfFailed(
        device->CreateBuffer(&bd, nullptr, m_spConstantBuffer.ReleaseAndGetAddressOf()));
}

void Game::CreateWindowSizeDependentResources()
{
}
