#include "Arch_InteriorArtifactSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchInteriorArtifact, Log, All);

UArch_InteriorArtifactSystem::UArch_InteriorArtifactSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;
    
    // Initialize artifact spawn settings
    MaxArtifactsPerRoom = 8;
    ArtifactSpawnRadius = 300.0f;
    MinDistanceBetweenArtifacts = 80.0f;
    ArtifactDensity = 0.6f;
    
    // Initialize wear patterns
    WearIntensity = 0.7f;
    AgeVariation = 0.4f;
    WeatheringFactor = 0.5f;
    
    // Initialize artifact categories with weights
    ArtifactCategoryWeights.Add(EArch_ArtifactCategory::Tools, 0.3f);
    ArtifactCategoryWeights.Add(EArch_ArtifactCategory::Containers, 0.25f);
    ArtifactCategoryWeights.Add(EArch_ArtifactCategory::Decorative, 0.15f);
    ArtifactCategoryWeights.Add(EArch_ArtifactCategory::Functional, 0.2f);
    ArtifactCategoryWeights.Add(EArch_ArtifactCategory::Personal, 0.1f);
    
    // Initialize placement rules
    PlacementRules.Add(EArch_ArtifactCategory::Tools, FArch_PlacementRule{
        true,  // bNearWalls
        false, // bNearCenter
        true,  // bOnSurfaces
        false, // bHanging
        0.8f   // PreferredHeight
    });
    
    PlacementRules.Add(EArch_ArtifactCategory::Containers, FArch_PlacementRule{
        false, // bNearWalls
        true,  // bNearCenter
        true,  // bOnSurfaces
        false, // bHanging
        0.2f   // PreferredHeight
    });
    
    PlacementRules.Add(EArch_ArtifactCategory::Decorative, FArch_PlacementRule{
        true,  // bNearWalls
        false, // bNearCenter
        false, // bOnSurfaces
        true,  // bHanging
        1.5f   // PreferredHeight
    });
}

void UArch_InteriorArtifactSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogArchInteriorArtifact, Log, TEXT("Interior Artifact System initialized"));
    
    // Start artifact generation after short delay
    GetWorld()->GetTimerManager().SetTimer(
        ArtifactGenerationTimer,
        this,
        &UArch_InteriorArtifactSystem::GenerateRoomArtifacts,
        2.0f,
        false
    );
}

void UArch_InteriorArtifactSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update artifact wear over time
    UpdateArtifactWear(DeltaTime);
    
    // Check for new rooms to populate
    CheckForNewRooms();
}

void UArch_InteriorArtifactSystem::GenerateRoomArtifacts()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    FVector RoomCenter = GetOwner()->GetActorLocation();
    FVector RoomBounds = FVector(ArtifactSpawnRadius, ArtifactSpawnRadius, 200.0f);
    
    UE_LOG(LogArchInteriorArtifact, Log, TEXT("Generating artifacts for room at %s"), *RoomCenter.ToString());
    
    // Calculate number of artifacts to spawn
    int32 ArtifactCount = FMath::RandRange(
        FMath::Max(1, FMath::FloorToInt(MaxArtifactsPerRoom * ArtifactDensity * 0.5f)),
        MaxArtifactsPerRoom
    );
    
    TArray<FVector> UsedPositions;
    
    for (int32 i = 0; i < ArtifactCount; i++)
    {
        EArch_ArtifactCategory Category = SelectArtifactCategory();
        FVector SpawnLocation = FindValidArtifactLocation(RoomCenter, RoomBounds, Category, UsedPositions);
        
        if (SpawnLocation != FVector::ZeroVector)
        {
            SpawnArtifact(Category, SpawnLocation);
            UsedPositions.Add(SpawnLocation);
        }
    }
    
    UE_LOG(LogArchInteriorArtifact, Log, TEXT("Generated %d artifacts in room"), UsedPositions.Num());
}

