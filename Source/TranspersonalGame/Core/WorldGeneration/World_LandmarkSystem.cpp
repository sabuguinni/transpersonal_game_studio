#include "World_LandmarkSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UWorld_LandmarkSystem::UWorld_LandmarkSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 2.0f; // Update every 2 seconds for performance
    
    // Initialize default spawn settings
    SpawnSettings.MaxLandmarksPerBiome = 15;
    SpawnSettings.MinDistanceBetweenLandmarks = 2000.0f;
    SpawnSettings.BiomeEdgeBuffer = 500.0f;
    SpawnSettings.bAvoidWaterBodies = true;
    SpawnSettings.WaterAvoidanceRadius = 800.0f;
    SpawnSettings.bPreferElevatedPositions = true;
    SpawnSettings.ElevationPreferenceBias = 0.7f;

    // Performance settings
    VisibilityUpdateInterval = 2.0f;
    MaxLandmarkRenderDistance = 10000.0f;
    bUseDistanceBasedLOD = true;

    // Initialize counters
    TotalLandmarkCount = 0;
    VisibleLandmarkCount = 0;
    LastVisibilityUpdateTime = 0.0f;
}

void UWorld_LandmarkSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeLandmarkMappings();
    InitializeDefaultAssetReferences();
    
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Initialized landmark system"));
}

void UWorld_LandmarkSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update landmark visibility based on player position
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            RefreshLandmarkVisibility(PlayerPawn->GetActorLocation());
        }
    }
}

void UWorld_LandmarkSystem::InitializeLandmarkSystem()
{
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Initializing landmark system"));
    
    // Clear existing landmarks
    ClearAllLandmarks();
    
    // Initialize biome-landmark type mappings
    InitializeBiomeLandmarkMappings();
    
    // Initialize asset references
    InitializeDefaultAssetReferences();
    
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Landmark system initialization complete"));
}

void UWorld_LandmarkSystem::GenerateLandmarksForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("World_LandmarkSystem: No valid world for landmark generation"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Generating landmarks for biome type %d at location %s"), 
           (int32)BiomeType, *BiomeCenter.ToString());

    // Generate landmark positions
    TArray<FVector> LandmarkPositions = GenerateLandmarkPositions(BiomeCenter, BiomeRadius, SpawnSettings.MaxLandmarksPerBiome);
    
    int32 LandmarksCreated = 0;
    
    for (const FVector& Position : LandmarkPositions)
    {
        // Select appropriate landmark type for this biome
        EWorld_LandmarkType LandmarkType = SelectLandmarkTypeForBiome(BiomeType);
        
        // Create landmark data
        FWorld_LandmarkData LandmarkData;
        LandmarkData.LandmarkType = LandmarkType;
        LandmarkData.Location = Position;
        LandmarkData.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        LandmarkData.Scale = FVector(FMath::RandRange(0.8f, 1.5f));
        LandmarkData.VisibilityRange = 5000.0f;
        LandmarkData.bHasAmbientSound = (LandmarkType == EWorld_LandmarkType::Waterfall || LandmarkType == EWorld_LandmarkType::GeyserSite);
        LandmarkData.AmbientSoundRadius = LandmarkData.bHasAmbientSound ? 1500.0f : 0.0f;
        LandmarkData.bIsNavigationLandmark = true;
        LandmarkData.LandmarkName = FString::Printf(TEXT("%s_%d"), 
            *UEnum::GetValueAsString(LandmarkType), LandmarksCreated);
        
        // Spawn the landmark
        AStaticMeshActor* LandmarkActor = SpawnLandmark(LandmarkData);
        if (LandmarkActor)
        {
            LandmarksCreated++;
            UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Created landmark %s at %s"), 
                   *LandmarkData.LandmarkName, *Position.ToString());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Created %d landmarks for biome"), LandmarksCreated);
}

void UWorld_LandmarkSystem::ClearAllLandmarks()
{
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Clearing all landmarks"));
    
    // Destroy all active landmark actors
    for (AStaticMeshActor* LandmarkActor : ActiveLandmarks)
    {
        if (IsValid(LandmarkActor))
        {
            LandmarkActor->Destroy();
        }
    }
    
    // Clear arrays and maps
    ActiveLandmarks.Empty();
    LandmarkDataMap.Empty();
    TotalLandmarkCount = 0;
    VisibleLandmarkCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: All landmarks cleared"));
}

