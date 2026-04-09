#include "NiagaraVFXIntegration.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogNiagaraVFXIntegration);

UNiagaraVFXIntegration::UNiagaraVFXIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize VFX quality settings
    VFXQualityLevel = EVFXQualityLevel::High;
    MaxParticlesPerSystem = 5000;
    LODDistanceScale = 1.0f;
    
    // Initialize prehistoric VFX categories
    InitializePrehistoricVFXSystems();
}

void UNiagaraVFXIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogNiagaraVFXIntegration, Log, TEXT("Niagara VFX Integration initialized for prehistoric environment"));
    
    // Load essential VFX systems
    LoadEssentialVFXSystems();
    
    // Configure performance settings based on platform
    ConfigurePerformanceSettings();
}

void UNiagaraVFXIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active VFX systems
    UpdateActiveVFXSystems(DeltaTime);
    
    // Manage VFX LOD based on distance and performance
    ManageVFXLOD();
}

void UNiagaraVFXIntegration::InitializePrehistoricVFXSystems()
{
    // Atmospheric VFX systems for prehistoric environment
    PrehistoricVFXSystems.Add(TEXT("MorningMist"), FPrehistoricVFXData{
        TEXT("/Game/VFX/Atmosphere/NS_PrehistoricMist"),
        EVFXCategory::Atmospheric,
        50.0f,      // Spawn rate
        8.0f,       // Lifetime
        FVector(0, 0, 20),  // Initial velocity
        true        // Affected by wind
    });
    
    PrehistoricVFXSystems.Add(TEXT("VolcanicAsh"), FPrehistoricVFXData{
        TEXT("/Game/VFX/Environment/NS_VolcanicAsh"),
        EVFXCategory::Environmental,
        200.0f,
        12.0f,
        FVector(10, 5, -30),
        true
    });
    
    PrehistoricVFXSystems.Add(TEXT("AncientPollen"), FPrehistoricVFXData{
        TEXT("/Game/VFX/Flora/NS_AncientPollen"),
        EVFXCategory::Flora,
        30.0f,
        15.0f,
        FVector(5, 0, 10),
        true
    });
    
    // Dinosaur interaction VFX
    PrehistoricVFXSystems.Add(TEXT("DinosaurFootstep"), FPrehistoricVFXData{
        TEXT("/Game/VFX/Creatures/NS_DinosaurFootsteps"),
        EVFXCategory::Creature,
        100.0f,
        3.0f,
        FVector(0, 0, 50),
        false
    });
    
    PrehistoricVFXSystems.Add(TEXT("TailSwipeDistortion"), FPrehistoricVFXData{
        TEXT("/Game/VFX/Creatures/NS_TailSwipeDistortion"),
        EVFXCategory::Creature,
        75.0f,
        2.0f,
        FVector(20, 0, 0),
        false
    });
    
    // Mystical/temporal VFX for narrative moments
    PrehistoricVFXSystems.Add(TEXT("GemResonance"), FPrehistoricVFXData{
        TEXT("/Game/VFX/Mystical/NS_GemResonance"),
        EVFXCategory::Mystical,
        25.0f,
        10.0f,
        FVector(0, 0, 0),
        false
    });
    
    PrehistoricVFXSystems.Add(TEXT("TemporalPortal"), FPrehistoricVFXData{
        TEXT("/Game/VFX/Mystical/NS_TemporalPortal"),
        EVFXCategory::Mystical,
        150.0f,
        20.0f,
        FVector(0, 0, 100),
        false
    });
}

void UNiagaraVFXIntegration::LoadEssentialVFXSystems()
{
    for (auto& VFXPair : PrehistoricVFXSystems)
    {
        const FString& VFXName = VFXPair.Key;
        const FPrehistoricVFXData& VFXData = VFXPair.Value;
        
        // Load Niagara system asset
        UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *VFXData.SystemPath);
        if (NiagaraSystem)
        {
            LoadedVFXSystems.Add(VFXName, NiagaraSystem);
            UE_LOG(LogNiagaraVFXIntegration, Log, TEXT("Loaded VFX system: %s"), *VFXName);
        }
        else
        {
            UE_LOG(LogNiagaraVFXIntegration, Warning, TEXT("Failed to load VFX system: %s at path %s"), *VFXName, *VFXData.SystemPath);
        }
    }
}

