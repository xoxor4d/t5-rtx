#pragma once

#pragma warning(push)
#pragma warning(disable: 4324)

namespace game
{
	struct GfxPortal;
	struct GfxCell;
	struct GfxBackEndData;
	struct GfxViewInfo;
	typedef float vec_t;
	typedef vec_t vec2_t[2];
	typedef vec_t vec3_t[3];
	typedef vec_t vec4_t[4];

	enum $9FC675B7EFAE70B57C2B9C1C22A5CC63 : std::uint8_t
	{
		R_RENDERTARGET_NULL = 0x0,
		R_RENDERTARGET_NONE = 0x0,
		R_RENDERTARGET_SAVED_SCREEN = 0x1,
		R_RENDERTARGET_FRAME_BUFFER = 0x2,
		R_RENDERTARGET_SCENE = 0x3,
		R_RENDERTARGET_SCENE_NULLCOLOR = 0x4,
		R_RENDERTARGET_RESOLVED_POST_SUN = 0x5,
		R_RENDERTARGET_RESOLVED_SCENE = 0x6,
		R_RENDERTARGET_FLOAT_Z = 0x7,
		R_RENDERTARGET_PINGPONG_0 = 0x8,
		R_RENDERTARGET_PINGPONG_1 = 0x9,
		R_RENDERTARGET_POST_EFFECT_SRC = 0xA,
		R_RENDERTARGET_POST_EFFECT_GODRAYS = 0xB,
		R_RENDERTARGET_POST_EFFECT_0 = 0xC,
		R_RENDERTARGET_POST_EFFECT_1 = 0xD,
		R_RENDERTARGET_SHADOWMAP_SUN = 0xE,
		R_RENDERTARGET_SHADOWMAP_SPOT = 0xF,
		R_RENDERTARGET_16BIT_SYSTEM = 0x10,
		R_RENDERTARGET_8BIT_SYSTEM = 0x11,
		R_RENDERTARGET_8BIT_SWAPCHAIN_BACKBUFFER = 0x12,
		R_RENDERTARGET_SEETHRU_DECAL = 0x13,
		R_RENDERTARGET_UI3D = 0x14,
		R_RENDERTARGET_UI3D_PING_PONG = 0x15,
		R_RENDERTARGET_MISSILE_CAM = 0x16,
		R_RENDERTARGET_FLOAT_Z_MISSILE_CAM = 0x17,
		R_RENDERTARGET_COMPOSITE = 0x18,
		R_RENDERTARGET_BLOOM_MIP1 = 0x19,
		R_RENDERTARGET_BLOOM_MIP2 = 0x1A,
		R_RENDERTARGET_BLOOM_MIP3 = 0x1B,
		R_RENDERTARGET_BLOOM_MIP3_PING = 0x1C,
		R_RENDERTARGET_BLOOM_MIP3_PONG = 0x1D,
		R_RENDERTARGET_BLOOM_STREAK = 0x1E,
		R_RENDERTARGET_SHADOWMAP_SUN_HI_FULL = 0x1F,
		R_RENDERTARGET_SHADOWMAP_SUN_HI_PARTITION_0 = 0x20,
		R_RENDERTARGET_SHADOWMAP_SUN_HI_PARTITION_1 = 0x21,
		R_RENDERTARGET_SHADOWMAP_SUN_LO_FULL = 0x22,
		R_RENDERTARGET_SHADOWMAP_SUN_LO_PARTITION_0 = 0x23,
		R_RENDERTARGET_SHADOWMAP_SUN_LO_PARTITION_1 = 0x24,
		R_RENDERTARGET_SHADOWMAP_SPOT_FULL = 0x25,
		R_RENDERTARGET_SHADOWMAP_SPOT_0 = 0x26,
		R_RENDERTARGET_SHADOWMAP_SPOT_1 = 0x27,
		R_RENDERTARGET_SHADOWMAP_SPOT_2 = 0x28,
		R_RENDERTARGET_SHADOWMAP_SPOT_3 = 0x29,
		R_RENDERTARGET_SHADOWMAP_SPOT_HI_0 = 0x2A,
		R_RENDERTARGET_SHADOWMAP_SPOT_HI_1 = 0x2B,
		R_RENDERTARGET_COUNT = 0x2C,
	};

	struct field_t
	{
		int cursor;
		int scroll;
		int drawWidth;
		int widthInPixels;
		float charHeight;
		int fixedSize;
		char buffer[256];
	};

	struct ScreenPlacement
	{
		float scaleVirtualToReal[2];
		float scaleVirtualToFull[2];
		float scaleRealToVirtual[2];
		float virtualViewableMin[2];
		float virtualViewableMax[2];
		float virtualTweakableMin[2];
		float virtualTweakableMax[2];
		float realViewportBase[2];
		float realViewportSize[2];
		float realViewportMid[2];
		float realViewableMin[2];
		float realViewableMax[2];
		float realTweakableMin[2];
		float realTweakableMax[2];
		float subScreen[2];
	};

	struct vidConfig_t
	{
		unsigned int sceneWidth;
		unsigned int sceneHeight;
		unsigned int displayWidth;
		unsigned int displayHeight;
		unsigned __int16 outputDisplayWidth;
		unsigned __int16 outputDisplayHeight;
		unsigned int displayFrequency;
		bool isToolMode;
		int isFullscreen;
		float aspectRatioWindow;
		float aspectRatioScenePixel;
		float aspectRatioDisplayPixel;
		unsigned int maxTextureSize;
		unsigned int maxTextureMaps;
		bool deviceSupportsGamma;
	};

	struct CmdArgs
	{
		int nesting;
		int localClientNum[8];
		int controllerIndex[8];
		struct itemDef_s* itemDef[8];
		int argshift[8];
		int argc[8];
		const char** argv[8];
		char textPool[8192];
		const char* argvPool[512];
		int usedTextPool[8];
		int totalUsedArgvPool;
		int totalUsedTextPool;
	};

	struct cmd_function_s
	{
		cmd_function_s* next; //OFS: 0x0 SIZE: 0x4
		const char* name; //OFS: 0x4 SIZE: 0x4
		const char* autoCompleteDir; //OFS: 0x8 SIZE: 0x4
		const char* autoCompleteExt; //OFS: 0xC SIZE: 0x4
		void(__cdecl* function)(); //OFS: 0x10 SIZE: 0x4
		bool consoleAccess;
	};

	struct GfxMatrix
	{
		float m[4][4];
	};

	struct GfxViewParms
	{
		GfxMatrix viewMatrix;
		GfxMatrix projectionMatrix;
		GfxMatrix viewProjectionMatrix;
		GfxMatrix inverseViewProjectionMatrix;
		float origin[4];
		float axis[3][3];
		float depthHackNearClip;
		float zNear;
		float zFar;
	};

	struct cursor_t
	{
		float x;
		float y;
	};

	struct UiContext
	{
		int contextIndex;
		float bias;
		int realTime;
		int frameTime;
		cursor_t cursor;
		cursor_t prevCursor;
		int isCursorVisible;
		int screenWidth;
		int screenHeight;
		float screenAspect;
		float FPS;
		float blurRadiusOut;
		// ...
	};

	struct uiInfo_s
	{
		UiContext uiDC;
		// ....
	};

	struct GfxWindowParms
	{
		HWND__* hwnd;
		int hz;
		bool fullscreen;
		int x;
		int y;
		int sceneWidth;
		int sceneHeight;
		int displayWidth;
		int displayHeight;
		int aaSamples;
	};

	struct DpvsPlane
	{
		float coeffs[4];
		char side[3];
		char pad;
	};

	struct DpvsView
	{
		unsigned int renderFxFlagsCull;
		DpvsPlane frustumPlanes[14];
		int frustumPlaneCount;
	};

	struct GfxWindowTarget
	{
		HWND__* hwnd;
		IDirect3DSwapChain9* swapChain;
		int width;
		int height;
	};

	struct GfxReadCmdBuf
	{
		const unsigned int* primDrawSurfPos;
	};

	struct GfxBspPreTessDrawSurf
	{
		unsigned __int16 baseSurfIndex;
		unsigned __int16 totalTriCount;
	};

	struct srfTriangles_t
	{
		float mins[3];
		int vertexLayerData;
		float maxs[3];
		int firstVertex;
		unsigned __int16 vertexCount;
		unsigned __int16 triCount;
		int baseIndex;
		float himipRadiusSq;
		int stream2ByteOffset;
	};

	enum MaterialVertexDeclType
	{
		VERTDECL_GENERIC = 0x0,
		VERTDECL_PACKED = 0x1,
		VERTDECL_WORLD = 0x2,
		VERTDECL_WORLD_T1N0 = 0x3,
		VERTDECL_WORLD_T1N1 = 0x4,
		VERTDECL_WORLD_T2N0 = 0x5,
		VERTDECL_WORLD_T2N1 = 0x6,
		VERTDECL_WORLD_T2N2 = 0x7,
		VERTDECL_WORLD_T3N0 = 0x8,
		VERTDECL_WORLD_T3N1 = 0x9,
		VERTDECL_WORLD_T3N2 = 0xA,
		VERTDECL_WORLD_T4N0 = 0xB,
		VERTDECL_WORLD_T4N1 = 0xC,
		VERTDECL_WORLD_T4N2 = 0xD,
		VERTDECL_POS_TEX = 0xE,
		VERTDECL_STATICMODELCACHE = 0xF,
		VERTDECL_WATER = 0x10,
		VERTDECL_PARTICLECLOUD = 0x11,
		VERTDECL_COUNT = 0x12,
	};

	struct $12633521C142B633A9BB4FE83ABA376C
	{
		unsigned int stride;
		IDirect3DVertexBuffer9* vb;
		unsigned int offset;
	};

	struct GfxPrimStats
	{
		int primCount;
		int triCount;
		int staticIndexCount;
		int staticVertexCount;
		int dynamicIndexCount;
		int dynamicVertexCount;
	};

	struct GfxViewStats
	{
		GfxPrimStats primStats[10];
		int drawSurfCount;
		int drawMatCount;
		int drawPrimHistogram[16];
	};

	struct GfxFrameStats
	{
		GfxViewStats viewStats[2];
		int gfxEntCount;
		int geoIndexCount;
		int fxIndexCount;
	};

	struct GfxCmdBufPrimState
	{
		IDirect3DDevice9* device;
		IDirect3DIndexBuffer9* indexBuffer;
		MaterialVertexDeclType vertDeclType;
		$12633521C142B633A9BB4FE83ABA376C streams[3];
		IDirect3DVertexDeclaration9* vertexDecl;
	};

