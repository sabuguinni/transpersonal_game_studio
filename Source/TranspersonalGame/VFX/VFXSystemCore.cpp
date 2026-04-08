#include "VFXSystemCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UVFXSystemCore::UVFXSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update LOD every 100ms
    
    CurrentLOD = EVFXLODLevel::High;
    
    InitializeLODSettings();
}

void UVFXSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with medium LOD for safety
    SetLODLevel(EVFXLODLevel::Medium);
}

void UVFXSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Performance-based LOD adjustment
    if (ShouldAdjustLOD())
    {
        UpdatePerformanceLOD();
    }
}

void UVFXSystemCore::PlayEffect(EVFXCategory Category, FVector Location, FRotator Rotation)
{
    // Stop existing effect of same category
    StopEffect(Category);
    
    // Get VFX asset for category
    if (!VFXAssets.Contains(Category))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Asset not found for category: %d"), (int32)Category);
        return;
    }
    
    TSoftObjectPtr<UNiagaraSystem> VFXAsset = VFXAssets[Category];
    if (!VFXAsset.IsValid())
    {
        VFXAsset.LoadSynchronous();
    }
    
    if (VFXAsset.IsValid())
    {
        // Spawn Niagara component
        UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            VFXAsset.Get(),
            Location,
            Rotation
        );
        
        if (NewEffect)
        {
            // Apply current LOD settings
            ApplyLODToEffect(NewEffect, CurrentLOD);
            
            // Store reference
            ActiveEffects.Add(Category, NewEffect);
            
            UE_LOG(LogTemp, Log, TEXT("VFX Effect started: Category %d at location %s"), 
                   (int32)Category, *Location.ToString());
        }
    }
}

void UVFXSystemCore::StopEffect(EVFXCategory Category)
{
    if (ActiveEffects.Contains(Category))
    {
        UNiagaraComponent* Effect = ActiveEffects[Category];
        if (IsValid(Effect))
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
        
        ActiveEffects.Remove(Category);
        
        UE_LOG(LogTemp, Log, TEXT("VFX Effect stopped: Category %d"), (int32)Category);
    }
}