EArch_ArtifactCategory UArch_InteriorArtifactSystem::SelectArtifactCategory() const
{
    float RandomValue = FMath::FRand();
    float CumulativeWeight = 0.0f;
    
    for (const auto& CategoryWeight : ArtifactCategoryWeights)
    {
        CumulativeWeight += CategoryWeight.Value;
        if (RandomValue <= CumulativeWeight)
        {
            return CategoryWeight.Key;
        }
    }
    
    return EArch_ArtifactCategory::Tools; // Fallback
}

FVector UArch_InteriorArtifactSystem::FindValidArtifactLocation(const FVector& RoomCenter, const FVector& RoomBounds, EArch_ArtifactCategory Category, const TArray<FVector>& UsedPositions) const
{
    const FArch_PlacementRule* Rule = PlacementRules.Find(Category);
    if (!Rule)
    {
        return FVector::ZeroVector;
    }
    
    for (int32 Attempt = 0; Attempt < 20; Attempt++)
    {
        FVector TestLocation = GenerateLocationByRule(RoomCenter, RoomBounds, *Rule);
        
        // Check minimum distance from other artifacts
        bool bValidLocation = true;
        for (const FVector& UsedPos : UsedPositions)
        {
            if (FVector::Dist(TestLocation, UsedPos) < MinDistanceBetweenArtifacts)
            {
                bValidLocation = false;
                break;
            }
        }
        
        if (bValidLocation && IsLocationAccessible(TestLocation))
        {
            return TestLocation;
        }
    }
    
    return FVector::ZeroVector;
}

FVector UArch_InteriorArtifactSystem::GenerateLocationByRule(const FVector& RoomCenter, const FVector& RoomBounds, const FArch_PlacementRule& Rule) const
{
    FVector Location = RoomCenter;
    
    if (Rule.bNearWalls)
    {
        // Place near room perimeter
        float Angle = FMath::FRand() * 2.0f * PI;
        float Distance = FMath::RandRange(RoomBounds.X * 0.7f, RoomBounds.X * 0.9f);
        Location.X += FMath::Cos(Angle) * Distance;
        Location.Y += FMath::Sin(Angle) * Distance;
    }
    else if (Rule.bNearCenter)
    {
        // Place near room center
        Location.X += FMath::RandRange(-RoomBounds.X * 0.3f, RoomBounds.X * 0.3f);
        Location.Y += FMath::RandRange(-RoomBounds.Y * 0.3f, RoomBounds.Y * 0.3f);
    }
    else
    {
        // Random placement
        Location.X += FMath::RandRange(-RoomBounds.X, RoomBounds.X);
        Location.Y += FMath::RandRange(-RoomBounds.Y, RoomBounds.Y);
    }
    
    // Set height based on placement rule
    if (Rule.bOnSurfaces)
    {
        Location.Z = RoomCenter.Z + Rule.PreferredHeight;
    }
    else if (Rule.bHanging)
    {
        Location.Z = RoomCenter.Z + RoomBounds.Z * 0.8f;
    }
    else
    {
        Location.Z = RoomCenter.Z + Rule.PreferredHeight;
    }
    
    return Location;
}

bool UArch_InteriorArtifactSystem::IsLocationAccessible(const FVector& Location) const
{
    // Simple accessibility check - can be expanded with proper collision detection
    if (!GetWorld())
    {
        return false;
    }
    
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 100);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic
    );
    
    return bHit; // Location is accessible if there's a surface below
}

void UArch_InteriorArtifactSystem::SpawnArtifact(EArch_ArtifactCategory Category, const FVector& Location)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Create artifact data
    FArch_ArtifactData ArtifactData;
    ArtifactData.Category = Category;
    ArtifactData.Location = Location;
    ArtifactData.WearLevel = FMath::RandRange(0.1f, WearIntensity);
    ArtifactData.Age = FMath::RandRange(0.0f, AgeVariation);
    ArtifactData.bIsInteractable = (Category == EArch_ArtifactCategory::Tools || Category == EArch_ArtifactCategory::Containers);
    
    // Generate artifact name based on category
    ArtifactData.ArtifactName = GenerateArtifactName(Category);
    
    // Store artifact data
    SpawnedArtifacts.Add(ArtifactData);
    
    UE_LOG(LogArchInteriorArtifact, Log, TEXT("Spawned %s artifact: %s at %s"), 
        *UEnum::GetValueAsString(Category), 
        *ArtifactData.ArtifactName, 
        *Location.ToString());
    
    // Trigger artifact spawned event
    OnArtifactSpawned.Broadcast(ArtifactData);
}