void UWorld_LandmarkSystem::RefreshLandmarkVisibility(const FVector& ViewerLocation)
{
    if (!GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastVisibilityUpdateTime < VisibilityUpdateInterval)
    {
        return; // Skip update to maintain performance
    }
    
    LastVisibilityUpdateTime = CurrentTime;
    VisibleLandmarkCount = 0;
    
    for (AStaticMeshActor* LandmarkActor : ActiveLandmarks)
    {
        if (!IsValid(LandmarkActor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(ViewerLocation, LandmarkActor->GetActorLocation());
        bool bShouldBeVisible = Distance <= MaxLandmarkRenderDistance;
        
        // Get landmark data for visibility range check
        if (LandmarkDataMap.Contains(LandmarkActor))
        {
            const FWorld_LandmarkData& LandmarkData = LandmarkDataMap[LandmarkActor];
            bShouldBeVisible = bShouldBeVisible && (Distance <= LandmarkData.VisibilityRange);
        }
        
        // Update visibility
        LandmarkActor->SetActorHiddenInGame(!bShouldBeVisible);
        
        if (bShouldBeVisible)
        {
            VisibleLandmarkCount++;
            
            // Update LOD if enabled
            if (bUseDistanceBasedLOD)
            {
                UStaticMeshComponent* MeshComp = LandmarkActor->GetStaticMeshComponent();
                if (MeshComp)
                {
                    float LODDistance = Distance / MaxLandmarkRenderDistance;
                    // Simple LOD: reduce quality at distance
                    if (LODDistance > 0.7f)
                    {
                        MeshComp->SetForcedLodModel(2); // Low LOD
                    }
                    else if (LODDistance > 0.4f)
                    {
                        MeshComp->SetForcedLodModel(1); // Medium LOD
                    }
                    else
                    {
                        MeshComp->SetForcedLodModel(0); // High LOD
                    }
                }
            }
        }
    }
}

AStaticMeshActor* UWorld_LandmarkSystem::SpawnLandmark(const FWorld_LandmarkData& LandmarkData)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("World_LandmarkSystem: No valid world for landmark spawning"));
        return nullptr;
    }

    // Find suitable ground position
    FVector SpawnLocation = FindSuitableGroundPosition(LandmarkData.Location, 200.0f);
    
    // Spawn static mesh actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* LandmarkActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        SpawnLocation,
        LandmarkData.Rotation,
        SpawnParams
    );
    
    if (!LandmarkActor)
    {
        UE_LOG(LogTemp, Error, TEXT("World_LandmarkSystem: Failed to spawn landmark actor"));
        return nullptr;
    }
    
    // Set scale
    LandmarkActor->SetActorScale3D(LandmarkData.Scale);
    
    // Set name
    LandmarkActor->SetActorLabel(LandmarkData.LandmarkName);
    
    // Setup mesh and material
    SetupLandmarkMeshAndMaterial(LandmarkActor, LandmarkData.LandmarkType);
    
    // Setup ambient audio if needed
    if (LandmarkData.bHasAmbientSound)
    {
        SetupLandmarkAmbientAudio(LandmarkActor, LandmarkData);
    }
    
    // Add to tracking arrays
    ActiveLandmarks.Add(LandmarkActor);
    LandmarkDataMap.Add(LandmarkActor, LandmarkData);
    TotalLandmarkCount++;
    
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Successfully spawned landmark %s"), 
           *LandmarkData.LandmarkName);
    
    return LandmarkActor;
}

