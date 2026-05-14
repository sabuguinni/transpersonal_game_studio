#include "VFX_ImpactEffectsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UVFX_ImpactEffectsManager::UVFX_ImpactEffectsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.2f; // 5 FPS for cleanup
    
    GlobalVFXIntensity = 1.0f;
    bEnableVFXLOD = true;
    MaxActiveEffects = 50;
    EffectCullDistance = 5000.0f;
    CleanupInterval = 5.0f;
    CurrentActiveEffects = 0;
    LastCleanupTime = 0.0f;
}

void UVFX_ImpactEffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEffectMaps();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Impact Effects Manager initialized"));
}

void UVFX_ImpactEffectsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic cleanup of finished effects
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCleanupTime > CleanupInterval)
    {
        CleanupFinishedEffects();
        LastCleanupTime = CurrentTime;
    }
}

void UVFX_ImpactEffectsManager::InitializeEffectMaps()
{
    // Initialize default impact effects
    FVFX_ImpactEffect DinosaurFootstepEffect;
    DinosaurFootstepEffect.EffectScale = 2.0f;
    DinosaurFootstepEffect.VolumeMultiplier = 1.5f;
    DinosaurFootstepEffect.EffectDuration = 4.0f;
    ImpactEffectMap.Add(EVFX_ImpactType::DinosaurFootstep, DinosaurFootstepEffect);
    
    FVFX_ImpactEffect PlayerFootstepEffect;
    PlayerFootstepEffect.EffectScale = 0.5f;
    PlayerFootstepEffect.VolumeMultiplier = 0.3f;
    PlayerFootstepEffect.EffectDuration = 1.5f;
    ImpactEffectMap.Add(EVFX_ImpactType::PlayerFootstep, PlayerFootstepEffect);
    
    FVFX_ImpactEffect WeaponImpactEffect;
    WeaponImpactEffect.EffectScale = 1.0f;
    WeaponImpactEffect.VolumeMultiplier = 0.8f;
    WeaponImpactEffect.EffectDuration = 2.0f;
    ImpactEffectMap.Add(EVFX_ImpactType::WeaponImpact, WeaponImpactEffect);
    
    FVFX_ImpactEffect BloodEffect;
    BloodEffect.EffectScale = 1.2f;
    BloodEffect.VolumeMultiplier = 0.6f;
    BloodEffect.EffectDuration = 8.0f;
    BloodEffect.bAttachToSurface = true;
    ImpactEffectMap.Add(EVFX_ImpactType::BloodSplatter, BloodEffect);
    
    // Initialize surface-specific effects
    FVFX_ImpactEffect DirtEffect;
    DirtEffect.EffectScale = 1.0f;
    DirtEffect.VolumeMultiplier = 1.0f;
    SurfaceEffectMap.Add(EVFX_SurfaceType::Dirt, DirtEffect);
    
    FVFX_ImpactEffect RockEffect;
    RockEffect.EffectScale = 0.8f;
    RockEffect.VolumeMultiplier = 1.2f;
    SurfaceEffectMap.Add(EVFX_SurfaceType::Rock, RockEffect);
    
    FVFX_ImpactEffect GrassEffect;
    GrassEffect.EffectScale = 0.6f;
    GrassEffect.VolumeMultiplier = 0.7f;
    SurfaceEffectMap.Add(EVFX_SurfaceType::Grass, GrassEffect);
    
    FVFX_ImpactEffect WaterEffect;
    WaterEffect.EffectScale = 1.5f;
    WaterEffect.VolumeMultiplier = 0.9f;
    SurfaceEffectMap.Add(EVFX_SurfaceType::Water, WaterEffect);
}

