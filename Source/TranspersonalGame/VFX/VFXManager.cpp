#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UVFXManager::UVFXManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for performance
}

void UVFXManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize VFX system
    UpdateLODDistances();
}

void UVFXManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Cleanup inactive VFX components
    CleanupInactiveVFX();
}

UNiagaraComponent* UVFXManager::PlayVFX(FName EffectName, FVector Location, FRotator Rotation, AActor* AttachActor)
{
    FVFXDefinition* VFXDef = GetVFXDefinition(EffectName);
    if (!VFXDef || !VFXDef->NiagaraSystem.LoadSynchronous())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX not found or failed to load: %s"), *EffectName.ToString());
        return nullptr;
    }

    // Check if we should cull this VFX for performance
    if (bEnableVFXCulling && ShouldCullVFX(*VFXDef, Location))
    {
        return nullptr;
    }

    // Check max instances limit
    int32 CurrentInstances = 0;
    for (UNiagaraComponent* Comp : ActiveVFXComponents)
    {
        if (Comp && Comp->GetAsset() == VFXDef->NiagaraSystem.Get())
        {
            CurrentInstances++;
        }
    }

    if (CurrentInstances >= VFXDef->MaxInstances)
    {
        // Remove oldest instance of this effect
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

    // Create and configure VFX component
    UNiagaraComponent* VFXComponent = nullptr;
    
    if (AttachActor && VFXDef->bAttachToActor)
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            VFXDef->NiagaraSystem.Get(),
            AttachActor->GetRootComponent(),
            VFXDef->AttachSocket,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    else
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            VFXDef->NiagaraSystem.Get(),
            Location,
            Rotation,
            FVector::OneVector,
            true
        );
    }

    if (VFXComponent)
    {
        // Apply global intensity multiplier
        VFXComponent->SetFloatParameter(TEXT("IntensityMultiplier"), GlobalVFXIntensityMultiplier);
        
        // Set duration for non-looping effects
        if (!VFXDef->bLooping && VFXDef->BaseDuration > 0.0f)
        {
            VFXComponent->SetFloatParameter(TEXT("Duration"), VFXDef->BaseDuration);
        }

        ActiveVFXComponents.Add(VFXComponent);
    }

    return VFXComponent;
}

void UVFXManager::StopVFX(UNiagaraComponent* VFXComponent)
{
    if (VFXComponent)
    {
        VFXComponent->Deactivate();
        ActiveVFXComponents.Remove(VFXComponent);
    }
}

void UVFXManager::StopAllVFXByCategory(EVFXCategory Category)
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Comp = ActiveVFXComponents[i];
        if (Comp)
        {
            // Find the VFX definition to check category
            for (const FVFXDefinition& VFXDef : VFXDefinitions)
            {
                if (VFXDef.NiagaraSystem.Get() == Comp->GetAsset() && VFXDef.Category == Category)
                {
                    Comp->Deactivate();
                    ActiveVFXComponents.RemoveAt(i);
                    break;
                }
            }
        }
    }
}

void UVFXManager::PlayDinosaurBreathingEffect(AActor* DinosaurActor, float IntensityMultiplier)
{
    if (!DinosaurActor) return;

    UNiagaraComponent* BreathVFX = PlayVFX(TEXT("DinosaurBreath"), DinosaurActor->GetActorLocation(), FRotator::ZeroRotator, DinosaurActor);
    if (BreathVFX)
    {
        BreathVFX->SetFloatParameter(TEXT("BreathIntensity"), IntensityMultiplier);
        BreathVFX->SetVectorParameter(TEXT("BreathColor"), FVector(1.0f, 1.0f, 1.0f));
    }
}

void UVFXManager::PlayDinosaurFootstepEffect(FVector Location, float DinosaurSize)
{
    UNiagaraComponent* FootstepVFX = PlayVFX(TEXT("DinosaurFootstep"), Location);
    if (FootstepVFX)
    {
        FootstepVFX->SetFloatParameter(TEXT("DinosaurSize"), DinosaurSize);
        FootstepVFX->SetFloatParameter(TEXT("DustAmount"), DinosaurSize * 0.5f);
    }
}

void UVFXManager::PlayDomesticationProgressEffect(AActor* DinosaurActor, float TrustLevel)
{
    if (!DinosaurActor) return;

    UNiagaraComponent* TrustVFX = PlayVFX(TEXT("DomesticationProgress"), DinosaurActor->GetActorLocation(), FRotator::ZeroRotator, DinosaurActor);
    if (TrustVFX)
    {
        // Color shifts from red (0 trust) to green (1 trust)
        FVector TrustColor = FVector(1.0f - TrustLevel, TrustLevel, 0.2f);
        VFX->SetVectorParameter(TEXT("TrustColor"), TrustColor);
        VFX->SetFloatParameter(TEXT("TrustLevel"), TrustLevel);
        VFX->SetFloatParameter(TEXT("ParticleCount"), TrustLevel * 20.0f);
    }
}

void UVFXManager::PlayAmbientForestEffect(FVector Location, float Radius)
{
    UNiagaraComponent* ForestVFX = PlayVFX(TEXT("AmbientForest"), Location);
    if (ForestVFX)
    {
        ForestVFX->SetFloatParameter(TEXT("EffectRadius"), Radius);
        ForestVFX->SetFloatParameter(TEXT("ParticleSpread"), Radius * 0.8f);
    }
}