TArray<FVector> UWorld_LandmarkSystem::GenerateLandmarkPositions(const FVector& BiomeCenter, float BiomeRadius, int32 LandmarkCount)
{
    TArray<FVector> Positions;
    
    // Use Poisson disk sampling for even distribution
    int32 MaxAttempts = LandmarkCount * 10; // Prevent infinite loops
    int32 Attempts = 0;
    
    while (Positions.Num() < LandmarkCount && Attempts < MaxAttempts)
    {
        Attempts++;
        
        // Generate random position within biome radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(SpawnSettings.BiomeEdgeBuffer, BiomeRadius - SpawnSettings.BiomeEdgeBuffer);
        
        FVector CandidatePosition = BiomeCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Check if position is valid
        if (IsValidLandmarkPosition(CandidatePosition, Positions))
        {
            Positions.Add(CandidatePosition);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Generated %d landmark positions in %d attempts"), 
           Positions.Num(), Attempts);
    
    return Positions;
}

EWorld_LandmarkType UWorld_LandmarkSystem::SelectLandmarkTypeForBiome(EBiomeType BiomeType)
{
    // Get available landmark types for this biome
    if (BiomeLandmarkTypes.Contains(BiomeType) && BiomeLandmarkTypes[BiomeType].Num() > 0)
    {
        const TArray<EWorld_LandmarkType>& AvailableTypes = BiomeLandmarkTypes[BiomeType];
        int32 RandomIndex = FMath::RandRange(0, AvailableTypes.Num() - 1);
        return AvailableTypes[RandomIndex];
    }
    
    // Default fallback
    return EWorld_LandmarkType::RockFormation;
}

TArray<AStaticMeshActor*> UWorld_LandmarkSystem::GetLandmarksInRadius(const FVector& Center, float Radius)
{
    TArray<AStaticMeshActor*> NearbyLandmarks;
    
    for (AStaticMeshActor* LandmarkActor : ActiveLandmarks)
    {
        if (IsValid(LandmarkActor))
        {
            float Distance = FVector::Dist(Center, LandmarkActor->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyLandmarks.Add(LandmarkActor);
            }
        }
    }
    
    return NearbyLandmarks;
}

AStaticMeshActor* UWorld_LandmarkSystem::GetNearestLandmark(const FVector& Location)
{
    AStaticMeshActor* NearestLandmark = nullptr;
    float NearestDistance = MAX_FLT;
    
    for (AStaticMeshActor* LandmarkActor : ActiveLandmarks)
    {
        if (IsValid(LandmarkActor))
        {
            float Distance = FVector::Dist(Location, LandmarkActor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestLandmark = LandmarkActor;
            }
        }
    }
    
    return NearestLandmark;
}

TArray<AStaticMeshActor*> UWorld_LandmarkSystem::GetLandmarksByType(EWorld_LandmarkType LandmarkType)
{
    TArray<AStaticMeshActor*> TypedLandmarks;
    
    for (auto& LandmarkPair : LandmarkDataMap)
    {
        if (LandmarkPair.Value.LandmarkType == LandmarkType && IsValid(LandmarkPair.Key))
        {
            TypedLandmarks.Add(LandmarkPair.Key);
        }
    }
    
    return TypedLandmarks;
}

void UWorld_LandmarkSystem::SetupLandmarkAmbientAudio(AStaticMeshActor* LandmarkActor, const FWorld_LandmarkData& LandmarkData)
{
    if (!IsValid(LandmarkActor) || !LandmarkData.bHasAmbientSound)
    {
        return;
    }
    
    // Create audio component
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(LandmarkActor);
    if (AudioComp)
    {
        // Set audio properties
        AudioComp->SetupAttachment(LandmarkActor->GetRootComponent());
        AudioComp->bAutoActivate = true;
        AudioComp->bShouldRemainActiveIfDropped = true;
        AudioComp->SetVolumeMultiplier(0.5f);
        AudioComp->SetPitchMultiplier(FMath::RandRange(0.9f, 1.1f));
        
        // Set attenuation
        AudioComp->AttenuationSettings.bAttenuate = true;
        AudioComp->AttenuationSettings.FalloffDistance = LandmarkData.AmbientSoundRadius;
        AudioComp->AttenuationSettings.AttenuationShape = EAttenuationShape::Sphere;
        
        // Try to load appropriate sound
        if (LandmarkAmbientSounds.Contains(LandmarkData.LandmarkType))
        {
            TSoftObjectPtr<USoundCue> SoundPtr = LandmarkAmbientSounds[LandmarkData.LandmarkType];
            if (SoundPtr.IsValid())
            {
                AudioComp->SetSound(SoundPtr.Get());
            }
        }
        
        // Register component
        AudioComp->RegisterComponent();
        
        UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Setup ambient audio for landmark %s"), 
               *LandmarkData.LandmarkName);
    }
}

void UWorld_LandmarkSystem::UpdateAmbientAudioForLandmarks(const FVector& ListenerLocation)
{
    // This could be used for dynamic audio updates based on listener position
    // For now, we rely on UE5's built-in audio attenuation
}

bool UWorld_LandmarkSystem::IsValidLandmarkPosition(const FVector& Position, const TArray<FVector>& ExistingPositions)
{
    // Check minimum distance to existing landmarks
    for (const FVector& ExistingPos : ExistingPositions)
    {
        float Distance = FVector::Dist(Position, ExistingPos);
        if (Distance < SpawnSettings.MinDistanceBetweenLandmarks)
        {
            return false;
        }
    }
    
    // Additional validation could include:
    // - Water body avoidance
    // - Terrain slope checking
    // - Biome boundary validation
    
    return true;
}