	struct __declspec(align(8)) DxGlobals
	{
		HINSTANCE__* hinst;
		IDirect3D9* d3d9;
		IDirect3DDevice9* device;
		unsigned int adapterIndex;
		unsigned int vendorId;
		bool adapterNativeIsValid;
		int adapterNativeWidth;
		int adapterNativeHeight;
		int adapterFullscreenWidth;
		int adapterFullscreenHeight;
		bool supportsSceneNullRenderTarget;
		bool supportsIntZ;
		bool nvInitialized;
		bool nvStereoActivated;
		void* nvStereoHandle;
		void* nvDepthBufferHandle;
		void* nvFloatZBufferHandle;
		bool resizeWindow;
		_D3DFORMAT depthStencilFormat;
		unsigned int displayModeCount;
		_D3DDISPLAYMODE displayModes[256];
		const char* resolutionNameTable[257];
		const char* refreshRateNameTable[257];
		char modeText[5120];
		IDirect3DQuery9* fencePool[8];
		unsigned int nextFence;
		int gpuSync;
		int gpuCount;
		_D3DMULTISAMPLE_TYPE multiSampleType;
		unsigned int multiSampleQuality;
		int unk;
		int sunSpriteSamples;
		IDirect3DSurface9* singleSampleDepthStencilSurface;
		bool inScene;
		int targetWindowIndex;
		int windowCount;
		GfxWindowTarget windows[1];
		IDirect3DQuery9* flushGpuQuery;
		unsigned __int64 gpuSyncDelay;
		unsigned __int64 gpuSyncStart;
		unsigned __int64 gpuSyncEnd;
		_D3DTEXTUREFILTERTYPE linearNonMippedMinFilter;
		_D3DTEXTUREFILTERTYPE linearNonMippedMagFilter;
		_D3DTEXTUREFILTERTYPE linearMippedMinFilter;
		_D3DTEXTUREFILTERTYPE linearMippedMagFilter;
		_D3DTEXTUREFILTERTYPE anisotropicMinFilter;
		_D3DTEXTUREFILTERTYPE anisotropicMagFilter;
		int linearMippedAnisotropy;
		int anisotropyFor2x;
		int anisotropyFor4x;
		int mipFilterMode;
		unsigned int mipBias;
		IDirect3DQuery9* swapFence[4];
	};

	union GfxColor
	{
		unsigned int packed;
		unsigned __int8 array[4];
	};

	struct GfxDrawSurfFields
	{
		unsigned __int64 objectId : 16;
		unsigned __int64 fade : 4;
		unsigned __int64 customIndex : 5;
		unsigned __int64 reflectionProbeIndex : 3;
		unsigned __int64 hdrBits : 1;
		unsigned __int64 glightRender : 1;
		unsigned __int64 dlightRender : 1;
		unsigned __int64 materialSortedIndex : 12;
		unsigned __int64 primaryLightIndex : 8;
		unsigned __int64 surfType : 4;
		unsigned __int64 prepass : 2;
		unsigned __int64 noDynamicShadow : 1;
		unsigned __int64 primarySortKey : 6;
	};

	union GfxDrawSurf
	{
		GfxDrawSurfFields fields;
		unsigned __int64 packed;
	};

	struct __declspec(align(4)) GfxImageLoadDef
	{
		char levelCount;
		char flags;
		int format;
		int resourceSize;
		char data[1];
	};

	union GfxTexture
	{
		IDirect3DBaseTexture9* basemap;
		IDirect3DTexture9* map;
		IDirect3DVolumeTexture9* volmap;
		IDirect3DCubeTexture9* cubemap;
		GfxImageLoadDef* loadDef;
	};

	struct Picmip
	{
		char platform[2];
	};

	struct CardMemory
	{
		int platform[2];
	};

	struct GfxImage
	{
		GfxTexture texture;
		char mapType;
		char semantic;
		char category;
		bool delayLoadPixels;
		Picmip picmip;
		bool noPicmip;
		char track;
		CardMemory cardMemory;
		unsigned __int16 width;
		unsigned __int16 height;
		unsigned __int16 depth;
		char levelCount;
		char streaming;
		unsigned int baseSize;
		char* pixels;
		unsigned int loadedSize;
		char skippedMipLevels;
		const char* name;
		unsigned int hash;
	};

	struct complex_s
	{
		float real;
		float imag;
	};

	struct WaterWritable
	{
		float floatTime;
	};

	struct water_t
	{
		WaterWritable writable;
		complex_s* H0;
		float* wTerm;
		int M;
		int N;
		float Lx;
		float Lz;
		float gravity;
		float windvel;
		float winddir[2];
		float amplitude;
		float codeConstant[4];
		GfxImage* image;
	};

	struct __declspec(align(8)) MaterialInfo
	{
		const char* name;
		unsigned int gameFlags;
		char pad;
		char sortKey;
		char textureAtlasRowCount;
		char textureAtlasColumnCount;
		GfxDrawSurf drawSurf;
		unsigned int surfaceTypeBits;
		unsigned int layeredSurfaceTypes;
		unsigned __int16 hashIndex;
	};

	struct MaterialStreamRouting
	{
		char source;
		char dest;
	};

	struct MaterialVertexStreamRouting
	{
		MaterialStreamRouting data[16];
		IDirect3DVertexDeclaration9* decl[18];
	};

	struct MaterialVertexDeclaration
	{
		char streamCount;
		bool hasOptionalSource;
		bool isLoaded;
		MaterialVertexStreamRouting routing;
	};

	struct __declspec(align(4)) GfxVertexShaderLoadDef
	{
		unsigned int* program;
		unsigned __int16 programSize;
	};

	struct MaterialVertexShaderProgram
	{
		IDirect3DVertexShader9* vs;
		GfxVertexShaderLoadDef loadDef;
	};

	struct MaterialVertexShader
	{
		const char* name;
		MaterialVertexShaderProgram prog;
	};

	struct __declspec(align(4)) GfxPixelShaderLoadDef
	{
		unsigned int* program;
		unsigned __int16 programSize;
	};

	struct MaterialPixelShaderProgram
	{
		IDirect3DPixelShader9* ps;
		GfxPixelShaderLoadDef loadDef;
	};

	struct MaterialPixelShader
	{
		const char* name;
		MaterialPixelShaderProgram prog;
	};

	union $3E38D60757422ABD074F0BB8A7F768B6
	{
		MaterialPixelShader* pixelShader;
		MaterialPixelShader* localPixelShader;
	};

	struct MaterialArgumentCodeConst
	{
		unsigned __int16 index;
		char firstRow;
		char rowCount;
	};

	union MaterialArgumentDef
	{
		const float* literalConst;
		MaterialArgumentCodeConst codeConst;
		unsigned int codeSampler;
		unsigned int nameHash;
	};

	struct MaterialShaderArgument
	{
		unsigned __int16 type;
		unsigned __int16 dest;
		MaterialArgumentDef u;
	};

	union $835B54ED439E881A12CFCCFD6075577F
	{
		MaterialShaderArgument* localArgs;
		MaterialShaderArgument* args;
	};

	struct MaterialPass
	{
		MaterialVertexDeclaration* vertexDecl;
		MaterialVertexShader* vertexShader;
		$3E38D60757422ABD074F0BB8A7F768B6 u_pixelshader;
		char perPrimArgCount;
		char perObjArgCount;
		char stableArgCount;
		char customSamplerFlags;
		$835B54ED439E881A12CFCCFD6075577F u_args;
	};

	struct MaterialTechnique
	{
		const char* name;
		unsigned __int16 flags;
		unsigned __int16 passCount;
		MaterialPass passArray[1];
	};

	struct MaterialTechniqueSet
	{
		const char* name;
		char worldVertFormat;
		char unused[1];
		unsigned __int16 techsetFlags;
		MaterialTechnique* techniques[130];
	};

	struct MaterialConstantDef
	{
		unsigned int nameHash;
		char name[12];
		float literal[4];
	};

	union $6ADFD931CD5BA920F612AF505CA16E3E
	{
		MaterialTechniqueSet* localTechniqueSet;
		MaterialTechniqueSet* techniqueSet;
	};

	union $C3F7A89422F64E7CE7B1D04043902CE5
	{
		MaterialConstantDef* localConstantTable;
		MaterialConstantDef* constantTable;
	};

	union MaterialTextureDefInfo
	{
		GfxImage* image;
		void* water;
	};

	/* 1300 */
	struct MaterialTextureDef
	{
		unsigned int nameHash;
		char nameStart;
		char nameEnd;
		char samplerState;
		char semantic;
		char isMatureContent;
		char pad[3];
		MaterialTextureDefInfo u;
	};

	struct GfxStateBits
	{
		unsigned int loadBits[2];
	};

	struct Material
	{
		MaterialInfo info;
		char stateBitsEntry[130];
		char textureCount;
		char constantCount;
		char stateBitsCount;
		char stateFlags;
		char cameraRegion;
		char maxStreamedMips;
		$6ADFD931CD5BA920F612AF505CA16E3E u_techset;
		MaterialTextureDef* textureTable;
		$C3F7A89422F64E7CE7B1D04043902CE5 u_constant;
		GfxStateBits* stateBitsTable;
	};

	struct GfxCodeMatrices
	{
		GfxMatrix matrix[32];
	};

	struct GfxCodeImageRenderTargetFields
	{
		unsigned __int64 renderTargetId : 8;
		unsigned __int64 enable : 1;
		unsigned __int64 fbufferTexture : 3;
		unsigned __int64 filtering : 20;
	};

	union GfxCodeImageRenderTarget
	{
		GfxCodeImageRenderTargetFields fields;
		unsigned int packed;
	};

	struct FxCodeMeshData
	{
		unsigned int triCount;
		unsigned __int16* indices;
		unsigned __int16 argOffset;
		unsigned __int16 argCount;
		unsigned int lightHandle;
	};

	struct GfxPlacement
	{
		float quat[4];
		float origin[3];
	};

	struct GfxScaledPlacement
	{
		GfxPlacement base;
		float scale;
	};

	struct GfxParticleCloud
	{
		GfxScaledPlacement placement;
		float endpos[3];
		GfxColor color;
		float radius[2];
		unsigned __int16 count;
		unsigned __int16 offset;
		char gaussian;
		char pad[3];
	};

	struct GfxVertexBufferState
	{
		int used;
		int total;
		IDirect3DVertexBuffer9* buffer;
		char* verts;
	};

	struct GfxMeshData
	{
		GfxMeshData* thisPtr;
		unsigned int indexCount;
		unsigned int totalIndexCount;
		unsigned __int16* indices;
		GfxVertexBufferState vb;
		unsigned int vertSize;
	};

	union PackedTexCoords
	{
		unsigned int packed;
	};

	union PackedUnitVec
	{
		unsigned int packed;
		char array[4];
	};

