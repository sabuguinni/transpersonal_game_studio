#include "VFX_ImpactManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance

    MaxConcurrentEffects = 20.0f;
    EffectCullDistance = 5000.0f;
    bEnableDistanceCulling = true;
    CurrentIntensityMultiplier = 1.0f;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultEffects();
    
    // Setup cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UVFX_ImpactManager::CleanupFinishedEffects,
            2.0f,
            true
        );
    }
}

void UVFX_ImpactManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Performance optimization - limit active effects
    if (ActiveEffects.Num() > MaxConcurrentEffects)
    {
        // Remove oldest effects
        int32 EffectsToRemove = ActiveEffects.Num() - MaxConcurrentEffects;
        for (int32 i = 0; i < EffectsToRemove; ++i)
        {
            if (ActiveEffects.IsValidIndex(0) && ActiveEffects[0])
            {
                ActiveEffects[0]->DestroyComponent();
                ActiveEffects.RemoveAt(0);
            }
        }
    }
}

void UVFX_ImpactManager::TriggerImpact(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation, AActor* AttachActor)
{
    if (!ImpactEffects.Contains(ImpactType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No effect data for impact type"));
        return;
    }

    // Distance culling check
    if (bEnableDistanceCulling && ShouldCullEffect(Location))
    {
        return;
    }

    const FVFX_ImpactData& EffectData = ImpactEffects[ImpactType];
    
    // Spawn particle effect
    if (EffectData.ParticleEffect)
    {
        UNiagaraComponent* NewEffect = nullptr;
        
        if (EffectData.bAttachToActor && AttachActor)
        {
            NewEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
                EffectData.ParticleEffect,
                AttachActor->GetRootComponent(),
                NAME_None,
                Location,
                Rotation,
                EAttachLocation::KeepWorldPosition,
                true
            );
        }
        else
        {
            NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                EffectData.ParticleEffect,
                Location,
                Rotation,
                EffectData.EffectScale * CurrentIntensityMultiplier,
                true
            );
        }
        
        if (NewEffect)
        {
            ActiveEffects.Add(NewEffect);
            
            // Set effect parameters based on intensity
            NewEffect->SetFloatParameter(TEXT("Intensity"), CurrentIntensityMultiplier);
            NewEffect->SetVectorParameter(TEXT("Scale"), EffectData.EffectScale * CurrentIntensityMultiplier);
        }
    }
    
    // Play impact sound
    if (EffectData.ImpactSound)
    {
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            EffectData.ImpactSound,
            Location,
            Rotation,
            CurrentIntensityMultiplier,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            true
        );
        
        if (AudioComp)
        {
            ActiveAudioComponents.Add(AudioComp);
        }
    }
}

void UVFX_ImpactManager::TriggerFootstepImpact(FVector Location, bool bIsHeavy, float IntensityMultiplier)
{
    EVFX_ImpactType FootstepType = bIsHeavy ? EVFX_ImpactType::FootstepHeavy : EVFX_ImpactType::FootstepLight;
    
    // Temporarily adjust intensity
    float PreviousIntensity = CurrentIntensityMultiplier;
    CurrentIntensityMultiplier *= IntensityMultiplier;
    
    TriggerImpact(FootstepType, Location, FRotator::ZeroRotator);
    
    // Also trigger dust cloud for heavy footsteps
    if (bIsHeavy)
    {
        TriggerDustCloud(Location, 150.0f * IntensityMultiplier);
    }
    
    // Restore previous intensity
    CurrentIntensityMultiplier = PreviousIntensity;
}

void UVFX_ImpactManager::TriggerBloodEffect(FVector Location, FVector Direction, float Intensity)
{
    // Calculate rotation from direction
    FRotator BloodRotation = Direction.Rotation();
    
    float PreviousIntensity = CurrentIntensityMultiplier;
    CurrentIntensityMultiplier *= Intensity;
    
    TriggerImpact(EVFX_ImpactType::BloodSplatter, Location, BloodRotation);
    
    CurrentIntensityMultiplier = PreviousIntensity;
}

