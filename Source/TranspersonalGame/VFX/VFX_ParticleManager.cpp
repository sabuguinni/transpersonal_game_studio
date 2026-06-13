#include "VFX_ParticleManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AVFX_ParticleManager::AVFX_ParticleManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    LoadDefaultParticleConfigs();
}

void AVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeParticleSystem();
}

void AVFX_ParticleManager::LoadDefaultParticleConfigs()
{
    ParticleConfigs.Empty();
    
    // Campfire configuration
    FVFX_ParticleConfig CampfireConfig;
    CampfireConfig.ParticleType = EVFX_ParticleType::Campfire;
    CampfireConfig.Intensity = 1.0f;
    CampfireConfig.Duration = -1.0f; // Infinite
    CampfireConfig.bAutoActivate = false;
    ParticleConfigs.Add(CampfireConfig);
    
    // Dust cloud configuration
    FVFX_ParticleConfig DustConfig;
    DustConfig.ParticleType = EVFX_ParticleType::DustCloud;
    DustConfig.Intensity = 0.8f;
    DustConfig.Duration = 2.0f;
    DustConfig.bAutoActivate = false;
    ParticleConfigs.Add(DustConfig);
    
    // Blood splatter configuration
    FVFX_ParticleConfig BloodConfig;
    BloodConfig.ParticleType = EVFX_ParticleType::BloodSplatter;
    BloodConfig.Intensity = 1.2f;
    BloodConfig.Duration = 1.5f;
    BloodConfig.bAutoActivate = false;
    ParticleConfigs.Add(BloodConfig);
    
    // Rain drops configuration
    FVFX_ParticleConfig RainConfig;
    RainConfig.ParticleType = EVFX_ParticleType::RainDrops;
    RainConfig.Intensity = 0.6f;
    RainConfig.Duration = -1.0f; // Infinite
    RainConfig.bAutoActivate = false;
    ParticleConfigs.Add(RainConfig);
    
    // Breath vapor configuration
    FVFX_ParticleConfig BreathConfig;
    BreathConfig.ParticleType = EVFX_ParticleType::BreathVapor;
    BreathConfig.Intensity = 0.4f;
    BreathConfig.Duration = 3.0f;
    BreathConfig.bAutoActivate = false;
    ParticleConfigs.Add(BreathConfig);
    
    // Sparks configuration
    FVFX_ParticleConfig SparksConfig;
    SparksConfig.ParticleType = EVFX_ParticleType::Sparks;
    SparksConfig.Intensity = 1.5f;
    SparksConfig.Duration = 0.8f;
    SparksConfig.bAutoActivate = false;
    ParticleConfigs.Add(SparksConfig);
    
    // Insects configuration
    FVFX_ParticleConfig InsectsConfig;
    InsectsConfig.ParticleType = EVFX_ParticleType::Insects;
    InsectsConfig.Intensity = 0.3f;
    InsectsConfig.Duration = -1.0f; // Infinite
    InsectsConfig.bAutoActivate = false;
    ParticleConfigs.Add(InsectsConfig);
    
    // Water spray configuration
    FVFX_ParticleConfig WaterConfig;
    WaterConfig.ParticleType = EVFX_ParticleType::WaterSpray;
    WaterConfig.Intensity = 1.1f;
    WaterConfig.Duration = 2.5f;
    WaterConfig.bAutoActivate = false;
    ParticleConfigs.Add(WaterConfig);
}

