#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UVFXSystemManager::UVFXSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = TickInterval;
    
    // Set default values
    MaxActiveVFX = 50;
    VFXCullDistance = 10000.0f;
    CurrentLODLevel = EVFXLODLevel::High;
}

void UVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultVFXDefinitions();
    
    UE_LOG(LogTemp, Log, TEXT("VFX System Manager initialized with %d VFX definitions"), VFXRegistry.Num());
}

void UVFXSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic cleanup of invalid VFX components
    LastCleanupTime += DeltaTime;
    if (LastCleanupTime >= CleanupInterval)
    {
        CleanupInvalidVFX();
        LastCleanupTime = 0.0f;
    }
    
    // Update LOD based on distance if we have a player
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        UpdateVFXLODBasedOnDistance(PlayerPawn->GetActorLocation());
    }
}

UNiagaraComponent* UVFXSystemManager::SpawnVFXAtLocation(EVFXType VFXType, const FString& EffectName, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    FString VFXKey = GenerateVFXKey(VFXType, EffectName);
    FVFXDefinition* VFXDef = VFXRegistry.Find(VFXKey);
    
    if (!VFXDef || !VFXDef->NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX definition not found or invalid: %s"), *VFXKey);
        return nullptr;
    }
    
    // Check if we're at max capacity
    if (ActiveVFXComponents.Num() >= MaxActiveVFX)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max VFX limit reached (%d), skipping spawn"), MaxActiveVFX);
        return nullptr;
    }
    
    // Load the Niagara system if needed
    UNiagaraSystem* NiagaraSystem = VFXDef->NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Niagara system for VFX: %s"), *VFXKey);
        return nullptr;
    }
    
    // Spawn the VFX
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        NiagaraSystem,
        Location,
        Rotation,
        Scale,
        VFXDef->bAutoDestroy
    );
    
    if (VFXComponent)
    {
        // Apply current LOD settings
        ApplyLODToVFX(VFXComponent, CurrentLODLevel);
        
        // Set max draw distance
        VFXComponent->SetCullDistance(VFXDef->MaxDrawDistance);
        
        // Track the component
        ActiveVFXComponents.Add(VFXComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX: %s at location %s"), *VFXKey, *Location.ToString());
    }
    
    return VFXComponent;
}

UNiagaraComponent* UVFXSystemManager::SpawnVFXAttached(EVFXType VFXType, const FString& EffectName, USceneComponent* AttachComponent, const FName& AttachPointName, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    if (!AttachComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn attached VFX: AttachComponent is null"));
        return nullptr;
    }
    
    FString VFXKey = GenerateVFXKey(VFXType, EffectName);
    FVFXDefinition* VFXDef = VFXRegistry.Find(VFXKey);
    
    if (!VFXDef || !VFXDef->NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX definition not found or invalid: %s"), *VFXKey);
        return nullptr;
    }
    
    // Check capacity
    if (ActiveVFXComponents.Num() >= MaxActiveVFX)
    {
        return nullptr;
    }
    
    UNiagaraSystem* NiagaraSystem = VFXDef->NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        return nullptr;
    }
    
    // Spawn attached VFX
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        NiagaraSystem,
        AttachComponent,
        AttachPointName,
        Location,
        Rotation,
        Scale,
        EAttachLocation::KeepRelativeOffset,
        VFXDef->bAutoDestroy
    );
    
    if (VFXComponent)
    {
        ApplyLODToVFX(VFXComponent, CurrentLODLevel);
        VFXComponent->SetCullDistance(VFXDef->MaxDrawDistance);
        ActiveVFXComponents.Add(VFXComponent);
    }
    
    return VFXComponent;
}

void UVFXSystemManager::StopVFX(UNiagaraComponent* VFXComponent)
{
    if (VFXComponent && IsValid(VFXComponent))
    {
        VFXComponent->Deactivate();
        ActiveVFXComponents.RemoveSingle(VFXComponent);
    }
}