void UVFXSystemCore::SetLODLevel(EVFXLODLevel NewLOD)
{
    if (CurrentLOD == NewLOD) return;
    
    CurrentLOD = NewLOD;
    
    // Apply LOD to all active effects
    for (auto& EffectPair : ActiveEffects)
    {
        if (IsValid(EffectPair.Value))
        {
            ApplyLODToEffect(EffectPair.Value, CurrentLOD);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX LOD changed to: %d"), (int32)NewLOD);
}

void UVFXSystemCore::UpdatePerformanceLOD()
{
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    // Downgrade LOD if performance is poor
    if (CurrentFPS < LODDowngradeThreshold)
    {
        switch (CurrentLOD)
        {
            case EVFXLODLevel::High:
                SetLODLevel(EVFXLODLevel::Medium);
                break;
            case EVFXLODLevel::Medium:
                SetLODLevel(EVFXLODLevel::Low);
                break;
            case EVFXLODLevel::Low:
                SetLODLevel(EVFXLODLevel::Disabled);
                break;
            default:
                break;
        }
    }
    // Upgrade LOD if performance allows
    else if (CurrentFPS > LODUpgradeThreshold)
    {
        switch (CurrentLOD)
        {
            case EVFXLODLevel::Disabled:
                SetLODLevel(EVFXLODLevel::Low);
                break;
            case EVFXLODLevel::Low:
                SetLODLevel(EVFXLODLevel::Medium);
                break;
            case EVFXLODLevel::Medium:
                SetLODLevel(EVFXLODLevel::High);
                break;
            default:
                break;
        }
    }
}

float UVFXSystemCore::GetCurrentGPULoad() const
{
    // Simplified GPU load estimation based on active effects
    float Load = 0.0f;
    
    for (const auto& EffectPair : ActiveEffects)
    {
        if (IsValid(EffectPair.Value))
        {
            Load += 0.1f; // Base cost per effect
        }
    }
    
    return FMath::Clamp(Load, 0.0f, 1.0f);
}

void UVFXSystemCore::SetWeatherIntensity(float Intensity)
{
    // Weather effects intensity control
    for (auto& EffectPair : ActiveEffects)
    {
        if (EffectPair.Key == EVFXCategory::Weather && IsValid(EffectPair.Value))
        {
            EffectPair.Value->SetFloatParameter(TEXT("Intensity"), Intensity);
        }
    }
}

void UVFXSystemCore::TriggerDangerAtmosphere(bool bEnable)
{
    if (bEnable)
    {
        // Trigger subtle danger VFX
        PlayEffect(EVFXCategory::Danger, GetOwner()->GetActorLocation());
    }
    else
    {
        StopEffect(EVFXCategory::Danger);
    }
}

void UVFXSystemCore::InitializeLODSettings()
{
    // High Quality LOD
    FVFXLODSettings HighLOD;
    HighLOD.MaxParticles = 2000;
    HighLOD.ViewDistanceMultiplier = 1.0f;
    HighLOD.UpdateRate = 60.0f;
    HighLOD.bEnableCollision = true;
    HighLOD.bCastShadows = true;
    LODSettings.Add(EVFXLODLevel::High, HighLOD);
    
    // Medium Quality LOD
    FVFXLODSettings MediumLOD;
    MediumLOD.MaxParticles = 1000;
    MediumLOD.ViewDistanceMultiplier = 0.8f;
    MediumLOD.UpdateRate = 30.0f;
    MediumLOD.bEnableCollision = true;
    MediumLOD.bCastShadows = false;
    LODSettings.Add(EVFXLODLevel::Medium, MediumLOD);
    
    // Low Quality LOD
    FVFXLODSettings LowLOD;
    LowLOD.MaxParticles = 500;
    LowLOD.ViewDistanceMultiplier = 0.5f;
    LowLOD.UpdateRate = 15.0f;
    LowLOD.bEnableCollision = false;
    LowLOD.bCastShadows = false;
    LODSettings.Add(EVFXLODLevel::Low, LowLOD);
    
    // Disabled LOD
    FVFXLODSettings DisabledLOD;
    DisabledLOD.MaxParticles = 0;
    DisabledLOD.ViewDistanceMultiplier = 0.0f;
    DisabledLOD.UpdateRate = 0.0f;
    DisabledLOD.bEnableCollision = false;
    DisabledLOD.bCastShadows = false;
    LODSettings.Add(EVFXLODLevel::Disabled, DisabledLOD);
}

void UVFXSystemCore::ApplyLODToEffect(UNiagaraComponent* Effect, EVFXLODLevel LOD)
{
    if (!IsValid(Effect) || !LODSettings.Contains(LOD))
    {
        return;
    }
    
    const FVFXLODSettings& Settings = LODSettings[LOD];
    
    // Apply LOD settings to Niagara component
    Effect->SetIntParameter(TEXT("MaxParticles"), Settings.MaxParticles);
    Effect->SetFloatParameter(TEXT("ViewDistanceMultiplier"), Settings.ViewDistanceMultiplier);
    Effect->SetFloatParameter(TEXT("UpdateRate"), Settings.UpdateRate);
    Effect->SetBoolParameter(TEXT("EnableCollision"), Settings.bEnableCollision);
    Effect->SetCastShadow(Settings.bCastShadows);
    
    // Disable component entirely if LOD is disabled
    if (LOD == EVFXLODLevel::Disabled)
    {
        Effect->Deactivate();
    }
    else if (!Effect->IsActive())
    {
        Effect->Activate();
    }
}

bool UVFXSystemCore::ShouldAdjustLOD() const
{
    // Check if we have enough active effects to warrant LOD management
    return ActiveEffects.Num() > 2;
}