	union PackedLightingCoords
	{
		unsigned int packed;
		char array[4];
	};

	struct GfxSModelCachedVertex
	{
		float xyz[3];
		GfxColor color;
		PackedTexCoords texCoord;
		PackedUnitVec normal;
		PackedUnitVec tangent;
		PackedLightingCoords baseLighting;
	};

	struct GfxModelLightingPatch
	{
		unsigned __int16 modelLightingIndex;
		float primaryLightWeight;
		char useHeroLighting;
		char colorsCount;
		char groundLighting[4];
		float colorsWeight[8];
		unsigned __int16 colorsIndex[8];
		float heroPos[3];
	};

	struct GfxBackEndPrimitiveData
	{
		int hasSunDirChanged;
	};

	struct GfxEntity
	{
		unsigned int renderFxFlags;
		float materialTime;
		float destructibleBurnAmount;
		float destructibleFadeAmount;
		float wetness;
		int textureOverrideIndex;
	};

	struct ShaderConstantSet
	{
		float value[7][4];
		char constantSource[7];
		char used;
	};

	struct GfxTextureOverride
	{
		Material* material;
		GfxImage* img1;
		GfxImage* img2;
		unsigned __int16 dobjModelMask;
		__int16 prev;
	};

	struct FxMarkMeshData
	{
		unsigned int triCount;
		unsigned __int16* indices;
		float normal[3];
		unsigned __int16 modelIndex;
		char modelTypeAndSurf;
		char pad0;
	};

	struct GfxIndexBufferState
	{
		volatile int used;
		int total;
		IDirect3DIndexBuffer9* buffer;
		unsigned __int16* indices;
	};

	struct GfxFog
	{
		int startTime;
		int finishTime;
		float color[4];
		float fogStart;
		float density;
		float heightDensity;
		float baseHeight;
		float sunFogColor[4];
		float sunFogDir[3];
		float sunFogStartAng;
		float sunFogEndAng;
		float maxDensity;
	};

	struct GfxCmdHeader
	{
		unsigned __int16 byteCount;
		char id;
		char ui3d;
	};

	struct GfxCmdArray
	{
		char* cmds;
		int usedTotal;
		int usedCritical;
		GfxCmdHeader* lastCmd;
		int byteSize;
		int warnSize;
	};

	struct GfxViewport
	{
		int x;
		int y;
		int width;
		int height;
	};

	enum GfxViewportBehavior
	{
		GFX_USE_VIEWPORT_FOR_VIEW = 0x0,
		GFX_USE_VIEWPORT_FULL = 0x1,
	};

	struct GfxCullViewInfo
	{
		GfxViewParms viewParms;
		GfxViewport sceneViewport;
		GfxViewport displayViewport;
		GfxViewport scissorViewport;
	};

	struct $19F1581A603298CB6762F171119B60F7
	{
		GfxViewParms viewParms;
		GfxViewport sceneViewport;
		GfxViewport displayViewport;
		GfxViewport scissorViewport;
	};

	union $E3A83F5AFE6B193109DAD0C9BAA28F2F
	{
		GfxCullViewInfo cullViewInfo;
		$19F1581A603298CB6762F171119B60F7 s1;
	};

	struct GfxSceneDef
	{
		int time;
		float floatTime;
		float viewOffset[3];
	};

	enum ShadowType
	{
		SHADOW_NONE = 0x0,
		SHADOW_MAP = 0x1,
	};

	union $ED5082F4EF9C51C3CAAE283CF5E38ECF
	{
		float m[4][4];
		float member[16];
	};

	struct float44
	{
		$ED5082F4EF9C51C3CAAE283CF5E38ECF u0;
	};

	struct __declspec(align(16)) GfxLight
	{
		char type;
		char canUseShadowMap;
		__int16 cullDist;
		float color[3];
		float dir[3];
		float origin[3];
		float radius;
		float cosHalfFovOuter;
		float cosHalfFovInner;
		int exponent;
		unsigned int spotShadowIndex;
		float angles[3];
		float spotShadowHiDistance;
		float diffuseColor[4];
		float specularColor[4];
		float shadowColor[4];
		float falloff[4];
		float attenuation[4];
		float aAbB[4];
		float cookieControl0[4];
		float cookieControl1[4];
		float cookieControl2[4];
		__declspec(align(8)) float44 viewMatrix;
		float44 projMatrix;
		void* def; // GfxLightDef
	};

	struct $2BD02F38FBEBD854EF9A531D8B9F9671
	{
		int QueuedBatchCount;
		int ExecutingBatchCount;
	};

	union $78B26C2CBD2C5335330BFD0A20A3805F
	{
		$2BD02F38FBEBD854EF9A531D8B9F9671 __s0;
		unsigned __int64 BatchCount;
	};

	struct jqBatchGroup
	{
		$78B26C2CBD2C5335330BFD0A20A3805F ___u0;
	};

	struct GfxDrawSurfListInfo
	{
		GfxDrawSurf* drawSurfs;
		unsigned int drawSurfCount;
		char baseTechType;
		GfxViewInfo* viewInfo;
		unsigned int viewInfoIndex;
		float viewOrigin[4];
		GfxLight* light;
		int cameraView;
		jqBatchGroup group[3];
		int shaderHDRmode;
		bool isMissileCamera;
	};

	struct PointLightPartition
	{
		GfxLight light;
		GfxDrawSurfListInfo info;
	};

	struct GfxLightScale
	{
		float diffuseScale;
		float specularScale;
	};

	struct GfxDepthOfField
	{
		float viewModelStart;
		float viewModelEnd;
		float nearStart;
		float nearEnd;
		float farStart;
		float farEnd;
		float nearBlur;
		float farBlur;
	};

	struct GfxFilm
	{
		float filmMidStart;
		float filmMidEnd;
		float filmDarkFeather;
		float filmLightFeather;
		float filmBleach[3];
		float filmColorTemp[3];
		float filmHue[3];
		float filmSaturation[3];
		float filmDarkTint[3];
		float filmMidTint[3];
		float filmLightTint[3];
		float filmContrast[3];
		bool enabled;
		float filmLut;
		float sCurveEnable;
		float sCurveShoulderStrength;
		float sCurveLinearStrength;
		float sCurveLinearAngle;
		float sCurveToeStrength;
		float sCurveToeNumerator;
		float sCurveToeDenominator;
	};

	struct __declspec(align(4)) GfxCompositeFx
	{
		float distortionScale[2];
		float blurRadius;
		float distortionMagnitude;
		float frameRate;
		int lastUpdate;
		int frame;
		int startMSec;
		int currentTime;
		int duration;
		bool enabled;
		bool scriptEnabled;
	};

	struct GfxReviveFx
	{
		bool enabled;
		float reviveEdgeColorTemp;
		float reviveEdgeSaturation;
		float reviveEdgeScale[3];
		float reviveEdgeContrast[3];
		float reviveEdgeOffset[3];
		float reviveEdgeMaskAdjust;
		float reviveEdgeAmount;
	};

	struct GfxDoubleVision
	{
		float direction[3];
		float motionBlurMagnitude;
		float deltaPerMS;
		float cur;
		float targ;
	};

	struct GfxGenericFilter
	{
		bool passEnabled[3][16];
		Material* passMaterial[3][16];
		int passTarget[3][16];
		int passSampler0[3][16];
		int passSampler1[3][16];
		float passParam[3][16][16];
		int passQuads[3][16];
		float passFlareOcclusion[3][16];
		float sunPosition[3];
	};

	struct GfxPoison
	{
		float curAmountTarget;
		float curAmount;
	};

	struct __declspec(align(4)) GfxSaveScreenParam
	{
		float s0;
		float t0;
		float ds;
		float dt;
		int screenTimerId;
		char mode;
	};

	struct __declspec(align(4)) GfxBlendSaveScreenBlurredParam
	{
		int fadeMsec;
		float s0;
		float t0;
		float ds;
		float dt;
		int screenTimerId;
		char enabled;
	};

	struct __declspec(align(4)) GfxBlendSaveScreenFlashedParam
	{
		float intensityWhiteout;
		float intensityScreengrab;
		float s0;
		float t0;
		float ds;
		float dt;
		int screenTimerId;
		char enabled;
	};

	struct GfxSaveScreenFx
	{
		GfxSaveScreenParam saveScreenParam;
		GfxBlendSaveScreenBlurredParam blendBlurredParam;
		GfxBlendSaveScreenFlashedParam blendFlashedParam;
	};

	struct GfxQuadMeshData
	{
		float x;
		float y;
		float width;
		float height;
		GfxMeshData meshData;
	};

	struct __declspec(align(16)) GfxCmdBufInput
	{
		float consts[197][4];
		GfxImage* codeImages[43];
		char codeImageSamplerStates[43];
		GfxCodeImageRenderTarget codeImageRenderTargetControl[43];
		GfxBackEndData* data;
	};

	struct GfxExposureValue
	{
		float blackPoint[4];
		float whitePoint[4];
		float linearStart[4];
		float linearEnd[4];
		float remapStart[4];
		float remapEnd[4];
		float scurveStart[4];
		float scurveEnd[4];
		float bloomCurveLDR[4];
		float bloomCurveHDR[4];
		float bloomScale[4];
	};

	struct GfxExposureShaderRemap
	{
		float remapMul[3];
		float remapAdd[3];
	};

	struct GfxViewRenderControl
	{
		char mainSceneMSAA;
		char mainScene;
		char mainSceneFinal;
		char mainSceneSaved;
		char extraCam;
		char ui3d;
		char seeThruDecal;
		char frameBuffer;
		float sceneWindowU0;
		float sceneWindowV0;
		float sceneWindowU1;
		float sceneWindowV1;
		float framebufferWindowY0;
		float framebufferWindowX1;
		float framebufferWindowY1;
		int opaqueShaderRemap;
		int alphaShaderRemap;
		int emissiveShaderRemap;
		unsigned int renderingMode;
		unsigned int additionalPostFX;
	};

	struct GfxBloom
	{
		float bloomTintWeights[4];
		float bloomColorScale[4];
		float bloomTintScale[4];
		float bloomCurveBreakpoint[4];
		float bloomCurveLoBlack[4];
		float bloomCurveLoGamma[4];
		float bloomCurveLoWhite[4];
		float bloomCurveLoRemapBlack[4];
		float bloomCurveLoRemapWhite[4];
		float bloomCurveHiBlack[4];
		float bloomCurveHiGamma[4];
		float bloomCurveHiWhite[4];
		float bloomCurveHiRemapBlack[4];
		float bloomCurveHiRemapWhite[4];
		float bloomExpansionControl[4];
		float bloomExpansionWeights[4];
		int bloomExpansionSource;
		float bloomBlurRadius;
		float bloomPersistence;
		float bloomStreakXLevels0[4];
		float bloomStreakXLevels1[4];
		float bloomStreakXInnerTint[3];
		float bloomStreakXOuterTint[3];
		float bloomStreakXTintControl[4];
		float bloomStreakXTint[3];
		float bloomStreakYLevels0[4];
		float bloomStreakYLevels1[4];
		float bloomStreakYInnerTint[3];
		float bloomStreakYOuterTint[3];
		float bloomStreakYTintControl[4];
		float bloomStreakYTint[3];
	};