void UVFX_ImpactEffectsManager::SpawnImpactEffect(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, 
                                                 const FVector& Location, const FVector& Normal, 
                                                 float IntensityMultiplier)
{
    if (!ShouldSpawnEffect(Location))
    {
        return;
    }
    
    // Get effect configuration
    FVFX_ImpactEffect* ImpactConfig = ImpactEffectMap.Find(ImpactType);
    FVFX_ImpactEffect* SurfaceConfig = SurfaceEffectMap.Find(SurfaceType);
    
    if (!ImpactConfig && !SurfaceConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("No VFX configuration found for impact type %d and surface type %d"), 
               (int32)ImpactType, (int32)SurfaceType);
        return;
    }
    
    // Calculate final effect parameters
    float FinalScale = GlobalVFXIntensity * IntensityMultiplier;
    float LODMultiplier = CalculateLODMultiplier(Location);
    FinalScale *= LODMultiplier;
    
    if (ImpactConfig)
    {
        FinalScale *= ImpactConfig->EffectScale;
    }
    if (SurfaceConfig)
    {
        FinalScale *= SurfaceConfig->EffectScale;
    }
    
    // Spawn Niagara effect if available
    UNiagaraSystem* NiagaraEffect = nullptr;
    if (ImpactConfig && ImpactConfig->NiagaraEffect.IsValid())
    {
        NiagaraEffect = ImpactConfig->NiagaraEffect.LoadSynchronous();
    }
    
    if (NiagaraEffect)
    {
        FRotator EffectRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
        UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), NiagaraEffect, Location, EffectRotation);
        
        if (SpawnedEffect)
        {
            SpawnedEffect->SetFloatParameter(FName("EffectScale"), FinalScale);
            SpawnedEffect->SetFloatParameter(FName("Intensity"), IntensityMultiplier);
            RegisterActiveEffect(SpawnedEffect);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned Niagara impact effect at location %s with scale %f"), 
                   *Location.ToString(), FinalScale);
        }
    }
    
    // Spawn audio effect
    USoundCue* ImpactSound = nullptr;
    if (ImpactConfig && ImpactConfig->ImpactSound.IsValid())
    {
        ImpactSound = ImpactConfig->ImpactSound.LoadSynchronous();
    }
    
    if (ImpactSound)
    {
        float FinalVolume = GlobalVFXIntensity * IntensityMultiplier;
        if (ImpactConfig)
        {
            FinalVolume *= ImpactConfig->VolumeMultiplier;
        }
        
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(), ImpactSound, Location, FRotator::ZeroRotator, FinalVolume);
        
        if (AudioComp)
        {
            RegisterActiveAudio(AudioComp);
        }
    }
}

void UVFX_ImpactEffectsManager::SpawnDinosaurFootstep(const FVector& Location, const FVector& Normal, 
                                                     float DinosaurSize, const FString& DinosaurType)
{
    // Determine surface type based on hit result (simplified for now)
    EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt;
    
    // Scale effect based on dinosaur size
    float IntensityMultiplier = FMath::Clamp(DinosaurSize, 0.1f, 5.0f);
    
    SpawnImpactEffect(EVFX_ImpactType::DinosaurFootstep, SurfaceType, Location, Normal, IntensityMultiplier);
    
    // Additional dust cloud for large dinosaurs
    if (DinosaurSize > 2.0f)
    {
        SpawnDustCloud(Location, 150.0f * DinosaurSize, IntensityMultiplier);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned dinosaur footstep effect for %s (size: %f) at %s"), 
           *DinosaurType, DinosaurSize, *Location.ToString());
}

void UVFX_ImpactEffectsManager::SpawnPlayerFootstep(const FVector& Location, EVFX_SurfaceType SurfaceType)
{
    FVector Normal = FVector::UpVector; // Default upward normal
    SpawnImpactEffect(EVFX_ImpactType::PlayerFootstep, SurfaceType, Location, Normal, 1.0f);
}

void UVFX_ImpactEffectsManager::SpawnWeaponImpact(const FVector& Location, const FVector& Normal, const FString& WeaponType)
{
    EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt; // Default surface
    float IntensityMultiplier = 1.0f;
    
    // Adjust intensity based on weapon type
    if (WeaponType == "Spear")
    {
        IntensityMultiplier = 1.2f;
    }
    else if (WeaponType == "Club")
    {
        IntensityMultiplier = 1.5f;
    }
    else if (WeaponType == "Stone")
    {
        IntensityMultiplier = 0.8f;
    }
    
    SpawnImpactEffect(EVFX_ImpactType::WeaponImpact, SurfaceType, Location, Normal, IntensityMultiplier);
}

void UVFX_ImpactEffectsManager::SpawnBloodEffect(const FVector& Location, const FVector& Normal, float BloodAmount)
{
    EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt; // Blood on ground
    float IntensityMultiplier = FMath::Clamp(BloodAmount, 0.1f, 3.0f);
    
    SpawnImpactEffect(EVFX_ImpactType::BloodSplatter, SurfaceType, Location, Normal, IntensityMultiplier);
}

