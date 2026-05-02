#include "BuildSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildSystem, Log, All);

UBuildSystemManager::UBuildSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update rate
    
    // Initialize build system parameters
    MaxBuildDistance = 500.0f;
    BuildGridSize = 100.0f;
    MaxStructuresPerPlayer = 50;
    
    // Initialize material references
    ValidBuildMaterial = nullptr;
    InvalidBuildMaterial = nullptr;
    
    // Initialize build state
    CurrentBuildMode = EEng_BuildMode::None;
    bIsBuildModeActive = false;
    bCanBuildAtLocation = false;
    
    UE_LOG(LogBuildSystem, Log, TEXT("BuildSystemManager initialized"));
}

void UBuildSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner reference
    OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogBuildSystem, Error, TEXT("BuildSystemManager has no owner actor"));
        return;
    }
    
    // Initialize build grid
    InitializeBuildGrid();
    
    // Load default materials
    LoadBuildMaterials();
    
    UE_LOG(LogBuildSystem, Log, TEXT("BuildSystemManager started for actor: %s"), *OwnerActor->GetName());
}

void UBuildSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsBuildModeActive)
    {
        UpdateBuildPreview();
        ValidateBuildLocation();
    }
}

void UBuildSystemManager::StartBuildMode(EEng_BuildMode BuildMode)
{
    if (BuildMode == EEng_BuildMode::None)
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Cannot start build mode with None type"));
        return;
    }
    
    CurrentBuildMode = BuildMode;
    bIsBuildModeActive = true;
    
    // Create build preview actor
    CreateBuildPreview();
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build mode started: %s"), *UEnum::GetValueAsString(BuildMode));
    
    // Broadcast build mode change
    OnBuildModeChanged.Broadcast(BuildMode, true);
}

void UBuildSystemManager::EndBuildMode()
{
    if (!bIsBuildModeActive)
    {
        return;
    }
    
    // Destroy build preview
    if (BuildPreviewActor)
    {
        BuildPreviewActor->Destroy();
        BuildPreviewActor = nullptr;
    }
    
    EEng_BuildMode PreviousMode = CurrentBuildMode;
    CurrentBuildMode = EEng_BuildMode::None;
    bIsBuildModeActive = false;
    bCanBuildAtLocation = false;
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build mode ended"));
    
    // Broadcast build mode change
    OnBuildModeChanged.Broadcast(PreviousMode, false);
}

bool UBuildSystemManager::AttemptBuild()
{
    if (!bIsBuildModeActive || !bCanBuildAtLocation)
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Cannot build: mode active=%d, can build=%d"), bIsBuildModeActive, bCanBuildAtLocation);
        return false;
    }
    
    // Get current build location
    FVector BuildLocation = GetBuildLocation();
    FRotator BuildRotation = GetBuildRotation();
    
    // Snap to grid
    BuildLocation = SnapToGrid(BuildLocation);
    
    // Create the actual structure
    AActor* NewStructure = CreateStructure(CurrentBuildMode, BuildLocation, BuildRotation);
    if (!NewStructure)
    {
        UE_LOG(LogBuildSystem, Error, TEXT("Failed to create structure"));
        return false;
    }
    
    // Add to built structures list
    BuiltStructures.Add(NewStructure);
    
    // Update grid occupancy
    MarkGridOccupied(BuildLocation);
    
    UE_LOG(LogBuildSystem, Log, TEXT("Structure built successfully at location: %s"), *BuildLocation.ToString());
    
    // Broadcast successful build
    OnStructureBuilt.Broadcast(NewStructure, CurrentBuildMode);
    
    return true;
}

