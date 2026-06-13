#include "VFX_ParticleManager.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AVFX_ParticleManager::AVFX_ParticleManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create primary particle system component
    PrimaryParticleSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PrimaryParticleSystem"));
    PrimaryParticleSystem->SetupAttachment(RootComponent);

    // Initialize VFX settings
    EffectIntensityMultiplier = 1.0f;
    bEnableWeatherEffects = true;
    bEnableCombatEffects = true;
    bEnableEnvironmentalEffects = true;

    // Performance settings
    MaxActiveParticles = 50;
    EffectCullDistance = 5000.0f;
    CurrentWeatherIntensity = 0.0f;
    bIsRaining = false;

    // Load default VFX assets (placeholder paths)
    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> CampfireFinder(TEXT("/Game/VFX/NS_Fire_Campfire"));
    if (CampfireFinder.Succeeded())
    {
        CampfireEffect = CampfireFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DustFinder(TEXT("/Game/VFX/NS_Environment_Dust"));
    if (DustFinder.Succeeded())
    {
        DustImpactEffect = DustFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> RainFinder(TEXT("/Game/VFX/NS_Weather_Rain"));
    if (RainFinder.Succeeded())
    {
        RainEffect = RainFinder.Object;
    }
}

void AVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    UpdatePerformanceSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX ParticleManager initialized with %d max particles"), MaxActiveParticles);
}

void AVFX_ParticleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Cleanup inactive effects periodically
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 5.0f)
    {
        CleanupInactiveEffects();
        CleanupTimer = 0.0f;
    }

    // Update weather effects
    if (bIsRaining && bEnableWeatherEffects)
    {
        // Update rain intensity based on weather system
        if (PrimaryParticleSystem && RainEffect)
        {
            PrimaryParticleSystem->SetFloatParameter(TEXT("Intensity"), CurrentWeatherIntensity);
        }
    }
}

void AVFX_ParticleManager::SpawnCampfireEffect(FVector Location, FRotator Rotation)
{
    if (!bEnableEnvironmentalEffects || !CampfireEffect || !ShouldSpawnEffect(Location))
    {
        return;
    }

    UNiagaraComponent* CampfireComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        CampfireEffect,
        Location,
        Rotation
    );

    if (CampfireComponent)
    {
        CampfireComponent->SetFloatParameter(TEXT("IntensityMultiplier"), EffectIntensityMultiplier);
        ActiveParticleSystems.Add(CampfireComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned campfire VFX at location: %s"), *Location.ToString());
    }
}

void AVFX_ParticleManager::SpawnDustImpact(FVector Location, float Intensity)
{
    if (!bEnableEnvironmentalEffects || !DustImpactEffect || !ShouldSpawnEffect(Location))
    {
        return;
    }

    UNiagaraComponent* DustComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        DustImpactEffect,
        Location,
        FRotator::ZeroRotator
    );

    if (DustComponent)
    {
        float FinalIntensity = Intensity * EffectIntensityMultiplier;
        DustComponent->SetFloatParameter(TEXT("ImpactIntensity"), FinalIntensity);
        DustComponent->SetFloatParameter(TEXT("ParticleCount"), FMath::Clamp(FinalIntensity * 100.0f, 10.0f, 500.0f));
        
        ActiveParticleSystems.Add(DustComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned dust impact VFX with intensity: %f"), FinalIntensity);
    }
}

void AVFX_ParticleManager::SpawnFootstepDust(FVector Location, float DinosaurSize)
{
    if (!bEnableEnvironmentalEffects || !FootstepDustEffect || !ShouldSpawnEffect(Location))
    {
        return;
    }

    UNiagaraComponent* FootstepComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        FootstepDustEffect,
        Location,
        FRotator::ZeroRotator
    );

    if (FootstepComponent)
    {
        float SizeMultiplier = FMath::Clamp(DinosaurSize, 0.1f, 5.0f);
        FootstepComponent->SetFloatParameter(TEXT("SizeMultiplier"), SizeMultiplier);
        FootstepComponent->SetFloatParameter(TEXT("DustAmount"), SizeMultiplier * EffectIntensityMultiplier * 50.0f);
        
        ActiveParticleSystems.Add(FootstepComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned footstep dust for dinosaur size: %f"), DinosaurSize);
    }
}

void AVFX_ParticleManager::StartRainEffect(float Intensity)
{
    if (!bEnableWeatherEffects || !RainEffect)
    {
        return;
    }

    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    bIsRaining = true;

    if (PrimaryParticleSystem)
    {
        PrimaryParticleSystem->SetAsset(RainEffect);
        PrimaryParticleSystem->SetFloatParameter(TEXT("RainIntensity"), CurrentWeatherIntensity);
        PrimaryParticleSystem->Activate();
        
        UE_LOG(LogTemp, Warning, TEXT("Started rain effect with intensity: %f"), CurrentWeatherIntensity);
    }
}

