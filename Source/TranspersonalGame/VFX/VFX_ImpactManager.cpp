#include "VFX_ImpactManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Optimize tick rate

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create Niagara component for effects
    ImpactEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ImpactEffectComponent"));
    ImpactEffectComponent->SetupAttachment(RootComponent);
    ImpactEffectComponent->SetAutoActivate(false);

    // Create audio component
    ImpactAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ImpactAudioComponent"));
    ImpactAudioComponent->SetupAttachment(RootComponent);
    ImpactAudioComponent->SetAutoActivate(false);

    // Initialize default values
    MaxSimultaneousEffects = 10;
    EffectCullingDistance = 5000.0f;
    MinTimeBetweenEffects = 0.1f;
    EnvironmentalEffectRadius = 2000.0f;
    
    LastEffectTime = 0.0f;
    CurrentQualityLevel = 2; // Medium quality by default
    bEffectsEnabled = true;

    // Initialize default effects
    InitializeDefaultEffects();
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Impact Manager initialized"));
}

void AVFX_ImpactManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Cleanup expired effects periodically
    CleanupExpiredEffects();
}

void AVFX_ImpactManager::InitializeDefaultEffects()
{
    // Initialize T-Rex footstep effects
    FVFX_DinosaurFootstepData TRexData;
    TRexData.DinosaurType = EDinosaurSpecies::TRex;
    TRexData.FootstepForce = 2000.0f;
    TRexData.BreathInterval = 4.0f;
    TRexData.FootstepImpact.EffectScale = 2.0f;
    TRexData.FootstepImpact.SoundVolume = 1.0f;
    TRexData.BreathVapor.EffectScale = 1.5f;
    TRexData.BreathVapor.EffectOffset = FVector(200, 0, 150);
    DinosaurFootstepEffects.Add(EDinosaurSpecies::TRex, TRexData);

    // Initialize Velociraptor footstep effects
    FVFX_DinosaurFootstepData VelociData;
    VelociData.DinosaurType = EDinosaurSpecies::Velociraptor;
    VelociData.FootstepForce = 300.0f;
    VelociData.BreathInterval = 2.0f;
    VelociData.FootstepImpact.EffectScale = 0.5f;
    VelociData.FootstepImpact.SoundVolume = 0.6f;
    VelociData.BreathVapor.EffectScale = 0.3f;
    VelociData.BreathVapor.EffectOffset = FVector(50, 0, 30);
    DinosaurFootstepEffects.Add(EDinosaurSpecies::Velociraptor, VelociData);

    // Initialize Triceratops footstep effects
    FVFX_DinosaurFootstepData TriceraData;
    TriceraData.DinosaurType = EDinosaurSpecies::Triceratops;
    TriceraData.FootstepForce = 1500.0f;
    TriceraData.BreathInterval = 3.5f;
    TriceraData.FootstepImpact.EffectScale = 1.8f;
    TriceraData.FootstepImpact.SoundVolume = 0.9f;
    TriceraData.BreathVapor.EffectScale = 1.2f;
    TriceraData.BreathVapor.EffectOffset = FVector(150, 0, 80);
    DinosaurFootstepEffects.Add(EDinosaurSpecies::Triceratops, TriceraData);

    // Initialize Brachiosaurus footstep effects
    FVFX_DinosaurFootstepData BrachioData;
    BrachioData.DinosaurType = EDinosaurSpecies::Brachiosaurus;
    BrachioData.FootstepForce = 3000.0f;
    BrachioData.BreathInterval = 5.0f;
    BrachioData.FootstepImpact.EffectScale = 3.0f;
    BrachioData.FootstepImpact.SoundVolume = 1.2f;
    BrachioData.BreathVapor.EffectScale = 2.0f;
    BrachioData.BreathVapor.EffectOffset = FVector(300, 0, 400);
    DinosaurFootstepEffects.Add(EDinosaurSpecies::Brachiosaurus, BrachioData);

    // Initialize weapon impact effect
    WeaponImpactEffect.EffectScale = 1.0f;
    WeaponImpactEffect.SoundVolume = 0.8f;
    WeaponImpactEffect.EffectOffset = FVector::ZeroVector;

    // Initialize blood splatter effect
    BloodSplatterEffect.EffectScale = 1.0f;
    BloodSplatterEffect.SoundVolume = 0.5f;
    BloodSplatterEffect.EffectOffset = FVector::ZeroVector;

    // Initialize environmental effects
    DustCloudEffect.EffectScale = 1.5f;
    DustCloudEffect.SoundVolume = 0.3f;
    
    WaterSplashEffect.EffectScale = 1.2f;
    WaterSplashEffect.SoundVolume = 0.7f;
}