	struct GfxUI3DBackend
	{
		GfxViewport viewport[6];
		float uvSetup[6][4];
		int renderCmdCount[6];
		int totalRenderCmds;
		float blurRadius;
	};

	struct __declspec(align(16)) GfxViewInfo
	{
		$E3A83F5AFE6B193109DAD0C9BAA28F2F u0;
		GfxSceneDef sceneDef;
		ShadowType dynamicShadowType;
		int localClientNum;
		int isRenderingFullScreen;
		int playerTeleported;
		bool needsFloatZ;
		bool renderingFloatZ;
		PointLightPartition pointLightPartitions[4];
		GfxMeshData pointLightMeshData[4];
		int pointLightCount;
		GfxLight visibleLights[4];
		int visibleLightCount;
		float blurRadius;
		float frustumPlanes[4][4];
		GfxLightScale charPrimaryLightScale;
		GfxDepthOfField dof;
		GfxFilm film;
		GfxCompositeFx flameFx;
		GfxReviveFx reviveFx;
		GfxCompositeFx waterSheetingFx;
		GfxDoubleVision doubleVision;
		GfxGenericFilter genericFilter;
		GfxPoison poisonFx;
		GfxCompositeFx electrifiedFx;
		GfxCompositeFx transportedFx;
		GfxSaveScreenFx saveScreenFx;
		const void* cmds;
		float sunVisibility;
		GfxQuadMeshData* fullSceneViewMesh;
		__declspec(align(8)) GfxDrawSurfListInfo drawList[14];
		GfxCmdBufInput input;
		int renderSeeThruDecals;
		char hdrRenderingMode;
		GfxExposureValue exposureValue;
		GfxExposureShaderRemap exposureRemap;
		GfxViewRenderControl sceneComposition;
		GfxBloom bloom;
		float bloomExpansionXpos;
		float bloomExpansionYpos;
		GfxUI3DBackend rbUI3D;
		bool isMissileCamera;
		bool hasCmdBuf;
		float postEmissiveBrightening;
		bool noLodCullOut;
		//DynSModelClientView* dynSModelView;
		//DynSModelGfxState* dynSModelState;
	};

	struct GfxDebugPoly
	{
		float faceColor[4];
		bool faceDepthTest;
		float edgeColor[4];
		bool edgeDepthTest;
		int firstVert;
		int vertCount;
	};

	struct PolySet
	{
		float(*verts)[3];
		int vertCount;
		int vertLimit;
		GfxDebugPoly* polys;
		int polyCount;
		int polyLimit;
	};

	struct trDebugString_t
	{
		float xyz[3];
		float color[4];
		float scale;
		char text[96];
	};

	struct trDebugLine_t
	{
		float start[3];
		float end[3];
		float color[4];
		int depthTest;
	};

	struct GfxDebugPlume
	{
		float origin[3];
		float color[4];
		int score;
		int startTime;
		int duration;
	};

	struct trDebugSphere_t
	{
		float center[3];
		float radius;
		int sideCount;
		float color[4];
		int depthTest;
	};

	struct DebugGlobals
	{
		PolySet polySet;
		trDebugString_t* strings;
		int stringCount;
		int stringLimit;
		trDebugString_t* externStrings;
		int externStringCount;
		int externMaxStringCount;
		trDebugLine_t* lines;
		int lineCount;
		int lineLimit;
		trDebugLine_t* externLines;
		int externLineCount;
		int externMaxLineCount;
		GfxDebugPlume* plumes;
		int plumeCount;
		int plumeLimit;
		trDebugSphere_t* spheres;
		int sphereCount;
		int sphereLimit;
		trDebugSphere_t* externSpheres;
		int externSphereCount;
		int externMaxSphereCount;
	}; STATIC_ASSERT_SIZE(DebugGlobals, 0x6C);


	const struct __declspec(align(32)) GfxBackEndData
	{
		char surfsBuffer[262144];
		FxCodeMeshData codeMeshes[2048];
		unsigned int primDrawSurfsBuf[65536];
		GfxViewParms viewParms[4];
		char primaryLightTechType[16][2][2][256];
		float codeMeshArgs[256][4];
		GfxParticleCloud clouds[256];
		GfxDrawSurf drawSurfs[32768];
		GfxMeshData* codeMeshPtr;
		__declspec(align(16)) GfxMeshData markMesh;
		__declspec(align(16)) GfxMeshData glassMesh;
		__declspec(align(16)) GfxSModelCachedVertex smcPatchVerts[8192];
		unsigned __int16 smcPatchList[256];
		unsigned int smcPatchCount;
		unsigned int smcPatchVertsUsed;
		GfxModelLightingPatch modelLightingPatchList[4096];
		volatile int modelLightingPatchCount;
		GfxBackEndPrimitiveData prim;
		unsigned int shadowableLightHasShadowMap[8];
		unsigned int frameCount;
		int drawSurfCount;
		volatile int surfPos;
		volatile int gfxEntCount;
		GfxEntity gfxEnts[256];
		volatile int shaderConstantSetCount;
		ShaderConstantSet shaderConstantSets[128];
		volatile int textureOverrideCount;
		GfxTextureOverride textureOverrides[256];
		volatile int cloudCount;
		volatile int codeMeshCount;
		volatile int codeMeshArgsCount;
		volatile int markMeshCount;
		FxMarkMeshData markMeshes[1536];
		volatile unsigned int* dynamicBufferCurrentFrame;
		GfxVertexBufferState* skinnedCacheVb;
		IDirect3DQuery9* endFence;
		char* tempSkinBuf;
		volatile int tempSkinPos;
		GfxIndexBufferState* preTessIb;
		int viewParmCount;
		GfxFog fogSettings;
		GfxCmdArray* commands;
		unsigned int viewInfoIndex;
		unsigned int viewInfoCount;
		GfxViewInfo* viewInfo;
		GfxUI3DBackend rbUI3D;
		const void* cmds;
		const void* compositingCmds;
		GfxLight sunLight;
		int hasApproxSunDirChanged;
		volatile int primDrawSurfPos;
		DebugGlobals debugGlobals;
		unsigned int drawType;
		int hideMatureContent;
		int splitscreen;
		GfxLight shadowableLights[255];
		unsigned int shadowableLightCount;
		unsigned int emissiveSpotLightIndex;
		GfxLight emissiveSpotLight;
		int emissiveSpotDrawSurfCount;
		GfxDrawSurf* emissiveSpotDrawSurfs;
		unsigned int emissiveSpotLightCount;
		bool extraCamLargeFrameSize;
		/*GfxSunShadow sunShadow;
		unsigned int spotShadowCount;
		GfxSpotShadow spotShadows[4];
		DynSModelGfxState dynSModelState;
		DynSModelClientView* dynSModelClientViewArray;*/
	};

	struct $30BC23168E01BE061EE67625456A67BE
	{
		BYTE gap0[3680];
		GfxViewParms viewParms;
	};

	union $C1E24B5ACD1937141180CCC4F9D67481
	{
		GfxCmdBufInput input;
		$30BC23168E01BE061EE67625456A67BE __s1;
	};

	enum GfxViewMode
	{
		VIEW_MODE_NONE = 0x0,
		VIEW_MODE_3D = 0x1,
		VIEW_MODE_2D = 0x2,
		VIEW_MODE_IDENTITY = 0x3,
	};

	struct GfxCmdBufSourceState
	{
		GfxCodeMatrices matrices;
		$C1E24B5ACD1937141180CCC4F9D67481 u;
		char pad[592];
		/*GfxMatrix shadowLookupMatrix;
		unsigned __int16 constVersions[226];
		unsigned __int16 matrixVersions[8];*/
		float eyeOffset[4];
		unsigned int shadowableLightForShadowLookupMatrix;
		GfxScaledPlacement* objectPlacement;
		GfxViewParms* viewParms3D;
		unsigned int depthHackFlags;
		GfxScaledPlacement skinnedPlacement;
		int cameraView;
		GfxViewMode viewMode;
		GfxSceneDef sceneDef;
		GfxViewport sceneViewport;
		GfxViewport scissorViewport;
		float materialTime;
		float destructibleBurnAmount;
		float destructibleFadeAmount;
		float wetness;
		GfxViewportBehavior viewportBehavior;
		int renderTargetWidth;
		int renderTargetHeight;
		bool viewportIsDirty;
		bool scissorEnabled;
		unsigned int shadowableLightIndex;
	};
	STATIC_ASSERT_OFFSET(GfxCmdBufSourceState, pad, 0x17A0);
	STATIC_ASSERT_OFFSET(GfxCmdBufSourceState, eyeOffset, 0x19F0);
	STATIC_ASSERT_OFFSET(GfxCmdBufSourceState, cameraView, 0x1A30);
	STATIC_ASSERT_OFFSET(GfxCmdBufSourceState, scissorViewport, 0x1A5C);
	STATIC_ASSERT_OFFSET(GfxCmdBufSourceState, renderTargetWidth, 0x1A80);


	enum GfxDepthRangeType
	{
		GFX_DEPTH_RANGE_SCENE = 0x0,
		GFX_DEPTH_RANGE_VIEWMODEL = 0x2,
		GFX_DEPTH_RANGE_FULL = 0xFFFFFFFF,
	};

	struct GfxCmdBufState
	{
		char refSamplerState[16];
		unsigned int samplerState[16];
		GfxTexture* samplerTexture[16];
		GfxCmdBufPrimState prim;
		Material* material;
		char techType;
		MaterialTechnique* technique;
		MaterialPass* pass;
		unsigned int passIndex;
		GfxDepthRangeType depthRangeType;
		float depthRangeNear;
		float depthRangeFar;
		unsigned __int64 vertexShaderConstState[256];
		unsigned __int64 pixelShaderConstState[256];
		char alphaRef;
		unsigned int refStateBits[2];
		unsigned int activeStateBits[2];
		MaterialPixelShader* pixelShader;
		MaterialVertexShader* vertexShader;
		GfxViewport viewport;
		GfxViewport scissor;
		int scissorEnabled;
		char renderTargetId;
		Material* origMaterial;
		char origTechType;
		int stateOverride;
	};

	struct SavedScreenParams
	{
		float s0;
		float t0;
		float ds;
		float dt;
		int isSet;
	};