void AVFX_ParticleManager::StopRainEffect()
{
    bIsRaining = false;
    CurrentWeatherIntensity = 0.0f;

    if (PrimaryParticleSystem)
    {
        PrimaryParticleSystem->Deactivate();
        UE_LOG(LogTemp, Warning, TEXT("Stopped rain effect"));
    }
}

void AVFX_ParticleManager::SpawnBloodSplatter(FVector Location, FVector Direction, float Amount)
{
    if (!bEnableCombatEffects || !BloodSplatterEffect || !ShouldSpawnEffect(Location))
    {
        return;
    }

    FRotator BloodRotation = Direction.Rotation();
    UNiagaraComponent* BloodComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        BloodSplatterEffect,
        Location,
        BloodRotation
    );

    if (BloodComponent)
    {
        float BloodAmount = FMath::Clamp(Amount * EffectIntensityMultiplier, 0.1f, 3.0f);
        BloodComponent->SetFloatParameter(TEXT("BloodAmount"), BloodAmount);
        BloodComponent->SetVectorParameter(TEXT("SplatterDirection"), Direction);
        
        ActiveParticleSystems.Add(BloodComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned blood splatter with amount: %f"), BloodAmount);
    }
}

void AVFX_ParticleManager::SpawnImpactSparks(FVector Location, FVector Normal)
{
    if (!bEnableCombatEffects || !ImpactSparkEffect || !ShouldSpawnEffect(Location))
    {
        return;
    }

    FRotator SparkRotation = Normal.Rotation();
    UNiagaraComponent* SparkComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        ImpactSparkEffect,
        Location,
        SparkRotation
    );

    if (SparkComponent)
    {
        SparkComponent->SetVectorParameter(TEXT("ImpactNormal"), Normal);
        SparkComponent->SetFloatParameter(TEXT("SparkIntensity"), EffectIntensityMultiplier);
        
        ActiveParticleSystems.Add(SparkComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned impact sparks at location: %s"), *Location.ToString());
    }
}

void AVFX_ParticleManager::SetGlobalVFXIntensity(float NewIntensity)
{
    EffectIntensityMultiplier = FMath::Clamp(NewIntensity, 0.0f, 10.0f);
    
    // Update all active particle systems
    for (UNiagaraComponent* ParticleComp : ActiveParticleSystems)
    {
        if (IsValid(ParticleComp))
        {
            ParticleComp->SetFloatParameter(TEXT("IntensityMultiplier"), EffectIntensityMultiplier);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Set global VFX intensity to: %f"), EffectIntensityMultiplier);
}

void AVFX_ParticleManager::EnableVFXCategory(EVFX_EffectCategory Category, bool bEnable)
{
    switch (Category)
    {
        case EVFX_EffectCategory::Weather:
            bEnableWeatherEffects = bEnable;
            if (!bEnable && bIsRaining)
            {
                StopRainEffect();
            }
            break;
            
        case EVFX_EffectCategory::Combat:
            bEnableCombatEffects = bEnable;
            break;
            
        case EVFX_EffectCategory::Environmental:
            bEnableEnvironmentalEffects = bEnable;
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Category %d enabled: %s"), (int32)Category, bEnable ? TEXT("true") : TEXT("false"));
}

void AVFX_ParticleManager::CleanupInactiveEffects()
{
    int32 CleanedCount = 0;
    
    for (int32 i = ActiveParticleSystems.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* ParticleComp = ActiveParticleSystems[i];
        if (!IsValid(ParticleComp) || !ParticleComp->IsActive())
        {
            ActiveParticleSystems.RemoveAt(i);
            CleanedCount++;
        }
    }
    
    if (CleanedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d inactive VFX components"), CleanedCount);
    }
}

void AVFX_ParticleManager::UpdatePerformanceSettings()
{
    // Adjust performance based on current actor count
    if (UWorld* World = GetWorld())
    {
        int32 ActorCount = World->GetCurrentLevel()->Actors.Num();
        
        if (ActorCount > 5000)
        {
            MaxActiveParticles = 25;
            EffectCullDistance = 3000.0f;
        }
        else if (ActorCount > 2000)
        {
            MaxActiveParticles = 35;
            EffectCullDistance = 4000.0f;
        }
        else
        {
            MaxActiveParticles = 50;
            EffectCullDistance = 5000.0f;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Updated VFX performance: %d max particles, %f cull distance"), 
               MaxActiveParticles, EffectCullDistance);
    }
}

bool AVFX_ParticleManager::ShouldSpawnEffect(FVector Location) const
{
    // Check if we're at particle limit
    if (ActiveParticleSystems.Num() >= MaxActiveParticles)
    {
        return false;
    }
    
    // Check distance culling
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float DistanceToPlayer = FVector::Dist(Location, PlayerPawn->GetActorLocation());
        if (DistanceToPlayer > EffectCullDistance)
        {
            return false;
        }
    }
    
    return true;
}