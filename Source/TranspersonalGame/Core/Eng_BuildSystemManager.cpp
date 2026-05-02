#include "BuildSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildSystem, Log, All);

UEng_BuildSystemManager::UEng_BuildSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize build system parameters
    MaxBuildDistance = 500.0f;
    BuildSnapDistance = 50.0f;
    MaxConcurrentBuilds = 5;
    
    // Initialize build costs
    BuildCosts.Add(EEng_BuildableType::Shelter, 10);
    BuildCosts.Add(EEng_BuildableType::Firepit, 5);
    BuildCosts.Add(EEng_BuildableType::StorageBox, 8);
    BuildCosts.Add(EEng_BuildableType::CraftingTable, 12);
    BuildCosts.Add(EEng_BuildableType::TrapBasic, 6);
    
    // Initialize resource requirements
    ResourceRequirements.Add(EEng_BuildableType::Shelter, 
        {EEng_ResourceType::Wood, EEng_ResourceType::Stone, EEng_ResourceType::Fiber});
    ResourceRequirements.Add(EEng_BuildableType::Firepit,
        {EEng_ResourceType::Stone, EEng_ResourceType::Wood});
    ResourceRequirements.Add(EEng_BuildableType::StorageBox,
        {EEng_ResourceType::Wood, EEng_ResourceType::Fiber});
    ResourceRequirements.Add(EEng_BuildableType::CraftingTable,
        {EEng_ResourceType::Wood, EEng_ResourceType::Stone});
    ResourceRequirements.Add(EEng_BuildableType::TrapBasic,
        {EEng_ResourceType::Wood, EEng_ResourceType::Fiber});
}

void UEng_BuildSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBuildSystem, Log, TEXT("Build System Manager initialized"));
    
    // Initialize build system state
    ActiveBuilds.Empty();
    bBuildModeActive = false;
    CurrentBuildType = EEng_BuildableType::None;
    
    // Cache world reference
    CachedWorld = GetWorld();
}

void UEng_BuildSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active builds
    UpdateActiveBuilds(DeltaTime);
    
    // Update build preview if in build mode
    if (bBuildModeActive)
    {
        UpdateBuildPreview();
    }
}

bool UEng_BuildSystemManager::CanBuildAt(const FVector& Location, EEng_BuildableType BuildType)
{
    if (!CachedWorld)
    {
        return false;
    }
    
    // Check if location is within build distance
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), Location);
        if (Distance > MaxBuildDistance)
        {
            UE_LOG(LogBuildSystem, Warning, TEXT("Build location too far: %.1f > %.1f"), Distance, MaxBuildDistance);
            return false;
        }
    }
    
    // Check for overlapping structures
    FVector BoxExtent(100.0f, 100.0f, 50.0f);
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHasOverlap = CachedWorld->OverlapMultiByChannel(
        Overlaps,
        Location,
        FQuat::Identity,
        ECollisionChannel::ECC_WorldStatic,
        FCollisionShape::MakeBox(BoxExtent),
        QueryParams
    );
    
    if (bHasOverlap)
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Build location blocked by existing structures"));
        return false;
    }
    
    // Check terrain suitability
    FHitResult HitResult;
    FVector StartTrace = Location + FVector(0, 0, 100);
    FVector EndTrace = Location - FVector(0, 0, 100);
    
    bool bHitTerrain = CachedWorld->LineTraceSingleByChannel(
        HitResult,
        StartTrace,
        EndTrace,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (!bHitTerrain)
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("No suitable terrain found at build location"));
        return false;
    }
    
    // Check terrain slope
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector)));
    if (SlopeAngle > 30.0f) // Max 30 degree slope
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Terrain too steep: %.1f degrees"), SlopeAngle);
        return false;
    }
    
    return true;
}

bool UEng_BuildSystemManager::StartBuild(const FVector& Location, EEng_BuildableType BuildType)
{
    if (!CanBuildAt(Location, BuildType))
    {
        return false;
    }
    
    if (ActiveBuilds.Num() >= MaxConcurrentBuilds)
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Maximum concurrent builds reached: %d"), MaxConcurrentBuilds);
        return false;
    }
    
    // Check resources
    if (!HasRequiredResources(BuildType))
    {
        UE_LOG(LogBuildSystem, Warning, TEXT("Insufficient resources for build type: %d"), (int32)BuildType);
        return false;
    }
    
    // Create build info
    FEng_BuildInfo BuildInfo;
    BuildInfo.BuildType = BuildType;
    BuildInfo.Location = Location;
    BuildInfo.BuildProgress = 0.0f;
    BuildInfo.BuildTime = GetBuildTime(BuildType);
    BuildInfo.bIsCompleted = false;
    
    // Consume resources
    ConsumeResources(BuildType);
    
    // Add to active builds
    int32 BuildIndex = ActiveBuilds.Add(BuildInfo);
    
    UE_LOG(LogBuildSystem, Log, TEXT("Started build: Type=%d, Location=%s, Index=%d"), 
        (int32)BuildType, *Location.ToString(), BuildIndex);
    
    return true;
}