	struct r_global_permap_assets_t
	{
		Material* flameThrowerFXMaterial;
		Material* electrifiedFXMaterial;
		Material* transportedFXMaterial;
	};

	struct GfxStreamingAabbTree
	{
		unsigned __int16 firstItem;
		unsigned __int16 itemCount;
		unsigned __int16 firstChild;
		unsigned __int16 childCount;
		float mins[3];
		float maxs[3];
	};

	struct GfxWorldStreamInfo
	{
		int aabbTreeCount;
		GfxStreamingAabbTree* aabbTrees;
		int leafRefCount;
		int* leafRefs;
	};

	struct GfxWorldSunColor
	{
		unsigned int control;
		float angles[3];
		float ambientColor[4];
		float sunDiffuseColor[4];
		float sunSpecularColor[4];
		float skyColor[4];
		float groundColor[4];
		float exposure;
		float sunShadowSampleSizeNear;
		float skyboxHDRScale;
	};

	struct SunLightParseParams
	{
		char name[64];
		float treeScatterIntensity;
		float treeScatterAmount;
		GfxWorldSunColor sunSettings[1];
	};

	struct GfxSkyDynamicIntensity
	{
		float angle0;
		float angle1;
		float factor0;
		float factor1;
	};

	struct cplane_s
	{
		float normal[3];
		float dist;
		char type;
		char signbits;
		char pad[2];
	};

	struct GfxWorldDpvsPlanes
	{
		int cellCount;
		cplane_s* planes;
		unsigned __int16* nodes;
		unsigned int* sceneEntCellBits;
	};

	struct DpvsGlob_sunShadow
	{
		float viewDir[3];
		float viewDirDist;
		float sunShadowDrawDist;
	};

	struct DpvsGlob
	{
		DpvsPlane nearPlane;
		DpvsPlane farPlane;
		bool farPlaneEnabled;
		GfxMatrix* viewProjMtx;
		GfxMatrix* invViewProjMtx;
		GfxMatrix* projMtx;
		float viewOrg[4];
		int viewOrgIsDir;
		DpvsGlob_sunShadow sunShadow;
		int queuedCount;
		void* portalQueue;
		void* nextFreeHullPoints;
		float cullDist;
		int pad[3];
		DpvsPlane childPlanes[2048];
		DpvsView views[4][3];
		DpvsPlane* sideFrustumPlanes;
		unsigned int* entVisBits[4];
		unsigned int* cellCasterBitsForCell;
		unsigned int cellVisibleBits[32];
		unsigned int cellForceInvisibleBits[32];
		__declspec(align(16)) float occluderPlanes[320][4];
		int numOccluders;
	};

	struct GfxWorldVertex
	{
		float xyz[3];
		float binormalSign;
		GfxColor color;
		float texCoord[2];
		float lmapCoord[2];
		PackedUnitVec normal;
		PackedUnitVec tangent;
	};

	struct GfxWorldVertexData
	{
		GfxWorldVertex* vertices;
		IDirect3DVertexBuffer9* worldVb;
	};

	struct GfxWorldVertexLayerData
	{
		char* data;
		IDirect3DVertexBuffer9* layerVb;
	};

	struct GfxWorldDraw
	{
		unsigned int reflectionProbeCount;
		void* reflectionProbes; // GfxReflectionProbe
		GfxTexture* reflectionProbeTextures;
		int lightmapCount;
		void* lightmaps; // GfxLightmapArray
		GfxTexture* lightmapPrimaryTextures;
		GfxTexture* lightmapSecondaryTextures;
		GfxTexture* lightmapSecondaryTexturesB;
		GfxImage* terrainScorchImages[31];
		unsigned int vertexCount;
		GfxWorldVertexData vd;
		unsigned int vertexLayerDataSize;
		GfxWorldVertexLayerData vld;
		unsigned int vertexStream2DataSize;
		int indexCount;
		unsigned __int16* indices;
	};

	STATIC_ASSERT_OFFSET(GfxWorldDraw, indices, 0xBC);

	struct GfxLightGrid
	{
		bool hasLightRegions;
		unsigned int sunPrimaryLightIndex;
		unsigned __int16 mins[3];
		unsigned __int16 maxs[3];
		unsigned int rowAxis;
		unsigned int colAxis;
		unsigned __int16* rowDataStart;
		unsigned int rawRowDataSize;
		char* rawRowData;
		unsigned int entryCount;
		void* entries; // GfxLightGridEntry
		unsigned int colorCount;
		void* colors; // GfxCompressedLightGridColors
	};

	struct sunflare_t
	{
		bool hasValidData;
		Material* spriteMaterial;
		Material* flareMaterial;
		float spriteSize;
		float flareMinSize;
		float flareMinDot;
		float flareMaxSize;
		float flareMaxDot;
		float flareMaxAlpha;
		int flareFadeInTime;
		int flareFadeOutTime;
		float blindMinDot;
		float blindMaxDot;
		float blindMaxDarken;
		int blindFadeInTime;
		int blindFadeOutTime;
		float glareMinDot;
		float glareMaxDot;
		float glareMaxLighten;
		int glareFadeInTime;
		int glareFadeOutTime;
		float sunFxPosition[3];
	};

	struct GfxStaticModelInst
	{
		float mins[3];
		float maxs[3];
		float lightingOrigin[3];
		GfxColor groundLighting;
	};

	struct GfxSurface
	{
		srfTriangles_t tris;
		Material* material;
		char lightmapIndex;
		char reflectionProbeIndex;
		char primaryLightIndex;
		char flags;
		float bounds[2][3];
	};

	struct GfxCullGroup
	{
		float mins[3];
		float maxs[3];
		int surfaceCount;
		int startSurfIndex;
	};

	struct GfxPackedPlacement
	{
		float origin[3];
		float axis[3][3];
		float scale;
	};

	union $C8C573B57ACA1D7542AD56C4163862EC
	{
		unsigned __int16* localBoneNames;
		unsigned __int16* boneNames;
	};

	union $CB213585254F53F8EF5B6A00F1025158
	{
		char* localParentList;
		char* parentList;
	};

	union $BF9640C0B3AB5E078C286DD9616EC22F
	{
		__int16* localQuats;
		__int16* quats;
	};

	union $E1CD6A013C1D28F2956F4983A8D1052C
	{
		float* localTrans;
		float* trans;
	};

	struct DObjAnimMat
	{
		float quat[4];
		float trans[3];
		float transWeight;
	};

	struct XSurfaceVertexInfo
	{
		__int16 vertCount[4];
		unsigned __int16* vertsBlend;
		float* tensionData;
	};

	struct GfxPackedVertex
	{
		float xyz[3];
		float binormalSign;
		GfxColor color;
		PackedTexCoords texCoord;
		PackedUnitVec normal;
		PackedUnitVec tangent;
	};

	struct XRigidVertList
	{
		unsigned __int16 boneOffset;
		unsigned __int16 vertCount;
		unsigned __int16 triOffset;
		unsigned __int16 triCount;
		void* collisionTree; // XSurfaceCollisionTree
	};

	struct XSurface
	{
		char tileMode;
		char vertListCount;
		unsigned __int16 flags;
		unsigned __int16 vertCount;
		unsigned __int16 triCount;
		unsigned __int16 baseTriIndex;
		unsigned __int16 baseVertIndex;
		unsigned __int16* triIndices;
		XSurfaceVertexInfo vertInfo;
		GfxPackedVertex* verts0;
		IDirect3DVertexBuffer9* vb0;
		XRigidVertList* vertList;
		IDirect3DIndexBuffer9* indexBuffer;
		int partBits[5];
	};

	struct XModelLodInfo
	{
		float dist;
		unsigned __int16 numsurfs;
		unsigned __int16 surfIndex;
		int partBits[5];
		char lod;
		char smcIndexPlusOne;
		char smcAllocBits;
		char unused;
	};

	union $F7671BFCFF2FB6E444F43E0BA4207580
	{
		void* localBoneInfo; // XBoneInfo
		void* boneInfo; // XBoneInfo
	};

	struct XModelHighMipBounds
	{
		float center[3];
		float himipRadiusSq;
	};

	struct XModelStreamInfo
	{
		XModelHighMipBounds* highMipBounds;
	};

	struct XModel
	{
		const char* name;
		char numBones;
		char numRootBones;
		char numsurfs;
		char lodRampType;
		$C8C573B57ACA1D7542AD56C4163862EC u5;
		$CB213585254F53F8EF5B6A00F1025158 u6;
		$BF9640C0B3AB5E078C286DD9616EC22F u7;
		$E1CD6A013C1D28F2956F4983A8D1052C u8;
		char* partClassification;
		DObjAnimMat* baseMat;
		XSurface* surfs;
		Material** materialHandles;
		XModelLodInfo lodInfo[4];
		char lodDistAutoGenerated;
		void* collSurfs; // XModelCollSurf_s
		int numCollSurfs;
		int contents;
		$F7671BFCFF2FB6E444F43E0BA4207580 ___u18;
		float radius;
		float mins[3];
		float maxs[3];
		__int16 numLods;
		__int16 collLod;
		XModelStreamInfo streamInfo;
		int memUsage;
		int flags;
		bool bad;
		void* physPreset; // PhysPreset
		char numCollmaps;
		void* collmaps; // Collmap
		void* physConstraints; // PhysConstraints
	};

	struct GfxStaticModelDrawInst
	{
		float cullDist;
		GfxPackedPlacement placement;
		XModel* model;
		int flags;
		unsigned __int16 smodelCacheIndex[4];
		unsigned __int16 lightingHandle;
		char reflectionProbeIndex;
		char primaryLightIndex;
	};

	struct GfxWorldDpvsStatic
	{
		unsigned int smodelCount;
		unsigned int dynamicSModelCount;
		unsigned int staticSurfaceCount;
		unsigned int litSurfsBegin;
		unsigned int litSurfsEnd;
		unsigned int decalSurfsBegin;
		unsigned int decalSurfsEnd;
		unsigned int emissiveSurfsBegin;
		unsigned int emissiveSurfsEnd;
		unsigned int smodelVisDataCount;
		unsigned int surfaceVisDataCount;
		char* smodelVisData[3];
		char* surfaceVisData[3];
		char* smodelVisDataCameraSaved;
		char* surfaceVisDataCameraSaved;
		unsigned int* lodData;
		unsigned __int16* sortedSurfIndex;
		GfxStaticModelInst* smodelInsts;
		GfxSurface* surfaces;
		GfxCullGroup* cullGroups;
		GfxStaticModelDrawInst* smodelDrawInsts;
		GfxDrawSurf* surfaceMaterials;
		unsigned int* surfaceCastsSunShadow;
		volatile int usageCount;
	};

