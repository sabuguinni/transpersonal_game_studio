#include "VFX_ImpactEffectManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"

UVFX_ImpactEffectManager::UVFX_ImpactEffectManager()
{
    MaxEffectLifetime = 5.0f;
    MaxActiveEffects = 50;
}

void UVFX_ImpactEffectManager::CreateImpactEffect(const FVFX_ImpactEffectData& EffectData)
{
    if (!GEngine || !GEngine->GetCurrentPlayWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactEffectManager: No valid world found"));
        return;
    }

    // Clean up old effects if we're at the limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupExpiredEffects();
    }

    SpawnEffectActor(EffectData);
}

void UVFX_ImpactEffectManager::CreateFootstepDust(FVector Location, float Intensity)
{
    FVFX_ImpactEffectData EffectData;
    EffectData.EffectType = EVFX_ImpactType::FootstepDust;
    EffectData.Location = Location;
    EffectData.Intensity = Intensity;
    EffectData.Duration = 3.0f;
    EffectData.Scale = FVector(Intensity, Intensity, Intensity);

    CreateImpactEffect(EffectData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Created footstep dust at %s with intensity %f"), 
           *Location.ToString(), Intensity);
}

void UVFX_ImpactEffectManager::CreateBloodSplatter(FVector Location, FVector Direction, float Amount)
{
    FVFX_ImpactEffectData EffectData;
    EffectData.EffectType = EVFX_ImpactType::BloodSplatter;
    EffectData.Location = Location;
    EffectData.Intensity = Amount;
    EffectData.Duration = 8.0f; // Blood stays longer
    EffectData.Scale = FVector(Amount, Amount, 1.0f);

    CreateImpactEffect(EffectData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Created blood splatter at %s"), *Location.ToString());
}

void UVFX_ImpactEffectManager::CreateWaterSplash(FVector Location, float SplashSize)
{
    FVFX_ImpactEffectData EffectData;
    EffectData.EffectType = EVFX_ImpactType::WaterSplash;
    EffectData.Location = Location;
    EffectData.Intensity = SplashSize;
    EffectData.Duration = 2.5f;
    EffectData.Scale = FVector(SplashSize, SplashSize, SplashSize);

    CreateImpactEffect(EffectData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Created water splash at %s with size %f"), 
           *Location.ToString(), SplashSize);
}

void UVFX_ImpactEffectManager::CreateFireSparks(FVector Location, int32 SparkCount)
{
    FVFX_ImpactEffectData EffectData;
    EffectData.EffectType = EVFX_ImpactType::FireSparks;
    EffectData.Location = Location;
    EffectData.Intensity = static_cast<float>(SparkCount) / 10.0f;
    EffectData.Duration = 4.0f;
    EffectData.Scale = FVector::OneVector;

    CreateImpactEffect(EffectData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Created fire sparks at %s with %d sparks"), 
           *Location.ToString(), SparkCount);
}

void UVFX_ImpactEffectManager::SpawnEffectActor(const FVFX_ImpactEffectData& EffectData)
{
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        return;
    }

    // Create a simple static mesh actor as VFX placeholder
    AStaticMeshActor* EffectActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        EffectData.Location,
        FRotator::ZeroRotator
    );

    if (EffectActor)
    {
        // Set the scale based on effect data
        EffectActor->SetActorScale3D(EffectData.Scale);
        
        // Set a label based on effect type
        FString EffectLabel;
        switch (EffectData.EffectType)
        {
            case EVFX_ImpactType::FootstepDust:
                EffectLabel = TEXT("VFX_FootstepDust");
                break;
            case EVFX_ImpactType::BloodSplatter:
                EffectLabel = TEXT("VFX_BloodSplatter");
                break;
            case EVFX_ImpactType::WaterSplash:
                EffectLabel = TEXT("VFX_WaterSplash");
                break;
            case EVFX_ImpactType::FireSparks:
                EffectLabel = TEXT("VFX_FireSparks");
                break;
            default:
                EffectLabel = TEXT("VFX_Unknown");
                break;
        }
        
        EffectActor->SetActorLabel(EffectLabel);
        RegisterEffect(EffectActor);
        
        // Schedule cleanup after duration
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, [this, EffectActor]()
        {
            if (IsValid(EffectActor))
            {
                ActiveEffects.Remove(EffectActor);
                EffectActor->Destroy();
            }
        }, EffectData.Duration, false);
    }
}

void UVFX_ImpactEffectManager::RegisterEffect(AActor* EffectActor)
{
    if (EffectActor)
    {
        ActiveEffects.Add(EffectActor);
    }
}

void UVFX_ImpactEffectManager::CleanupExpiredEffects()
{
    // Remove null or invalid actors from the array
    ActiveEffects.RemoveAll([](AActor* Actor)
    {
        return !IsValid(Actor);
    });

    // If still too many, destroy the oldest ones
    while (ActiveEffects.Num() >= MaxActiveEffects)
    {
        AActor* OldestEffect = ActiveEffects[0];
        if (IsValid(OldestEffect))
        {
            OldestEffect->Destroy();
        }
        ActiveEffects.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Cleaned up effects, %d active"), ActiveEffects.Num());
}