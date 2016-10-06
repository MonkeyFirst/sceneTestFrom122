#include "Urho3DAll.h"

using namespace Urho3D;
class MyApp : public Application
{
public:
	Scene* scene_;
	Viewport* viewport_;
	Camera* camera_;
	Node* cameraNode_;
	Node* sceneNode_;
	Node* lightNode_;
	Light* light_;

	DebugHud* debugHud_;
	Vector<SharedPtr<Material>> colorBoxMat_;
	Vector<SharedPtr<Material>> colorTeaPotMat_;


public:
    MyApp(Context* context) :
        Application(context)
    {
    }
    virtual void Setup()
    {
        // Called before engine initialization. engineParameters_ member variable can be modified here
#if 0
		engineParameters_["WindowWidth"] = 1280;
		engineParameters_["WindowHeight"] = 720;
		engineParameters_["FullScreen"] = false;
#else
		//engineParameters_["WindowWidth"] = 1920;
		//engineParameters_["WindowHeight"] = 1080;
		engineParameters_["FullScreen"] = true;
#endif
		engineParameters_["VSync"] = false;
		engineParameters_["FrameLimiter"] = false;
		engineParameters_["RenderPath"] = "bin/CoreData/RenderPaths/Forward.xml";
    }
    virtual void Start()
    {
		CreateConsoleAndDebug();
		CreateScene();
		CreateCamera();
		CreateLights();
		GenerateMats();
		FillScene();
        // Called after engine initialization. Setup application & subscribe to events here
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MyApp, HandleKeyDown));
		SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MyApp, HandleUpdate));

    }
    virtual void Stop()
    {
        // Perform optional cleanup after main loop has terminated
    }
    void HandleKeyDown(StringHash eventType, VariantMap& eventData)
    {
        using namespace KeyDown;
        // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE)
            engine_->Exit();

		if (key == KEY_F1)
			GetSubsystem<Console>()->Toggle();
			
		if (key == KEY_F3)
			debugHud_->ToggleAll();

    }
	void HandleUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace Update;
		float timeStep = eventData[P_TIMESTEP].GetFloat();
		Quaternion q = Quaternion(5.0f * timeStep, -20.0f * timeStep, 10.0f * timeStep);
		sceneNode_->SetRotation(sceneNode_->GetRotation() * q);
	}
	void CreateConsoleAndDebug()
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		XMLFile* file = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

		Console* console = engine_->CreateConsole();
		console->SetDefaultStyle(file);
		console->GetBackground()->SetOpacity(0.8f);

		debugHud_ = engine_->CreateDebugHud();
		debugHud_->SetDefaultStyle(file);
	}
	void CreateScene() 
	{
		scene_ = new Scene(context_);
		Octree* oct = scene_->CreateComponent<Octree>();
		const int side = 1000;
		const int levels = 3;
		oct->SetSize(BoundingBox(Vector3(-side,-side,-side), Vector3(side, side, side)), levels);

		Zone* fog = scene_->CreateComponent<Zone>();
		fog->SetBoundingBox(BoundingBox(Vector3(-100, -100, -100), Vector3(100, 100, 100)));
		fog->SetFogColor(Color(0.1f, 0.1f, 0.1f));
		fog->SetFogStart(8.0f);
		fog->SetFogEnd(25.0f);
		fog->SetAmbientColor(Color(0.15f, 0.15f, 0.1f));

		sceneNode_ = scene_->CreateChild();
		cameraNode_ = scene_->CreateChild();
		cameraNode_->Translate(Vector3(0.0f, 0.0f, -15.0f));
	}
	void CreateCamera() 
	{
		camera_ = cameraNode_->CreateComponent<Camera>();
		camera_->SetFov(45.0f);
		camera_->SetNearClip(1.0f);
		camera_->SetFarClip(50.0f);
		viewport_ = new Viewport(context_, scene_, camera_);
		
		GetSubsystem<Renderer>()->SetViewport(0, viewport_);
	}
	void CreateLights() 
	{
		lightNode_ = scene_->CreateChild();
		lightNode_->Translate(Vector3(0, 0, 0));
		light_ = lightNode_->CreateComponent<Light>();
		light_->SetLightType(LightType::LIGHT_POINT);
		light_->SetRange(100.0f);
		light_->SetColor(Color(1.0f, 1.0f, 1.0f));
		light_->SetEnabled(true);
		light_->SetCastShadows(true);
		light_->SetBrightness(3.0f);
		light_->SetPerVertex(true);

	}
	void GenerateMats() 
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		Material* baseMat = cache->GetResource<Material>("Materials/ColorBox.xml");
		colorBoxMat_.Resize(11);
		colorTeaPotMat_.Resize(11);
		for (int i = 0; i < 10; ++i) 
		{
			colorBoxMat_[i] = baseMat->Clone(String(i));
			//colorBoxMat_[i]->SetVertexShaderDefines("TRANSLUCENT");
			//colorBoxMat_[i]->SetPixelShaderDefines("TRANSLUCENT");
			colorBoxMat_[i]->SetShaderParameter("UOffset", Vector4(0.1f, 0.0f, 0.0f, 0.1f * i));
			colorBoxMat_[i]->SetShaderParameter("VOffset", Vector4(0.0f, 1.0f, 0.0f, 0.0f));

			colorTeaPotMat_[i] = baseMat->Clone();
			//colorTeaPotMat_[i]->SetVertexShaderDefines("TRANSLUCENT");
			//colorTeaPotMat_[i]->SetPixelShaderDefines("TRANSLUCENT");
			colorTeaPotMat_[i]->SetShaderParameter("UOffset", Vector4(0.1f, 0.0f, 0.0f, 0.1f * i));
			colorTeaPotMat_[i]->SetShaderParameter("VOffset", Vector4(0.0f, 1.0f, 0.0f, 0.0f));
			colorTeaPotMat_[i]->SetCullMode(CULL_NONE);

		}

		colorBoxMat_[10] = cache->GetResource<Material>("Materials/Water2.xml");

	}
	void AddCube(int c, Vector3 pos) 
	{
		static ResourceCache* cache = GetSubsystem<ResourceCache>();
		static Model* boxModel = cache->GetResource<Model>("Models/Box.mdl");


		Node* node = sceneNode_->CreateChild();
		node->SetPosition(pos);
		StaticModel* sm = node->CreateComponent<StaticModel>();
		sm->SetModel(boxModel);
		sm->SetMaterial(0, colorBoxMat_[c]);
		
		//sm->SetCastShadows(true);
	}

	void AddTeaPot(int c, Vector3 pos)
	{
		static ResourceCache* cache = GetSubsystem<ResourceCache>();
		static Model* boxModel = cache->GetResource<Model>("Models/teapot.mdl");
	
		Node* node = sceneNode_->CreateChild();
		node->SetPosition(pos);
		node->SetScale(Vector3::ONE * 2.3f);
		StaticModel* sm = node->CreateComponent<StaticModel>();
		sm->SetModel(boxModel);
		sm->SetMaterial(0, colorTeaPotMat_[c]);
		//sm->SetCastShadows(true);
	}

	void FillScene() 
	{
		int x, y, w;
		int color = 0;
		for (w = -13; w <= 11; w += 7) {
			for (x = -13; x <= 11; x += 7) {
				
				if (x == 8 && w == 1) 
				{ 
					color = 2;
				}
				else 
				{ 
					color = 0;
				}
				
				if ((x == 1 && w == 8) || (x == -6 && w == -6)) 
				{ 
					color = 1;
				}
				
				AddTeaPot(color, Vector3(x + (w % 2) * 2, -3, w + (x % 2) * 2));   // teapot model
			}
		}
		
		for (w = -15; w <= 15; w++) {
			for (x = -15; x <= 15; x++) {
				if ((abs(x) + abs(w))<8) {
					AddCube(0, Vector3(x, -7.0f, w));                 // cube model
				}
				if ((abs(x) + abs(w))>5) {
					//Add_Cube(colour_two, x, -4, w);				// cube model
					AddCube(2, Vector3(x, -4.0f, w));
				}
				if ((abs(x) + abs(w))>6) {
					//Add_Cube(colour_one, x, 5, w);                  // cube model
					AddCube(1, Vector3(x, 5.0f, w));
				}
				else {
					AddCube(10, Vector3(x, 5.0f, w));                  // glass cube model
				}
			}
		}


		for (w = -7; w <= 7; w++) 
		{
			for (x = -7; x <= 7; x++) {
				for (y = -9; y <= -5; y++) {
					if (Random(0, 99)<15) {
						
						AddCube(Random(0,10), Vector3(x, y, w));               // cube model
					}
				}
			}
		}

		for (w = -9; w <= 9; w++) {
			for (x = -9; x <= 9; x++) {
				for (y = 4; y <= 12; y++) {
					if (Random(0, 99) < 8) {
						if (y == 4) 
						{ color = 0; }
						else 
						{ color = 1; }
						
						AddCube(color, Vector3(x, y, w));                      // cube model
					}
				}
			}
		}
	}
};
URHO3D_DEFINE_APPLICATION_MAIN(MyApp)
