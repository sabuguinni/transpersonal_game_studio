#include "Arch_InteriorManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

UArch_InteriorManager::UArch_InteriorManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize interior type probabilities
    InteriorTypeProbabilities.Add(EArch_InteriorType::CaveDwelling, 0.4f);
    InteriorTypeProbabilities.Add(EArch_InteriorType::ElevatedShelter, 0.25f);
    InteriorTypeProbabilities.Add(EArch_InteriorType::RockShelter, 0.2f);
    InteriorTypeProbabilities.Add(EArch_InteriorType::GatheringCircle, 0.15f);
    
    // Initialize default interior settings
    DefaultInteriorSettings.MaxOccupants = 6;
    DefaultInteriorSettings.FirePitRadius = 150.0f;
    DefaultInteriorSettings.SleepingAreaRadius = 200.0f;
    DefaultInteriorSettings.StorageCapacity = 20;
    DefaultInteriorSettings.ComfortLevel = 0.7f;
    DefaultInteriorSettings.ProtectionLevel = 0.8f;
    
    // Initialize component arrays
    ActiveInteriors.Empty();
    InteriorComponents.Empty();
    FirePitLocations.Empty();
    SleepingAreas.Empty();
    StorageAreas.Empty();
}

void UArch_InteriorManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Initialized in world %s"), *GetWorld()->GetName());
        InitializeInteriorSystem();
    }
}

void UArch_InteriorManager::InitializeInteriorSystem()
{
    // Clear existing data
    ActiveInteriors.Empty();
    InteriorComponents.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Interior system initialized"));
}

bool UArch_InteriorManager::CreateInterior(const FVector& Location, EArch_InteriorType Type, const FArch_InteriorSettings& Settings)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Arch_InteriorManager: Cannot create interior - no valid world"));
        return false;
    }
    
    // Create interior data structure
    FArch_InteriorData NewInterior;
    NewInterior.InteriorID = FGuid::NewGuid();
    NewInterior.Location = Location;
    NewInterior.Type = Type;
    NewInterior.Settings = Settings;
    NewInterior.CreationTime = GetWorld()->GetTimeSeconds();
    NewInterior.bIsActive = true;
    NewInterior.CurrentOccupants = 0;
    
    // Setup interior based on type
    switch (Type)
    {
        case EArch_InteriorType::CaveDwelling:
            SetupCaveDwelling(NewInterior);
            break;
        case EArch_InteriorType::ElevatedShelter:
            SetupElevatedShelter(NewInterior);
            break;
        case EArch_InteriorType::RockShelter:
            SetupRockShelter(NewInterior);
            break;
        case EArch_InteriorType::GatheringCircle:
            SetupGatheringCircle(NewInterior);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Arch_InteriorManager: Unknown interior type"));
            return false;
    }
    
    // Add to active interiors
    ActiveInteriors.Add(NewInterior);
    
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Created interior of type %d at location %s"), 
           (int32)Type, *Location.ToString());
    
    return true;
}

