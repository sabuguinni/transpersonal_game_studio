#include "Arch_StructuralManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UArch_StructuralManager::UArch_StructuralManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    WeatheringRate = 0.001f; // Very slow degradation
    BaseIntegrity = 1.0f;
    bEnableWeathering = true;
}

void UArch_StructuralManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default structural elements if none exist
    if (StructuralElements.Num() == 0)
    {
        FArch_StructuralElement DefaultArch;
        DefaultArch.ElementName = TEXT("Ancient Stone Arch");
        DefaultArch.Location = FVector(0.0f, 0.0f, 0.0f);
        DefaultArch.Rotation = FRotator(0.0f, 0.0f, 0.0f);
        DefaultArch.Scale = FVector(2.0f, 1.0f, 3.0f);
        DefaultArch.IntegrityLevel = 0.8f; // Partially weathered
        DefaultArch.AssociatedBiome = EBiomeType::Temperate;
        
        StructuralElements.Add(DefaultArch);
    }
    
    // Initialize default interior space
    if (InteriorSpaces.Num() == 0)
    {
        FArch_InteriorSpace DefaultChamber;
        DefaultChamber.SpaceName = TEXT("Ancient Chamber");
        DefaultChamber.Dimensions = FVector(800.0f, 600.0f, 400.0f);
        DefaultChamber.ContainedObjects.Add(TEXT("Stone Fragments"));
        DefaultChamber.ContainedObjects.Add(TEXT("Moss Growth"));
        DefaultChamber.AmbientLightLevel = 0.2f;
        DefaultChamber.bHasRoof = false; // Partially collapsed
        DefaultChamber.StructuralSoundness = 0.6f;
        
        InteriorSpaces.Add(DefaultChamber);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_StructuralManager initialized with %d elements and %d spaces"), 
           StructuralElements.Num(), InteriorSpaces.Num());
}

void UArch_StructuralManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableWeathering)
    {
        ApplyWeathering(DeltaTime);
    }
    
    CheckStructuralStability();
}

void UArch_StructuralManager::SpawnStructuralElement(const FArch_StructuralElement& Element)
{
    StructuralElements.Add(Element);
    
    // Try to spawn physical representation in world
    UWorld* World = GetWorld();
    if (World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*Element.ElementName);
        
        AStaticMeshActor* SpawnedActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Element.Location,
            Element.Rotation,
            SpawnParams
        );
        
        if (SpawnedActor)
        {
            SpawnedActor->SetActorScale3D(Element.Scale);
            SpawnedActor->SetActorLabel(Element.ElementName);
            
            UE_LOG(LogTemp, Warning, TEXT("Spawned structural element: %s"), *Element.ElementName);
        }
    }
}

void UArch_StructuralManager::CreateInteriorSpace(const FArch_InteriorSpace& Space)
{
    InteriorSpaces.Add(Space);
    
    UE_LOG(LogTemp, Warning, TEXT("Created interior space: %s with dimensions %s"), 
           *Space.SpaceName, *Space.Dimensions.ToString());
}

void UArch_StructuralManager::ApplyWeathering(float DeltaTime)
{
    for (FArch_StructuralElement& Element : StructuralElements)
    {
        UpdateElementIntegrity(Element, DeltaTime);
    }
    
    for (FArch_InteriorSpace& Space : InteriorSpaces)
    {
        // Apply weathering to interior spaces
        float WeatheringDamage = WeatheringRate * DeltaTime;
        if (!Space.bHasRoof)
        {
            WeatheringDamage *= 2.0f; // Exposed spaces weather faster
        }
        
        Space.StructuralSoundness = FMath::Max(0.0f, Space.StructuralSoundness - WeatheringDamage);
        Space.AmbientLightLevel = FMath::Min(1.0f, Space.AmbientLightLevel + (WeatheringDamage * 0.1f));
    }
}

float UArch_StructuralManager::GetStructuralIntegrity() const
{
    if (StructuralElements.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalIntegrity = 0.0f;
    for (const FArch_StructuralElement& Element : StructuralElements)
    {
        TotalIntegrity += Element.IntegrityLevel;
    }
    
    return TotalIntegrity / StructuralElements.Num();
}

TArray<FArch_StructuralElement> UArch_StructuralManager::GetElementsByBiome(EBiomeType BiomeType) const
{
    TArray<FArch_StructuralElement> FilteredElements;
    
    for (const FArch_StructuralElement& Element : StructuralElements)
    {
        if (Element.AssociatedBiome == BiomeType)
        {
            FilteredElements.Add(Element);
        }
    }
    
    return FilteredElements;
}

void UArch_StructuralManager::RepairStructure(float RepairAmount)
{
    for (FArch_StructuralElement& Element : StructuralElements)
    {
        Element.IntegrityLevel = FMath::Min(1.0f, Element.IntegrityLevel + RepairAmount);
    }
    
    for (FArch_InteriorSpace& Space : InteriorSpaces)
    {
        Space.StructuralSoundness = FMath::Min(1.0f, Space.StructuralSoundness + RepairAmount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Structure repaired by %f"), RepairAmount);
}

bool UArch_StructuralManager::IsStructureSafe() const
{
    float OverallIntegrity = GetStructuralIntegrity();
    return OverallIntegrity > 0.3f; // Structure is safe if above 30% integrity
}

void UArch_StructuralManager::SetWeatheringEnabled(bool bEnabled)
{
    bEnableWeathering = bEnabled;
    UE_LOG(LogTemp, Warning, TEXT("Weathering %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UArch_StructuralManager::UpdateElementIntegrity(FArch_StructuralElement& Element, float DeltaTime)
{
    float EnvironmentalDamage = CalculateEnvironmentalDamage(Element);
    float WeatheringDamage = WeatheringRate * DeltaTime * EnvironmentalDamage;
    
    Element.IntegrityLevel = FMath::Max(0.0f, Element.IntegrityLevel - WeatheringDamage);
}

void UArch_StructuralManager::CheckStructuralStability()
{
    float OverallIntegrity = GetStructuralIntegrity();
    
    if (OverallIntegrity < 0.2f)
    {
        // Structure is critically damaged
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Structure integrity at %f - collapse imminent!"), OverallIntegrity);
    }
    else if (OverallIntegrity < 0.5f)
    {
        // Structure is damaged but stable
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Structure integrity at %f - repairs recommended"), OverallIntegrity);
    }
}

float UArch_StructuralManager::CalculateEnvironmentalDamage(const FArch_StructuralElement& Element) const
{
    float DamageMultiplier = 1.0f;
    
    // Different biomes cause different weathering rates
    switch (Element.AssociatedBiome)
    {
        case EBiomeType::Tropical:
            DamageMultiplier = 1.5f; // High humidity and temperature
            break;
        case EBiomeType::Desert:
            DamageMultiplier = 1.2f; // Sand erosion and temperature extremes
            break;
        case EBiomeType::Arctic:
            DamageMultiplier = 1.3f; // Freeze-thaw cycles
            break;
        case EBiomeType::Temperate:
        default:
            DamageMultiplier = 1.0f; // Baseline weathering
            break;
    }
    
    return DamageMultiplier;
}