#include "VFXManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UVFXManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Initializing prehistoric VFX systems..."));
    
    InitializeVFXDatabase();
    
    // Setup cleanup timer for finished effects
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UVFXManager::CleanupFinishedEffects,
            1.0f,
            true
        );
        
        // Setup performance optimization timer
        World->GetTimerManager().SetTimer(
            PerformanceTimerHandle,
            this,
            &UVFXManager::OptimizeVFXPerformance,
            5.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Prehistoric VFX systems initialized successfully"));
}

void UVFXManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Shutting down VFX systems..."));
    
    // Stop all active VFX
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp))
        {
            VFXComp->DestroyComponent();
        }
    }
    
    ActiveVFXComponents.Empty();
    ActiveVFXByType.Empty();
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
        World->GetTimerManager().ClearTimer(PerformanceTimerHandle);
    }
    
    Super::Deinitialize();
}

void UVFXManager::InitializeVFXDatabase()
{
    // Environmental VFX
    FVFXEffectData WeatherData;
    WeatherData.DefaultScale = FVector(2.0f, 2.0f, 1.0f);
    WeatherData.DefaultLifetime = -1.0f; // Persistent
    WeatherData.bAutoDestroy = false;
    VFXDatabase.Add(EVFXType::WeatherEffect, WeatherData);
    
    FVFXEffectData FogData;
    FogData.DefaultScale = FVector(5.0f, 5.0f, 2.0f);
    FogData.DefaultLifetime = -1.0f; // Persistent
    FogData.bAutoDestroy = false;
    VFXDatabase.Add(EVFXType::FogSystem, FogData);
    
    // Combat VFX
    FVFXEffectData BloodData;
    BloodData.DefaultScale = FVector(1.0f);
    BloodData.DefaultLifetime = 3.0f;
    BloodData.bAutoDestroy = true;
    VFXDatabase.Add(EVFXType::BloodSplatter, BloodData);
    
    FVFXEffectData ImpactData;
    ImpactData.DefaultScale = FVector(0.8f);
    ImpactData.DefaultLifetime = 2.0f;
    ImpactData.bAutoDestroy = true;
    VFXDatabase.Add(EVFXType::ImpactEffect, ImpactData);
    
    // Mystical VFX
    FVFXEffectData GemData;
    GemData.DefaultScale = FVector(1.5f);
    GemData.DefaultLifetime = -1.0f; // Persistent
    GemData.bAutoDestroy = false;
    GemData.MinimumQuality = EVFXQuality::Medium;
    VFXDatabase.Add(EVFXType::GemGlow, GemData);
    
    FVFXEffectData PortalData;
    PortalData.DefaultScale = FVector(3.0f);
    PortalData.DefaultLifetime = 10.0f;
    PortalData.bAutoDestroy = true;
    PortalData.MinimumQuality = EVFXQuality::High;
    VFXDatabase.Add(EVFXType::PortalEffect, PortalData);
    
    UE_LOG(LogTemp, Log, TEXT("VFXManager: VFX Database initialized with %d effect types"), VFXDatabase.Num());
}

UNiagaraComponent* UVFXManager::SpawnVFXAtLocation(EVFXType VFXType, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    if (!ShouldSpawnVFX(VFXType))
    {
        return nullptr;
    }
    
    const FVFXEffectData* EffectData = VFXDatabase.Find(VFXType);
    if (!EffectData || !EffectData->NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: No valid Niagara system found for VFX type"));
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData->NiagaraSystem.LoadSynchronous(),
        Location,
        Rotation,
        Scale * EffectData->DefaultScale,
        EffectData->bAutoDestroy
    );
    
    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        ActiveVFXByType.FindOrAdd(VFXType).Add(VFXComponent);
        
        UE_LOG(LogTemp, Log, TEXT("VFXManager: Spawned VFX at location %s"), *Location.ToString());
    }
    
    return VFXComponent;
}

UNiagaraComponent* UVFXManager::SpawnVFXAttached(EVFXType VFXType, USceneComponent* AttachComponent, const FName& AttachPointName, const FVector& RelativeLocation)
{
    if (!ShouldSpawnVFX(VFXType) || !IsValid(AttachComponent))
    {
        return nullptr;
    }
    
    const FVFXEffectData* EffectData = VFXDatabase.Find(VFXType);
    if (!EffectData || !EffectData->NiagaraSystem.IsValid())
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        EffectData->NiagaraSystem.LoadSynchronous(),
        AttachComponent,
        AttachPointName,
        RelativeLocation,
        FRotator::ZeroRotator,
        EffectData->DefaultScale,
        EAttachLocation::KeepRelativeOffset,
        EffectData->bAutoDestroy
    );
    
    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        ActiveVFXByType.FindOrAdd(VFXType).Add(VFXComponent);
        
        UE_LOG(LogTemp, Log, TEXT("VFXManager: Spawned attached VFX on component %s"), *AttachComponent->GetName());
    }
    
    return VFXComponent;
}

void UVFXManager::StopVFXEffect(UNiagaraComponent* VFXComponent)
{
    if (IsValid(VFXComponent))
    {
        VFXComponent->DeactivateImmediate();
        ActiveVFXComponents.Remove(VFXComponent);
        
        // Remove from type-specific tracking
        for (auto& TypePair : ActiveVFXByType)
        {
            TypePair.Value.Remove(VFXComponent);
        }
    }
}