void UArch_InteriorManager::SetupCaveDwelling(FArch_InteriorData& InteriorData)
{
    const FVector& Location = InteriorData.Location;
    
    // Create fire pit at center
    FVector FirePitLocation = Location;
    FirePitLocations.Add(FirePitLocation);
    
    // Create sleeping areas around the fire pit
    const float SleepingRadius = InteriorData.Settings.SleepingAreaRadius;
    const int32 NumSleepingAreas = FMath::Min(InteriorData.Settings.MaxOccupants, 8);
    
    for (int32 i = 0; i < NumSleepingAreas; i++)
    {
        float Angle = (2.0f * PI * i) / NumSleepingAreas;
        FVector SleepingLocation = Location + FVector(
            FMath::Cos(Angle) * SleepingRadius,
            FMath::Sin(Angle) * SleepingRadius,
            0.0f
        );
        
        FArch_SleepingArea SleepingArea;
        SleepingArea.Location = SleepingLocation;
        SleepingArea.Rotation = FRotator(0.0f, FMath::RadiansToDegrees(Angle + PI), 0.0f);
        SleepingArea.bIsOccupied = false;
        SleepingArea.ComfortLevel = InteriorData.Settings.ComfortLevel;
        
        SleepingAreas.Add(SleepingArea);
    }
    
    // Create storage areas along walls
    const int32 NumStorageAreas = FMath::Min(InteriorData.Settings.StorageCapacity / 5, 6);
    for (int32 i = 0; i < NumStorageAreas; i++)
    {
        float Angle = (2.0f * PI * i) / NumStorageAreas;
        FVector StorageLocation = Location + FVector(
            FMath::Cos(Angle) * (SleepingRadius + 100.0f),
            FMath::Sin(Angle) * (SleepingRadius + 100.0f),
            50.0f
        );
        
        FArch_StorageArea StorageArea;
        StorageArea.Location = StorageLocation;
        StorageArea.Capacity = 5;
        StorageArea.CurrentItems = 0;
        StorageArea.StorageType = EArch_StorageType::WallNiche;
        
        StorageAreas.Add(StorageArea);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Setup cave dwelling with %d sleeping areas and %d storage areas"), 
           NumSleepingAreas, NumStorageAreas);
}