	struct GfxWorld
	{
		const char* name;
		const char* baseName;
		int planeCount;
		int nodeCount;
		int surfaceCount;
		GfxWorldStreamInfo streamInfo;
		int skySurfCount;
		int* skyStartSurfs;
		GfxImage* skyImage;
		char skySamplerState;
		const char* skyBoxModel;
		SunLightParseParams sunParse;
		GfxLight* sunLight;
		float sunColorFromBsp[3];
		unsigned int sunPrimaryLightIndex;
		unsigned int primaryLightCount;
		int cullGroupCount;
		unsigned int coronaCount;
		void* coronas; // GfxLightCorona
		unsigned int shadowMapVolumeCount;
		void* shadowMapVolumes; // GfxShadowMapVolume
		unsigned int shadowMapVolumePlaneCount;
		void* shadowMapVolumePlanes; // GfxVolumePlane
		unsigned int exposureVolumeCount;
		void* exposureVolumes; // GfxExposureVolume
		unsigned int exposureVolumePlaneCount;
		void* exposureVolumePlanes; // GfxVolumePlane
		GfxSkyDynamicIntensity skyDynIntensity;
		GfxWorldDpvsPlanes dpvsPlanes;
		int cellBitsCount;
		GfxCell* cells;
		GfxWorldDraw draw;
		GfxLightGrid lightGrid;
		int modelCount;
		void* models; // GfxBrushModel
		float mins[3];
		float maxs[3];
		unsigned int checksum;
		int materialMemoryCount;
		void* materialMemory; // MaterialMemory
		sunflare_t sun;
		float outdoorLookupMatrix[4][4];
		GfxImage* outdoorImage;
		unsigned int* cellCasterBits;
		void* sceneDynModel; // GfxSceneDynModel
		void* sceneDynBrush; // GfxSceneDynBrush
		unsigned int* primaryLightEntityShadowVis;
		unsigned int* primaryLightDynEntShadowVis[2];
		char* nonSunPrimaryLightForModelDynEnt;
		void* shadowGeom; // GfxShadowGeometry
		void* lightRegion; // GfxLightRegion
		GfxWorldDpvsStatic dpvs;
		/*GfxWorldDpvsDynamic dpvsDyn;
		unsigned int worldLodChainCount;
		GfxWorldLodChain* worldLodChains;
		unsigned int worldLodInfoCount;
		GfxWorldLodInfo* worldLodInfos;
		unsigned int worldLodSurfaceCount;
		unsigned int* worldLodSurfaces;
		float waterDirection;
		GfxWaterBuffer waterBuffers[2];
		Material* waterMaterial;
		Material* coronaMaterial;
		Material* ropeMaterial;
		unsigned int numOccluders;
		Occluder* occluders;
		unsigned int numOutdoorBounds;
		GfxOutdoorBounds* outdoorBounds;
		unsigned int heroLightCount;
		unsigned int heroLightTreeCount;
		GfxHeroLight* heroLights;
		GfxHeroLightTree* heroLightTree;*/
	};

	struct GfxTrianglesDrawStream
	{
		GfxTexture* whiteTexture;
		const unsigned int* primDrawSurfPos;
		GfxTexture* reflectionProbeTexture;
		GfxTexture* lightmapPrimaryTexture;
		GfxTexture* lightmapSecondaryTexture;
		GfxTexture* lightmapSecondaryTextureB;
		unsigned int customSamplerFlags;
		int hasSunDirChanged;
	};

	struct GfxDynamicIndices
	{
		volatile int used;
		int total;
		unsigned __int16* indices;
	};

	struct __declspec(align(4)) GfxBuffers
	{
		GfxDynamicIndices smodelCache;
		IDirect3DVertexBuffer9* smodelCacheVb;
		GfxIndexBufferState preTessIndexBufferPool[2];
		GfxIndexBufferState* preTessIndexBuffer;
		int preTessBufferFrame;
		GfxIndexBufferState dynamicIndexBufferPool[1];
		GfxIndexBufferState* dynamicIndexBuffer;
		GfxVertexBufferState skinnedCacheVbPool[2];
		char* skinnedCacheLockAddr;
		GfxVertexBufferState dynamicVertexBufferPool[1];
		GfxVertexBufferState* dynamicVertexBuffer;
	};

	struct GfxStaticModelDrawStream
	{
		const unsigned int* primDrawSurfPos;
		GfxTexture* reflectionProbeTexture;
		unsigned int customSamplerFlags;
		int usingCrossFade;
		XSurface* localSurf;
		unsigned int smodelCount;
		const unsigned __int16* smodelList;
		unsigned int reflectionProbeIndex;
		unsigned __int32 viewInfoIndex : 2;
		unsigned __int32 which_lod : 2;
		unsigned __int32 pad : 28;
		void* dynSModelView; // DynSModelClientView
		void* dynSModelState; // DynSModelGfxState
	};

	struct GfxModelSurfaceInfo
	{
		DObjAnimMat* baseMat;
		char boneIndex;
		char boneCount;
		unsigned __int16 gfxEntIndex;
		unsigned __int16 lightingHandle;
		char dobjModelIndex;
	};

	union $35D977DB71E523C40A6AAD0369DD9EE3
	{
		GfxPackedVertex* skinnedVert;
		int oldSkinnedCachedOffset;
	};

	struct GfxModelSkinnedSurface
	{
		int skinnedCachedOffset;
		XSurface* xsurf;
		GfxModelSurfaceInfo info;
		$35D977DB71E523C40A6AAD0369DD9EE3 u;
	};

	struct GfxModelRigidSurface
	{
		GfxModelSkinnedSurface surf;
		GfxScaledPlacement placement;
	};

	struct GfxCmdBufContext
	{
		GfxCmdBufSourceState* source;
		GfxCmdBufState* state;
	};

	struct GfxDrawSurfListArgs
	{
		GfxCmdBufContext context;
		unsigned int firstDrawSurfIndex;
		GfxDrawSurfListInfo* info;
	};

	struct GfxAabbTree
	{
		float mins[3];
		float maxs[3];
		unsigned __int16 childCount;
		unsigned __int16 surfaceCount;
		unsigned __int16 startSurfIndex;
		unsigned __int16 smodelIndexCount;
		unsigned __int16* smodelIndexes;
		int childrenOffset;
	};

	struct GfxPortalWritable
	{
		bool isQueued;
		bool isAncestor;
		char recursionDepth;
		char hullPointCount;
		float(*hullPoints)[2];
		GfxPortal* queuedParent;
	};

	struct GfxPortal
	{
		GfxPortalWritable writable;
		DpvsPlane plane;
		GfxCell* cell;
		float(*vertices)[3];
		char vertexCount;
		float hullAxis[2][3];
	};

	struct GfxCell
	{
		float mins[3];
		float maxs[3];
		int aabbTreeCount;
		GfxAabbTree* aabbTree;
		int portalCount;
		GfxPortal* portals;
		int cullGroupCount;
		int* cullGroups;
		char reflectionProbeCount;
		char* reflectionProbes;
	};


	struct __declspec(align(8)) SkinXModelCmd
	{
		void* modelSurfs;
		DObjAnimMat* mat;
		int surfacePartBits[5];
		float viewoffset[3];
		unsigned __int16 surfCount;
	};

	struct BModelSurface
	{
		GfxScaledPlacement* placement;
		GfxSurface* surf;
		ShaderConstantSet* shaderConstSet;
		float bmodelBurnAmt;
		float bmodelFadeAmt;
	};

#pragma warning(push)
#pragma warning(disable: 4324)
	struct __declspec(align(128)) r_global_permanent_t
	{
		Material* sortedMaterials[4096];
		int needSortMaterials;
		int materialCount;
		int needMaterialPreload;
		GfxImage* whiteImage;
		GfxImage* blackImage;
		GfxImage* blankImage;
		GfxImage* g16r16Image;
		GfxImage* r5g6b5Image;
		GfxImage* grayImage;
		GfxImage* identityNormalMapImage;
		GfxImage* whiteTransparentImage;
		GfxImage* blackTransparentImage;
		GfxImage* outdoorImage;
		GfxImage* heatMapImage;
		GfxImage* pixelCostColorCodeImage;
		void* dlightDef; // GfxLightDef
		void* flashLightDef; // GfxLightDef
		Material* defaultMaterial;
		Material* whiteMaterial;
		Material* additiveMaterial;
		Material* additiveMaterialNoDepth;
		Material* pointMaterial;
		Material* lineMaterial;
		Material* lineMaterialNoDepth;
		Material* blendMaterial;
		Material* blendMaterialNoDepth;
		Material* whiteDebugExteriorMaterial;
		Material* whiteDepthInteriorMaterial;
		Material* clearAlphaMaterial;
		Material* clearAlphaStencilMaterial;
		Material* setAlphaMaterial;
		Material* shadowClearMaterial;
		Material* shadowCasterMaterial;
		Material* shadowOverlayMaterial;
		Material* depthPrepassMaterial;
		Material* glareBlindMaterial;
		Material* lightCoronaMaterial;
		Material* pixelCostAddDepthAlwaysMaterial;
		Material* pixelCostAddDepthDisableMaterial;
		Material* pixelCostAddDepthEqualMaterial;
		Material* pixelCostAddDepthLessMaterial;
		Material* pixelCostAddDepthWriteMaterial;
		Material* pixelCostAddNoDepthWriteMaterial;
		Material* pixelCostColorCodeMaterial;
		Material* stencilShadowMaterial;
		Material* stencilDisplayMaterial;
		Material* floatZDisplayMaterial;
		Material* colorChannelMixerMaterial;
		Material* frameColorDebugMaterial;
		Material* frameAlphaDebugMaterial;
		GfxImage* rawImage;
		GfxWorld* world; // GfxWorld
		Material* feedbackReplaceMaterial;
		Material* feedbackBlendMaterial;
		Material* feedbackFilmBlendMaterial;
		Material* cinematicMaterial;
		Material* dofDownsampleMaterial;
		Material* dofDownsampleNvIntzMaterial;
		Material* dofNearCocMaterial;
		Material* smallBlurMaterial;
		Material* postFxDofMaterial;
		Material* postFxDofNvIntzMaterial;
		Material* postFxDofColorMaterial;
		Material* postFxDofColorNvIntzMaterial;
		Material* motionblur1Material;
		Material* motionblur2Material;
		Material* motionblur3Material;
		Material* motionblur4Material;
		Material* motionblur5Material;
		Material* motionblur6Material;
		Material* motionblur7Material;
		Material* motionblur8Material;
		Material* motionblurRadialMaterial;
		Material* motionblurFrameBasedMaterial;
		Material* redactMaterial1;
		Material* redactMaterial2;
		Material* redactMaterial3;
		Material* waterSheetingFXMaterial;
		Material* waterDropletMaterial;
		Material* reviveFXMaterial;
		Material* postFxColorMaterial;
		Material* postFxLutMaterial[10];
		Material* postFxMaterial;
		Material* poisonFXMaterial;
		Material* symmetricFilterMaterial[8];
		Material* godRaysFilterMaterial;
		Material* shellShockBlurredMaterial;
		Material* shellShockFlashedMaterial;
		Material* glowConsistentSetupMaterial;
		Material* glowApplyBloomMaterial;
		Material* ropeMaterial;
		int savedScreenTimes[4];
		SavedScreenParams savedScreenParams[4];
		GfxImage* r32fWhiteImage;
		Material* resampleFinal;
		Material* resampleShift;
		Material* bloomDownsampleLDR;
		Material* bloomDownsampleHDR;
		Material* bloomDownsampleConvolution;
		Material* bloomRemap;
		Material* bloomExpansion;
		Material* bloomRemapStreak;
		Material* bloomStreakX;
		Material* bloomStreakY;
		Material* bloomApplyStreak;
		Material* bloomBlurX;
		Material* bloomBlurY;
		Material* bloomMip3Blur;
		Material* bloomApplyHDR;
		Material* bloomApplyLDR;
		Material* bloomSmoothA;
		Material* bloomSmoothB;
		Material* superFlareApply;
		Material* superFlareOccluder;
		Material* superFlareOccluderDebug;
		Material* superFlareOccluderNoZ;
		Material* superFlareOccluderNoZDebug;
		Material* createLut2d;
		Material* createLut2dv;
		Material* applyLut2d;
		Material* applyLut3d;
		Material* dummyMaterial;
		Material* compositeResult;
		Material* infraredWhite;
		r_global_permap_assets_t permapAssets;
	};
#pragma warning(pop)