void UVFXManager::StopAllVFXOfType(EVFXType VFXType)
{
    TArray<UNiagaraComponent*>* VFXArray = ActiveVFXByType.Find(VFXType);
    if (!VFXArray)
    {
        return;
    }
    
    for (UNiagaraComponent* VFXComp : *VFXArray)
    {
        if (IsValid(VFXComp))
        {
            VFXComp->DeactivateImmediate();
            ActiveVFXComponents.Remove(VFXComp);
        }
    }
    
    VFXArray->Empty();
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Stopped all VFX of specified type"));
}

void UVFXManager::SpawnBloodEffect(const FVector& Location, const FVector& Normal, float Intensity)
{
    UNiagaraComponent* BloodVFX = SpawnVFXAtLocation(EVFXType::BloodSplatter, Location);
    if (BloodVFX)
    {
        BloodVFX->SetNiagaraVariableVec3(FString("BloodDirection"), Normal);
        BloodVFX->SetNiagaraVariableFloat(FString("Intensity"), Intensity);
    }
}

void UVFXManager::SpawnImpactEffect(const FVector& Location, const FVector& Normal, const FString& SurfaceType)
{
    UNiagaraComponent* ImpactVFX = SpawnVFXAtLocation(EVFXType::ImpactEffect, Location);
    if (ImpactVFX)
    {
        BloodVFX->SetNiagaraVariableVec3(FString("ImpactNormal"), Normal);
        ImpactVFX->SetNiagaraVariableFloat(FString("SurfaceType"), SurfaceType == "Stone" ? 1.0f : 0.0f);
    }
}

void UVFXManager::SpawnGemGlowEffect(const FVector& Location, const FLinearColor& GemColor)
{
    UNiagaraComponent* GemVFX = SpawnVFXAtLocation(EVFXType::GemGlow, Location);
    if (GemVFX)
    {
        GemVFX->SetNiagaraVariableLinearColor(FString("GemColor"), GemColor);
        GemVFX->SetNiagaraVariableFloat(FString("GlowIntensity"), 2.0f);
    }
}

void UVFXManager::TriggerConsciousnessShift(float Duration)
{
    // Trigger global consciousness shift VFX
    SetGlobalVFXParameter(FName("ConsciousnessShiftActive"), 1.0f);
    
    // Set timer to disable effect
    if (UWorld* World = GetWorld())
    {
        FTimerHandle ShiftTimerHandle;
        World->GetTimerManager().SetTimer(
            ShiftTimerHandle,
            [this]()
            {
                SetGlobalVFXParameter(FName("ConsciousnessShiftActive"), 0.0f);
            },
            Duration,
            false
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Triggered consciousness shift effect for %.2f seconds"), Duration);
}

void UVFXManager::SetVFXQuality(EVFXQuality Quality)
{
    CurrentVFXQuality = Quality;
    
    // Update quality settings based on level
    float QualityMultiplier = 1.0f;
    switch (Quality)
    {
        case EVFXQuality::Low:
            QualityMultiplier = 0.5f;
            MaxConcurrentVFX = 50;
            break;
        case EVFXQuality::Medium:
            QualityMultiplier = 0.75f;
            MaxConcurrentVFX = 75;
            break;
        case EVFXQuality::High:
            QualityMultiplier = 1.0f;
            MaxConcurrentVFX = 100;
            break;
        case EVFXQuality::Ultra:
            QualityMultiplier = 1.25f;
            MaxConcurrentVFX = 150;
            break;
    }
    
    SetGlobalVFXParameter(FName("QualityMultiplier"), QualityMultiplier);
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Set VFX quality to level %d"), (int32)Quality);
}

void UVFXManager::SetGlobalVFXParameter(const FName& ParameterName, float Value)
{
    if (GlobalVFXParameters)
    {
        UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), GlobalVFXParameters, ParameterName, Value);
    }
}

void UVFXManager::SetGlobalVFXVectorParameter(const FName& ParameterName, const FLinearColor& Value)
{
    if (GlobalVFXParameters)
    {
        UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), GlobalVFXParameters, ParameterName, Value);
    }
}

int32 UVFXManager::GetActiveVFXCount() const
{
    return ActiveVFXComponents.Num();
}

void UVFXManager::CleanupFinishedEffects()
{
    ActiveVFXComponents.RemoveAll([](UNiagaraComponent* VFXComp)
    {
        return !IsValid(VFXComp) || !VFXComp->IsActive();
    });
    
    // Clean up type-specific arrays
    for (auto& TypePair : ActiveVFXByType)
    {
        TypePair.Value.RemoveAll([](UNiagaraComponent* VFXComp)
        {
            return !IsValid(VFXComp) || !VFXComp->IsActive();
        });
    }
}

bool UVFXManager::ShouldSpawnVFX(EVFXType VFXType) const
{
    // Check if we're at max capacity
    if (ActiveVFXComponents.Num() >= MaxConcurrentVFX)
    {
        return false;
    }
    
    // Check quality requirements
    const FVFXEffectData* EffectData = VFXDatabase.Find(VFXType);
    if (EffectData && EffectData->MinimumQuality > CurrentVFXQuality)
    {
        return false;
    }
    
    return true;
}

void UVFXManager::OptimizeVFXPerformance()
{
    // Remove distant VFX to maintain performance
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        ActiveVFXComponents.RemoveAll([this, PlayerLocation](UNiagaraComponent* VFXComp)
        {
            if (!IsValid(VFXComp))
            {
                return true;
            }
            
            float Distance = FVector::Dist(VFXComp->GetComponentLocation(), PlayerLocation);
            if (Distance > VFXCullingDistance)
            {
                VFXComp->DeactivateImmediate();
                return true;
            }
            
            return false;
        });
    }
}