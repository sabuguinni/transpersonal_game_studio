#include "VFX_ParticleManager.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVFX_ParticleManager::AVFX_ParticleManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    InitializeParticleConfigs();
}

void AVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set up cleanup timer for expired effects
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &AVFX_ParticleManager::CleanupExpiredEffects,
            2.0f,  // Check every 2 seconds
            true   // Loop
        );
    }
}

void AVFX_ParticleManager::InitializeParticleConfigs()
{
    // Initialize default particle configurations
    FVFX_ParticleConfig CampfireConfig;
    CampfireConfig.Duration = 0.0f;  // Infinite duration
    CampfireConfig.bAutoDestroy = false;
    ParticleConfigs.Add(EVFX_EffectType::Fire_Campfire, CampfireConfig);

    FVFX_ParticleConfig FootstepConfig;
    FootstepConfig.Duration = 3.0f;
    FootstepConfig.bAutoDestroy = true;
    ParticleConfigs.Add(EVFX_EffectType::Dust_Footstep, FootstepConfig);

    FVFX_ParticleConfig BloodConfig;
    BloodConfig.Duration = 8.0f;
    BloodConfig.bAutoDestroy = true;
    ParticleConfigs.Add(EVFX_EffectType::Blood_Splatter, BloodConfig);

    FVFX_ParticleConfig WaterConfig;
    WaterConfig.Duration = 4.0f;
    WaterConfig.bAutoDestroy = true;
    ParticleConfigs.Add(EVFX_EffectType::Water_Splash, WaterConfig);

    FVFX_ParticleConfig SmokeConfig;
    SmokeConfig.Duration = 6.0f;
    SmokeConfig.bAutoDestroy = true;
    ParticleConfigs.Add(EVFX_EffectType::Smoke_Rising, SmokeConfig);

    FVFX_ParticleConfig SparksConfig;
    SparksConfig.Duration = 2.0f;
    SparksConfig.bAutoDestroy = true;
    ParticleConfigs.Add(EVFX_EffectType::Sparks_Impact, SparksConfig);
}

UNiagaraComponent* AVFX_ParticleManager::SpawnParticleEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (EffectType == EVFX_EffectType::None)
    {
        return nullptr;
    }

    FVFX_ParticleConfig* Config = ParticleConfigs.Find(EffectType);
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: No configuration found for effect type"));
        return nullptr;
    }

    // For now, create a basic Niagara component without a specific system
    // In a real implementation, you would load the appropriate Niagara system asset
    UNiagaraComponent* ParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        Config->ParticleSystem,  // This would be loaded from content browser
        Location,
        Rotation,
        FVector::OneVector,
        Config->bAutoDestroy
    );

    if (ParticleComponent)
    {
        ActiveParticles.Add(ParticleComponent);
        
        // Set up auto-destroy timer if needed
        if (Config->bAutoDestroy && Config->Duration > 0.0f)
        {
            FTimerHandle DestroyTimer;
            GetWorld()->GetTimerManager().SetTimer(
                DestroyTimer,
                [ParticleComponent]()
                {
                    if (IsValid(ParticleComponent))
                    {
                        ParticleComponent->DestroyComponent();
                    }
                },
                Config->Duration,
                false
            );
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned particle effect at location %s"), *Location.ToString());
    }

    return ParticleComponent;
}

void AVFX_ParticleManager::SpawnCampfireEffect(FVector Location)
{
    UNiagaraComponent* Effect = SpawnParticleEffect(EVFX_EffectType::Fire_Campfire, Location);
    if (Effect)
    {
        // Set campfire-specific parameters
        Effect->SetFloatParameter(TEXT("FireIntensity"), 1.0f);
        Effect->SetFloatParameter(TEXT("SmokeAmount"), 0.7f);
        Effect->SetVectorParameter(TEXT("WindDirection"), FVector(0.1f, 0.0f, 1.0f));
    }
}

void AVFX_ParticleManager::SpawnFootstepDust(FVector Location, float ImpactForce)
{
    UNiagaraComponent* Effect = SpawnParticleEffect(EVFX_EffectType::Dust_Footstep, Location);
    if (Effect)
    {
        // Set dust-specific parameters based on impact force
        float DustAmount = FMath::Clamp(ImpactForce, 0.5f, 2.0f);
        Effect->SetFloatParameter(TEXT("DustAmount"), DustAmount);
        Effect->SetFloatParameter(TEXT("ParticleSize"), DustAmount * 0.5f);
        Effect->SetVectorParameter(TEXT("ImpactDirection"), FVector(0.0f, 0.0f, 1.0f));
    }
}

void AVFX_ParticleManager::SpawnBloodSplatter(FVector Location, FVector ImpactDirection)
{
    UNiagaraComponent* Effect = SpawnParticleEffect(EVFX_EffectType::Blood_Splatter, Location);
    if (Effect)
    {
        // Set blood-specific parameters
        FVector NormalizedDirection = ImpactDirection.GetSafeNormal();
        Effect->SetVectorParameter(TEXT("SplatterDirection"), NormalizedDirection);
        Effect->SetFloatParameter(TEXT("BloodAmount"), 1.0f);
        Effect->SetVectorParameter(TEXT("BloodColor"), FVector(0.8f, 0.1f, 0.1f));
    }
}

void AVFX_ParticleManager::SpawnWaterSplash(FVector Location, float SplashSize)
{
    UNiagaraComponent* Effect = SpawnParticleEffect(EVFX_EffectType::Water_Splash, Location);
    if (Effect)
    {
        // Set water splash parameters
        float ClampedSize = FMath::Clamp(SplashSize, 0.5f, 3.0f);
        Effect->SetFloatParameter(TEXT("SplashSize"), ClampedSize);
        Effect->SetFloatParameter(TEXT("DropletCount"), ClampedSize * 50.0f);
        Effect->SetVectorParameter(TEXT("WaterColor"), FVector(0.2f, 0.6f, 0.9f));
    }
}

void AVFX_ParticleManager::CleanupExpiredEffects()
{
    CleanupNullComponents();
}

int32 AVFX_ParticleManager::GetActiveEffectCount() const
{
    int32 ValidCount = 0;
    for (UNiagaraComponent* Component : ActiveParticles)
    {
        if (IsValid(Component))
        {
            ValidCount++;
        }
    }
    return ValidCount;
}

void AVFX_ParticleManager::CleanupNullComponents()
{
    ActiveParticles.RemoveAll([](UNiagaraComponent* Component)
    {
        return !IsValid(Component);
    });
}