UNiagaraComponent* UNiagaraVFXIntegration::SpawnPrehistoricVFX(const FString& VFXName, const FVector& Location, const FRotator& Rotation, AActor* AttachToActor)
{
    if (!LoadedVFXSystems.Contains(VFXName))
    {
        UE_LOG(LogNiagaraVFXIntegration, Warning, TEXT("VFX system not found: %s"), *VFXName);
        return nullptr;
    }
    
    UNiagaraSystem* VFXSystem = LoadedVFXSystems[VFXName];
    if (!VFXSystem)
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = nullptr;
    
    if (AttachToActor)
    {
        // Attach VFX to actor
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            VFXSystem,
            AttachToActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepWorldPosition,
            true
        );
    }
    else
    {
        // Spawn VFX at world location
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            VFXSystem,
            Location,
            Rotation,
            FVector::OneVector,
            true
        );
    }
    
    if (VFXComponent)
    {
        // Configure VFX parameters based on prehistoric data
        ConfigureVFXParameters(VFXComponent, VFXName);
        
        // Track active VFX for management
        ActiveVFXComponents.Add(VFXComponent);
        
        UE_LOG(LogNiagaraVFXIntegration, Log, TEXT("Spawned prehistoric VFX: %s at location %s"), *VFXName, *Location.ToString());
    }
    
    return VFXComponent;
}

void UNiagaraVFXIntegration::ConfigureVFXParameters(UNiagaraComponent* VFXComponent, const FString& VFXName)
{
    if (!VFXComponent || !PrehistoricVFXSystems.Contains(VFXName))
    {
        return;
    }
    
    const FPrehistoricVFXData& VFXData = PrehistoricVFXSystems[VFXName];
    
    // Set spawn rate
    VFXComponent->SetFloatParameter(TEXT("SpawnRate"), VFXData.SpawnRate);
    
    // Set particle lifetime
    VFXComponent->SetFloatParameter(TEXT("ParticleLifetime"), VFXData.Lifetime);
    
    // Set initial velocity
    VFXComponent->SetVectorParameter(TEXT("InitialVelocity"), VFXData.InitialVelocity);
    
    // Configure category-specific parameters
    switch (VFXData.Category)
    {
        case EVFXCategory::Atmospheric:
            ConfigureAtmosphericVFX(VFXComponent, VFXName);
            break;
        case EVFXCategory::Environmental:
            ConfigureEnvironmentalVFX(VFXComponent, VFXName);
            break;
        case EVFXCategory::Flora:
            ConfigureFloraVFX(VFXComponent, VFXName);
            break;
        case EVFXCategory::Creature:
            ConfigureCreatureVFX(VFXComponent, VFXName);
            break;
        case EVFXCategory::Mystical:
            ConfigureMysticalVFX(VFXComponent, VFXName);
            break;
    }
}

void UNiagaraVFXIntegration::ConfigureAtmosphericVFX(UNiagaraComponent* VFXComponent, const FString& VFXName)
{
    if (VFXName == TEXT("MorningMist"))
    {
        VFXComponent->SetFloatParameter(TEXT("Opacity"), 0.3f);
        VFXComponent->SetFloatParameter(TEXT("SizeMultiplier"), 2.0f);
        VFXComponent->SetColorParameter(TEXT("TintColor"), FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
    }
    else if (VFXName == TEXT("VolcanicAsh"))
    {
        VFXComponent->SetFloatParameter(TEXT("WindInfluence"), 0.8f);
        VFXComponent->SetFloatParameter(TEXT("Turbulence"), 0.5f);
        VFXComponent->SetColorParameter(TEXT("TintColor"), FLinearColor(0.4f, 0.3f, 0.2f, 1.0f));
    }
}

void UNiagaraVFXIntegration::ConfigureEnvironmentalVFX(UNiagaraComponent* VFXComponent, const FString& VFXName)
{
    // Configure environmental VFX parameters
    VFXComponent->SetFloatParameter(TEXT("EnvironmentalScale"), 1.5f);
    VFXComponent->SetBoolParameter(TEXT("UseEnvironmentalLighting"), true);
}

void UNiagaraVFXIntegration::ConfigureFloraVFX(UNiagaraComponent* VFXComponent, const FString& VFXName)
{
    if (VFXName == TEXT("AncientPollen"))
    {
        VFXComponent->SetFloatParameter(TEXT("FloatBehavior"), 1.0f);
        VFXComponent->SetFloatParameter(TEXT("SizeVariation"), 0.3f);
        VFXComponent->SetColorParameter(TEXT("PollenColor"), FLinearColor(1.0f, 0.8f, 0.2f, 0.6f));
    }
}

void UNiagaraVFXIntegration::ConfigureCreatureVFX(UNiagaraComponent* VFXComponent, const FString& VFXName)
{
    if (VFXName == TEXT("DinosaurFootstep"))
    {
        VFXComponent->SetFloatParameter(TEXT("ImpactForce"), 100.0f);
        VFXComponent->SetFloatParameter(TEXT("DustAmount"), 1.0f);
    }
    else if (VFXName == TEXT("TailSwipeDistortion"))
    {
        VFXComponent->SetFloatParameter(TEXT("DistortionStrength"), 0.5f);
        VFXComponent->SetFloatParameter(TEXT("AirDisplacement"), 1.2f);
    }
}

void UNiagaraVFXIntegration::ConfigureMysticalVFX(UNiagaraComponent* VFXComponent, const FString& VFXName)
{
    if (VFXName == TEXT("GemResonance"))
    {
        VFXComponent->SetFloatParameter(TEXT("EnergyIntensity"), 0.8f);
        VFXComponent->SetColorParameter(TEXT("ResonanceColor"), FLinearColor(0.2f, 0.8f, 1.0f, 1.0f));
        VFXComponent->SetFloatParameter(TEXT("PulseFrequency"), 2.0f);
    }
    else if (VFXName == TEXT("TemporalPortal"))
    {
        VFXComponent->SetFloatParameter(TEXT("PortalStability"), 0.7f);
        VFXComponent->SetFloatParameter(TEXT("TimeDistortion"), 1.5f);
        VFXComponent->SetColorParameter(TEXT("PortalColor"), FLinearColor(1.0f, 0.3f, 0.8f, 1.0f));
    }
}

void UNiagaraVFXIntegration::UpdateActiveVFXSystems(float DeltaTime)
{
    // Clean up finished VFX components
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* VFXComponent = ActiveVFXComponents[i];
        if (!IsValid(VFXComponent) || !VFXComponent->IsActive())
        {
            ActiveVFXComponents.RemoveAt(i);
        }
    }
    
    // Update VFX based on environmental conditions
    UpdateEnvironmentalInfluences(DeltaTime);
}