void AVFX_ImpactManager::TriggerDinosaurFootstep(EDinosaurSpecies DinosaurType, const FVector& Location, const FVector& Normal)
{
    if (!bEffectsEnabled || !CanSpawnNewEffect())
    {
        return;
    }

    const FVFX_DinosaurFootstepData* FootstepData = DinosaurFootstepEffects.Find(DinosaurType);
    if (!FootstepData)
    {
        UE_LOG(LogTemp, Warning, TEXT("No footstep data found for dinosaur type"));
        return;
    }

    // Calculate effect scale based on dinosaur size
    float EffectScale = FootstepData->FootstepImpact.EffectScale;
    
    // Spawn dust cloud effect
    SpawnImpactEffect(FootstepData->FootstepImpact, Location, Normal, EffectScale);
    
    // Trigger environmental dust if on dirt/sand
    SpawnImpactEffect(DustCloudEffect, Location + FVector(0, 0, 20), FVector::UpVector, EffectScale * 0.8f);

    LastEffectTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Triggered dinosaur footstep VFX for %s at location %s"), 
           *UEnum::GetValueAsString(DinosaurType), *Location.ToString());
}

void AVFX_ImpactManager::TriggerWeaponImpact(const FVector& Location, const FVector& Normal, float ImpactForce)
{
    if (!bEffectsEnabled || !CanSpawnNewEffect())
    {
        return;
    }

    float EffectScale = FMath::Clamp(ImpactForce / 1000.0f, 0.5f, 2.0f);
    SpawnImpactEffect(WeaponImpactEffect, Location, Normal, EffectScale);

    LastEffectTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Triggered weapon impact VFX at location %s with force %f"), 
           *Location.ToString(), ImpactForce);
}

void AVFX_ImpactManager::TriggerBloodSplatter(const FVector& Location, const FVector& Direction, float BloodAmount)
{
    if (!bEffectsEnabled || !CanSpawnNewEffect())
    {
        return;
    }

    float EffectScale = FMath::Clamp(BloodAmount, 0.5f, 2.0f);
    FVector Normal = Direction.GetSafeNormal();
    
    SpawnImpactEffect(BloodSplatterEffect, Location, Normal, EffectScale);

    LastEffectTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Triggered blood splatter VFX at location %s"), *Location.ToString());
}

void AVFX_ImpactManager::TriggerEnvironmentalEffect(const FVector& Location, const FString& EffectType)
{
    if (!bEffectsEnabled || !CanSpawnNewEffect())
    {
        return;
    }

    FVFX_ImpactData* EffectData = nullptr;
    
    if (EffectType == TEXT("Dust"))
    {
        EffectData = &DustCloudEffect;
    }
    else if (EffectType == TEXT("Water"))
    {
        EffectData = &WaterSplashEffect;
    }
    
    if (EffectData)
    {
        SpawnImpactEffect(*EffectData, Location, FVector::UpVector, 1.0f);
        LastEffectTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("Triggered environmental effect %s at location %s"), 
               *EffectType, *Location.ToString());
    }
}

void AVFX_ImpactManager::TriggerDinosaurBreath(EDinosaurSpecies DinosaurType, const FVector& Location, const FVector& Direction)
{
    if (!bEffectsEnabled || !CanSpawnNewEffect())
    {
        return;
    }

    const FVFX_DinosaurFootstepData* DinoData = DinosaurFootstepEffects.Find(DinosaurType);
    if (!DinoData)
    {
        return;
    }

    FVector BreathLocation = Location + DinoData->BreathVapor.EffectOffset;
    SpawnImpactEffect(DinoData->BreathVapor, BreathLocation, Direction, 1.0f);

    LastEffectTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Triggered dinosaur breath VFX for %s"), *UEnum::GetValueAsString(DinosaurType));
}

