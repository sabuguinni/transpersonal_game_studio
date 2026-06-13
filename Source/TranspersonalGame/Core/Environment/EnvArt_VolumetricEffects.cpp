#include "EnvArt_VolumetricEffects.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_VolumetricEffects::AEnvArt_VolumetricEffects()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    EffectBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("EffectBounds"));
    EffectBounds->SetupAttachment(RootComponent);
    EffectBounds->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
    EffectBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ParticleSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ParticleSystem"));
    ParticleSystem->SetupAttachment(RootComponent);
    ParticleSystem->SetAutoActivate(false);

    // Initialize default settings
    EffectSettings.EffectType = EEnvArt_EffectType::Dust;
    EffectSettings.Intensity = 1.0f;
    EffectSettings.EffectScale = FVector(1.0f);
    EffectSettings.TintColor = FLinearColor::White;
    EffectSettings.bAffectedByWind = true;
    EffectSettings.WindStrength = 1.0f;
}

void AEnvArt_VolumetricEffects::BeginPlay()
{
    Super::BeginPlay();
    
    PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    
    if (bAutoActivate)
    {
        ActivateEffect();
    }
    
    UpdateEffectParameters();
}

void AEnvArt_VolumetricEffects::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeAccumulator += DeltaTime;

    if (bPlayerProximityActivation)
    {
        CheckPlayerProximity();
    }

    if (bIsActive && bTimeOfDayAffected)
    {
        // Update intensity based on time of day
        float TimeMultiplier = GetTimeOfDayMultiplier();
        float AdjustedIntensity = EffectSettings.Intensity * TimeMultiplier;
        
        if (ParticleSystem && ParticleSystem->GetAsset())
        {
            ParticleSystem->SetFloatParameter(FName("Intensity"), AdjustedIntensity);
        }
    }
}

void AEnvArt_VolumetricEffects::SetEffectType(EEnvArt_EffectType NewType)
{
    EffectSettings.EffectType = NewType;
    UpdateEffectParameters();
}

void AEnvArt_VolumetricEffects::SetEffectIntensity(float NewIntensity)
{
    EffectSettings.Intensity = FMath::Clamp(NewIntensity, 0.0f, 10.0f);
    UpdateEffectParameters();
}

void AEnvArt_VolumetricEffects::ActivateEffect()
{
    if (ParticleSystem)
    {
        ParticleSystem->Activate(true);
        bIsActive = true;
        UpdateEffectParameters();
    }
}

void AEnvArt_VolumetricEffects::DeactivateEffect()
{
    if (ParticleSystem)
    {
        ParticleSystem->Deactivate();
        bIsActive = false;
    }
}

void AEnvArt_VolumetricEffects::CreateDustEffect()
{
    EffectSettings.EffectType = EEnvArt_EffectType::Dust;
    EffectSettings.TintColor = FLinearColor(0.8f, 0.7f, 0.6f, 0.3f);
    EffectSettings.Intensity = 0.5f;
    EffectSettings.bAffectedByWind = true;
    UpdateEffectParameters();
}

void AEnvArt_VolumetricEffects::CreatePollenEffect()
{
    EffectSettings.EffectType = EEnvArt_EffectType::Pollen;
    EffectSettings.TintColor = FLinearColor(1.0f, 0.9f, 0.4f, 0.6f);
    EffectSettings.Intensity = 0.8f;
    EffectSettings.bAffectedByWind = true;
    UpdateEffectParameters();
}

void AEnvArt_VolumetricEffects::CreateMistEffect()
{
    EffectSettings.EffectType = EEnvArt_EffectType::Mist;
    EffectSettings.TintColor = FLinearColor(0.9f, 0.9f, 1.0f, 0.2f);
    EffectSettings.Intensity = 1.2f;
    EffectSettings.bAffectedByWind = false;
    UpdateEffectParameters();
}