void UNiagaraVFXIntegration::UpdateEnvironmentalInfluences(float DeltaTime)
{
    // Update wind influence on atmospheric VFX
    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (IsValid(VFXComponent))
        {
            // Apply wind effects
            FVector WindDirection = GetCurrentWindDirection();
            float WindStrength = GetCurrentWindStrength();
            
            VFXComponent->SetVectorParameter(TEXT("WindDirection"), WindDirection);
            VFXComponent->SetFloatParameter(TEXT("WindStrength"), WindStrength);
        }
    }
}

void UNiagaraVFXIntegration::ManageVFXLOD()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
    
    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (!IsValid(VFXComponent))
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, VFXComponent->GetComponentLocation());
        
        // Adjust VFX quality based on distance
        if (Distance < 500.0f)
        {
            // Close range - full quality
            VFXComponent->SetFloatParameter(TEXT("LODLevel"), 0.0f);
            VFXComponent->SetFloatParameter(TEXT("ParticleCount"), 1.0f);
        }
        else if (Distance < 1500.0f)
        {
            // Medium range - reduced quality
            VFXComponent->SetFloatParameter(TEXT("LODLevel"), 1.0f);
            VFXComponent->SetFloatParameter(TEXT("ParticleCount"), 0.6f);
        }
        else
        {
            // Far range - minimal quality
            VFXComponent->SetFloatParameter(TEXT("LODLevel"), 2.0f);
            VFXComponent->SetFloatParameter(TEXT("ParticleCount"), 0.3f);
        }
    }
}

void UNiagaraVFXIntegration::ConfigurePerformanceSettings()
{
    // Adjust settings based on platform capabilities
    switch (VFXQualityLevel)
    {
        case EVFXQualityLevel::Low:
            MaxParticlesPerSystem = 1000;
            LODDistanceScale = 0.5f;
            break;
        case EVFXQualityLevel::Medium:
            MaxParticlesPerSystem = 3000;
            LODDistanceScale = 0.75f;
            break;
        case EVFXQualityLevel::High:
            MaxParticlesPerSystem = 5000;
            LODDistanceScale = 1.0f;
            break;
        case EVFXQualityLevel::Ultra:
            MaxParticlesPerSystem = 8000;
            LODDistanceScale = 1.25f;
            break;
    }
    
    UE_LOG(LogNiagaraVFXIntegration, Log, TEXT("VFX performance configured: Quality=%d, MaxParticles=%d, LODScale=%.2f"), 
           (int32)VFXQualityLevel, MaxParticlesPerSystem, LODDistanceScale);
}

FVector UNiagaraVFXIntegration::GetCurrentWindDirection() const
{
    // Get wind direction from environment system
    // For now, return a default prehistoric wind pattern
    return FVector(1.0f, 0.3f, 0.1f).GetSafeNormal();
}

float UNiagaraVFXIntegration::GetCurrentWindStrength() const
{
    // Get wind strength from environment system
    // For now, return a default value with some variation
    return 0.5f + 0.3f * FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f);
}

void UNiagaraVFXIntegration::StopAllVFX()
{
    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (IsValid(VFXComponent))
        {
            VFXComponent->DeactivateImmediate();
        }
    }
    
    ActiveVFXComponents.Empty();
    UE_LOG(LogNiagaraVFXIntegration, Log, TEXT("All VFX systems stopped"));
}

void UNiagaraVFXIntegration::SetVFXQuality(EVFXQualityLevel NewQualityLevel)
{
    VFXQualityLevel = NewQualityLevel;
    ConfigurePerformanceSettings();
    
    UE_LOG(LogNiagaraVFXIntegration, Log, TEXT("VFX quality level changed to: %d"), (int32)NewQualityLevel);
}