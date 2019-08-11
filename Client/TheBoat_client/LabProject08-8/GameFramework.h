#pragma once

#define FRAME_BUFFER_WIDTH		800
#define FRAME_BUFFER_HEIGHT		600

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "Shader.h"
#include "ServerMgr.h"



class CGameFramework
{
private:
	// 서버 관련 
	ServerMgr server_mgr;
	bool is_pushed[11] = { 0 };

	XMFLOAT3 sc_player_pos[MAX_PLAYER_SIZE];
	bool first_recv;
	int recvd_client_id; 

	bool player_ready = false;

	// 아이템 생성
	bool is_item_gen = false;
	XMFLOAT3 item_pos;
	//빌딩
	XMFLOAT3 building_pos[OBJECT_BUILDING];
	XMFLOAT3 buliding_extents[OBJECT_BUILDING];


	// 총알 충돌프레임
	bool is_collide = false;
	int collide_frame = 100;
public:
	static int my_client_id;
	static XMFLOAT3 buildingPos[OBJECT_BUILDING];
	// 룩벡터
	static XMFLOAT3 sendLook;
	int charstate = 0;
	bool isRun = false;
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();

	void OnResizeBackBuffers();

    void BuildObjects();
    void ReleaseObjects();

	// 로긴용
	void SendLoginREQ();
	// 채팅용
	void SendChatREQ();


    void ProcessInput();
    void AnimateObjects(CCamera *pCamera);
    void FrameAdvance();


	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	static CPlayer				*m_pPlayer[4];
	static CPlayer				*m_pObject[NUM_OBJECT];
	static CPlayer				*m_pObject2[NUM_OBJECT2];
	CPlayer						*m_pBlueBox[1];

	static CCamera				*m_pCamera;
	static int					boxBound;
	float						playerHp = 100;
	int							gameMode = 1;
	bool						damageCheck = false;
	bool						writeMode = true;
	bool						writeStart = false;
	int							writeNum = 0;

	unique_ptr<DirectX::DescriptorHeap>	m_resourceDescriptors;
	unique_ptr<DirectX::SpriteFont>		m_font;

	enum Descriptors
	{
		MyFont,
		Count
	};

	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	DirectX::SimpleMath::Vector2 m_fontPos;

private:
	int							mainScreenSelect = 0;
	bool						itemUI[4] = {};
	bool						alphaMapOn = false;
	bool						blueScreenMode = false;
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
        
	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ID3D12Device				*m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;
	UINT						m_nRtvDescriptorIncrementSize;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;
	UINT						m_nDsvDescriptorIncrementSize;

	ID3D12CommandAllocator		*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue			*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

	CScene						*m_pScene = NULL;

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[50];

	

};

