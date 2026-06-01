#include "VFX_EffectManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"

UVFX_EffectManager::UVFX_EffectManager()
{
    MaxActiveEffects = 50;
    EffectCullDistance = 10000.0f;
    CurrentWeatherVFX = nullptr;
}

void UVFX_EffectManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Initializing VFX subsystem"));
    
    InitializeDefaultEffects();
    
    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CleanupTimerHandle, this, &UVFX_EffectManager::CleanupExpiredEffects, 2.0f, true);
    }
}

void UVFX_EffectManager::Deinitialize()
{
    // Stop all active effects
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
    
    if (IsValid(CurrentWeatherVFX))
    {
        CurrentWeatherVFX->DestroyComponent();
        CurrentWeatherVFX = nullptr;
    }
    
    // Clear cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    Super::Deinitialize();
}

UNiagaraComponent* UVFX_EffectManager::SpawnVFXAtLocation(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation)
{
    if (!CanSpawnNewEffect())
    {
        return nullptr;
    }
    
    const FVFX_EffectData* EffectData = EffectRegistry.Find(EffectType);
    if (!EffectData || !EffectData->NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: No valid Niagara system for effect type %d"), (int32)EffectType);
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData->NiagaraSystem.LoadSynchronous(),
        Location,
        Rotation,
        EffectData->Scale,
        EffectData->bAutoDestroy
    );
    
    if (VFXComponent)
    {
        ActiveEffects.Add(VFXComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Spawned VFX at location %s"), *Location.ToString());
    }
    
    return VFXComponent;
}

UNiagaraComponent* UVFX_EffectManager::AttachVFXToActor(EVFX_EffectType EffectType, AActor* TargetActor, const FName& SocketName)
{
    if (!IsValid(TargetActor) || !CanSpawnNewEffect())
    {
        return nullptr;
    }
    
    const FVFX_EffectData* EffectData = EffectRegistry.Find(EffectType);
    if (!EffectData || !EffectData->NiagaraSystem.IsValid())
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        EffectData->NiagaraSystem.LoadSynchronous(),
        TargetActor->GetRootComponent(),
        SocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EffectData->Scale,
        EAttachLocation::KeepRelativeOffset,
        EffectData->bAutoDestroy
    );
    
    if (VFXComponent)
    {
        ActiveEffects.Add(VFXComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Attached VFX to actor %s"), *TargetActor->GetName());
    }
    
    return VFXComponent;
}

void UVFX_EffectManager::StopVFXEffect(UNiagaraComponent* VFXComponent)
{
    if (IsValid(VFXComponent))
    {
        VFXComponent->Deactivate();
        ActiveEffects.Remove(VFXComponent);
    }
}

void UVFX_EffectManager::RegisterEffectData(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData)
{
    EffectRegistry.Add(EffectType, EffectData);
    UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Registered effect type %d"), (int32)EffectType);
}

void UVFX_EffectManager::PlayFootstepVFX(const FVector& Location, float DinosaurSize)
{
    EVFX_EffectType EffectType = (DinosaurSize > 5.0f) ? EVFX_EffectType::DustImpact : EVFX_EffectType::FootstepDust;
    
    FVector ScaledLocation = Location;
    FVector Scale = FVector(DinosaurSize * 0.5f);
    
    // Create temporary effect data for dynamic scaling
    FVFX_EffectData TempData;
    TempData.EffectType = EffectType;
    TempData.Scale = Scale;
    TempData.Duration = 2.0f;
    TempData.bAutoDestroy = true;
    
    SpawnVFXAtLocation(EffectType, ScaledLocation);
}

void UVFX_EffectManager::PlayCombatImpactVFX(const FVector& Location, const FVector& ImpactNormal)
{
    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    SpawnVFXAtLocation(EVFX_EffectType::BloodSplatter, Location, ImpactRotation);
}

void UVFX_EffectManager::PlayCampfireVFX(const FVector& Location)
{
    SpawnVFXAtLocation(EVFX_EffectType::CampfireFire, Location);
}

void UVFX_EffectManager::StartWeatherVFX(EVFX_EffectType WeatherType, float Intensity)
{
    StopWeatherVFX();
    
    if (WeatherType == EVFX_EffectType::WeatherRain || WeatherType == EVFX_EffectType::WeatherSnow)
    {
        FVector WorldCenter = FVector::ZeroVector;
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            WorldCenter = PlayerPawn->GetActorLocation();
        }
        
        CurrentWeatherVFX = SpawnVFXAtLocation(WeatherType, WorldCenter + FVector(0, 0, 2000));
        
        if (CurrentWeatherVFX)
        {
            CurrentWeatherVFX->SetFloatParameter(TEXT("Intensity"), Intensity);
        }
    }
}

void UVFX_EffectManager::StopWeatherVFX()
{
    if (IsValid(CurrentWeatherVFX))
    {
        CurrentWeatherVFX->Deactivate();
        ActiveEffects.Remove(CurrentWeatherVFX);
        CurrentWeatherVFX = nullptr;
    }
}

void UVFX_EffectManager::InitializeDefaultEffects()
{
    // Register default effect configurations
    FVFX_EffectData DustImpactData;
    DustImpactData.EffectType = EVFX_EffectType::DustImpact;
    DustImpactData.Scale = FVector(2.0f);
    DustImpactData.Duration = 3.0f;
    DustImpactData.bAutoDestroy = true;
    RegisterEffectData(EVFX_EffectType::DustImpact, DustImpactData);
    
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::BloodSplatter;
    BloodData.Scale = FVector(1.0f);
    BloodData.Duration = 5.0f;
    BloodData.bAutoDestroy = true;
    RegisterEffectData(EVFX_EffectType::BloodSplatter, BloodData);
    
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::CampfireFire;
    CampfireData.Scale = FVector(1.5f);
    CampfireData.Duration = 0.0f; // Continuous
    CampfireData.bAutoDestroy = false;
    RegisterEffectData(EVFX_EffectType::CampfireFire, CampfireData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Default effects initialized"));
}

void UVFX_EffectManager::CleanupExpiredEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !IsValid(Effect) || !Effect->IsActive();
    });
    
    CullDistantEffects();
}

bool UVFX_EffectManager::CanSpawnNewEffect() const
{
    return ActiveEffects.Num() < MaxActiveEffects;
}

void UVFX_EffectManager::CullDistantEffects()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (IsValid(Effect))
        {
            float Distance = FVector::Dist(Effect->GetComponentLocation(), PlayerLocation);
            if (Distance > EffectCullDistance)
            {
                Effect->DestroyComponent();
                ActiveEffects.RemoveAt(i);
            }
        }
    }
}