	// codetextures
	enum MaterialTextureSource
	{
		TEXTURE_SRC_CODE_BLACK = 0x0,
		TEXTURE_SRC_CODE_WHITE = 0x1,
		TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP = 0x2,
		TEXTURE_SRC_CODE_MODEL_LIGHTING = 0x3,
		TEXTURE_SRC_CODE_LIGHTMAP_PRIMARY = 0x4,
		TEXTURE_SRC_CODE_LIGHTMAP_SECONDARY = 0x5,
		TEXTURE_SRC_CODE_SHADOWMAP_SUN = 0x6,
		TEXTURE_SRC_CODE_SHADOWMAP_SPOT = 0x7,
		TEXTURE_SRC_CODE_FEEDBACK = 0x8,
		TEXTURE_SRC_CODE_RESOLVED_POST_SUN = 0x9,
		TEXTURE_SRC_CODE_RESOLVED_SCENE = 0xA,
		TEXTURE_SRC_CODE_POST_EFFECT_SRC = 0xB,
		TEXTURE_SRC_CODE_POST_EFFECT_GODRAYS = 0xC,
		TEXTURE_SRC_CODE_POST_EFFECT_0 = 0xD,
		TEXTURE_SRC_CODE_POST_EFFECT_1 = 0xE,
		TEXTURE_SRC_CODE_SKY = 0xF,
		TEXTURE_SRC_CODE_LIGHT_ATTENUATION = 0x10,
		TEXTURE_SRC_CODE_DLIGHT_ATTENUATION = 0x11,
		TEXTURE_SRC_CODE_OUTDOOR = 0x12,
		TEXTURE_SRC_CODE_FLOATZ = 0x13,
		TEXTURE_SRC_CODE_PROCESSED_FLOATZ = 0x14,
		TEXTURE_SRC_CODE_RAW_FLOATZ = 0x15,
		TEXTURE_SRC_CODE_CASE_TEXTURE = 0x16,
		TEXTURE_SRC_CODE_CINEMATIC_Y = 0x17,
		TEXTURE_SRC_CODE_CINEMATIC_CR = 0x18,
		TEXTURE_SRC_CODE_CINEMATIC_CB = 0x19,
		TEXTURE_SRC_CODE_CINEMATIC_A = 0x1A,
		TEXTURE_SRC_CODE_REFLECTION_PROBE = 0x1B,
		TEXTURE_SRC_CODE_FEATHER_FLOAT_Z = 0x1C,
		TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_0 = 0x1D,
		TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_1 = 0x1E,
		TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_2 = 0x1F,
		TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_3 = 0x20,
		TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_LAST = 0x20,
		TEXTURE_SRC_CODE_LIGHTMAP_SECONDARYB = 0x21,
		TEXTURE_SRC_CODE_TEXTURE_0 = 0x22,
		TEXTURE_SRC_CODE_TEXTURE_1 = 0x23,
		TEXTURE_SRC_CODE_TEXTURE_2 = 0x24,
		TEXTURE_SRC_CODE_TEXTURE_3 = 0x25,
		TEXTURE_SRC_CODE_IMPACT_MASK = 0x26,
		TEXTURE_SRC_CODE_UI3D = 0x27,
		TEXTURE_SRC_CODE_MISSILE_CAM = 0x28,
		TEXTURE_SRC_CODE_COMPOSITE_RESULT = 0x29,
		TEXTURE_SRC_CODE_HEATMAP = 0x2A,
		TEXTURE_SRC_CODE_COUNT = 0x2B,
	};

	struct GfxRenderTargetSurface
	{
		IDirect3DSurface9* color;
		IDirect3DSurface9* depthStencil;
	};

	struct Ui3dTextureWindow
	{
		GfxViewport vp;
		ScreenPlacement scrPlace;
		float normX;
		float normY;
		float normW;
		float normH;
		int numRenderCmds;
	};

	struct GfxRenderTarget
	{
		GfxImage* image;
		GfxRenderTargetSurface surface;
		unsigned __int16 width;
		unsigned __int16 height;
		bool cleared;
	};

	enum sysEventType_t
	{
		SE_NONE = 0x0,
		SE_KEY = 0x1,
		SE_CHAR = 0x2,
		SE_CONSOLE = 0x3,
	};

	struct sysEvent_t
	{
		int evTime;
		sysEventType_t evType;
		int evValue;
		int evValue2;
		int evPtrLength;
		void* evPtr;
	};

	enum DB_FILE_EXISTS_PATH
	{
		DB_PATH_ZONE = 0,
		DB_PATH_MOD = 1,
		DB_PATH_USERMAPS = 2
	};

	struct XZoneInfo
	{
		const char* name;
		int allocFlags;
		int freeFlags;
	};

	enum dvarType_t
	{
		DVAR_TYPE_BOOL = 0x0,
		DVAR_TYPE_FLOAT = 0x1,
		DVAR_TYPE_FLOAT_2 = 0x2,
		DVAR_TYPE_FLOAT_3 = 0x3,
		DVAR_TYPE_FLOAT_4 = 0x4,
		DVAR_TYPE_INT = 0x5,
		DVAR_TYPE_ENUM = 0x6,
		DVAR_TYPE_STRING = 0x7,
		DVAR_TYPE_COLOR = 0x8,
		DVAR_TYPE_INT64 = 0x9,
		DVAR_TYPE_LINEAR_COLOR_RGB = 0xA,
		DVAR_TYPE_COLOR_XYZ = 0xB,
		DVAR_TYPE_COUNT = 0xC,
	};

	union DvarValue
	{
		bool enabled;
		int integer;
		unsigned int unsignedInt;
		__int64 integer64;
		unsigned __int64 unsignedInt64;
		float value;
		float vector[4];
		const char* string;
		char color[4];
	};

	union DvarLimits
	{
		struct
		{
			int stringCount;
			const char** strings;
		} enumeration;

		struct
		{
			int min;
			int max;
		} integer;

		struct
		{
			std::int64_t min;
			std::int64_t max;
		} integer64;

		struct
		{
			float min;
			float max;
		} value;

		struct
		{
			float min;
			float max;
		} vector;
	}; STATIC_ASSERT_SIZE(DvarLimits, 0x10);


	enum dvar_flags : int
	{
		none = 0,
		archive = 1 << 0,			// 0x0001
		userinfo = 1 << 1,			// 0x0002
		serverinfo = 1 << 2,		// 0x0004
		systeminfo = 1 << 3,		// 0x0008
		init_flag = 1 << 4,			// 0x0010
		latched = 1 << 5,			// 0x0020
		readonly = 1 << 6,			// 0x0040
		cheat = 1 << 7,				// 0x0080
		developer = 1 << 8,			// 0x0100
		saved = 1 << 9,				// 0x0200
		no_restart = 1 << 10,		// 0x0400
		changeable_reset = 1 << 12,	// 0x1000
		external = 1 << 14,			// 0x4000
		autoexec = 1 << 15,			// 0x8000
		unrestricted = 1 << 16,		// 0x10000
	};

	struct dvar_s
	{
		const char* name;
		const char* description;
		int hash;
		unsigned int flags;
		dvarType_t type;
		bool modified;
		bool loadedFromSaveGame;
		DvarValue current;
		DvarValue latched;
		DvarValue reset;
		DvarValue saved;
		DvarLimits domain;
		dvar_s* hashNext;
	};