void UEng_BuildSystemManager::UpdateActiveBuilds(float DeltaTime)
{
    for (int32 i = ActiveBuilds.Num() - 1; i >= 0; i--)
    {
        FEng_BuildInfo& BuildInfo = ActiveBuilds[i];
        
        if (BuildInfo.bIsCompleted)
        {
            continue;
        }
        
        // Update build progress
        BuildInfo.BuildProgress += DeltaTime / BuildInfo.BuildTime;
        
        if (BuildInfo.BuildProgress >= 1.0f)
        {
            // Complete the build
            CompleteBuild(i);
        }
    }
}

void UEng_BuildSystemManager::CompleteBuild(int32 BuildIndex)
{
    if (!ActiveBuilds.IsValidIndex(BuildIndex))
    {
        return;
    }
    
    FEng_BuildInfo& BuildInfo = ActiveBuilds[BuildIndex];
    BuildInfo.bIsCompleted = true;
    BuildInfo.BuildProgress = 1.0f;
    
    // Spawn the actual building structure
    SpawnBuildingStructure(BuildInfo);
    
    UE_LOG(LogBuildSystem, Log, TEXT("Completed build: Type=%d, Location=%s"), 
        (int32)BuildInfo.BuildType, *BuildInfo.Location.ToString());
    
    // Remove from active builds after a delay
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this, BuildIndex]() { 
            if (ActiveBuilds.IsValidIndex(BuildIndex)) 
            {
                ActiveBuilds.RemoveAt(BuildIndex);
            }
        },
        1.0f,
        false
    );
}

void UEng_BuildSystemManager::SpawnBuildingStructure(const FEng_BuildInfo& BuildInfo)
{
    if (!CachedWorld)
    {
        return;
    }
    
    // For now, spawn a simple placeholder actor
    // In a full implementation, this would spawn the appropriate building blueprint
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* BuildingActor = CachedWorld->SpawnActor<AActor>(
        AActor::StaticClass(),
        BuildInfo.Location,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (BuildingActor)
    {
        // Add a static mesh component as a placeholder
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(BuildingActor);
        BuildingActor->SetRootComponent(MeshComp);
        
        // Set a simple cube mesh as placeholder
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            MeshComp->SetStaticMesh(CubeMesh);
        }
        
        BuildingActor->SetActorLabel(FString::Printf(TEXT("Building_%s"), 
            *UEnum::GetValueAsString(BuildInfo.BuildType)));
        
        UE_LOG(LogBuildSystem, Log, TEXT("Spawned building structure: %s"), *BuildingActor->GetName());
    }
}

bool UEng_BuildSystemManager::HasRequiredResources(EEng_BuildableType BuildType)
{
    // Simplified resource check - in full implementation would check player inventory
    return true;
}

void UEng_BuildSystemManager::ConsumeResources(EEng_BuildableType BuildType)
{
    // Simplified resource consumption - in full implementation would modify player inventory
    UE_LOG(LogBuildSystem, Log, TEXT("Consumed resources for build type: %d"), (int32)BuildType);
}

float UEng_BuildSystemManager::GetBuildTime(EEng_BuildableType BuildType)
{
    switch (BuildType)
    {
        case EEng_BuildableType::Shelter: return 10.0f;
        case EEng_BuildableType::Firepit: return 5.0f;
        case EEng_BuildableType::StorageBox: return 8.0f;
        case EEng_BuildableType::CraftingTable: return 12.0f;
        case EEng_BuildableType::TrapBasic: return 6.0f;
        default: return 5.0f;
    }
}

void UEng_BuildSystemManager::UpdateBuildPreview()
{
    // Build preview logic would go here
    // This would show a ghost/preview of the structure at the cursor location
}

void UEng_BuildSystemManager::EnterBuildMode(EEng_BuildableType BuildType)
{
    bBuildModeActive = true;
    CurrentBuildType = BuildType;
    
    UE_LOG(LogBuildSystem, Log, TEXT("Entered build mode: Type=%d"), (int32)BuildType);
}

void UEng_BuildSystemManager::ExitBuildMode()
{
    bBuildModeActive = false;
    CurrentBuildType = EEng_BuildableType::None;
    
    UE_LOG(LogBuildSystem, Log, TEXT("Exited build mode"));
}

TArray<FEng_BuildInfo> UEng_BuildSystemManager::GetActiveBuilds() const
{
    return ActiveBuilds;
}

float UEng_BuildSystemManager::GetBuildProgress(int32 BuildIndex) const
{
    if (ActiveBuilds.IsValidIndex(BuildIndex))
    {
        return ActiveBuilds[BuildIndex].BuildProgress;
    }
    return 0.0f;
}