void UVFXManager::PlayDangerIndicatorEffect(FVector Location, float ThreatLevel)
{
    UNiagaraComponent* DangerVFX = PlayVFX(TEXT("DangerIndicator"), Location);
    if (DangerVFX)
    {
        // Red intensity based on threat level
        FVector DangerColor = FVector(ThreatLevel, 0.1f, 0.1f);
        DangerVFX->SetVectorParameter(TEXT("DangerColor"), DangerColor);
        DangerVFX->SetFloatParameter(TEXT("ThreatLevel"), ThreatLevel);
        DangerVFX->SetFloatParameter(TEXT("PulseSpeed"), ThreatLevel * 2.0f);
    }
}

void UVFXManager::PlayWeatherEffect(FName WeatherType, float Intensity)
{
    UNiagaraComponent* WeatherVFX = PlayVFX(WeatherType, GetOwner()->GetActorLocation());
    if (WeatherVFX)
    {
        WeatherVFX->SetFloatParameter(TEXT("WeatherIntensity"), Intensity);
        WeatherVFX->SetFloatParameter(TEXT("CoverageArea"), 10000.0f); // Large area coverage
    }
}

void UVFXManager::PlayCraftingEffect(FVector Location, FName ItemType)
{
    UNiagaraComponent* CraftVFX = PlayVFX(TEXT("CraftingEffect"), Location);
    if (CraftVFX)
    {
        // Different colors for different item types
        FVector ItemColor = FVector(0.8f, 0.6f, 0.2f); // Default golden
        if (ItemType == TEXT("Weapon"))
        {
            ItemColor = FVector(0.9f, 0.3f, 0.1f); // Red for weapons
        }
        else if (ItemType == TEXT("Tool"))
        {
            ItemColor = FVector(0.3f, 0.7f, 0.9f); // Blue for tools
        }
        
        CraftVFX->SetVectorParameter(TEXT("ItemColor"), ItemColor);
    }
}

void UVFXManager::PlayGatheringEffect(FVector Location, FName ResourceType)
{
    UNiagaraComponent* GatherVFX = PlayVFX(TEXT("GatheringEffect"), Location);
    if (GatherVFX)
    {
        // Resource-specific colors
        FVector ResourceColor = FVector(0.5f, 0.8f, 0.3f); // Default green
        if (ResourceType == TEXT("Stone"))
        {
            ResourceColor = FVector(0.6f, 0.6f, 0.6f); // Gray for stone
        }
        else if (ResourceType == TEXT("Wood"))
        {
            ResourceColor = FVector(0.8f, 0.5f, 0.2f); // Brown for wood
        }
        
        GatherVFX->SetVectorParameter(TEXT("ResourceColor"), ResourceColor);
    }
}

void UVFXManager::SetVFXQualityLevel(int32 QualityLevel)
{
    // Adjust global settings based on quality level (0-3)
    switch (QualityLevel)
    {
        case 0: // Low
            GlobalVFXIntensityMultiplier = 0.5f;
            MaxActiveVFX = 20;
            break;
        case 1: // Medium
            GlobalVFXIntensityMultiplier = 0.75f;
            MaxActiveVFX = 35;
            break;
        case 2: // High
            GlobalVFXIntensityMultiplier = 1.0f;
            MaxActiveVFX = 50;
            break;
        case 3: // Ultra
            GlobalVFXIntensityMultiplier = 1.25f;
            MaxActiveVFX = 75;
            break;
    }
    
    UpdateLODDistances();
}

void UVFXManager::UpdateLODDistances()
{
    for (FVFXDefinition& VFXDef : VFXDefinitions)
    {
        // Adjust cull distances based on quality and effect intensity
        float BaseDistance = VFXDef.CullDistance;
        float QualityMultiplier = GlobalVFXIntensityMultiplier;
        
        switch (VFXDef.Intensity)
        {
            case EVFXIntensity::Subtle:
                VFXDef.CullDistance = BaseDistance * 0.5f * QualityMultiplier;
                break;
            case EVFXIntensity::Medium:
                VFXDef.CullDistance = BaseDistance * 0.75f * QualityMultiplier;
                break;
            case EVFXIntensity::High:
                VFXDef.CullDistance = BaseDistance * QualityMultiplier;
                break;
            case EVFXIntensity::Critical:
                VFXDef.CullDistance = BaseDistance * 1.5f * QualityMultiplier;
                break;
        }
    }
}

void UVFXManager::CleanupInactiveVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Comp = ActiveVFXComponents[i];
        if (!Comp || !IsValid(Comp) || !Comp->IsActive())
        {
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

FVFXDefinition* UVFXManager::GetVFXDefinition(FName EffectName)
{
    for (FVFXDefinition& VFXDef : VFXDefinitions)
    {
        if (VFXDef.EffectName == EffectName)
        {
            return &VFXDef;
        }
    }
    return nullptr;
}

bool UVFXManager::ShouldCullVFX(const FVFXDefinition& VFXDef, FVector Location)
{
    if (!bEnableVFXCulling) return false;
    
    // Get player location for distance check
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player) return false;
    
    float Distance = FVector::Dist(Player->GetActorLocation(), Location);
    return Distance > VFXDef.CullDistance;
}