	enum MaterialTechniqueType : std::uint8_t
	{
		TECHNIQUE_DEPTH_PREPASS = 0x0,
		TECHNIQUE_BUILD_FLOAT_Z = 0x1,
		TECHNIQUE_BUILD_SHADOWMAP_DEPTH = 0x2,
		TECHNIQUE_BUILD_SHADOWMAP_COLOR = 0x3,
		TECHNIQUE_UNLIT = 0x4,
		TECHNIQUE_EMISSIVE = 0x5,
		TECHNIQUE_EMISSIVE_SHADOW = 0x6,
		TECHNIQUE_EMISSIVE_NV_INTZ = 0x7,
		TECHNIQUE_EMISSIVE_SHADOW_NV_INTZ = 0x8,
		TECHNIQUE_EMISSIVE_REFLECTED = 0x9,
		TECHNIQUE_LIT_BEGIN = 0xA,
		TECHNIQUE_LIT = 0xA,
		TECHNIQUE_LIT_SUN = 0xB,
		TECHNIQUE_LIT_SUN_SHADOW = 0xC,
		TECHNIQUE_LIT_SPOT = 0xD,
		TECHNIQUE_LIT_SPOT_SHADOW = 0xE,
		TECHNIQUE_LIT_OMNI = 0xF,
		TECHNIQUE_LIT_OMNI_SHADOW = 0x10,
		TECHNIQUE_LIT_DLIGHT = 0x11,
		TECHNIQUE_LIT_SUN_DLIGHT = 0x12,
		TECHNIQUE_LIT_SUN_SHADOW_DLIGHT = 0x13,
		TECHNIQUE_LIT_SPOT_DLIGHT = 0x14,
		TECHNIQUE_LIT_SPOT_SHADOW_DLIGHT = 0x15,
		TECHNIQUE_LIT_OMNI_DLIGHT = 0x16,
		TECHNIQUE_LIT_OMNI_SHADOW_DLIGHT = 0x17,
		TECHNIQUE_LIT_GLIGHT = 0x18,
		TECHNIQUE_LIT_SUN_GLIGHT = 0x19,
		TECHNIQUE_LIT_SUN_SHADOW_GLIGHT = 0x1A,
		TECHNIQUE_LIT_SPOT_GLIGHT = 0x1B,
		TECHNIQUE_LIT_SPOT_SHADOW_GLIGHT = 0x1C,
		TECHNIQUE_LIT_OMNI_GLIGHT = 0x1D,
		TECHNIQUE_LIT_OMNI_SHADOW_GLIGHT = 0x1E,
		TECHNIQUE_LIT_DLIGHT_GLIGHT = 0x1F,
		TECHNIQUE_LIT_SUN_DLIGHT_GLIGHT = 0x20,
		TECHNIQUE_LIT_SUN_SHADOW_DLIGHT_GLIGHT = 0x21,
		TECHNIQUE_LIT_SPOT_DLIGHT_GLIGHT = 0x22,
		TECHNIQUE_LIT_SPOT_SHADOW_DLIGHT_GLIGHT = 0x23,
		TECHNIQUE_LIT_OMNI_DLIGHT_GLIGHT = 0x24,
		TECHNIQUE_LIT_OMNI_SHADOW_DLIGHT_GLIGHT = 0x25,
		TECHNIQUE_LIT_ALPHA = 0x26,
		TECHNIQUE_LIT_SUN_ALPHA = 0x27,
		TECHNIQUE_LIT_SUN_SHADOW_ALPHA = 0x28,
		TECHNIQUE_LIT_SPOT_ALPHA = 0x29,
		TECHNIQUE_LIT_SPOT_SHADOW_ALPHA = 0x2A,
		TECHNIQUE_LIT_OMNI_ALPHA = 0x2B,
		TECHNIQUE_LIT_OMNI_SHADOW_ALPHA = 0x2C,
		TECHNIQUE_LIT_REMAP = 0x2D,
		TECHNIQUE_LIT_SUN_REMAP = 0x2E,
		TECHNIQUE_LIT_SUN_SHADOW_REMAP = 0x2F,
		TECHNIQUE_LIT_SPOT_REMAP = 0x30,
		TECHNIQUE_LIT_SPOT_SHADOW_REMAP = 0x31,
		TECHNIQUE_LIT_OMNI_REMAP = 0x32,
		TECHNIQUE_LIT_OMNI_SHADOW_REMAP = 0x33,
		TECHNIQUE_LIT_NO_HDR_SUPPORT = 0x34,
		TECHNIQUE_LIT_FADE = 0x34,
		TECHNIQUE_LIT_SUN_FADE = 0x35,
		TECHNIQUE_LIT_SUN_SHADOW_FADE = 0x36,
		TECHNIQUE_LIT_SPOT_FADE = 0x37,
		TECHNIQUE_LIT_SPOT_SHADOW_FADE = 0x38,
		TECHNIQUE_LIT_OMNI_FADE = 0x39,
		TECHNIQUE_LIT_OMNI_SHADOW_FADE = 0x3A,
		TECHNIQUE_LIT_CHARRED = 0x3B,
		TECHNIQUE_LIT_FADE_CHARRED = 0x3C,
		TECHNIQUE_LIT_SUN_CHARRED = 0x3D,
		TECHNIQUE_LIT_SUN_FADE_CHARRED = 0x3E,
		TECHNIQUE_LIT_SUN_SHADOW_CHARRED = 0x3F,
		TECHNIQUE_LIT_SUN_SHADOW_FADE_CHARRED = 0x40,
		TECHNIQUE_LIT_SPOT_CHARRED = 0x41,
		TECHNIQUE_LIT_SPOT_FADE_CHARRED = 0x42,
		TECHNIQUE_LIT_SPOT_SHADOW_CHARRED = 0x43,
		TECHNIQUE_LIT_SPOT_SHADOW_FADE_CHARRED = 0x44,
		TECHNIQUE_LIT_OMNI_CHARRED = 0x45,
		TECHNIQUE_LIT_OMNI_FADE_CHARRED = 0x46,
		TECHNIQUE_LIT_OMNI_SHADOW_CHARRED = 0x47,
		TECHNIQUE_LIT_OMNI_SHADOW_FADE_CHARRED = 0x48,
		TECHNIQUE_LIT_INSTANCED = 0x49,
		TECHNIQUE_LIT_INSTANCED_SUN = 0x4A,
		TECHNIQUE_LIT_INSTANCED_SUN_SHADOW = 0x4B,
		TECHNIQUE_LIT_INSTANCED_SPOT = 0x4C,
		TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW = 0x4D,
		TECHNIQUE_LIT_INSTANCED_OMNI = 0x4E,
		TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW = 0x4F,
		TECHNIQUE_LIT_NV_BEGIN = 0x50,
		TECHNIQUE_LIT_NV_INTZ = 0x50,
		TECHNIQUE_LIT_SUN_NV_INTZ = 0x51,
		TECHNIQUE_LIT_SUN_SHADOW_NV_INTZ = 0x52,
		TECHNIQUE_LIT_SPOT_NV_INTZ = 0x53,
		TECHNIQUE_LIT_SPOT_SHADOW_NV_INTZ = 0x54,
		TECHNIQUE_LIT_OMNI_NV_INTZ = 0x55,
		TECHNIQUE_LIT_OMNI_SHADOW_NV_INTZ = 0x56,
		TECHNIQUE_LIT_DLIGHT_NV_INTZ = 0x57,
		TECHNIQUE_LIT_SUN_DLIGHT_NV_INTZ = 0x58,
		TECHNIQUE_LIT_SUN_SHADOW_DLIGHT_NV_INTZ = 0x59,
		TECHNIQUE_LIT_SPOT_DLIGHT_NV_INTZ = 0x5A,
		TECHNIQUE_LIT_SPOT_SHADOW_DLIGHT_NV_INTZ = 0x5B,
		TECHNIQUE_LIT_OMNI_DLIGHT_NV_INTZ = 0x5C,
		TECHNIQUE_LIT_OMNI_SHADOW_DLIGHT_NV_INTZ = 0x5D,
		TECHNIQUE_LIT_GLIGHT_NV_INTZ = 0x5E,
		TECHNIQUE_LIT_SUN_GLIGHT_NV_INTZ = 0x5F,
		TECHNIQUE_LIT_SUN_SHADOW_GLIGHT_NV_INTZ = 0x60,
		TECHNIQUE_LIT_SPOT_GLIGHT_NV_INTZ = 0x61,
		TECHNIQUE_LIT_SPOT_SHADOW_GLIGHT_NV_INTZ = 0x62,
		TECHNIQUE_LIT_OMNI_GLIGHT_NV_INTZ = 0x63,
		TECHNIQUE_LIT_OMNI_SHADOW_GLIGHT_NV_INTZ = 0x64,
		TECHNIQUE_LIT_DLIGHT_GLIGHT_NV_INTZ = 0x65,
		TECHNIQUE_LIT_SUN_DLIGHT_GLIGHT_NV_INTZ = 0x66,
		TECHNIQUE_LIT_SUN_SHADOW_DLIGHT_GLIGHT_NV_INTZ = 0x67,
		TECHNIQUE_LIT_SPOT_DLIGHT_GLIGHT_NV_INTZ = 0x68,
		TECHNIQUE_LIT_SPOT_SHADOW_DLIGHT_GLIGHT_NV_INTZ = 0x69,
		TECHNIQUE_LIT_OMNI_DLIGHT_GLIGHT_NV_INTZ = 0x6A,
		TECHNIQUE_LIT_OMNI_SHADOW_DLIGHT_GLIGHT_NV_INTZ = 0x6B,
		TECHNIQUE_LIT_INSTANCED_NV_INTZ = 0x6C,
		TECHNIQUE_LIT_INSTANCED_SUN_NV_INTZ = 0x6D,
		TECHNIQUE_LIT_INSTANCED_SUN_SHADOW_NV_INTZ = 0x6E,
		TECHNIQUE_LIT_INSTANCED_SPOT_NV_INTZ = 0x6F,
		TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW_NV_INTZ = 0x70,
		TECHNIQUE_LIT_INSTANCED_OMNI_NV_INTZ = 0x71,
		TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW_NV_INTZ = 0x72,
		TECHNIQUE_LIT_NV_END = 0x73,
		TECHNIQUE_LIT_END = 0x73,
		TECHNIQUE_LIGHT_SPOT = 0x73,
		TECHNIQUE_LIGHT_OMNI = 0x74,
		TECHNIQUE_LIGHT_SPOT_SHADOW = 0x75,
		TECHNIQUE_LIGHT_SPOT_CHARRED = 0x76,
		TECHNIQUE_LIGHT_OMNI_CHARRED = 0x77,
		TECHNIQUE_LIGHT_SPOT_SHADOW_CHARRED = 0x78,
		TECHNIQUE_FAKELIGHT_NORMAL = 0x79,
		TECHNIQUE_FAKELIGHT_VIEW = 0x7A,
		TECHNIQUE_SUNLIGHT_PREVIEW = 0x7B,
		TECHNIQUE_CASE_TEXTURE = 0x7C,
		TECHNIQUE_WIREFRAME_SOLID = 0x7D,
		TECHNIQUE_WIREFRAME_SHADED = 0x7E,
		TECHNIQUE_DEBUG_BUMPMAP = 0x7F,
		TECHNIQUE_DEBUG_BUMPMAP_INSTANCED = 0x80,
		TECHNIQUE_IMPACT_MASK = 0x81,
		TECHNIQUE_COUNT = 0x82,
		TECHNIQUE_TOTAL_COUNT = 0x83,
		TECHNIQUE_NONE = 0xFF,
	};

	struct switch_material_t
	{
		bool switch_material;
		bool switch_technique;
		bool switch_technique_type;

		Material* current_material;
		MaterialTechnique* current_technique;

		Material* material;
		MaterialTechnique* technique;

		MaterialTechniqueType technique_type;
	};

	// ------------------------------------------------------------------------------------------------------------
	// ############################################################################################################
	// ------------------------------------------------------------------------------------------------------------

	namespace sp
	{

	}

	// ------------------------------------------------------------------------------------------------------------
	// ############################################################################################################
	// ------------------------------------------------------------------------------------------------------------

	namespace mp
	{

	}

#pragma warning(pop)
}