void UBuildSystemManager::InitializeBuildGrid()
{
    // Clear existing grid
    BuildGrid.Empty();
    
    // Initialize grid based on world bounds
    // For now, use a fixed 100x100 grid centered at origin
    const int32 GridSize = 100;
    const float CellSize = BuildGridSize;
    
    for (int32 X = -GridSize/2; X < GridSize/2; X++)
    {
        for (int32 Y = -GridSize/2; Y < GridSize/2; Y++)
        {
            FVector GridPosition(X * CellSize, Y * CellSize, 0.0f);
            BuildGrid.Add(GridPosition, false); // false = not occupied
        }
    }
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build grid initialized with %d cells"), BuildGrid.Num());
}

void UBuildSystemManager::CreateBuildPreview()
{
    if (BuildPreviewActor)
    {
        BuildPreviewActor->Destroy();
    }
    
    // Spawn preview actor
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    BuildPreviewActor = World->SpawnActor<AActor>();
    if (!BuildPreviewActor)
    {
        UE_LOG(LogBuildSystem, Error, TEXT("Failed to spawn build preview actor"));
        return;
    }
    
    // Add static mesh component for preview
    UStaticMeshComponent* PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
    if (PreviewMesh)
    {
        BuildPreviewActor->SetRootComponent(PreviewMesh);
        
        // Set preview mesh based on build mode
        UStaticMesh* PreviewStaticMesh = GetPreviewMesh(CurrentBuildMode);
        if (PreviewStaticMesh)
        {
            PreviewMesh->SetStaticMesh(PreviewStaticMesh);
        }
        
        // Set preview material
        if (ValidBuildMaterial)
        {
            PreviewMesh->SetMaterial(0, ValidBuildMaterial);
        }
        
        // Make preview semi-transparent
        PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    BuildPreviewActor->SetActorLabel(TEXT("BuildPreview"));
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build preview created"));
}

void UBuildSystemManager::UpdateBuildPreview()
{
    if (!BuildPreviewActor)
    {
        return;
    }
    
    // Update preview position
    FVector BuildLocation = GetBuildLocation();
    FRotator BuildRotation = GetBuildRotation();
    
    // Snap to grid
    BuildLocation = SnapToGrid(BuildLocation);
    
    BuildPreviewActor->SetActorLocation(BuildLocation);
    BuildPreviewActor->SetActorRotation(BuildRotation);
    
    // Update preview material based on build validity
    UStaticMeshComponent* PreviewMesh = BuildPreviewActor->FindComponentByClass<UStaticMeshComponent>();
    if (PreviewMesh)
    {
        UMaterialInterface* MaterialToUse = bCanBuildAtLocation ? ValidBuildMaterial : InvalidBuildMaterial;
        if (MaterialToUse)
        {
            PreviewMesh->SetMaterial(0, MaterialToUse);
        }
    }
}

void UBuildSystemManager::ValidateBuildLocation()
{
    FVector BuildLocation = GetBuildLocation();
    BuildLocation = SnapToGrid(BuildLocation);
    
    // Check if location is within build distance
    if (OwnerActor)
    {
        float DistanceToOwner = FVector::Dist(OwnerActor->GetActorLocation(), BuildLocation);
        if (DistanceToOwner > MaxBuildDistance)
        {
            bCanBuildAtLocation = false;
            return;
        }
    }
    
    // Check if grid cell is occupied
    if (IsGridOccupied(BuildLocation))
    {
        bCanBuildAtLocation = false;
        return;
    }
    
    // Check for overlapping actors
    if (HasOverlappingStructures(BuildLocation))
    {
        bCanBuildAtLocation = false;
        return;
    }
    
    // Check terrain suitability
    if (!IsTerrainSuitable(BuildLocation))
    {
        bCanBuildAtLocation = false;
        return;
    }
    
    bCanBuildAtLocation = true;
}

FVector UBuildSystemManager::GetBuildLocation() const
{
    // For now, return a fixed location in front of the owner
    // In a real implementation, this would be based on player camera/cursor
    if (OwnerActor)
    {
        FVector ForwardVector = OwnerActor->GetActorForwardVector();
        return OwnerActor->GetActorLocation() + (ForwardVector * 300.0f);
    }
    
    return FVector::ZeroVector;
}

FRotator UBuildSystemManager::GetBuildRotation() const
{
    // For now, align with owner rotation
    if (OwnerActor)
    {
        return OwnerActor->GetActorRotation();
    }
    
    return FRotator::ZeroRotator;
}

FVector UBuildSystemManager::SnapToGrid(const FVector& Location) const
{
    FVector SnappedLocation = Location;
    
    // Snap X and Y to grid
    SnappedLocation.X = FMath::RoundToFloat(SnappedLocation.X / BuildGridSize) * BuildGridSize;
    SnappedLocation.Y = FMath::RoundToFloat(SnappedLocation.Y / BuildGridSize) * BuildGridSize;
    
    // Keep Z as-is for terrain following
    
    return SnappedLocation;
}

bool UBuildSystemManager::IsGridOccupied(const FVector& Location) const
{
    FVector GridKey = SnapToGrid(Location);
    const bool* Occupied = BuildGrid.Find(GridKey);
    return Occupied ? *Occupied : false;
}

void UBuildSystemManager::MarkGridOccupied(const FVector& Location)
{
    FVector GridKey = SnapToGrid(Location);
    BuildGrid.Add(GridKey, true);
}

bool UBuildSystemManager::HasOverlappingStructures(const FVector& Location) const
{
    // Check for nearby structures
    const float CheckRadius = BuildGridSize * 0.8f;
    
    for (AActor* Structure : BuiltStructures)
    {
        if (Structure && IsValid(Structure))
        {
            float Distance = FVector::Dist(Structure->GetActorLocation(), Location);
            if (Distance < CheckRadius)
            {
                return true;
            }
        }
    }
    
    return false;
}

bool UBuildSystemManager::IsTerrainSuitable(const FVector& Location) const
{
    // Perform a line trace to check terrain slope and material
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FVector StartLocation = Location + FVector(0, 0, 1000.0f);
    FVector EndLocation = Location - FVector(0, 0, 1000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Check slope - too steep terrain is not suitable
        FVector SurfaceNormal = HitResult.Normal;
        float SlopeAngle = FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector));
        float SlopeDegrees = FMath::RadiansToDegrees(SlopeAngle);
        
        // Max slope of 30 degrees
        return SlopeDegrees <= 30.0f;
    }
    
    return false;
}

