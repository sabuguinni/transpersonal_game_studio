#include "VFX_EnvironmentalEffects.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UVFX_EnvironmentalEffects::UVFX_EnvironmentalEffects()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    GlobalIntensityMultiplier = 1.0f;
    bEnableEnvironmentalVFX = true;

    // Initialize default configurations
    FVFX_EnvironmentalConfig CampfireConfig;
    CampfireConfig.EffectType = EVFX_EnvironmentalType::Campfire;
    CampfireConfig.IntensityScale = 1.0f;
    CampfireConfig.Duration = 0.0f; // Persistent
    CampfireConfig.bAutoDestroy = false;
    EnvironmentalConfigs.Add(CampfireConfig);

    FVFX_EnvironmentalConfig WaterConfig;
    WaterConfig.EffectType = EVFX_EnvironmentalType::WaterSplash;
    WaterConfig.IntensityScale = 1.5f;
    WaterConfig.Duration = 3.0f;
    WaterConfig.bAutoDestroy = true;
    EnvironmentalConfigs.Add(WaterConfig);

    FVFX_EnvironmentalConfig DustConfig;
    DustConfig.EffectType = EVFX_EnvironmentalType::DustCloud;
    DustConfig.IntensityScale = 1.2f;
    DustConfig.Duration = 4.0f;
    DustConfig.bAutoDestroy = true;
    EnvironmentalConfigs.Add(DustConfig);

    FVFX_EnvironmentalConfig FootstepConfig;
    FootstepConfig.EffectType = EVFX_EnvironmentalType::FootstepImpact;
    FootstepConfig.IntensityScale = 0.8f;
    FootstepConfig.Duration = 2.0f;
    FootstepConfig.bAutoDestroy = true;
    EnvironmentalConfigs.Add(FootstepConfig);
}

void UVFX_EnvironmentalEffects::BeginPlay()
{
    Super::BeginPlay();
    
    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CleanupTimerHandle, this, &UVFX_EnvironmentalEffects::CleanupExpiredEffects, 5.0f, true);
    }
}

void UVFX_EnvironmentalEffects::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active particle systems
    for (int32 i = ActiveParticleSystems.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveParticleSystems[i]) || !ActiveParticleSystems[i]->IsActive())
        {
            ActiveParticleSystems.RemoveAt(i);
        }
    }
}

void UVFX_EnvironmentalEffects::SpawnEnvironmentalEffect(EVFX_EnvironmentalType EffectType, FVector Location, float Intensity)
{
    if (!bEnableEnvironmentalVFX)
    {
        return;
    }

    switch (EffectType)
    {
        case EVFX_EnvironmentalType::Campfire:
            SpawnCampfireEffect(Location, Intensity);
            break;
        case EVFX_EnvironmentalType::WaterSplash:
            SpawnWaterSplashEffect(Location, Intensity);
            break;
        case EVFX_EnvironmentalType::DustCloud:
            SpawnDustCloudEffect(Location, Intensity);
            break;
        case EVFX_EnvironmentalType::FootstepImpact:
            SpawnFootstepImpact(Location, Intensity);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown environmental VFX type"));
            break;
    }
}

void UVFX_EnvironmentalEffects::SpawnCampfireEffect(FVector Location, float FireIntensity)
{
    UParticleSystemComponent* ParticleComp = CreateParticleEffect(Location, EVFX_EnvironmentalType::Campfire);
    if (ParticleComp)
    {
        // Scale fire intensity
        float FinalIntensity = FireIntensity * GlobalIntensityMultiplier;
        ParticleComp->SetFloatParameter(FName("FireIntensity"), FinalIntensity);
        ParticleComp->SetVectorParameter(FName("FireColor"), FVector(1.0f, 0.6f, 0.2f));
        
        ActiveParticleSystems.Add(ParticleComp);
        UE_LOG(LogTemp, Log, TEXT("Spawned campfire VFX at location: %s"), *Location.ToString());
    }
}

void UVFX_EnvironmentalEffects::SpawnWaterSplashEffect(FVector Location, float SplashSize)
{
    UParticleSystemComponent* ParticleComp = CreateParticleEffect(Location, EVFX_EnvironmentalType::WaterSplash);
    if (ParticleComp)
    {
        float FinalSize = SplashSize * GlobalIntensityMultiplier;
        ParticleComp->SetFloatParameter(FName("SplashSize"), FinalSize);
        ParticleComp->SetVectorParameter(FName("WaterColor"), FVector(0.2f, 0.6f, 1.0f));
        
        ActiveParticleSystems.Add(ParticleComp);
        UE_LOG(LogTemp, Log, TEXT("Spawned water splash VFX at location: %s"), *Location.ToString());
    }
}

void UVFX_EnvironmentalEffects::SpawnDustCloudEffect(FVector Location, float CloudSize)
{
    UParticleSystemComponent* ParticleComp = CreateParticleEffect(Location, EVFX_EnvironmentalType::DustCloud);
    if (ParticleComp)
    {
        float FinalSize = CloudSize * GlobalIntensityMultiplier;
        ParticleComp->SetFloatParameter(FName("CloudSize"), FinalSize);
        ParticleComp->SetVectorParameter(FName("DustColor"), FVector(0.8f, 0.6f, 0.4f));
        
        ActiveParticleSystems.Add(ParticleComp);
        UE_LOG(LogTemp, Log, TEXT("Spawned dust cloud VFX at location: %s"), *Location.ToString());
    }
}

void UVFX_EnvironmentalEffects::SpawnFootstepImpact(FVector Location, float ImpactForce)
{
    UParticleSystemComponent* ParticleComp = CreateParticleEffect(Location, EVFX_EnvironmentalType::FootstepImpact);
    if (ParticleComp)
    {
        float FinalForce = ImpactForce * GlobalIntensityMultiplier;
        ParticleComp->SetFloatParameter(FName("ImpactForce"), FinalForce);
        ParticleComp->SetVectorParameter(FName("ImpactDirection"), FVector(0.0f, 0.0f, 1.0f));
        
        ActiveParticleSystems.Add(ParticleComp);
        UE_LOG(LogTemp, Log, TEXT("Spawned footstep impact VFX at location: %s"), *Location.ToString());
    }
}

UParticleSystemComponent* UVFX_EnvironmentalEffects::CreateParticleEffect(FVector Location, EVFX_EnvironmentalType EffectType)
{
    if (!GetOwner() || !GetWorld())
    {
        return nullptr;
    }

    // Create a basic particle system component
    UParticleSystemComponent* ParticleComp = NewObject<UParticleSystemComponent>(GetOwner());
    if (ParticleComp)
    {
        ParticleComp->SetWorldLocation(Location);
        ParticleComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        
        // Find configuration for this effect type
        const FVFX_EnvironmentalConfig* Config = EnvironmentalConfigs.FindByPredicate([EffectType](const FVFX_EnvironmentalConfig& Cfg) {
            return Cfg.EffectType == EffectType;
        });

        if (Config && Config->bAutoDestroy && Config->Duration > 0.0f)
        {
            // Schedule destruction
            FTimerHandle DestroyTimer;
            GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [ParticleComp]() {
                if (IsValid(ParticleComp))
                {
                    ParticleComp->DestroyComponent();
                }
            }, Config->Duration, false);
        }

        ParticleComp->RegisterComponent();
        ParticleComp->Activate();
    }

    return ParticleComp;
}

void UVFX_EnvironmentalEffects::CleanupExpiredEffects()
{
    for (int32 i = ActiveParticleSystems.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveParticleSystems[i]))
        {
            ActiveParticleSystems.RemoveAt(i);
        }
    }
}