void UVFX_ImpactManager::TriggerDustCloud(FVector Location, float Radius)
{
    TriggerImpact(EVFX_ImpactType::DustCloud, Location, FRotator::ZeroRotator);
    
    // Additional dust particles for larger radius
    if (Radius > 100.0f)
    {
        int32 ExtraParticles = FMath::Min(5, (int32)(Radius / 100.0f));
        for (int32 i = 0; i < ExtraParticles; ++i)
        {
            FVector RandomOffset = FVector(
                FMath::RandRange(-Radius * 0.5f, Radius * 0.5f),
                FMath::RandRange(-Radius * 0.5f, Radius * 0.5f),
                0.0f
            );
            TriggerImpact(EVFX_ImpactType::DustCloud, Location + RandomOffset, FRotator::ZeroRotator);
        }
    }
}

void UVFX_ImpactManager::StopAllEffects()
{
    // Stop all particle effects
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
    
    // Stop all audio components
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && IsValid(AudioComp))
        {
            AudioComp->Stop();
        }
    }
    ActiveAudioComponents.Empty();
}

void UVFX_ImpactManager::SetEffectIntensity(float NewIntensity)
{
    CurrentIntensityMultiplier = FMath::Clamp(NewIntensity, 0.1f, 5.0f);
}

int32 UVFX_ImpactManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void UVFX_ImpactManager::CleanupFinishedEffects()
{
    // Remove null or destroyed effects
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect) {
        return !Effect || !IsValid(Effect) || !Effect->IsActive();
    });
    
    // Remove finished audio components
    ActiveAudioComponents.RemoveAll([](UAudioComponent* AudioComp) {
        return !AudioComp || !IsValid(AudioComp) || !AudioComp->IsPlaying();
    });
}

void UVFX_ImpactManager::InitializeDefaultEffects()
{
    // Initialize default effect data
    // These would normally be set in Blueprint or loaded from assets
    
    FVFX_ImpactData FootstepLightData;
    FootstepLightData.EffectScale = FVector(0.5f, 0.5f, 0.5f);
    FootstepLightData.Duration = 1.0f;
    FootstepLightData.bAttachToActor = false;
    ImpactEffects.Add(EVFX_ImpactType::FootstepLight, FootstepLightData);
    
    FVFX_ImpactData FootstepHeavyData;
    FootstepHeavyData.EffectScale = FVector(1.5f, 1.5f, 1.0f);
    FootstepHeavyData.Duration = 2.0f;
    FootstepHeavyData.bAttachToActor = false;
    ImpactEffects.Add(EVFX_ImpactType::FootstepHeavy, FootstepHeavyData);
    
    FVFX_ImpactData BloodData;
    BloodData.EffectScale = FVector(1.0f, 1.0f, 1.0f);
    BloodData.Duration = 3.0f;
    BloodData.bAttachToActor = false;
    ImpactEffects.Add(EVFX_ImpactType::BloodSplatter, BloodData);
    
    FVFX_ImpactData DustData;
    DustData.EffectScale = FVector(2.0f, 2.0f, 1.0f);
    DustData.Duration = 2.5f;
    DustData.bAttachToActor = false;
    ImpactEffects.Add(EVFX_ImpactType::DustCloud, DustData);
    
    FVFX_ImpactData RockData;
    RockData.EffectScale = FVector(0.8f, 0.8f, 0.8f);
    RockData.Duration = 1.5f;
    RockData.bAttachToActor = false;
    ImpactEffects.Add(EVFX_ImpactType::RockImpact, RockData);
    
    FVFX_ImpactData WoodData;
    WoodData.EffectScale = FVector(0.6f, 0.6f, 0.6f);
    WoodData.Duration = 1.2f;
    WoodData.bAttachToActor = false;
    ImpactEffects.Add(EVFX_ImpactType::WoodImpact, WoodData);
    
    FVFX_ImpactData WaterData;
    WaterData.EffectScale = FVector(1.2f, 1.2f, 0.8f);
    WaterData.Duration = 2.0f;
    WaterData.bAttachToActor = false;
    ImpactEffects.Add(EVFX_ImpactType::WaterSplash, WaterData);
    
    FVFX_ImpactData SparkData;
    SparkData.EffectScale = FVector(0.7f, 0.7f, 0.7f);
    SparkData.Duration = 1.8f;
    SparkData.bAttachToActor = false;
    ImpactEffects.Add(EVFX_ImpactType::SparkShower, SparkData);
}

bool UVFX_ImpactManager::ShouldCullEffect(FVector EffectLocation) const
{
    if (!bEnableDistanceCulling)
    {
        return false;
    }
    
    // Get player location for distance check
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            float DistanceToPlayer = FVector::Dist(PlayerPawn->GetActorLocation(), EffectLocation);
            return DistanceToPlayer > EffectCullDistance;
        }
    }
    
    return false;
}