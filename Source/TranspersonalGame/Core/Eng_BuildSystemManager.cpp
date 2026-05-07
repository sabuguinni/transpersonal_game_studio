#include "Eng_BuildSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogEngBuildSystem, Log, All);

UEng_BuildSystemManager::UEng_BuildSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance
    
    // Initialize build system parameters
    MaxBuildDistance = 500.0f;
    BuildSnapDistance = 50.0f;
    MaxStructuresPerPlayer = 50;
    bAllowOverlappingStructures = false;
    
    // Initialize material references
    static ConstructorHelpers::FObjectFinder<UMaterial> ValidMaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (ValidMaterialFinder.Succeeded())
    {
        ValidPlacementMaterial = ValidMaterialFinder.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<UMaterial> InvalidMaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (InvalidMaterialFinder.Succeeded())
    {
        InvalidPlacementMaterial = InvalidMaterialFinder.Object;
    }
    
    // Initialize build categories
    InitializeBuildCategories();
}

void UEng_BuildSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogEngBuildSystem, Log, TEXT("Build System Manager initialized"));
    
    // Register with game state if available
    if (UWorld* World = GetWorld())
    {
        if (AGameStateBase* GameState = World->GetGameState())
        {
            // Store reference for future use
            CachedGameState = GameState;
        }
    }
}

void UEng_BuildSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update build preview if active
    if (bIsBuildModeActive && CurrentPreviewActor)
    {
        UpdateBuildPreview();
    }
    
    // Validate existing structures periodically
    ValidateExistingStructures();
}

void UEng_BuildSystemManager::InitializeBuildCategories()
{
    // Shelter category
    FEng_BuildCategory ShelterCategory;
    ShelterCategory.CategoryName = TEXT("Shelter");
    ShelterCategory.CategoryDescription = TEXT("Basic shelters and dwellings");
    ShelterCategory.MaxItemsPerCategory = 10;
    ShelterCategory.RequiredResources.Add(TEXT("Wood"), 5);
    ShelterCategory.RequiredResources.Add(TEXT("Stone"), 3);
    BuildCategories.Add(TEXT("Shelter"), ShelterCategory);
    
    // Crafting category
    FEng_BuildCategory CraftingCategory;
    CraftingCategory.CategoryName = TEXT("Crafting");
    CraftingCategory.CategoryDescription = TEXT("Crafting stations and tools");
    CraftingCategory.MaxItemsPerCategory = 5;
    CraftingCategory.RequiredResources.Add(TEXT("Stone"), 2);
    CraftingCategory.RequiredResources.Add(TEXT("Bone"), 1);
    BuildCategories.Add(TEXT("Crafting"), CraftingCategory);
    
    // Defense category
    FEng_BuildCategory DefenseCategory;
    DefenseCategory.CategoryName = TEXT("Defense");
    DefenseCategory.CategoryDescription = TEXT("Defensive structures and traps");
    DefenseCategory.MaxItemsPerCategory = 15;
    DefenseCategory.RequiredResources.Add(TEXT("Wood"), 3);
    DefenseCategory.RequiredResources.Add(TEXT("Stone"), 5);
    BuildCategories.Add(TEXT("Defense"), DefenseCategory);
    
    UE_LOG(LogEngBuildSystem, Log, TEXT("Initialized %d build categories"), BuildCategories.Num());
}

bool UEng_BuildSystemManager::CanPlaceStructure(const FVector& Location, const FString& StructureType)
{
    if (!GetOwner())
    {
        return false;
    }
    
    // Check distance from player
    FVector PlayerLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, Location);
    
    if (Distance > MaxBuildDistance)
    {
        UE_LOG(LogEngBuildSystem, Warning, TEXT("Structure too far from player: %f > %f"), Distance, MaxBuildDistance);
        return false;
    }
    
    // Check for overlapping structures if not allowed
    if (!bAllowOverlappingStructures)
    {
        if (IsLocationOccupied(Location))
        {
            UE_LOG(LogEngBuildSystem, Warning, TEXT("Location is occupied by another structure"));
            return false;
        }
    }
    
    // Check structure count limit
    if (PlayerStructures.Num() >= MaxStructuresPerPlayer)
    {
        UE_LOG(LogEngBuildSystem, Warning, TEXT("Maximum structures reached: %d"), MaxStructuresPerPlayer);
        return false;
    }
    
    // Check terrain suitability
    if (!IsTerrainSuitable(Location))
    {
        UE_LOG(LogEngBuildSystem, Warning, TEXT("Terrain not suitable for building"));
        return false;
    }
    
    return true;
}