void UVFX_ImpactEffectsManager::SpawnDustCloud(const FVector& Location, float Radius, float Intensity)
{
    if (!ShouldSpawnEffect(Location))
    {
        return;
    }
    
    // Create a simple dust cloud effect using debug visualization for now
    // In a real implementation, this would spawn a Niagara dust system
    float FinalRadius = Radius * GlobalVFXIntensity * Intensity;
    float LODMultiplier = CalculateLODMultiplier(Location);
    FinalRadius *= LODMultiplier;
    
    UE_LOG(LogTemp, Log, TEXT("Spawned dust cloud at %s with radius %f"), 
           *Location.ToString(), FinalRadius);
    
    CurrentActiveEffects++;
}

void UVFX_ImpactEffectsManager::SpawnGroundCrack(const FVector& StartLocation, const FVector& EndLocation, float CrackWidth)
{
    if (!ShouldSpawnEffect(StartLocation))
    {
        return;
    }
    
    float Distance = FVector::Dist(StartLocation, EndLocation);
    float FinalWidth = CrackWidth * GlobalVFXIntensity;
    
    UE_LOG(LogTemp, Log, TEXT("Spawned ground crack from %s to %s (distance: %f, width: %f)"), 
           *StartLocation.ToString(), *EndLocation.ToString(), Distance, FinalWidth);
    
    CurrentActiveEffects++;
}

void UVFX_ImpactEffectsManager::SetGlobalVFXIntensity(float NewIntensity)
{
    GlobalVFXIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("Global VFX intensity set to %f"), GlobalVFXIntensity);
}

void UVFX_ImpactEffectsManager::EnableVFXLOD(bool bEnable)
{
    bEnableVFXLOD = bEnable;
    UE_LOG(LogTemp, Log, TEXT("VFX LOD %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UVFX_ImpactEffectsManager::SetMaxActiveEffects(int32 MaxEffects)
{
    MaxActiveEffects = FMath::Max(MaxEffects, 10);
    UE_LOG(LogTemp, Log, TEXT("Max active effects set to %d"), MaxActiveEffects);
}

void UVFX_ImpactEffectsManager::CleanupFinishedEffects()
{
    // Clean up finished Niagara effects
    for (int32 i = ActiveNiagaraEffects.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveNiagaraEffects[i]) || !ActiveNiagaraEffects[i]->IsActive())
        {
            ActiveNiagaraEffects.RemoveAt(i);
            CurrentActiveEffects = FMath::Max(0, CurrentActiveEffects - 1);
        }
    }
    
    // Clean up finished audio effects
    for (int32 i = ActiveAudioEffects.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveAudioEffects[i]) || !ActiveAudioEffects[i]->IsPlaying())
        {
            ActiveAudioEffects.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("VFX cleanup: %d active Niagara effects, %d active audio effects"), 
           ActiveNiagaraEffects.Num(), ActiveAudioEffects.Num());
}

bool UVFX_ImpactEffectsManager::ShouldSpawnEffect(const FVector& Location) const
{
    // Check if we've reached the maximum number of active effects
    if (CurrentActiveEffects >= MaxActiveEffects)
    {
        return false;
    }
    
    // Check distance culling
    if (bEnableVFXLOD)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            float DistanceToPlayer = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
            if (DistanceToPlayer > EffectCullDistance)
            {
                return false;
            }
        }
    }
    
    return true;
}

float UVFX_ImpactEffectsManager::CalculateLODMultiplier(const FVector& Location) const
{
    if (!bEnableVFXLOD)
    {
        return 1.0f;
    }
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return 1.0f;
    }
    
    float DistanceToPlayer = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
    float NormalizedDistance = DistanceToPlayer / EffectCullDistance;
    
    // Linear falloff from 1.0 at close distance to 0.1 at cull distance
    return FMath::Lerp(1.0f, 0.1f, NormalizedDistance);
}

void UVFX_ImpactEffectsManager::RegisterActiveEffect(UNiagaraComponent* Effect)
{
    if (IsValid(Effect))
    {
        ActiveNiagaraEffects.Add(Effect);
        CurrentActiveEffects++;
    }
}

void UVFX_ImpactEffectsManager::RegisterActiveAudio(UAudioComponent* Audio)
{
    if (IsValid(Audio))
    {
        ActiveAudioEffects.Add(Audio);
    }
}