void AVFX_ImpactManager::SetEffectQualityLevel(int32 QualityLevel)
{
    CurrentQualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    
    // Adjust max simultaneous effects based on quality
    switch (CurrentQualityLevel)
    {
        case 0: // Low
            MaxSimultaneousEffects = 5;
            EffectCullingDistance = 2000.0f;
            break;
        case 1: // Medium
            MaxSimultaneousEffects = 8;
            EffectCullingDistance = 3500.0f;
            break;
        case 2: // High
            MaxSimultaneousEffects = 12;
            EffectCullingDistance = 5000.0f;
            break;
        case 3: // Ultra
            MaxSimultaneousEffects = 20;
            EffectCullingDistance = 8000.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Quality level set to %d"), CurrentQualityLevel);
}

void AVFX_ImpactManager::EnableDisableEffects(bool bEnabled)
{
    bEffectsEnabled = bEnabled;
    
    if (!bEnabled)
    {
        ClearAllActiveEffects();
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Effects %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

int32 AVFX_ImpactManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void AVFX_ImpactManager::ClearAllActiveEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DeactivateImmediate();
            Effect->DestroyComponent();
        }
    }
    
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared all active VFX effects"));
}

void AVFX_ImpactManager::SpawnImpactEffect(const FVFX_ImpactData& ImpactData, const FVector& Location, const FVector& Normal, float Scale)
{
    if (!IsValid(GetWorld()))
    {
        return;
    }

    // Check distance culling
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (IsValid(PlayerPawn))
    {
        float DistanceToPlayer = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
        if (DistanceToPlayer > EffectCullingDistance)
        {
            return;
        }
    }

    // Create Niagara effect if system is available
    if (ImpactData.ParticleEffect.IsValid())
    {
        UNiagaraSystem* NiagaraSystem = ImpactData.ParticleEffect.LoadSynchronous();
        if (IsValid(NiagaraSystem))
        {
            FRotator EffectRotation = Normal.Rotation();
            FVector EffectLocation = Location + ImpactData.EffectOffset;
            
            UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                NiagaraSystem,
                EffectLocation,
                EffectRotation,
                FVector(Scale * ImpactData.EffectScale),
                true,
                true,
                ENCPoolMethod::None,
                true
            );
            
            if (IsValid(SpawnedEffect))
            {
                ActiveEffects.Add(SpawnedEffect);
            }
        }
    }

    // Play sound effect if available
    if (ImpactData.ImpactSound.IsValid())
    {
        USoundCue* SoundCue = ImpactData.ImpactSound.LoadSynchronous();
        if (IsValid(SoundCue))
        {
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                SoundCue,
                Location,
                ImpactData.SoundVolume
            );
        }
    }
}

void AVFX_ImpactManager::CleanupExpiredEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        if (!IsValid(Effect))
        {
            return true;
        }
        
        if (!Effect->IsActive())
        {
            Effect->DestroyComponent();
            return true;
        }
        
        return false;
    });
    
    // Enforce max simultaneous effects limit
    while (ActiveEffects.Num() > MaxSimultaneousEffects)
    {
        UNiagaraComponent* OldestEffect = ActiveEffects[0];
        if (IsValid(OldestEffect))
        {
            OldestEffect->DeactivateImmediate();
            OldestEffect->DestroyComponent();
        }
        ActiveEffects.RemoveAt(0);
    }
}

bool AVFX_ImpactManager::CanSpawnNewEffect() const
{
    if (!IsValid(GetWorld()))
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastEffectTime < MinTimeBetweenEffects)
    {
        return false;
    }
    
    return ActiveEffects.Num() < MaxSimultaneousEffects;
}

FVector AVFX_ImpactManager::CalculateEffectRotation(const FVector& Normal) const
{
    return Normal.Rotation().Euler();
}