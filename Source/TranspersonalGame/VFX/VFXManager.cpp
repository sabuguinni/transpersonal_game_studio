#include "VFXManager.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Default performance settings
    MaxActiveVFXCount = 50;
    bUseLODSystem = true;
    CurrentQualityLevel = 2; // Medium quality by default
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize VFX system
    UE_LOG(LogTemp, Log, TEXT("VFX Manager initialized with quality level: %d"), CurrentQualityLevel);
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Clean up finished VFX every frame
    CleanupFinishedVFX();
}

UNiagaraComponent* AVFXManager::PlayVFXAtLocation(UNiagaraSystem* VFXSystem, const FVector& Location, const FRotator& Rotation, const FVFXSettings& Settings)
{
    if (!VFXSystem || !CanSpawnNewVFX())
    {
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        VFXSystem,
        Location,
        Rotation,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (VFXComponent)
    {
        // Apply settings
        if (Settings.Duration > 0.0f && !Settings.bLooping)
        {
            VFXComponent->SetAutoDestroy(true);
        }

        // Apply LOD if enabled
        if (bUseLODSystem)
        {
            float Distance = FVector::Dist(Location, GetActorLocation());
            ApplyLODSettings(VFXComponent, Distance);
        }

        ActiveVFXComponents.Add(VFXComponent);
        
        UE_LOG(LogTemp, Log, TEXT("VFX spawned at location: %s, Category: %d"), 
               *Location.ToString(), (int32)Settings.Category);
    }

    return VFXComponent;
}

UNiagaraComponent* AVFXManager::PlayVFXAttached(UNiagaraSystem* VFXSystem, USceneComponent* AttachComponent, const FName& AttachPointName, const FVector& Location, const FRotator& Rotation, const FVFXSettings& Settings)
{
    if (!VFXSystem || !AttachComponent || !CanSpawnNewVFX())
    {
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        VFXSystem,
        AttachComponent,
        AttachPointName,
        Location,
        Rotation,
        EAttachLocation::KeepRelativeOffset,
        true
    );

    if (VFXComponent)
    {
        // Apply settings
        if (Settings.Duration > 0.0f && !Settings.bLooping)
        {
            VFXComponent->SetAutoDestroy(true);
        }

        ActiveVFXComponents.Add(VFXComponent);
        
        UE_LOG(LogTemp, Log, TEXT("VFX attached to component: %s, Category: %d"), 
               *AttachComponent->GetName(), (int32)Settings.Category);
    }

    return VFXComponent;
}

void AVFXManager::SpawnAmbientFog(const FVector& Location, float Radius, float Density)
{
    if (!NS_AmbientFog) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Environmental;
    Settings.Intensity = EVFXIntensity::Subtle;
    Settings.bLooping = true;

    UNiagaraComponent* FogComponent = PlayVFXAtLocation(NS_AmbientFog, Location, FRotator::ZeroRotator, Settings);
    
    if (FogComponent)
    {
        FogComponent->SetFloatParameter(TEXT("Radius"), Radius);
        FogComponent->SetFloatParameter(TEXT("Density"), Density);
    }
}

void AVFXManager::SpawnDustParticles(const FVector& Location, const FVector& WindDirection, float Intensity)
{
    if (!NS_DustParticles) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Environmental;
    Settings.Intensity = EVFXIntensity::Subtle;
    Settings.Duration = 10.0f;

    UNiagaraComponent* DustComponent = PlayVFXAtLocation(NS_DustParticles, Location, FRotator::ZeroRotator, Settings);
    
    if (DustComponent)
    {
        DustComponent->SetVectorParameter(TEXT("WindDirection"), WindDirection);
        DustComponent->SetFloatParameter(TEXT("Intensity"), Intensity);
    }
}

void AVFXManager::SpawnLeafFall(const FVector& Location, float Radius)
{
    if (!NS_LeafFall) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Environmental;
    Settings.Intensity = EVFXIntensity::Subtle;
    Settings.bLooping = true;

    UNiagaraComponent* LeafComponent = PlayVFXAtLocation(NS_LeafFall, Location, FRotator::ZeroRotator, Settings);
    
    if (LeafComponent)
    {
        LeafComponent->SetFloatParameter(TEXT("Radius"), Radius);
    }
}

void AVFXManager::SpawnFootstepDust(const FVector& Location, float CreatureSize)
{
    if (!NS_FootstepDust) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Creature;
    Settings.Intensity = EVFXIntensity::Medium;
    Settings.Duration = 2.0f;

    UNiagaraComponent* FootstepComponent = PlayVFXAtLocation(NS_FootstepDust, Location, FRotator::ZeroRotator, Settings);
    
    if (FootstepComponent)
    {
        FootstepComponent->SetFloatParameter(TEXT("CreatureSize"), CreatureSize);
    }
}

void AVFXManager::SpawnBreathSteam(USceneComponent* AttachPoint, float Temperature)
{
    if (!NS_BreathSteam || !AttachPoint) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Creature;
    Settings.Intensity = EVFXIntensity::Subtle;
    Settings.bLooping = true;
    Settings.bAttachToActor = true;

    UNiagaraComponent* BreathComponent = PlayVFXAttached(NS_BreathSteam, AttachPoint, TEXT("mouth"), FVector::ZeroVector, FRotator::ZeroRotator, Settings);
    
    if (BreathComponent)
    {
        BreathComponent->SetFloatParameter(TEXT("Temperature"), Temperature);
    }
}

void AVFXManager::SpawnTerritoryMarker(const FVector& Location, AActor* OwnerCreature)
{
    if (!NS_TerritoryMarker || !OwnerCreature) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Creature;
    Settings.Intensity = EVFXIntensity::Medium;
    Settings.Duration = 30.0f; // Territory markers last longer

    UNiagaraComponent* MarkerComponent = PlayVFXAtLocation(NS_TerritoryMarker, Location, FRotator::ZeroRotator, Settings);
    
    if (MarkerComponent)
    {
        // Set creature-specific parameters
        MarkerComponent->SetObjectParameter(TEXT("OwnerCreature"), OwnerCreature);
    }
}

void AVFXManager::SpawnImpactEffect(const FVector& Location, const FVector& Normal, float ImpactForce)
{
    if (!NS_ImpactEffect) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Combat;
    Settings.Intensity = EVFXIntensity::High;
    Settings.Duration = 3.0f;

    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
    UNiagaraComponent* ImpactComponent = PlayVFXAtLocation(NS_ImpactEffect, Location, ImpactRotation, Settings);
    
    if (ImpactComponent)
    {
        ImpactComponent->SetFloatParameter(TEXT("ImpactForce"), ImpactForce);
        ImpactComponent->SetVectorParameter(TEXT("ImpactNormal"), Normal);
    }
}

void AVFXManager::SpawnBloodSpray(const FVector& Location, const FVector& Direction, float Amount)
{
    if (!NS_BloodSpray) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Combat;
    Settings.Intensity = EVFXIntensity::High;
    Settings.Duration = 5.0f;

    FRotator SprayRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    UNiagaraComponent* BloodComponent = PlayVFXAtLocation(NS_BloodSpray, Location, SprayRotation, Settings);
    
    if (BloodComponent)
    {
        BloodComponent->SetFloatParameter(TEXT("Amount"), Amount);
        BloodComponent->SetVectorParameter(TEXT("Direction"), Direction);
    }
}

void AVFXManager::SpawnClawScratch(const FVector& StartLocation, const FVector& EndLocation)
{
    if (!NS_ClawScratch) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Combat;
    Settings.Intensity = EVFXIntensity::Medium;
    Settings.Duration = 4.0f;

    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
    FRotator ScratchRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    
    UNiagaraComponent* ScratchComponent = PlayVFXAtLocation(NS_ClawScratch, StartLocation, ScratchRotation, Settings);
    
    if (ScratchComponent)
    {
        ScratchComponent->SetVectorParameter(TEXT("StartLocation"), StartLocation);
        ScratchComponent->SetVectorParameter(TEXT("EndLocation"), EndLocation);
    }
}

void AVFXManager::SpawnCampfire(const FVector& Location, float Size)
{
    if (!NS_Campfire) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Survival;
    Settings.Intensity = EVFXIntensity::Medium;
    Settings.bLooping = true;

    UNiagaraComponent* FireComponent = PlayVFXAtLocation(NS_Campfire, Location, FRotator::ZeroRotator, Settings);
    
    if (FireComponent)
    {
        FireComponent->SetFloatParameter(TEXT("Size"), Size);
    }
}

void AVFXManager::SpawnCraftingSparks(const FVector& Location, float Intensity)
{
    if (!NS_CraftingSparks) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Survival;
    Settings.Intensity = EVFXIntensity::Medium;
    Settings.Duration = 3.0f;

    UNiagaraComponent* SparksComponent = PlayVFXAtLocation(NS_CraftingSparks, Location, FRotator::ZeroRotator, Settings);
    
    if (SparksComponent)
    {
        SparksComponent->SetFloatParameter(TEXT("Intensity"), Intensity);
    }
}

void AVFXManager::SpawnCookingSmoke(const FVector& Location)
{
    if (!NS_CookingSmoke) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Survival;
    Settings.Intensity = EVFXIntensity::Subtle;
    Settings.bLooping = true;

    PlayVFXAtLocation(NS_CookingSmoke, Location, FRotator::ZeroRotator, Settings);
}

void AVFXManager::SpawnDomesticationProgress(AActor* Creature, float TrustLevel)
{
    if (!NS_DomesticationProgress || !Creature) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Interaction;
    Settings.Intensity = EVFXIntensity::Subtle;
    Settings.Duration = 5.0f;
    Settings.bAttachToActor = true;

    UNiagaraComponent* ProgressComponent = PlayVFXAttached(
        NS_DomesticationProgress, 
        Creature->GetRootComponent(), 
        NAME_None, 
        FVector(0, 0, 100), 
        FRotator::ZeroRotator, 
        Settings
    );
    
    if (ProgressComponent)
    {
        ProgressComponent->SetFloatParameter(TEXT("TrustLevel"), TrustLevel);
    }
}

void AVFXManager::SpawnFearIndicator(AActor* Creature, float FearLevel)
{
    if (!NS_FearIndicator || !Creature) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Interaction;
    Settings.Intensity = EVFXIntensity::Medium;
    Settings.Duration = 3.0f;
    Settings.bAttachToActor = true;

    UNiagaraComponent* FearComponent = PlayVFXAttached(
        NS_FearIndicator, 
        Creature->GetRootComponent(), 
        NAME_None, 
        FVector(0, 0, 150), 
        FRotator::ZeroRotator, 
        Settings
    );
    
    if (FearComponent)
    {
        FearComponent->SetFloatParameter(TEXT("FearLevel"), FearLevel);
    }
}

void AVFXManager::SpawnCalmingAura(const FVector& Location, float Radius)
{
    if (!NS_CalmingAura) return;

    FVFXSettings Settings;
    Settings.Category = EVFXCategory::Interaction;
    Settings.Intensity = EVFXIntensity::Subtle;
    Settings.Duration = 10.0f;

    UNiagaraComponent* AuraComponent = PlayVFXAtLocation(NS_CalmingAura, Location, FRotator::ZeroRotator, Settings);
    
    if (AuraComponent)
    {
        AuraComponent->SetFloatParameter(TEXT("Radius"), Radius);
    }
}

void AVFXManager::SetVFXQualityLevel(int32 QualityLevel)
{
    CurrentQualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    
    // Adjust max VFX count based on quality
    switch (CurrentQualityLevel)
    {
        case 0: // Low
            MaxActiveVFXCount = 20;
            break;
        case 1: // Medium
            MaxActiveVFXCount = 35;
            break;
        case 2: // High
            MaxActiveVFXCount = 50;
            break;
        case 3: // Ultra
            MaxActiveVFXCount = 75;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Quality set to level: %d, Max VFX: %d"), CurrentQualityLevel, MaxActiveVFXCount);
}

void AVFXManager::EnableLODSystem(bool bEnable)
{
    bUseLODSystem = bEnable;
    UE_LOG(LogTemp, Log, TEXT("VFX LOD System: %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
}

void AVFXManager::SetMaxActiveVFX(int32 MaxCount)
{
    MaxActiveVFXCount = FMath::Max(MaxCount, 10);
    UE_LOG(LogTemp, Log, TEXT("Max Active VFX set to: %d"), MaxActiveVFXCount);
}

void AVFXManager::CleanupFinishedVFX()
{
    ActiveVFXComponents.RemoveAll([](UNiagaraComponent* Component)
    {
        return !IsValid(Component) || !Component->IsActive();
    });
}

void AVFXManager::ApplyLODSettings(UNiagaraComponent* VFXComponent, float Distance)
{
    if (!VFXComponent) return;

    // Distance-based LOD scaling
    float LODScale = 1.0f;
    
    if (Distance > 5000.0f) // Far distance
    {
        LODScale = 0.3f;
    }
    else if (Distance > 2000.0f) // Medium distance
    {
        LODScale = 0.6f;
    }
    else if (Distance > 1000.0f) // Close distance
    {
        LODScale = 0.8f;
    }
    
    // Apply LOD scaling to particle count
    VFXComponent->SetFloatParameter(TEXT("LODScale"), LODScale);
}

bool AVFXManager::CanSpawnNewVFX() const
{
    return ActiveVFXComponents.Num() < MaxActiveVFXCount;
}