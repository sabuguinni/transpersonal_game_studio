#include "VFX_ParticleSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UVFX_ParticleSystemManager::UVFX_ParticleSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance
    
    GlobalIntensityMultiplier = 1.0f;
    MaxActiveEffects = 50;
    bEnableVFXLOD = true;
    LODDistanceNear = 1000.0f;
    LODDistanceFar = 5000.0f;
}

void UVFX_ParticleSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeNiagaraSystems();
    
    // Clean up effects periodically
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UVFX_ParticleSystemManager::CleanupExpiredEffects,
            5.0f, // Every 5 seconds
            true
        );
    }
}

void UVFX_ParticleSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update LOD for active effects based on distance to player
    if (bEnableVFXLOD && ActiveEffects.Num() > 0)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            
            for (UNiagaraComponent* Effect : ActiveEffects)
            {
                if (IsValid(Effect))
                {
                    float Distance = FVector::Dist(Effect->GetComponentLocation(), PlayerLocation);
                    float LODScale = CalculateLODScale(Effect->GetComponentLocation());
                    
                    // Adjust effect intensity based on distance
                    Effect->SetFloatParameter(TEXT("IntensityMultiplier"), LODScale * GlobalIntensityMultiplier);
                    
                    // Disable very distant effects
                    if (Distance > LODDistanceFar * 2.0f)
                    {
                        Effect->SetVisibility(false);
                    }
                    else
                    {
                        Effect->SetVisibility(true);
                    }
                }
            }
        }
    }
}

void UVFX_ParticleSystemManager::SpawnDustCloud(const FVector& Location, float Intensity)
{
    if (!ShouldSpawnEffect(Location) || !DustCloudSystem)
    {
        return;
    }
    
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::DustCloud;
    EffectData.Intensity = Intensity;
    EffectData.Duration = 3.0f;
    EffectData.Scale = FVector(1.0f, 1.0f, 1.0f);
    EffectData.Color = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f); // Dusty brown color
    
    SpawnEnvironmentalEffect(EffectData, Location);
}

void UVFX_ParticleSystemManager::SpawnWaterSplash(const FVector& Location, float Scale)
{
    if (!ShouldSpawnEffect(Location) || !WaterSplashSystem)
    {
        return;
    }
    
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::WaterSplash;
    EffectData.Intensity = 1.0f;
    EffectData.Duration = 2.0f;
    EffectData.Scale = FVector(Scale, Scale, Scale);
    EffectData.Color = FLinearColor(0.8f, 0.9f, 1.0f, 0.8f); // Clear blue water
    
    SpawnEnvironmentalEffect(EffectData, Location);
}

void UVFX_ParticleSystemManager::SpawnFootstepImpact(const FVector& Location, const FVector& ImpactNormal)
{
    if (!ShouldSpawnEffect(Location) || !FootstepSystem)
    {
        return;
    }
    
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::FootstepImpact;
    EffectData.Intensity = 0.8f;
    EffectData.Duration = 1.5f;
    EffectData.Scale = FVector(0.5f, 0.5f, 0.5f);
    EffectData.Color = FLinearColor(0.7f, 0.6f, 0.5f, 1.0f); // Earth tones
    
    SpawnEnvironmentalEffect(EffectData, Location);
}

void UVFX_ParticleSystemManager::SpawnRainEffect(const FVector& Location, float Intensity)
{
    if (!ShouldSpawnEffect(Location) || !RainSystem)
    {
        return;
    }
    
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::RainDroplets;
    EffectData.Intensity = Intensity;
    EffectData.Duration = 10.0f; // Longer duration for rain
    EffectData.Scale = FVector(2.0f, 2.0f, 2.0f);
    EffectData.Color = FLinearColor(0.9f, 0.9f, 1.0f, 0.6f); // Light blue rain
    
    SpawnEnvironmentalEffect(EffectData, Location);
}

void UVFX_ParticleSystemManager::SpawnVolcanicAsh(const FVector& Location, const FVector& WindDirection)
{
    if (!ShouldSpawnEffect(Location) || !VolcanicAshSystem)
    {
        return;
    }
    
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::VolcanicAsh;
    EffectData.Intensity = 1.2f;
    EffectData.Duration = 8.0f;
    EffectData.Scale = FVector(1.5f, 1.5f, 1.5f);
    EffectData.Color = FLinearColor(0.3f, 0.3f, 0.3f, 0.8f); // Dark ash color
    
    SpawnEnvironmentalEffect(EffectData, Location);
}

void UVFX_ParticleSystemManager::SpawnWindParticles(const FVector& Location, const FVector& WindDirection, float Strength)
{
    if (!ShouldSpawnEffect(Location) || !WindParticleSystem)
    {
        return;
    }
    
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::WindParticles;
    EffectData.Intensity = Strength;
    EffectData.Duration = 5.0f;
    EffectData.Scale = FVector(1.0f, 1.0f, 1.0f);
    EffectData.Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.3f); // Subtle white particles
    
    SpawnEnvironmentalEffect(EffectData, Location);
}