void AEnvArt_VolumetricEffects::UpdateEffectParameters()
{
    if (!ParticleSystem) return;

    // Set common parameters
    ParticleSystem->SetFloatParameter(FName("Intensity"), EffectSettings.Intensity);
    ParticleSystem->SetVectorParameter(FName("Scale"), EffectSettings.EffectScale);
    ParticleSystem->SetColorParameter(FName("TintColor"), EffectSettings.TintColor);
    
    if (EffectSettings.bAffectedByWind)
    {
        ParticleSystem->SetFloatParameter(FName("WindStrength"), EffectSettings.WindStrength);
    }

    // Set bounds
    FVector BoundsExtent = EffectBounds->GetScaledBoxExtent();
    ParticleSystem->SetVectorParameter(FName("BoundsMin"), -BoundsExtent);
    ParticleSystem->SetVectorParameter(FName("BoundsMax"), BoundsExtent);

    // Effect-specific parameters
    switch (EffectSettings.EffectType)
    {
        case EEnvArt_EffectType::Dust:
            ParticleSystem->SetFloatParameter(FName("ParticleSize"), 2.0f);
            ParticleSystem->SetFloatParameter(FName("SpawnRate"), 50.0f);
            ParticleSystem->SetFloatParameter(FName("Lifetime"), 8.0f);
            break;
            
        case EEnvArt_EffectType::Pollen:
            ParticleSystem->SetFloatParameter(FName("ParticleSize"), 1.5f);
            ParticleSystem->SetFloatParameter(FName("SpawnRate"), 30.0f);
            ParticleSystem->SetFloatParameter(FName("Lifetime"), 12.0f);
            break;
            
        case EEnvArt_EffectType::Mist:
            ParticleSystem->SetFloatParameter(FName("ParticleSize"), 10.0f);
            ParticleSystem->SetFloatParameter(FName("SpawnRate"), 20.0f);
            ParticleSystem->SetFloatParameter(FName("Lifetime"), 15.0f);
            break;
            
        case EEnvArt_EffectType::Fireflies:
            ParticleSystem->SetFloatParameter(FName("ParticleSize"), 3.0f);
            ParticleSystem->SetFloatParameter(FName("SpawnRate"), 5.0f);
            ParticleSystem->SetFloatParameter(FName("Lifetime"), 20.0f);
            ParticleSystem->SetFloatParameter(FName("GlowIntensity"), 2.0f);
            break;
            
        case EEnvArt_EffectType::Leaves:
            ParticleSystem->SetFloatParameter(FName("ParticleSize"), 4.0f);
            ParticleSystem->SetFloatParameter(FName("SpawnRate"), 10.0f);
            ParticleSystem->SetFloatParameter(FName("Lifetime"), 25.0f);
            ParticleSystem->SetFloatParameter(FName("FallSpeed"), 100.0f);
            break;
            
        case EEnvArt_EffectType::Spores:
            ParticleSystem->SetFloatParameter(FName("ParticleSize"), 1.0f);
            ParticleSystem->SetFloatParameter(FName("SpawnRate"), 80.0f);
            ParticleSystem->SetFloatParameter(FName("Lifetime"), 6.0f);
            break;
    }
}

void AEnvArt_VolumetricEffects::CheckPlayerProximity()
{
    if (!PlayerController || !PlayerController->GetPawn()) return;

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector EffectLocation = GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, EffectLocation);

    if (Distance <= ActivationDistance && !bIsActive)
    {
        ActivateEffect();
    }
    else if (Distance > ActivationDistance * 1.2f && bIsActive && bPlayerProximityActivation)
    {
        DeactivateEffect();
    }
}

float AEnvArt_VolumetricEffects::GetTimeOfDayMultiplier()
{
    // Simple time-based multiplier - in a real implementation this would
    // query the AtmosphereManager for actual time of day
    float TimeOfDay = FMath::Fmod(TimeAccumulator / 60.0f, 24.0f); // Simulate 24 hour cycle in 24 minutes
    
    if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f) // Day time
    {
        return DayIntensityMultiplier;
    }
    else // Night time
    {
        return NightIntensityMultiplier;
    }
}