#include "VFX_ParticleSystemManager.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

AVFX_ParticleSystemManager::AVFX_ParticleSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root mesh component
    RootMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMeshComponent"));
    RootComponent = RootMeshComponent;

    // Create Niagara component
    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
    NiagaraComponent->SetupAttachment(RootComponent);

    // Initialize default values
    CurrentIntensity = 1.0f;
    bIsEffectActive = false;
    EffectTimer = 0.0f;

    // Set default particle config
    ParticleConfig.ParticleType = EVFX_ParticleType::Fire_Campfire;
    ParticleConfig.Intensity = 1.0f;
    ParticleConfig.Duration = 5.0f;
    ParticleConfig.bAutoDestroy = false;
    ParticleConfig.Scale = FVector(1.0f, 1.0f, 1.0f);
}

void AVFX_ParticleSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeParticleAssets();
    
    // Auto-start if configured
    if (ParticleConfig.ParticleType != EVFX_ParticleType::Fire_Campfire || ParticleConfig.Intensity > 0.0f)
    {
        StartParticleEffect(ParticleConfig.ParticleType, ParticleConfig.Intensity);
    }
}

void AVFX_ParticleSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsEffectActive)
    {
        EffectTimer += DeltaTime;
        
        // Auto-destroy after duration if configured
        if (ParticleConfig.bAutoDestroy && EffectTimer >= ParticleConfig.Duration)
        {
            StopParticleEffect();
            if (ParticleConfig.bAutoDestroy)
            {
                Destroy();
            }
        }
        
        UpdateParticleParameters();
    }
}

void AVFX_ParticleSystemManager::StartParticleEffect(EVFX_ParticleType ParticleType, float Intensity)
{
    if (!NiagaraComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleSystemManager: NiagaraComponent is null"));
        return;
    }

    // Find the appropriate particle system asset
    UNiagaraSystem** FoundSystem = ParticleSystemAssets.Find(ParticleType);
    if (FoundSystem && *FoundSystem)
    {
        NiagaraComponent->SetAsset(*FoundSystem);
        NiagaraComponent->Activate();
        
        bIsEffectActive = true;
        CurrentIntensity = Intensity;
        EffectTimer = 0.0f;
        ParticleConfig.ParticleType = ParticleType;
        ParticleConfig.Intensity = Intensity;
        
        // Set initial parameters
        UpdateParticleParameters();
        
        // Broadcast event
        OnParticleEffectStarted(ParticleType);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Started particle effect type %d with intensity %f"), 
               (int32)ParticleType, Intensity);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleSystemManager: No particle system found for type %d"), (int32)ParticleType);
    }
}

void AVFX_ParticleSystemManager::StopParticleEffect()
{
    if (NiagaraComponent)
    {
        NiagaraComponent->Deactivate();
        bIsEffectActive = false;
        EffectTimer = 0.0f;
        
        // Broadcast event
        OnParticleEffectStopped();
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Stopped particle effect"));
    }
}

void AVFX_ParticleSystemManager::SetParticleIntensity(float NewIntensity)
{
    CurrentIntensity = FMath::Clamp(NewIntensity, 0.0f, 10.0f);
    ParticleConfig.Intensity = CurrentIntensity;
    
    if (bIsEffectActive)
    {
        UpdateParticleParameters();
    }
}

void AVFX_ParticleSystemManager::SetParticleScale(FVector NewScale)
{
    ParticleConfig.Scale = NewScale;
    
    if (NiagaraComponent)
    {
        NiagaraComponent->SetRelativeScale3D(NewScale);
    }
}

void AVFX_ParticleSystemManager::InitializeParticleAssets()
{
    // Initialize particle system assets map
    // These would be loaded from Content Browser in a real implementation
    // For now, we set up the structure for future asset assignment
    
    ParticleSystemAssets.Empty();
    
    // Placeholder entries - actual Niagara assets would be assigned in Blueprint or loaded at runtime
    ParticleSystemAssets.Add(EVFX_ParticleType::Fire_Campfire, nullptr);
    ParticleSystemAssets.Add(EVFX_ParticleType::Dust_Footstep, nullptr);
    ParticleSystemAssets.Add(EVFX_ParticleType::Weather_Rain, nullptr);
    ParticleSystemAssets.Add(EVFX_ParticleType::Combat_Blood, nullptr);
    ParticleSystemAssets.Add(EVFX_ParticleType::Environment_Fog, nullptr);
    ParticleSystemAssets.Add(EVFX_ParticleType::Impact_Rock, nullptr);
    ParticleSystemAssets.Add(EVFX_ParticleType::Water_Splash, nullptr);
    ParticleSystemAssets.Add(EVFX_ParticleType::Wind_Leaves, nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Initialized particle assets map with %d entries"), 
           ParticleSystemAssets.Num());
}

void AVFX_ParticleSystemManager::UpdateParticleParameters()
{
    if (!NiagaraComponent || !bIsEffectActive)
    {
        return;
    }
    
    // Set common parameters that most particle systems use
    NiagaraComponent->SetFloatParameter(TEXT("Intensity"), CurrentIntensity);
    NiagaraComponent->SetFloatParameter(TEXT("SpawnRate"), CurrentIntensity * 100.0f);
    NiagaraComponent->SetVectorParameter(TEXT("Scale"), ParticleConfig.Scale);
    
    // Type-specific parameter updates
    switch (ParticleConfig.ParticleType)
    {
        case EVFX_ParticleType::Fire_Campfire:
            NiagaraComponent->SetFloatParameter(TEXT("FlameHeight"), CurrentIntensity * 2.0f);
            NiagaraComponent->SetFloatParameter(TEXT("EmberCount"), CurrentIntensity * 50.0f);
            break;
            
        case EVFX_ParticleType::Dust_Footstep:
            NiagaraComponent->SetFloatParameter(TEXT("DustAmount"), CurrentIntensity * 1.5f);
            NiagaraComponent->SetFloatParameter(TEXT("ImpactForce"), CurrentIntensity);
            break;
            
        case EVFX_ParticleType::Weather_Rain:
            NiagaraComponent->SetFloatParameter(TEXT("RainIntensity"), CurrentIntensity);
            NiagaraComponent->SetFloatParameter(TEXT("DropletSize"), 0.5f + CurrentIntensity * 0.5f);
            break;
            
        case EVFX_ParticleType::Combat_Blood:
            NiagaraComponent->SetFloatParameter(TEXT("BloodAmount"), CurrentIntensity);
            NiagaraComponent->SetFloatParameter(TEXT("SplatterRadius"), CurrentIntensity * 0.8f);
            break;
            
        default:
            // Generic parameters for other types
            break;
    }
}