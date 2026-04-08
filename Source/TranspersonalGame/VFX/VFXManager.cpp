#include "VFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize VFX Definitions with default values
    VFXDefinitions.SetNum(static_cast<int32>(EVFXType::MAX));
    
    // Atmospheric Effects
    VFXDefinitions[static_cast<int32>(EVFXType::DustMotes)].Type = EVFXType::DustMotes;
    VFXDefinitions[static_cast<int32>(EVFXType::DustMotes)].DefaultLifetime = -1.0f; // Infinite
    VFXDefinitions[static_cast<int32>(EVFXType::DustMotes)].MaxSimultaneousInstances = 5;
    
    VFXDefinitions[static_cast<int32>(EVFXType::Fog)].Type = EVFXType::Fog;
    VFXDefinitions[static_cast<int32>(EVFXType::Fog)].DefaultLifetime = -1.0f; // Infinite
    VFXDefinitions[static_cast<int32>(EVFXType::Fog)].MaxSimultaneousInstances = 3;
    
    // Danger Indicators
    VFXDefinitions[static_cast<int32>(EVFXType::DinosaurFootprint)].Type = EVFXType::DinosaurFootprint;
    VFXDefinitions[static_cast<int32>(EVFXType::DinosaurFootprint)].DefaultLifetime = 2.0f;
    VFXDefinitions[static_cast<int32>(EVFXType::DinosaurFootprint)].MaxSimultaneousInstances = 20;
    
    VFXDefinitions[static_cast<int32>(EVFXType::BrokenVegetation)].Type = EVFXType::BrokenVegetation;
    VFXDefinitions[static_cast<int32>(EVFXType::BrokenVegetation)].DefaultLifetime = 3.0f;
    VFXDefinitions[static_cast<int32>(EVFXType::BrokenVegetation)].MaxSimultaneousInstances = 15;
    
    VFXDefinitions[static_cast<int32>(EVFXType::BloodSplatter)].Type = EVFXType::BloodSplatter;
    VFXDefinitions[static_cast<int32>(EVFXType::BloodSplatter)].DefaultLifetime = 1.5f;
    VFXDefinitions[static_cast<int32>(EVFXType::BloodSplatter)].MaxSimultaneousInstances = 10;
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Cleanup finished VFX every frame
    CleanupFinishedVFX();
    
    // Performance management - cull distant VFX
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
    {
        CullDistantVFX(PlayerPawn->GetActorLocation());
    }
}

UNiagaraComponent* AVFXManager::SpawnVFX(EVFXType Type, FVector Location, FRotator Rotation, FVector Scale)
{
    FVFXDefinition* VFXDef = GetVFXDefinition(Type);
    if (!VFXDef || !VFXDef->NiagaraSystem.LoadSynchronous())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Invalid VFX type or missing Niagara system"));
        return nullptr;
    }
    
    // Check if we've reached the maximum number of instances for this type
    int32 CurrentInstances = 0;
    for (UNiagaraComponent* Component : ActiveVFXComponents)
    {
        if (Component && Component->GetAsset() == VFXDef->NiagaraSystem.Get())
        {
            CurrentInstances++;
        }
    }
    
    if (CurrentInstances >= VFXDef->MaxSimultaneousInstances)
    {
        // Remove oldest instance of this type
        for (int32 i = 0; i < ActiveVFXComponents.Num(); i++)
        {
            if (ActiveVFXComponents[i] && ActiveVFXComponents[i]->GetAsset() == VFXDef->NiagaraSystem.Get())
            {
                ActiveVFXComponents[i]->DestroyComponent();
                ActiveVFXComponents.RemoveAt(i);
                break;
            }
        }
    }
    
    // Spawn the VFX
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        VFXDef->NiagaraSystem.Get(),
        Location,
        Rotation,
        Scale,
        VFXDef->bAutoDestroy
    );
    
    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        
        // Apply LOD settings based on distance to player
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
        {
            float Distance = FVector::Dist(Location, PlayerPawn->GetActorLocation());
            float LODLevel = CalculateLODLevel(Location, PlayerPawn->GetActorLocation());
            ApplyLODSettings(VFXComponent, LODLevel);
        }
        
        // Set lifetime if specified
        if (VFXDef->DefaultLifetime > 0.0f)
        {
            VFXComponent->SetFloatParameter(TEXT("Lifetime"), VFXDef->DefaultLifetime);
        }
    }
    
    return VFXComponent;
}