bool UEng_BuildSystemManager::PlaceStructure(const FVector& Location, const FString& StructureType)
{
    if (!CanPlaceStructure(Location, StructureType))
    {
        return false;
    }
    
    // Create structure data
    FEng_StructureData NewStructure;
    NewStructure.StructureID = FGuid::NewGuid();
    NewStructure.StructureType = StructureType;
    NewStructure.Location = Location;
    NewStructure.Rotation = FRotator::ZeroRotator;
    NewStructure.Health = 100.0f;
    NewStructure.MaxHealth = 100.0f;
    NewStructure.bIsCompleted = true;
    NewStructure.PlacementTime = FDateTime::Now();
    
    // Add to player structures
    PlayerStructures.Add(NewStructure.StructureID, NewStructure);
    
    // Spawn visual representation (placeholder cube for now)
    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AActor* StructureActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
        if (StructureActor)
        {
            // Add static mesh component
            UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(StructureActor);
            StructureActor->SetRootComponent(MeshComponent);
            
            // Set a basic cube mesh
            static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
            if (CubeMeshFinder.Succeeded())
            {
                MeshComponent->SetStaticMesh(CubeMeshFinder.Object);
                MeshComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 2.0f)); // Make it look like a basic shelter
            }
            
            StructureActor->SetActorLabel(FString::Printf(TEXT("Structure_%s"), *StructureType));
            
            // Store actor reference
            NewStructure.ActorReference = StructureActor;
            PlayerStructures[NewStructure.StructureID] = NewStructure;
        }
    }
    
    UE_LOG(LogEngBuildSystem, Log, TEXT("Placed structure %s at location %s"), *StructureType, *Location.ToString());
    
    // Broadcast structure placed event
    OnStructurePlaced.Broadcast(NewStructure.StructureID, StructureType, Location);
    
    return true;
}

bool UEng_BuildSystemManager::RemoveStructure(const FGuid& StructureID)
{
    if (!PlayerStructures.Contains(StructureID))
    {
        return false;
    }
    
    FEng_StructureData& Structure = PlayerStructures[StructureID];
    
    // Destroy actor if it exists
    if (Structure.ActorReference && IsValid(Structure.ActorReference))
    {
        Structure.ActorReference->Destroy();
    }
    
    // Remove from structures map
    PlayerStructures.Remove(StructureID);
    
    UE_LOG(LogEngBuildSystem, Log, TEXT("Removed structure %s"), *StructureID.ToString());
    
    // Broadcast structure removed event
    OnStructureRemoved.Broadcast(StructureID);
    
    return true;
}

void UEng_BuildSystemManager::StartBuildMode(const FString& StructureType)
{
    if (bIsBuildModeActive)
    {
        StopBuildMode();
    }
    
    bIsBuildModeActive = true;
    CurrentBuildType = StructureType;
    
    // Create preview actor
    CreateBuildPreview();
    
    UE_LOG(LogEngBuildSystem, Log, TEXT("Started build mode for %s"), *StructureType);
}

void UEng_BuildSystemManager::StopBuildMode()
{
    bIsBuildModeActive = false;
    CurrentBuildType = TEXT("");
    
    // Destroy preview actor
    if (CurrentPreviewActor && IsValid(CurrentPreviewActor))
    {
        CurrentPreviewActor->Destroy();
        CurrentPreviewActor = nullptr;
    }
    
    UE_LOG(LogEngBuildSystem, Log, TEXT("Stopped build mode"));
}