FVector UWorld_LandmarkSystem::FindSuitableGroundPosition(const FVector& StartPosition, float SearchRadius)
{
    if (!GetWorld())
    {
        return StartPosition;
    }
    
    // Perform line trace downward to find ground
    FVector TraceStart = StartPosition + FVector(0.0f, 0.0f, 1000.0f);
    FVector TraceEnd = StartPosition - FVector(0.0f, 0.0f, 1000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location;
    }
    
    // Fallback to original position
    return StartPosition;
}

void UWorld_LandmarkSystem::SetupLandmarkMeshAndMaterial(AStaticMeshActor* LandmarkActor, EWorld_LandmarkType LandmarkType)
{
    if (!IsValid(LandmarkActor))
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = LandmarkActor->GetStaticMeshComponent();
    if (!MeshComp)
    {
        return;
    }
    
    // Try to load appropriate mesh
    if (LandmarkMeshes.Contains(LandmarkType))
    {
        TSoftObjectPtr<UStaticMesh> MeshPtr = LandmarkMeshes[LandmarkType];
        if (MeshPtr.IsValid())
        {
            MeshComp->SetStaticMesh(MeshPtr.Get());
        }
    }
    
    // Try to load appropriate material
    if (LandmarkMaterials.Contains(LandmarkType))
    {
        TSoftObjectPtr<UMaterialInterface> MaterialPtr = LandmarkMaterials[LandmarkType];
        if (MaterialPtr.IsValid())
        {
            MeshComp->SetMaterial(0, MaterialPtr.Get());
        }
    }
    
    // Enable collision
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
}

void UWorld_LandmarkSystem::InitializeBiomeLandmarkMappings()
{
    // Clear existing mappings
    BiomeLandmarkTypes.Empty();
    
    // Forest biome landmarks
    TArray<EWorld_LandmarkType> ForestLandmarks;
    ForestLandmarks.Add(EWorld_LandmarkType::AncientTree);
    ForestLandmarks.Add(EWorld_LandmarkType::RockFormation);
    ForestLandmarks.Add(EWorld_LandmarkType::Boulder);
    BiomeLandmarkTypes.Add(EBiomeType::Forest, ForestLandmarks);
    
    // Mountain biome landmarks
    TArray<EWorld_LandmarkType> MountainLandmarks;
    MountainLandmarks.Add(EWorld_LandmarkType::CliffFace);
    MountainLandmarks.Add(EWorld_LandmarkType::RockFormation);
    MountainLandmarks.Add(EWorld_LandmarkType::NaturalArch);
    MountainLandmarks.Add(EWorld_LandmarkType::CaveEntrance);
    BiomeLandmarkTypes.Add(EBiomeType::Mountain, MountainLandmarks);
    
    // Desert biome landmarks
    TArray<EWorld_LandmarkType> DesertLandmarks;
    DesertLandmarks.Add(EWorld_LandmarkType::RockFormation);
    DesertLandmarks.Add(EWorld_LandmarkType::NaturalArch);
    DesertLandmarks.Add(EWorld_LandmarkType::Boulder);
    BiomeLandmarkTypes.Add(EBiomeType::Desert, DesertLandmarks);
    
    // Plains biome landmarks
    TArray<EWorld_LandmarkType> PlainsLandmarks;
    PlainsLandmarks.Add(EWorld_LandmarkType::AncientTree);
    PlainsLandmarks.Add(EWorld_LandmarkType::Boulder);
    PlainsLandmarks.Add(EWorld_LandmarkType::RockFormation);
    BiomeLandmarkTypes.Add(EBiomeType::Plains, PlainsLandmarks);
    
    // Swamp biome landmarks
    TArray<EWorld_LandmarkType> SwampLandmarks;
    SwampLandmarks.Add(EWorld_LandmarkType::AncientTree);
    SwampLandmarks.Add(EWorld_LandmarkType::GeyserSite);
    BiomeLandmarkTypes.Add(EBiomeType::Swamp, SwampLandmarks);
    
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Initialized biome-landmark type mappings"));
}

void UWorld_LandmarkSystem::InitializeDefaultAssetReferences()
{
    // Initialize default mesh references (these would be set in Blueprint or loaded from content)
    // For now, we'll leave them empty and rely on Blueprint setup
    
    UE_LOG(LogTemp, Log, TEXT("World_LandmarkSystem: Default asset references initialized"));
}