void UVFXSystemManager::StopAllVFXOfType(EVFXType VFXType)
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        if (ActiveVFXComponents[i].IsValid())
        {
            UNiagaraComponent* VFXComp = ActiveVFXComponents[i].Get();
            // Note: In a full implementation, we'd track VFX type per component
            // For now, we'll stop all VFX - this could be enhanced with component tagging
            VFXComp->Deactivate();
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

void UVFXSystemManager::SetVFXLODLevel(EVFXLODLevel LODLevel)
{
    CurrentLODLevel = LODLevel;
    
    // Apply LOD to all active VFX
    for (const TWeakObjectPtr<UNiagaraComponent>& VFXPtr : ActiveVFXComponents)
    {
        if (VFXPtr.IsValid())
        {
            ApplyLODToVFX(VFXPtr.Get(), LODLevel);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX LOD level set to: %d"), (int32)LODLevel);
}

void UVFXSystemManager::UpdateVFXLODBasedOnDistance(const FVector& ViewerLocation)
{
    for (const TWeakObjectPtr<UNiagaraComponent>& VFXPtr : ActiveVFXComponents)
    {
        if (VFXPtr.IsValid())
        {
            UNiagaraComponent* VFXComp = VFXPtr.Get();
            float Distance = FVector::Dist(ViewerLocation, VFXComp->GetComponentLocation());
            
            EVFXLODLevel LODLevel = EVFXLODLevel::High;
            if (Distance > 2000.0f)
            {
                LODLevel = EVFXLODLevel::Low;
            }
            else if (Distance > 1000.0f)
            {
                LODLevel = EVFXLODLevel::Medium;
            }
            
            ApplyLODToVFX(VFXComp, LODLevel);
        }
    }
}

void UVFXSystemManager::RegisterVFXDefinition(const FVFXDefinition& VFXDef)
{
    FString VFXKey = GenerateVFXKey(VFXDef.VFXType, VFXDef.EffectName);
    VFXRegistry.Add(VFXKey, VFXDef);
    
    UE_LOG(LogTemp, Log, TEXT("Registered VFX definition: %s"), *VFXKey);
}

FVFXDefinition UVFXSystemManager::GetVFXDefinition(EVFXType VFXType, const FString& EffectName) const
{
    FString VFXKey = GenerateVFXKey(VFXType, EffectName);
    const FVFXDefinition* VFXDef = VFXRegistry.Find(VFXKey);
    
    if (VFXDef)
    {
        return *VFXDef;
    }
    
    return FVFXDefinition(); // Return default definition
}

// Consciousness VFX implementations
UNiagaraComponent* UVFXSystemManager::SpawnConsciousnessAura(const FVector& Location, float Intensity, const FLinearColor& Color)
{
    UNiagaraComponent* VFXComp = SpawnVFXAtLocation(EVFXType::Consciousness, TEXT("ConsciousnessAura"), Location);
    
    if (VFXComp)
    {
        // Set consciousness-specific parameters
        VFXComp->SetFloatParameter(TEXT("Intensity"), Intensity);
        VFXComp->SetColorParameter(TEXT("AuraColor"), Color);
    }
    
    return VFXComp;
}

UNiagaraComponent* UVFXSystemManager::SpawnEnergySwirl(const FVector& Location, float Radius, float Speed)
{
    UNiagaraComponent* VFXComp = SpawnVFXAtLocation(EVFXType::Consciousness, TEXT("EnergySwirl"), Location);
    
    if (VFXComp)
    {
        VFXComp->SetFloatParameter(TEXT("SwirlRadius"), Radius);
        VFXComp->SetFloatParameter(TEXT("SwirlSpeed"), Speed);
    }
    
    return VFXComp;
}

// Environmental VFX implementations
UNiagaraComponent* UVFXSystemManager::SpawnWeatherEffect(const FString& WeatherType, const FVector& Location, float Intensity)
{
    UNiagaraComponent* VFXComp = SpawnVFXAtLocation(EVFXType::Environmental, WeatherType, Location);
    
    if (VFXComp)
    {
        VFXComp->SetFloatParameter(TEXT("WeatherIntensity"), Intensity);
    }
    
    return VFXComp;
}

UNiagaraComponent* UVFXSystemManager::SpawnAtmosphericFog(const FVector& Location, float Density, const FLinearColor& Color)
{
    UNiagaraComponent* VFXComp = SpawnVFXAtLocation(EVFXType::Environmental, TEXT("AtmosphericFog"), Location);
    
    if (VFXComp)
    {
        VFXComp->SetFloatParameter(TEXT("FogDensity"), Density);
        VFXComp->SetColorParameter(TEXT("FogColor"), Color);
    }
    
    return VFXComp;
}

// Combat VFX implementations
UNiagaraComponent* UVFXSystemManager::SpawnImpactEffect(const FVector& Location, const FVector& ImpactNormal, float Intensity)
{
    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    UNiagaraComponent* VFXComp = SpawnVFXAtLocation(EVFXType::Combat, TEXT("ImpactEffect"), Location, ImpactRotation);
    
    if (VFXComp)
    {
        VFXComp->SetFloatParameter(TEXT("ImpactIntensity"), Intensity);
        VFXComp->SetVectorParameter(TEXT("ImpactNormal"), ImpactNormal);
    }
    
    return VFXComp;
}

UNiagaraComponent* UVFXSystemManager::SpawnExplosion(const FVector& Location, float Radius, float Intensity)
{
    UNiagaraComponent* VFXComp = SpawnVFXAtLocation(EVFXType::Combat, TEXT("Explosion"), Location);
    
    if (VFXComp)
    {
        VFXComp->SetFloatParameter(TEXT("ExplosionRadius"), Radius);
        VFXComp->SetFloatParameter(TEXT("ExplosionIntensity"), Intensity);
    }
    
    return VFXComp;
}

// Private helper functions
void UVFXSystemManager::CleanupInvalidVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        if (!ActiveVFXComponents[i].IsValid())
        {
            ActiveVFXComponents.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("VFX cleanup complete. Active VFX count: %d"), ActiveVFXComponents.Num());
}

void UVFXSystemManager::ApplyLODToVFX(UNiagaraComponent* VFXComponent, EVFXLODLevel LODLevel)
{
    if (!VFXComponent)
        return;
    
    float LODMultiplier = 1.0f;
    switch (LODLevel)
    {
        case EVFXLODLevel::High:
            LODMultiplier = 1.0f;
            break;
        case EVFXLODLevel::Medium:
            LODMultiplier = 0.6f;
            break;
        case EVFXLODLevel::Low:
            LODMultiplier = 0.3f;
            break;
    }
    
    // Apply LOD multiplier to particle count and quality
    VFXComponent->SetFloatParameter(TEXT("LODMultiplier"), LODMultiplier);
}

FString UVFXSystemManager::GenerateVFXKey(EVFXType VFXType, const FString& EffectName) const
{
    return FString::Printf(TEXT("%s_%s"), *UEnum::GetValueAsString(VFXType), *EffectName);
}

void UVFXSystemManager::InitializeDefaultVFXDefinitions()
{
    // Initialize default VFX definitions
    // Note: In production, these would reference actual Niagara assets
    
    // Consciousness VFX
    FVFXDefinition ConsciousnessAura;
    ConsciousnessAura.VFXType = EVFXType::Consciousness;
    ConsciousnessAura.EffectName = TEXT("ConsciousnessAura");
    ConsciousnessAura.MaxDrawDistance = 3000.0f;
    ConsciousnessAura.Duration = 10.0f;
    RegisterVFXDefinition(ConsciousnessAura);
    
    FVFXDefinition EnergySwirl;
    EnergySwirl.VFXType = EVFXType::Consciousness;
    EnergySwirl.EffectName = TEXT("EnergySwirl");
    EnergySwirl.MaxDrawDistance = 2000.0f;
    EnergySwirl.Duration = 5.0f;
    RegisterVFXDefinition(EnergySwirl);
    
    // Environmental VFX
    FVFXDefinition Rain;
    Rain.VFXType = EVFXType::Environmental;
    Rain.EffectName = TEXT("Rain");
    Rain.MaxDrawDistance = 5000.0f;
    Rain.bAutoDestroy = false;
    RegisterVFXDefinition(Rain);
    
    FVFXDefinition AtmosphericFog;
    AtmosphericFog.VFXType = EVFXType::Environmental;
    AtmosphericFog.EffectName = TEXT("AtmosphericFog");
    AtmosphericFog.MaxDrawDistance = 8000.0f;
    AtmosphericFog.bAutoDestroy = false;
    RegisterVFXDefinition(AtmosphericFog);
    
    // Combat VFX
    FVFXDefinition ImpactEffect;
    ImpactEffect.VFXType = EVFXType::Combat;
    ImpactEffect.EffectName = TEXT("ImpactEffect");
    ImpactEffect.MaxDrawDistance = 1500.0f;
    ImpactEffect.Duration = 2.0f;
    RegisterVFXDefinition(ImpactEffect);
    
    FVFXDefinition Explosion;
    Explosion.VFXType = EVFXType::Combat;
    Explosion.EffectName = TEXT("Explosion");
    Explosion.MaxDrawDistance = 4000.0f;
    Explosion.Duration = 3.0f;
    RegisterVFXDefinition(Explosion);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d default VFX definitions"), VFXRegistry.Num());
}