void UEng_BuildSystemManager::CreateBuildPreview()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Destroy existing preview
    if (CurrentPreviewActor && IsValid(CurrentPreviewActor))
    {
        CurrentPreviewActor->Destroy();
    }
    
    // Create new preview actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    CurrentPreviewActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    
    if (CurrentPreviewActor)
    {
        // Add static mesh component
        UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(CurrentPreviewActor);
        CurrentPreviewActor->SetRootComponent(MeshComponent);
        
        // Set a basic cube mesh for preview
        static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMeshFinder.Succeeded())
        {
            MeshComponent->SetStaticMesh(CubeMeshFinder.Object);
            MeshComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 2.0f));
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        
        CurrentPreviewActor->SetActorLabel(TEXT("BuildPreview"));
        CurrentPreviewActor->SetActorHiddenInGame(false);
        
        // Make it semi-transparent
        if (MeshComponent && ValidPlacementMaterial)
        {
            MeshComponent->SetMaterial(0, ValidPlacementMaterial);
        }
    }
}

void UEng_BuildSystemManager::UpdateBuildPreview()
{
    if (!CurrentPreviewActor || !GetOwner())
    {
        return;
    }
    
    // Get player location and forward vector for preview placement
    FVector PlayerLocation = GetOwner()->GetActorLocation();
    FVector PlayerForward = GetOwner()->GetActorForwardVector();
    
    // Place preview in front of player
    FVector PreviewLocation = PlayerLocation + (PlayerForward * 300.0f);
    PreviewLocation.Z = PlayerLocation.Z; // Keep at same height for now
    
    CurrentPreviewActor->SetActorLocation(PreviewLocation);
    
    // Update material based on placement validity
    bool bCanPlace = CanPlaceStructure(PreviewLocation, CurrentBuildType);
    
    if (UStaticMeshComponent* MeshComponent = CurrentPreviewActor->FindComponentByClass<UStaticMeshComponent>())
    {
        UMaterialInterface* MaterialToUse = bCanPlace ? ValidPlacementMaterial : InvalidPlacementMaterial;
        if (MaterialToUse)
        {
            MeshComponent->SetMaterial(0, MaterialToUse);
        }
    }
}

bool UEng_BuildSystemManager::IsLocationOccupied(const FVector& Location)
{
    // Check if any existing structure is too close
    for (const auto& StructurePair : PlayerStructures)
    {
        const FEng_StructureData& Structure = StructurePair.Value;
        float Distance = FVector::Dist(Structure.Location, Location);
        
        if (Distance < BuildSnapDistance)
        {
            return true;
        }
    }
    
    return false;
}

bool UEng_BuildSystemManager::IsTerrainSuitable(const FVector& Location)
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Perform line trace downward to check terrain
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 200);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Check if the surface is relatively flat
        FVector HitNormal = HitResult.Normal;
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitNormal, FVector::UpVector)));
        
        // Allow building on surfaces with slope less than 30 degrees
        return SlopeAngle < 30.0f;
    }
    
    return false;
}

void UEng_BuildSystemManager::ValidateExistingStructures()
{
    // Remove invalid structure references
    TArray<FGuid> StructuresToRemove;
    
    for (auto& StructurePair : PlayerStructures)
    {
        FEng_StructureData& Structure = StructurePair.Value;
        
        // Check if actor reference is still valid
        if (Structure.ActorReference && !IsValid(Structure.ActorReference))
        {
            StructuresToRemove.Add(StructurePair.Key);
        }
    }
    
    // Remove invalid structures
    for (const FGuid& StructureID : StructuresToRemove)
    {
        PlayerStructures.Remove(StructureID);
        UE_LOG(LogEngBuildSystem, Warning, TEXT("Removed invalid structure reference: %s"), *StructureID.ToString());
    }
}

TArray<FEng_StructureData> UEng_BuildSystemManager::GetPlayerStructures() const
{
    TArray<FEng_StructureData> Structures;
    PlayerStructures.GenerateValueArray(Structures);
    return Structures;
}

FEng_StructureData* UEng_BuildSystemManager::GetStructureByID(const FGuid& StructureID)
{
    return PlayerStructures.Find(StructureID);
}

int32 UEng_BuildSystemManager::GetStructureCount() const
{
    return PlayerStructures.Num();
}

bool UEng_BuildSystemManager::HasBuildCategory(const FString& CategoryName) const
{
    return BuildCategories.Contains(CategoryName);
}

FEng_BuildCategory* UEng_BuildSystemManager::GetBuildCategory(const FString& CategoryName)
{
    return BuildCategories.Find(CategoryName);
}