void AVFX_ParticleManager::InitializeParticleSystem()
{
    ActiveParticles.Empty();
    
    for (const FVFX_ParticleConfig& Config : ParticleConfigs)
    {
        if (Config.bAutoActivate)
        {
            UNiagaraComponent* ParticleComponent = CreateParticleComponent(Config);
            if (ParticleComponent)
            {
                ActiveParticles.Add(Config.ParticleType, ParticleComponent);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Initialized with %d particle configs"), ParticleConfigs.Num());
}

UNiagaraComponent* AVFX_ParticleManager::CreateParticleComponent(const FVFX_ParticleConfig& Config)
{
    if (!Config.NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: No Niagara system assigned for particle type %d"), (int32)Config.ParticleType);
        return nullptr;
    }
    
    UNiagaraComponent* ParticleComponent = NewObject<UNiagaraComponent>(this);
    if (ParticleComponent)
    {
        ParticleComponent->SetAsset(Config.NiagaraSystem.Get());
        ParticleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        ParticleComponent->SetAutoActivate(Config.bAutoActivate);
        
        // Set intensity parameter if supported
        ParticleComponent->SetFloatParameter(TEXT("Intensity"), Config.Intensity);
        
        // Set duration if finite
        if (Config.Duration > 0.0f)
        {
            ParticleComponent->SetFloatParameter(TEXT("Duration"), Config.Duration);
        }
        
        return ParticleComponent;
    }
    
    return nullptr;
}

void AVFX_ParticleManager::SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation, float CustomIntensity)
{
    // Find config for this particle type
    const FVFX_ParticleConfig* Config = ParticleConfigs.FindByPredicate([ParticleType](const FVFX_ParticleConfig& Cfg)
    {
        return Cfg.ParticleType == ParticleType;
    });
    
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: No config found for particle type %d"), (int32)ParticleType);
        return;
    }
    
    // Stop existing effect if active
    StopParticleEffect(ParticleType);
    
    // Create new particle component
    UNiagaraComponent* ParticleComponent = CreateParticleComponent(*Config);
    if (ParticleComponent)
    {
        ParticleComponent->SetWorldLocationAndRotation(Location, Rotation);
        ParticleComponent->SetFloatParameter(TEXT("Intensity"), CustomIntensity);
        ParticleComponent->Activate();
        
        ActiveParticles.Add(ParticleType, ParticleComponent);
        
        // Auto-destroy finite duration effects
        if (Config->Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, ParticleType]()
            {
                StopParticleEffect(ParticleType);
            }, Config->Duration, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned particle effect %d at location %s"), (int32)ParticleType, *Location.ToString());
    }
}

void AVFX_ParticleManager::StopParticleEffect(EVFX_ParticleType ParticleType)
{
    UNiagaraComponent** FoundComponent = ActiveParticles.Find(ParticleType);
    if (FoundComponent && *FoundComponent)
    {
        (*FoundComponent)->Deactivate();
        (*FoundComponent)->DestroyComponent();
        ActiveParticles.Remove(ParticleType);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Stopped particle effect %d"), (int32)ParticleType);
    }
}

void AVFX_ParticleManager::StopAllParticleEffects()
{
    for (auto& ParticlePair : ActiveParticles)
    {
        if (ParticlePair.Value)
        {
            ParticlePair.Value->Deactivate();
            ParticlePair.Value->DestroyComponent();
        }
    }
    
    ActiveParticles.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Stopped all particle effects"));
}

bool AVFX_ParticleManager::IsParticleEffectActive(EVFX_ParticleType ParticleType) const
{
    UNiagaraComponent* const* FoundComponent = ActiveParticles.Find(ParticleType);
    return FoundComponent && *FoundComponent && (*FoundComponent)->IsActive();
}

void AVFX_ParticleManager::SetParticleIntensity(EVFX_ParticleType ParticleType, float NewIntensity)
{
    UNiagaraComponent** FoundComponent = ActiveParticles.Find(ParticleType);
    if (FoundComponent && *FoundComponent)
    {
        (*FoundComponent)->SetFloatParameter(TEXT("Intensity"), NewIntensity);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Set intensity %.2f for particle effect %d"), NewIntensity, (int32)ParticleType);
    }
}

UNiagaraComponent* AVFX_ParticleManager::GetParticleComponent(EVFX_ParticleType ParticleType) const
{
    UNiagaraComponent* const* FoundComponent = ActiveParticles.Find(ParticleType);
    return FoundComponent ? *FoundComponent : nullptr;
}

void AVFX_ParticleManager::RefreshParticleConfigs()
{
    StopAllParticleEffects();
    LoadDefaultParticleConfigs();
    InitializeParticleSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Refreshed particle configurations"));
}