void AVFXManager::SpawnVFXAtLocation(EVFXType Type, FVector Location)
{
    SpawnVFX(Type, Location);
}

void AVFXManager::SpawnVFXAttached(EVFXType Type, USceneComponent* AttachToComponent, FName SocketName)
{
    if (!AttachToComponent)
    {
        return;
    }
    
    FVFXDefinition* VFXDef = GetVFXDefinition(Type);
    if (!VFXDef || !VFXDef->NiagaraSystem.LoadSynchronous())
    {
        return;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        VFXDef->NiagaraSystem.Get(),
        AttachToComponent,
        SocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        VFXDef->bAutoDestroy
    );
    
    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
    }
}

void AVFXManager::CreateDinosaurFootprint(FVector Location, float DinosaurSize, bool bIsRunning)
{
    UNiagaraComponent* FootprintVFX = SpawnVFX(EVFXType::DinosaurFootprint, Location);
    if (FootprintVFX)
    {
        FootprintVFX->SetFloatParameter(TEXT("DinosaurSize"), DinosaurSize);
        FootprintVFX->SetFloatParameter(TEXT("Intensity"), bIsRunning ? 1.5f : 1.0f);
        FootprintVFX->SetVectorParameter(TEXT("ImpactLocation"), Location);
    }
}

void AVFXManager::CreateVegetationDisturbance(FVector Location, FVector Direction, float Intensity)
{
    UNiagaraComponent* VegetationVFX = SpawnVFX(EVFXType::BrokenVegetation, Location);
    if (VegetationVFX)
    {
        VegetationVFX->SetVectorParameter(TEXT("Direction"), Direction.GetSafeNormal());
        VegetationVFX->SetFloatParameter(TEXT("Intensity"), Intensity);
        VegetationVFX->SetVectorParameter(TEXT("DisturbanceLocation"), Location);
    }
}

void AVFXManager::CreateBloodEffect(FVector Location, FVector Direction, bool bIsLarge)
{
    UNiagaraComponent* BloodVFX = SpawnVFX(EVFXType::BloodSplatter, Location);
    if (BloodVFX)
    {
        BloodVFX->SetVectorParameter(TEXT("ImpactDirection"), Direction.GetSafeNormal());
        BloodVFX->SetFloatParameter(TEXT("Scale"), bIsLarge ? 2.0f : 1.0f);
        BloodVFX->SetFloatParameter(TEXT("Intensity"), bIsLarge ? 1.5f : 1.0f);
    }
}

void AVFXManager::CreateWaterSplash(FVector Location, float Intensity)
{
    UNiagaraComponent* WaterVFX = SpawnVFX(EVFXType::WaterSplash, Location);
    if (WaterVFX)
    {
        WaterVFX->SetFloatParameter(TEXT("Intensity"), Intensity);
        WaterVFX->SetVectorParameter(TEXT("SplashLocation"), Location);
    }
}

void AVFXManager::CreateRockImpact(FVector Location, FVector ImpactDirection)
{
    UNiagaraComponent* RockVFX = SpawnVFX(EVFXType::RockDebris, Location);
    if (RockVFX)
    {
        RockVFX->SetVectorParameter(TEXT("ImpactDirection"), ImpactDirection.GetSafeNormal());
        RockVFX->SetVectorParameter(TEXT("ImpactLocation"), Location);
    }
}