void UArch_InteriorManager::SetupElevatedShelter(FArch_InteriorData& InteriorData)
{
    const FVector& Location = InteriorData.Location;
    
    // Elevated shelter is 4-5 meters above ground
    FVector ElevatedLocation = Location + FVector(0.0f, 0.0f, 450.0f);
    
    // Central fire pit (smaller for safety)
    FirePitLocations.Add(ElevatedLocation);
    
    // Sleeping areas around perimeter
    const float PlatformRadius = InteriorData.Settings.SleepingAreaRadius * 0.8f; // Smaller platform
    const int32 NumSleepingAreas = FMath::Min(InteriorData.Settings.MaxOccupants, 6);
    
    for (int32 i = 0; i < NumSleepingAreas; i++)
    {
        float Angle = (2.0f * PI * i) / NumSleepingAreas;
        FVector SleepingLocation = ElevatedLocation + FVector(
            FMath::Cos(Angle) * PlatformRadius,
            FMath::Sin(Angle) * PlatformRadius,
            0.0f
        );
        
        FArch_SleepingArea SleepingArea;
        SleepingArea.Location = SleepingLocation;
        SleepingArea.Rotation = FRotator(0.0f, FMath::RadiansToDegrees(Angle + PI), 0.0f);
        SleepingArea.bIsOccupied = false;
        SleepingArea.ComfortLevel = InteriorData.Settings.ComfortLevel * 0.9f; // Slightly less comfortable
        
        SleepingAreas.Add(SleepingArea);
    }
    
    // Storage areas hanging from platform
    const int32 NumStorageAreas = 4;
    for (int32 i = 0; i < NumStorageAreas; i++)
    {
        float Angle = (PI * 0.5f * i); // Cardinal directions
        FVector StorageLocation = ElevatedLocation + FVector(
            FMath::Cos(Angle) * (PlatformRadius + 50.0f),
            FMath::Sin(Angle) * (PlatformRadius + 50.0f),
            -100.0f
        );
        
        FArch_StorageArea StorageArea;
        StorageArea.Location = StorageLocation;
        StorageArea.Capacity = 3;
        StorageArea.CurrentItems = 0;
        StorageArea.StorageType = EArch_StorageType::HangingBasket;
        
        StorageAreas.Add(StorageArea);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Setup elevated shelter at height %f"), ElevatedLocation.Z);
}

void UArch_InteriorManager::SetupRockShelter(FArch_InteriorData& InteriorData)
{
    const FVector& Location = InteriorData.Location;
    
    // Fire pit near the front of the shelter
    FVector FirePitLocation = Location + FVector(200.0f, 0.0f, 0.0f);
    FirePitLocations.Add(FirePitLocation);
    
    // Sleeping areas along the back wall
    const int32 NumSleepingAreas = InteriorData.Settings.MaxOccupants;
    const float ShelterWidth = 400.0f;
    
    for (int32 i = 0; i < NumSleepingAreas; i++)
    {
        float YOffset = (i - (NumSleepingAreas - 1) * 0.5f) * (ShelterWidth / NumSleepingAreas);
        FVector SleepingLocation = Location + FVector(-150.0f, YOffset, 0.0f);
        
        FArch_SleepingArea SleepingArea;
        SleepingArea.Location = SleepingLocation;
        SleepingArea.Rotation = FRotator(0.0f, 0.0f, 0.0f);
        SleepingArea.bIsOccupied = false;
        SleepingArea.ComfortLevel = InteriorData.Settings.ComfortLevel;
        
        SleepingAreas.Add(SleepingArea);
    }
    
    // Storage areas carved into rock walls
    const int32 NumStorageAreas = 6;
    for (int32 i = 0; i < NumStorageAreas; i++)
    {
        float YOffset = (i - (NumStorageAreas - 1) * 0.5f) * 80.0f;
        FVector StorageLocation = Location + FVector(-200.0f, YOffset, 100.0f);
        
        FArch_StorageArea StorageArea;
        StorageArea.Location = StorageLocation;
        StorageArea.Capacity = 4;
        StorageArea.CurrentItems = 0;
        StorageArea.StorageType = EArch_StorageType::RockNiche;
        
        StorageAreas.Add(StorageArea);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Setup rock shelter with %d sleeping areas"), NumSleepingAreas);
}

void UArch_InteriorManager::SetupGatheringCircle(FArch_InteriorData& InteriorData)
{
    const FVector& Location = InteriorData.Location;
    
    // Central fire pit
    FirePitLocations.Add(Location);
    
    // Stone seating arranged in circle
    const float CircleRadius = InteriorData.Settings.SleepingAreaRadius;
    const int32 NumSeats = InteriorData.Settings.MaxOccupants * 2; // More seating for gatherings
    
    for (int32 i = 0; i < NumSeats; i++)
    {
        float Angle = (2.0f * PI * i) / NumSeats;
        FVector SeatLocation = Location + FVector(
            FMath::Cos(Angle) * CircleRadius,
            FMath::Sin(Angle) * CircleRadius,
            0.0f
        );
        
        FArch_SleepingArea SeatArea; // Reusing sleeping area struct for seating
        SeatArea.Location = SeatLocation;
        SeatArea.Rotation = FRotator(0.0f, FMath::RadiansToDegrees(Angle + PI), 0.0f);
        SeatArea.bIsOccupied = false;
        SeatArea.ComfortLevel = 0.5f; // Stone seating is less comfortable
        
        SleepingAreas.Add(SeatArea);
    }
    
    // Tool storage areas around perimeter
    const int32 NumToolAreas = 8;
    for (int32 i = 0; i < NumToolAreas; i++)
    {
        float Angle = (2.0f * PI * i) / NumToolAreas;
        FVector StorageLocation = Location + FVector(
            FMath::Cos(Angle) * (CircleRadius + 150.0f),
            FMath::Sin(Angle) * (CircleRadius + 150.0f),
            0.0f
        );
        
        FArch_StorageArea StorageArea;
        StorageArea.Location = StorageLocation;
        StorageArea.Capacity = 2;
        StorageArea.CurrentItems = 0;
        StorageArea.StorageType = EArch_StorageType::GroundCache;
        
        StorageAreas.Add(StorageArea);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Setup gathering circle with %d seats"), NumSeats);
}

bool UArch_InteriorManager::RemoveInterior(const FGuid& InteriorID)
{
    for (int32 i = ActiveInteriors.Num() - 1; i >= 0; i--)
    {
        if (ActiveInteriors[i].InteriorID == InteriorID)
        {
            ActiveInteriors.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Removed interior %s"), *InteriorID.ToString());
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_InteriorManager: Interior %s not found for removal"), *InteriorID.ToString());
    return false;
}

FArch_InteriorData* UArch_InteriorManager::GetInteriorByID(const FGuid& InteriorID)
{
    for (FArch_InteriorData& Interior : ActiveInteriors)
    {
        if (Interior.InteriorID == InteriorID)
        {
            return &Interior;
        }
    }
    
    return nullptr;
}

TArray<FArch_InteriorData*> UArch_InteriorManager::GetInteriorsInRadius(const FVector& Location, float Radius)
{
    TArray<FArch_InteriorData*> NearbyInteriors;
    
    for (FArch_InteriorData& Interior : ActiveInteriors)
    {
        float Distance = FVector::Dist(Interior.Location, Location);
        if (Distance <= Radius)
        {
            NearbyInteriors.Add(&Interior);
        }
    }
    
    return NearbyInteriors;
}

bool UArch_InteriorManager::CanEnterInterior(const FGuid& InteriorID, int32 NumOccupants)
{
    FArch_InteriorData* Interior = GetInteriorByID(InteriorID);
    if (!Interior)
    {
        return false;
    }
    
    return (Interior->CurrentOccupants + NumOccupants) <= Interior->Settings.MaxOccupants;
}

bool UArch_InteriorManager::EnterInterior(const FGuid& InteriorID, int32 NumOccupants)
{
    FArch_InteriorData* Interior = GetInteriorByID(InteriorID);
    if (!Interior || !CanEnterInterior(InteriorID, NumOccupants))
    {
        return false;
    }
    
    Interior->CurrentOccupants += NumOccupants;
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: %d occupants entered interior. Total: %d/%d"), 
           NumOccupants, Interior->CurrentOccupants, Interior->Settings.MaxOccupants);
    
    return true;
}

bool UArch_InteriorManager::ExitInterior(const FGuid& InteriorID, int32 NumOccupants)
{
    FArch_InteriorData* Interior = GetInteriorByID(InteriorID);
    if (!Interior)
    {
        return false;
    }
    
    Interior->CurrentOccupants = FMath::Max(0, Interior->CurrentOccupants - NumOccupants);
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: %d occupants exited interior. Total: %d/%d"), 
           NumOccupants, Interior->CurrentOccupants, Interior->Settings.MaxOccupants);
    
    return true;
}

EArch_InteriorType UArch_InteriorManager::GetRandomInteriorType() const
{
    float RandomValue = FMath::FRand();
    float CumulativeProbability = 0.0f;
    
    for (const auto& Pair : InteriorTypeProbabilities)
    {
        CumulativeProbability += Pair.Value;
        if (RandomValue <= CumulativeProbability)
        {
            return Pair.Key;
        }
    }
    
    // Fallback to cave dwelling
    return EArch_InteriorType::CaveDwelling;
}

void UArch_InteriorManager::UpdateInteriorSystem(float DeltaTime)
{
    // Update interior states, maintenance, etc.
    for (FArch_InteriorData& Interior : ActiveInteriors)
    {
        if (Interior.bIsActive)
        {
            // Update interior logic here
            // For now, just ensure it stays active
        }
    }
}

int32 UArch_InteriorManager::GetActiveInteriorCount() const
{
    return ActiveInteriors.Num();
}

TArray<FVector> UArch_InteriorManager::GetAllFirePitLocations() const
{
    return FirePitLocations;
}

TArray<FArch_SleepingArea> UArch_InteriorManager::GetAllSleepingAreas() const
{
    return SleepingAreas;
}

TArray<FArch_StorageArea> UArch_InteriorManager::GetAllStorageAreas() const
{
    return StorageAreas;
}