AActor* UBuildSystemManager::CreateStructure(EEng_BuildMode BuildMode, const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Spawn structure actor
    AActor* NewStructure = World->SpawnActor<AActor>(Location, Rotation);
    if (!NewStructure)
    {
        return nullptr;
    }
    
    // Add static mesh component
    UStaticMeshComponent* StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    if (StructureMesh)
    {
        NewStructure->SetRootComponent(StructureMesh);
        
        // Set mesh based on build mode
        UStaticMesh* StructureStaticMesh = GetStructureMesh(BuildMode);
        if (StructureStaticMesh)
        {
            StructureMesh->SetStaticMesh(StructureStaticMesh);
        }
        
        // Enable collision
        StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
    
    // Set structure label
    FString StructureName = FString::Printf(TEXT("Structure_%s"), *UEnum::GetValueAsString(BuildMode));
    NewStructure->SetActorLabel(StructureName);
    
    return NewStructure;
}

UStaticMesh* UBuildSystemManager::GetPreviewMesh(EEng_BuildMode BuildMode) const
{
    // Return default cube mesh for now
    // In a real implementation, this would return specific meshes for each build mode
    return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
}

UStaticMesh* UBuildSystemManager::GetStructureMesh(EEng_BuildMode BuildMode) const
{
    // Return default cube mesh for now
    // In a real implementation, this would return specific meshes for each build mode
    return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
}

void UBuildSystemManager::LoadBuildMaterials()
{
    // Load default materials for build preview
    ValidBuildMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    InvalidBuildMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    
    if (!ValidBuildMaterial)
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Failed to load valid build material"));
    }
    
    if (!InvalidBuildMaterial)
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Failed to load invalid build material"));
    }
}