FString UArch_InteriorArtifactSystem::GenerateArtifactName(EArch_ArtifactCategory Category) const
{
    TArray<FString> Names;
    
    switch (Category)
    {
        case EArch_ArtifactCategory::Tools:
            Names = {"Stone Scraper", "Bone Needle", "Flint Knife", "Wooden Spear Tip", "Hide Scraper"};
            break;
        case EArch_ArtifactCategory::Containers:
            Names = {"Woven Basket", "Gourd Bowl", "Stone Mortar", "Hide Pouch", "Clay Vessel"};
            break;
        case EArch_ArtifactCategory::Decorative:
            Names = {"Painted Stone", "Carved Bone", "Shell Necklace", "Feather Bundle", "Ochre Drawing"};
            break;
        case EArch_ArtifactCategory::Functional:
            Names = {"Fire Hearth", "Sleeping Fur", "Tool Rack", "Food Cache", "Water Basin"};
            break;
        case EArch_ArtifactCategory::Personal:
            Names = {"Personal Trinket", "Lucky Stone", "Ancestor Token", "Memory Object", "Worn Tool"};
            break;
        default:
            Names = {"Unknown Artifact"};
            break;
    }
    
    if (Names.Num() > 0)
    {
        return Names[FMath::RandRange(0, Names.Num() - 1)];
    }
    
    return TEXT("Mysterious Object");
}

void UArch_InteriorArtifactSystem::UpdateArtifactWear(float DeltaTime)
{
    for (FArch_ArtifactData& Artifact : SpawnedArtifacts)
    {
        // Gradually increase wear over time
        Artifact.WearLevel += DeltaTime * WeatheringFactor * 0.001f; // Very slow weathering
        Artifact.WearLevel = FMath::Clamp(Artifact.WearLevel, 0.0f, 1.0f);
        
        // Increase age
        Artifact.Age += DeltaTime * 0.0001f;
    }
}

void UArch_InteriorArtifactSystem::CheckForNewRooms()
{
    // This would check for new architectural spaces that need artifact population
    // Implementation depends on the broader architecture system
}

TArray<FArch_ArtifactData> UArch_InteriorArtifactSystem::GetArtifactsInRadius(const FVector& Center, float Radius) const
{
    TArray<FArch_ArtifactData> NearbyArtifacts;
    
    for (const FArch_ArtifactData& Artifact : SpawnedArtifacts)
    {
        if (FVector::Dist(Artifact.Location, Center) <= Radius)
        {
            NearbyArtifacts.Add(Artifact);
        }
    }
    
    return NearbyArtifacts;
}

void UArch_InteriorArtifactSystem::ClearArtifactsInArea(const FVector& Center, float Radius)
{
    SpawnedArtifacts.RemoveAll([Center, Radius](const FArch_ArtifactData& Artifact)
    {
        return FVector::Dist(Artifact.Location, Center) <= Radius;
    });
    
    UE_LOG(LogArchInteriorArtifact, Log, TEXT("Cleared artifacts in radius %.1f around %s"), Radius, *Center.ToString());
}

void UArch_InteriorArtifactSystem::SetArtifactDensity(float NewDensity)
{
    ArtifactDensity = FMath::Clamp(NewDensity, 0.1f, 2.0f);
    UE_LOG(LogArchInteriorArtifact, Log, TEXT("Artifact density set to %.2f"), ArtifactDensity);
}

void UArch_InteriorArtifactSystem::RegenerateArtifacts()
{
    // Clear existing artifacts
    SpawnedArtifacts.Empty();
    
    // Regenerate with current settings
    GenerateRoomArtifacts();
    
    UE_LOG(LogArchInteriorArtifact, Log, TEXT("Regenerated all artifacts"));
}