void AVFXManager::CreateCraftingEffect(FVector Location)
{
    UNiagaraComponent* CraftingVFX = SpawnVFX(EVFXType::CraftingSparkles, Location);
    if (CraftingVFX)
    {
        CraftingVFX->SetVectorParameter(TEXT("CraftingLocation"), Location);
    }
}

void AVFXManager::CreateToolImpact(FVector Location, FVector Direction)
{
    UNiagaraComponent* ToolVFX = SpawnVFX(EVFXType::ToolImpact, Location);
    if (ToolVFX)
    {
        ToolVFX->SetVectorParameter(TEXT("ImpactDirection"), Direction.GetSafeNormal());
        ToolVFX->SetVectorParameter(TEXT("ImpactLocation"), Location);
    }
}

void AVFXManager::CreateFireEffect(FVector Location, float Intensity)
{
    UNiagaraComponent* FireVFX = SpawnVFX(EVFXType::FireEmbers, Location);
    if (FireVFX)
    {
        FireVFX->SetFloatParameter(TEXT("Intensity"), Intensity);
        FireVFX->SetVectorParameter(TEXT("FireLocation"), Location);
    }
}

void AVFXManager::SetVFXQualityLevel(int32 QualityLevel)
{
    CurrentQualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    
    // Apply quality settings to all active VFX
    for (UNiagaraComponent* Component : ActiveVFXComponents)
    {
        if (Component)
        {
            Component->SetFloatParameter(TEXT("QualityLevel"), static_cast<float>(CurrentQualityLevel));
        }
    }
}

void AVFXManager::CullDistantVFX(FVector ViewerLocation)
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        if (!ActiveVFXComponents[i] || !IsValid(ActiveVFXComponents[i]))
        {
            ActiveVFXComponents.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(ActiveVFXComponents[i]->GetComponentLocation(), ViewerLocation);
        if (Distance > MaxVFXDistance)
        {
            ActiveVFXComponents[i]->DestroyComponent();
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

FVFXDefinition* AVFXManager::GetVFXDefinition(EVFXType Type)
{
    int32 TypeIndex = static_cast<int32>(Type);
    if (VFXDefinitions.IsValidIndex(TypeIndex))
    {
        return &VFXDefinitions[TypeIndex];
    }
    return nullptr;
}

void AVFXManager::CleanupFinishedVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        if (!ActiveVFXComponents[i] || !IsValid(ActiveVFXComponents[i]) || 
            !ActiveVFXComponents[i]->IsActive())
        {
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

float AVFXManager::CalculateLODLevel(FVector VFXLocation, FVector ViewerLocation)
{
    float Distance = FVector::Dist(VFXLocation, ViewerLocation);
    
    // Return LOD level based on distance (0 = highest quality, 3 = lowest quality)
    if (Distance < 1000.0f)
        return 0.0f; // High quality
    else if (Distance < 2500.0f)
        return 1.0f; // Medium quality
    else if (Distance < 5000.0f)
        return 2.0f; // Low quality
    else
        return 3.0f; // Very low quality or cull
}

void AVFXManager::ApplyLODSettings(UNiagaraComponent* VFXComponent, float LODLevel)
{
    if (!VFXComponent)
        return;
        
    VFXComponent->SetFloatParameter(TEXT("LODLevel"), LODLevel);
    
    // Adjust particle count based on LOD
    float ParticleMultiplier = 1.0f;
    switch (static_cast<int32>(LODLevel))
    {
        case 0: ParticleMultiplier = 1.0f; break;   // High
        case 1: ParticleMultiplier = 0.7f; break;  // Medium
        case 2: ParticleMultiplier = 0.4f; break;  // Low
        case 3: ParticleMultiplier = 0.1f; break;  // Very Low
    }
    
    VFXComponent->SetFloatParameter(TEXT("ParticleMultiplier"), ParticleMultiplier);
}