void UVFX_ParticleSystemManager::SpawnEnvironmentalEffect(const FVFX_EffectData& EffectData, const FVector& Location)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        // Remove oldest effect to make room
        if (ActiveEffects.Num() > 0 && IsValid(ActiveEffects[0]))
        {
            ActiveEffects[0]->DestroyComponent();
            ActiveEffects.RemoveAt(0);
        }
    }
    
    UNiagaraSystem* SystemToUse = nullptr;
    
    switch (EffectData.EffectType)
    {
        case EVFX_EnvironmentalType::DustCloud:
            SystemToUse = DustCloudSystem;
            break;
        case EVFX_EnvironmentalType::WaterSplash:
            SystemToUse = WaterSplashSystem;
            break;
        case EVFX_EnvironmentalType::FootstepImpact:
            SystemToUse = FootstepSystem;
            break;
        case EVFX_EnvironmentalType::RainDroplets:
            SystemToUse = RainSystem;
            break;
        case EVFX_EnvironmentalType::VolcanicAsh:
            SystemToUse = VolcanicAshSystem;
            break;
        case EVFX_EnvironmentalType::WindParticles:
            SystemToUse = WindParticleSystem;
            break;
    }
    
    if (SystemToUse && GetWorld())
    {
        UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            SystemToUse,
            Location,
            FRotator::ZeroRotator,
            EffectData.Scale,
            true, // Auto destroy
            true, // Auto activate
            ENCPoolMethod::None,
            true  // Pre-cull check
        );
        
        if (SpawnedEffect)
        {
            // Set effect parameters
            float LODScale = CalculateLODScale(Location);
            SpawnedEffect->SetFloatParameter(TEXT("IntensityMultiplier"), EffectData.Intensity * LODScale * GlobalIntensityMultiplier);
            SpawnedEffect->SetFloatParameter(TEXT("Duration"), EffectData.Duration);
            SpawnedEffect->SetVectorParameter(TEXT("EffectColor"), FVector(EffectData.Color.R, EffectData.Color.G, EffectData.Color.B));
            SpawnedEffect->SetFloatParameter(TEXT("Alpha"), EffectData.Color.A);
            
            ActiveEffects.Add(SpawnedEffect);
            
            // Auto-cleanup after duration
            if (GetWorld())
            {
                FTimerHandle CleanupTimer;
                GetWorld()->GetTimerManager().SetTimer(
                    CleanupTimer,
                    [this, SpawnedEffect]()
                    {
                        if (IsValid(SpawnedEffect))
                        {
                            ActiveEffects.Remove(SpawnedEffect);
                            SpawnedEffect->DestroyComponent();
                        }
                    },
                    EffectData.Duration + 1.0f, // Add buffer time
                    false
                );
            }
        }
    }
}

void UVFX_ParticleSystemManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
}

void UVFX_ParticleSystemManager::SetGlobalVFXIntensity(float NewIntensity)
{
    GlobalIntensityMultiplier = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    
    // Update all active effects
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            float CurrentIntensity = Effect->GetFloatParameter(TEXT("IntensityMultiplier"));
            Effect->SetFloatParameter(TEXT("IntensityMultiplier"), CurrentIntensity * GlobalIntensityMultiplier);
        }
    }
}

void UVFX_ParticleSystemManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveEffects[i]) || !ActiveEffects[i]->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

float UVFX_ParticleSystemManager::CalculateLODScale(const FVector& EffectLocation) const
{
    if (!bEnableVFXLOD)
    {
        return 1.0f;
    }
    
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return 1.0f;
    }
    
    float Distance = FVector::Dist(EffectLocation, PC->GetPawn()->GetActorLocation());
    
    if (Distance <= LODDistanceNear)
    {
        return 1.0f; // Full quality
    }
    else if (Distance >= LODDistanceFar)
    {
        return 0.2f; // Minimum quality
    }
    else
    {
        // Linear interpolation between near and far distances
        float Alpha = (Distance - LODDistanceNear) / (LODDistanceFar - LODDistanceNear);
        return FMath::Lerp(1.0f, 0.2f, Alpha);
    }
}

bool UVFX_ParticleSystemManager::ShouldSpawnEffect(const FVector& Location) const
{
    // Don't spawn if we're at max capacity
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        return false;
    }
    
    // Don't spawn if too far from player (performance optimization)
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        float Distance = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
        if (Distance > LODDistanceFar * 2.0f)
        {
            return false;
        }
    }
    
    return true;
}

void UVFX_ParticleSystemManager::InitializeNiagaraSystems()
{
    // Load Niagara systems from content browser
    // These would be created by the VFX artist and referenced here
    
    // For now, we'll leave these as null and they can be set in Blueprint
    // or loaded from specific asset paths when the Niagara systems are created
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("VFX Particle System Manager initialized - Ready for Niagara system assignment"));
    }
}