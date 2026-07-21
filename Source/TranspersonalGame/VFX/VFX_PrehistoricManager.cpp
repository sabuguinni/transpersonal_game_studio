#include "VFX_PrehistoricManager.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AVFX_PrehistoricManager::AVFX_PrehistoricManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
    RootComponent = RootMeshComponent;

    // Create Niagara VFX component
    MainVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MainVFX"));
    MainVFXComponent->SetupAttachment(RootComponent);

    // Initialize default values
    CurrentEffectType = EVFX_EffectType::Fire_Campfire;
    EffectSettings = FVFX_EffectSettings();
    EffectTimer = 0.0f;
    bEffectActive = false;

    // Set default VFX asset paths (will be loaded at runtime)
    CampfireVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Campfire")));
    FootstepImpactVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_FootstepImpact")));
    DustCloudVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_DustCloud")));
    WaterSplashVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_WaterSplash")));
    RainVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Rain")));
    BloodImpactVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_BloodImpact")));
}

void AVFX_PrehistoricManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Load initial VFX asset
    LoadVFXAsset(CurrentEffectType);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_PrehistoricManager: BeginPlay - Effect Type: %d"), (int32)CurrentEffectType);
}

void AVFX_PrehistoricManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEffectActive)
    {
        HandleEffectTimer(DeltaTime);
        UpdateVFXParameters();
    }
}

void AVFX_PrehistoricManager::PlayEffect(EVFX_EffectType EffectType, const FVFX_EffectSettings& Settings)
{
    CurrentEffectType = EffectType;
    EffectSettings = Settings;
    
    // Load appropriate VFX asset
    LoadVFXAsset(EffectType);
    
    // Start the effect
    if (MainVFXComponent && MainVFXComponent->GetAsset())
    {
        MainVFXComponent->Activate(true);
        bEffectActive = true;
        EffectTimer = 0.0f;
        
        // Apply initial settings
        SetEffectIntensity(Settings.Intensity);
        SetEffectScale(Settings.Scale);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_PrehistoricManager: Playing effect type %d with intensity %.2f"), 
               (int32)EffectType, Settings.Intensity);
    }
}

void AVFX_PrehistoricManager::StopEffect()
{
    if (MainVFXComponent)
    {
        MainVFXComponent->Deactivate();
        bEffectActive = false;
        EffectTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("VFX_PrehistoricManager: Effect stopped"));
    }
}

void AVFX_PrehistoricManager::SetEffectIntensity(float NewIntensity)
{
    EffectSettings.Intensity = FMath::Clamp(NewIntensity, 0.0f, 10.0f);
    
    if (MainVFXComponent)
    {
        MainVFXComponent->SetFloatParameter(TEXT("Intensity"), EffectSettings.Intensity);
    }
}

void AVFX_PrehistoricManager::SetEffectScale(const FVector& NewScale)
{
    EffectSettings.Scale = NewScale;
    
    if (MainVFXComponent)
    {
        MainVFXComponent->SetVectorParameter(TEXT("Scale"), EffectSettings.Scale);
        MainVFXComponent->SetRelativeScale3D(NewScale);
    }
}

void AVFX_PrehistoricManager::TestCampfireEffect()
{
    FVFX_EffectSettings TestSettings;
    TestSettings.Intensity = 1.5f;
    TestSettings.Duration = 10.0f;
    TestSettings.Scale = FVector(1.2f, 1.2f, 1.0f);
    
    PlayEffect(EVFX_EffectType::Fire_Campfire, TestSettings);
}

void AVFX_PrehistoricManager::TestFootstepEffect()
{
    FVFX_EffectSettings TestSettings;
    TestSettings.Intensity = 2.0f;
    TestSettings.Duration = 3.0f;
    TestSettings.Scale = FVector(1.5f, 1.5f, 0.8f);
    
    PlayEffect(EVFX_EffectType::Impact_Footstep, TestSettings);
}

void AVFX_PrehistoricManager::TestDustEffect()
{
    FVFX_EffectSettings TestSettings;
    TestSettings.Intensity = 1.0f;
    TestSettings.Duration = 5.0f;
    TestSettings.Scale = FVector(1.0f, 1.0f, 1.0f);
    
    PlayEffect(EVFX_EffectType::Dust_Movement, TestSettings);
}

void AVFX_PrehistoricManager::LoadVFXAsset(EVFX_EffectType EffectType)
{
    UNiagaraSystem* VFXAsset = nullptr;
    
    switch (EffectType)
    {
        case EVFX_EffectType::Fire_Campfire:
            VFXAsset = CampfireVFX.LoadSynchronous();
            break;
        case EVFX_EffectType::Impact_Footstep:
            VFXAsset = FootstepImpactVFX.LoadSynchronous();
            break;
        case EVFX_EffectType::Dust_Movement:
            VFXAsset = DustCloudVFX.LoadSynchronous();
            break;
        case EVFX_EffectType::Water_Splash:
            VFXAsset = WaterSplashVFX.LoadSynchronous();
            break;
        case EVFX_EffectType::Weather_Rain:
            VFXAsset = RainVFX.LoadSynchronous();
            break;
        case EVFX_EffectType::Blood_Impact:
            VFXAsset = BloodImpactVFX.LoadSynchronous();
            break;
    }
    
    if (MainVFXComponent && VFXAsset)
    {
        MainVFXComponent->SetAsset(VFXAsset);
        UE_LOG(LogTemp, Log, TEXT("VFX_PrehistoricManager: Loaded VFX asset for effect type %d"), (int32)EffectType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_PrehistoricManager: Failed to load VFX asset for effect type %d"), (int32)EffectType);
    }
}

void AVFX_PrehistoricManager::UpdateVFXParameters()
{
    if (!MainVFXComponent)
        return;
    
    // Update time-based parameters
    MainVFXComponent->SetFloatParameter(TEXT("Time"), EffectTimer);
    
    // Update intensity based on distance or other factors
    float CurrentIntensity = EffectSettings.Intensity;
    
    // Example: Fade out effect near end of duration
    if (EffectSettings.bAutoDestroy && EffectSettings.Duration > 0.0f)
    {
        float TimeRemaining = EffectSettings.Duration - EffectTimer;
        if (TimeRemaining < 1.0f)
        {
            CurrentIntensity *= FMath::Clamp(TimeRemaining, 0.0f, 1.0f);
        }
    }
    
    MainVFXComponent->SetFloatParameter(TEXT("CurrentIntensity"), CurrentIntensity);
}

void AVFX_PrehistoricManager::HandleEffectTimer(float DeltaTime)
{
    EffectTimer += DeltaTime;
    
    // Auto-destroy effect if duration exceeded
    if (EffectSettings.bAutoDestroy && EffectSettings.Duration > 0.0f)
    {
        if (EffectTimer >= EffectSettings.Duration)
        {
            StopEffect();
        }
    }
}