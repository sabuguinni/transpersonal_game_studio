#include "Arch_StructuralComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"

UArch_StructuralComponent::UArch_StructuralComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance
    
    // Set default collision settings for architectural elements
    SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    
    // Initialize default structure properties
    StructureProperties = FArch_StructureProperties();
    StructuralIntegrity = 100.0f;
    bCanCollapse = false;
    bIsClimbable = false;
    bProvidesShade = true;
    LastWeatheringUpdate = 0.0f;
}

void UArch_StructuralComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial material and collision settings
    UpdateMaterialProperties();
    UpdateCollisionSettings();
    ApplyAgeEffects();
    
    UE_LOG(LogTemp, Log, TEXT("Arch_StructuralComponent initialized: Type=%d, Material=%d, Integrity=%.1f"), 
           (int32)StructureProperties.StructureType, 
           (int32)StructureProperties.MaterialType, 
           StructuralIntegrity);
}

void UArch_StructuralComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Gradual weathering over time
    if (GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastWeatheringUpdate > 60.0f) // Update every minute
        {
            ApplyWeathering(0.01f); // Very slow weathering
            LastWeatheringUpdate = CurrentTime;
        }
    }
    
    // Check structural stability
    if (StructuralIntegrity <= 0.0f && bCanCollapse)
    {
        // TODO: Implement collapse mechanics
        UE_LOG(LogTemp, Warning, TEXT("Structure %s has collapsed due to zero integrity"), 
               *GetOwner()->GetName());
    }
}

void UArch_StructuralComponent::ApplyWeathering(float WeatheringAmount)
{
    if (WeatheringAmount <= 0.0f) return;
    
    // Increase weathering level
    StructureProperties.WeatheringLevel = FMath::Clamp(
        StructureProperties.WeatheringLevel + WeatheringAmount, 
        0.0f, 1.0f
    );
    
    // Reduce structural integrity based on weathering
    float IntegrityLoss = WeatheringAmount * 10.0f; // Weathering affects integrity
    StructuralIntegrity = FMath::Clamp(StructuralIntegrity - IntegrityLoss, 0.0f, 100.0f);
    
    // Update material properties to reflect weathering
    UpdateMaterialProperties();
    
    UE_LOG(LogTemp, Log, TEXT("Applied weathering %.3f to %s. New weathering level: %.3f, Integrity: %.1f"), 
           WeatheringAmount, *GetOwner()->GetName(), 
           StructureProperties.WeatheringLevel, StructuralIntegrity);
}

void UArch_StructuralComponent::SetStructureType(EArch_StructureType NewType)
{
    StructureProperties.StructureType = NewType;
    UpdateCollisionSettings();
    UpdateMaterialProperties();
    
    UE_LOG(LogTemp, Log, TEXT("Structure type changed to %d for %s"), 
           (int32)NewType, *GetOwner()->GetName());
}

void UArch_StructuralComponent::SetMaterialType(EArch_MaterialType NewMaterial)
{
    StructureProperties.MaterialType = NewMaterial;
    UpdateMaterialProperties();
    
    // Different materials have different base integrity
    switch (NewMaterial)
    {
        case EArch_MaterialType::Granite:
            StructuralIntegrity = FMath::Min(StructuralIntegrity, 100.0f);
            break;
        case EArch_MaterialType::Basalt:
            StructuralIntegrity = FMath::Min(StructuralIntegrity, 90.0f);
            break;
        case EArch_MaterialType::Limestone:
            StructuralIntegrity = FMath::Min(StructuralIntegrity, 80.0f);
            break;
        case EArch_MaterialType::Sandstone:
            StructuralIntegrity = FMath::Min(StructuralIntegrity, 70.0f);
            break;
        case EArch_MaterialType::Weathered:
            StructuralIntegrity = FMath::Min(StructuralIntegrity, 50.0f);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Material type changed to %d for %s. Integrity adjusted to %.1f"), 
           (int32)NewMaterial, *GetOwner()->GetName(), StructuralIntegrity);
}

void UArch_StructuralComponent::UpdateMaterialProperties()
{
    // TODO: Apply material instance parameters based on structure properties
    // This would set material parameters for weathering, moss, vines, etc.
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Updated material properties for %s"), 
           *GetOwner()->GetName());
}

void UArch_StructuralComponent::UpdateCollisionSettings()
{
    // Adjust collision based on structure type
    switch (StructureProperties.StructureType)
    {
        case EArch_StructureType::Pillar:
        case EArch_StructureType::Wall:
            SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
            bIsClimbable = true;
            break;
            
        case EArch_StructureType::Platform:
            SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
            bIsClimbable = false;
            break;
            
        case EArch_StructureType::Archway:
            // Archways should allow passage underneath
            SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
            bIsClimbable = true;
            break;
            
        case EArch_StructureType::Ruins:
            SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
            bIsClimbable = true;
            bCanCollapse = true;
            break;
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Updated collision settings for %s"), 
           *GetOwner()->GetName());
}

void UArch_StructuralComponent::ApplyAgeEffects()
{
    // Apply age-based effects to the structure
    float AgeYears = StructureProperties.AgeInYears;
    
    if (AgeYears > 500.0f)
    {
        StructureProperties.bHasMoss = true;
        StructureProperties.WeatheringLevel = FMath::Min(StructureProperties.WeatheringLevel + 0.2f, 1.0f);
    }
    
    if (AgeYears > 1000.0f)
    {
        StructureProperties.bHasVines = true;
        StructureProperties.WeatheringLevel = FMath::Min(StructureProperties.WeatheringLevel + 0.3f, 1.0f);
        bCanCollapse = true;
    }
    
    if (AgeYears > 2000.0f)
    {
        StructuralIntegrity = FMath::Min(StructuralIntegrity, 60.0f);
        StructureProperties.WeatheringLevel = FMath::Min(StructureProperties.WeatheringLevel + 0.4f, 1.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied age effects for %s (%.0f years): Moss=%s, Vines=%s, Weathering=%.3f"), 
           *GetOwner()->GetName(), AgeYears,
           StructureProperties.bHasMoss ? TEXT("Yes") : TEXT("No"),
           StructureProperties.bHasVines ? TEXT("Yes") : TEXT("No"),
           StructureProperties.WeatheringLevel);
}