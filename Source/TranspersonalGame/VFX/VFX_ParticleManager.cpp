#include "VFX_ParticleManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AVFX_ParticleManager::AVFX_ParticleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Tick once per second for cleanup
    
    MaxActiveEffects = 50.0f;
    EffectCullDistance = 5000.0f;
}

void AVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectSystems();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Initialized with %d effect types"), EffectSystems.Num());
}

void AVFX_ParticleManager::InitializeEffectSystems()
{
    // Initialize with placeholder paths - these would be replaced with actual Niagara systems
    // For now, we create the mapping structure
    
    // Fire effects
    EffectSystems.Add(EVFX_EffectType::Fire_Campfire, nullptr);
    
    // Dust and impact effects
    EffectSystems.Add(EVFX_EffectType::Dust_Footstep, nullptr);
    EffectSystems.Add(EVFX_EffectType::Blood_Impact, nullptr);
    
    // Weather effects
    EffectSystems.Add(EVFX_EffectType::Weather_Rain, nullptr);
    EffectSystems.Add(EVFX_EffectType::Weather_Fog, nullptr);
    
    // Utility effects
    EffectSystems.Add(EVFX_EffectType::Smoke_General, nullptr);
    EffectSystems.Add(EVFX_EffectType::Sparks_Crafting, nullptr);
    EffectSystems.Add(EVFX_EffectType::Water_Splash, nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Effect systems initialized"));
}

UNiagaraComponent* AVFX_ParticleManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Duration)
{
    if (!IsLocationVisible(Location))
    {
        return nullptr;
    }
    
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupExpiredEffects();
        if (ActiveEffects.Num() >= MaxActiveEffects)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Max effects reached, skipping spawn"));
            return nullptr;
        }
    }
    
    UNiagaraSystem** FoundSystem = EffectSystems.Find(EffectType);
    if (!FoundSystem || !*FoundSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Effect system not found for type %d"), (int32)EffectType);
        return nullptr;
    }
    
    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        *FoundSystem,
        Location,
        Rotation,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned effect type %d at %s"), (int32)EffectType, *Location.ToString());
        
        // Set auto-destroy timer if duration is specified
        if (Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NewEffect]()
            {
                if (NewEffect && IsValid(NewEffect))
                {
                    ActiveEffects.Remove(NewEffect);
                    NewEffect->DestroyComponent();
                }
            }, Duration, false);
        }
    }
    
    return NewEffect;
}

void AVFX_ParticleManager::SpawnCampfireEffect(FVector Location)
{
    UNiagaraComponent* FireEffect = SpawnEffect(EVFX_EffectType::Fire_Campfire, Location, FRotator::ZeroRotator, 0.0f); // Persistent fire
    if (FireEffect)
    {
        // Set fire-specific parameters
        SetEffectIntensity(FireEffect, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Campfire effect spawned at %s"), *Location.ToString());
    }
}

void AVFX_ParticleManager::SpawnFootstepDust(FVector Location, float Intensity)
{
    UNiagaraComponent* DustEffect = SpawnEffect(EVFX_EffectType::Dust_Footstep, Location, FRotator::ZeroRotator, 2.0f);
    if (DustEffect)
    {
        SetEffectIntensity(DustEffect, Intensity);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Footstep dust spawned at %s with intensity %f"), *Location.ToString(), Intensity);
    }
}

void AVFX_ParticleManager::SpawnBloodImpact(FVector Location, FVector ImpactNormal)
{
    FRotator ImpactRotation = ImpactNormal.Rotation();
    UNiagaraComponent* BloodEffect = SpawnEffect(EVFX_EffectType::Blood_Impact, Location, ImpactRotation, 3.0f);
    if (BloodEffect)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Blood impact spawned at %s"), *Location.ToString());
    }
}

void AVFX_ParticleManager::SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Intensity)
{
    if (WeatherType == EVFX_EffectType::Weather_Rain || WeatherType == EVFX_EffectType::Weather_Fog)
    {
        UNiagaraComponent* WeatherEffect = SpawnEffect(WeatherType, Location, FRotator::ZeroRotator, 0.0f); // Persistent weather
        if (WeatherEffect)
        {
            SetEffectIntensity(WeatherEffect, Intensity);
            UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Weather effect type %d spawned at %s"), (int32)WeatherType, *Location.ToString());
        }
    }
}

void AVFX_ParticleManager::CleanupExpiredEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !IsValid(Effect) || !Effect->IsActive();
    });
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Cleanup complete, %d active effects remaining"), ActiveEffects.Num());
}

void AVFX_ParticleManager::SetEffectIntensity(UNiagaraComponent* Effect, float Intensity)
{
    if (Effect && IsValid(Effect))
    {
        // Set common intensity parameters that most Niagara systems should have
        Effect->SetFloatParameter(FName("Intensity"), Intensity);
        Effect->SetFloatParameter(FName("SpawnRate"), Intensity * 100.0f);
        Effect->SetFloatParameter(FName("Scale"), Intensity);
    }
}

bool AVFX_ParticleManager::IsLocationVisible(FVector Location)
{
    // Simple distance check for now - could be expanded with frustum culling
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
        return Distance <= EffectCullDistance;
    }
    return true; // Default to visible if no player found
}

void AVFX_ParticleManager::CullDistantEffects()
{
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        ActiveEffects.RemoveAll([this, PlayerLocation](UNiagaraComponent* Effect)
        {
            if (!IsValid(Effect))
                return true;
                
            float Distance = FVector::Dist(PlayerLocation, Effect->GetComponentLocation());
            if (Distance > EffectCullDistance)
            {
                Effect->DestroyComponent();
                return true;
            }
            return false;
        });
    }
}