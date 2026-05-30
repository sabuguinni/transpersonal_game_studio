#include "VFX_ImpactManager.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root mesh component
    RootMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
    RootComponent = RootMeshComponent;
    RootMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RootMeshComponent->SetVisibility(false);

    // Create dust particle component
    DustParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticles"));
    DustParticleComponent->SetupAttachment(RootComponent);
    DustParticleComponent->bAutoActivate = false;

    // Create blood particle component
    BloodParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BloodParticles"));
    BloodParticleComponent->SetupAttachment(RootComponent);
    BloodParticleComponent->bAutoActivate = false;

    // Set default values
    DefaultDustScale = 1.0f;
    DefaultBloodScale = 0.8f;
    bAutoCleanupEffects = true;
    EffectLifetime = 5.0f;
    CleanupTimer = 0.0f;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("VFX Impact Manager initialized"));
    }
}

void UVFX_ImpactManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoCleanupEffects)
    {
        UpdateActiveEffects(DeltaTime);
    }
}

void UVFX_ImpactManager::TriggerImpact(const FVFX_ImpactData& ImpactData)
{
    switch (ImpactData.ImpactType)
    {
        case EVFX_ImpactType::FootstepDust:
            TriggerFootstepDust(ImpactData.ImpactLocation, ImpactData.ImpactIntensity);
            break;
        case EVFX_ImpactType::BloodSplatter:
            TriggerBloodSplatter(ImpactData.ImpactLocation, ImpactData.ImpactIntensity);
            break;
        case EVFX_ImpactType::RockImpact:
            CreateDustCloud(ImpactData.ImpactLocation, ImpactData.ImpactIntensity);
            break;
        case EVFX_ImpactType::WaterSplash:
            // Water splash implementation
            break;
        case EVFX_ImpactType::FireSparks:
            // Fire sparks implementation
            break;
    }

    // Add to active impacts for tracking
    ActiveImpacts.Add(ImpactData);

    if (GEngine)
    {
        FString ImpactMsg = FString::Printf(TEXT("VFX Impact triggered at %s"), *ImpactData.ImpactLocation.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, ImpactMsg);
    }
}

void UVFX_ImpactManager::TriggerFootstepDust(FVector Location, float Intensity)
{
    if (DustParticleComponent)
    {
        SetActorLocation(Location);
        DustParticleComponent->SetWorldScale3D(FVector(Intensity * DefaultDustScale));
        DustParticleComponent->ActivateSystem();

        if (GEngine)
        {
            FString DustMsg = FString::Printf(TEXT("Footstep dust at %s, intensity: %.2f"), *Location.ToString(), Intensity);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Brown, DustMsg);
        }
    }
}

void UVFX_ImpactManager::TriggerBloodSplatter(FVector Location, float Intensity)
{
    if (BloodParticleComponent)
    {
        SetActorLocation(Location);
        BloodParticleComponent->SetWorldScale3D(FVector(Intensity * DefaultBloodScale));
        BloodParticleComponent->ActivateSystem();

        if (GEngine)
        {
            FString BloodMsg = FString::Printf(TEXT("Blood splatter at %s, intensity: %.2f"), *Location.ToString(), Intensity);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, BloodMsg);
        }
    }
}

void UVFX_ImpactManager::CreateDustCloud(FVector Location, float Scale)
{
    if (DustParticleComponent)
    {
        SetActorLocation(Location);
        DustParticleComponent->SetWorldScale3D(FVector(Scale * 1.5f));
        DustParticleComponent->ActivateSystem();

        if (GEngine)
        {
            FString CloudMsg = FString::Printf(TEXT("Dust cloud created at %s, scale: %.2f"), *Location.ToString(), Scale);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, CloudMsg);
        }
    }
}

void UVFX_ImpactManager::StopAllEffects()
{
    if (DustParticleComponent)
    {
        DustParticleComponent->DeactivateSystem();
    }

    if (BloodParticleComponent)
    {
        BloodParticleComponent->DeactivateSystem();
    }

    ActiveImpacts.Empty();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("All VFX effects stopped"));
    }
}

void UVFX_ImpactManager::UpdateActiveEffects(float DeltaTime)
{
    CleanupTimer += DeltaTime;

    if (CleanupTimer >= 1.0f) // Check every second
    {
        CleanupExpiredEffects();
        CleanupTimer = 0.0f;
    }
}

void UVFX_ImpactManager::CleanupExpiredEffects()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveImpacts.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - ActiveImpacts[i].Duration > EffectLifetime)
        {
            ActiveImpacts.RemoveAt(i);
        }
    }
}