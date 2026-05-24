#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_LandmarkSystem.generated.h"

// Forward declarations
class AStaticMeshActor;
class UMaterialInstance;
class USoundCue;

UENUM(BlueprintType)
enum class EWorld_LandmarkType : uint8
{
    AncientTree         UMETA(DisplayName = "Ancient Tree"),
    RockFormation       UMETA(DisplayName = "Rock Formation"),
    Waterfall           UMETA(DisplayName = "Waterfall"),
    CaveEntrance        UMETA(DisplayName = "Cave Entrance"),
    CliffFace           UMETA(DisplayName = "Cliff Face"),
    Boulder             UMETA(DisplayName = "Boulder"),
    NaturalArch         UMETA(DisplayName = "Natural Arch"),
    GeyserSite          UMETA(DisplayName = "Geyser Site")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandmarkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    EWorld_LandmarkType LandmarkType = EWorld_LandmarkType::RockFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    float VisibilityRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    bool bHasAmbientSound = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    float AmbientSoundRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    bool bIsNavigationLandmark = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FString LandmarkName = TEXT("Unnamed Landmark");

    FWorld_LandmarkData()
    {
        LandmarkType = EWorld_LandmarkType::RockFormation;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        VisibilityRange = 5000.0f;
        bHasAmbientSound = false;
        AmbientSoundRadius = 1000.0f;
        bIsNavigationLandmark = true;
        LandmarkName = TEXT("Unnamed Landmark");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandmarkSpawnSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    int32 MaxLandmarksPerBiome = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float MinDistanceBetweenLandmarks = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float BiomeEdgeBuffer = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    bool bAvoidWaterBodies = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float WaterAvoidanceRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    bool bPreferElevatedPositions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float ElevationPreferenceBias = 0.7f;

    FWorld_LandmarkSpawnSettings()
    {
        MaxLandmarksPerBiome = 15;
        MinDistanceBetweenLandmarks = 2000.0f;
        BiomeEdgeBuffer = 500.0f;
        bAvoidWaterBodies = true;
        WaterAvoidanceRadius = 800.0f;
        bPreferElevatedPositions = true;
        ElevationPreferenceBias = 0.7f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_LandmarkSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_LandmarkSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core landmark management
    UFUNCTION(BlueprintCallable, Category = "Landmark System")
    void InitializeLandmarkSystem();

    UFUNCTION(BlueprintCallable, Category = "Landmark System")
    void GenerateLandmarksForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Landmark System")
    void ClearAllLandmarks();

    UFUNCTION(BlueprintCallable, Category = "Landmark System")
    void RefreshLandmarkVisibility(const FVector& ViewerLocation);

    // Landmark spawning
    UFUNCTION(BlueprintCallable, Category = "Landmark Spawning")
    AStaticMeshActor* SpawnLandmark(const FWorld_LandmarkData& LandmarkData);

    UFUNCTION(BlueprintCallable, Category = "Landmark Spawning")
    TArray<FVector> GenerateLandmarkPositions(const FVector& BiomeCenter, float BiomeRadius, int32 LandmarkCount);

    UFUNCTION(BlueprintCallable, Category = "Landmark Spawning")
    EWorld_LandmarkType SelectLandmarkTypeForBiome(EBiomeType BiomeType);

    // Landmark queries
    UFUNCTION(BlueprintCallable, Category = "Landmark Queries")
    TArray<AStaticMeshActor*> GetLandmarksInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Landmark Queries")
    AStaticMeshActor* GetNearestLandmark(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Landmark Queries")
    TArray<AStaticMeshActor*> GetLandmarksByType(EWorld_LandmarkType LandmarkType);

    // Audio integration
    UFUNCTION(BlueprintCallable, Category = "Landmark Audio")
    void SetupLandmarkAmbientAudio(AStaticMeshActor* LandmarkActor, const FWorld_LandmarkData& LandmarkData);

    UFUNCTION(BlueprintCallable, Category = "Landmark Audio")
    void UpdateAmbientAudioForLandmarks(const FVector& ListenerLocation);

protected:
    // Landmark data storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landmark System")
    TArray<AStaticMeshActor*> ActiveLandmarks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landmark System")
    TMap<AStaticMeshActor*, FWorld_LandmarkData> LandmarkDataMap;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FWorld_LandmarkSpawnSettings SpawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EBiomeType, TArray<EWorld_LandmarkType>> BiomeLandmarkTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EWorld_LandmarkType, TSoftObjectPtr<UStaticMesh>> LandmarkMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EWorld_LandmarkType, TSoftObjectPtr<UMaterialInterface>> LandmarkMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EWorld_LandmarkType, TSoftObjectPtr<USoundCue>> LandmarkAmbientSounds;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TotalLandmarkCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 VisibleLandmarkCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastVisibilityUpdateTime = 0.0f;

private:
    // Internal helpers
    bool IsValidLandmarkPosition(const FVector& Position, const TArray<FVector>& ExistingPositions);
    FVector FindSuitableGroundPosition(const FVector& StartPosition, float SearchRadius);
    void SetupLandmarkMeshAndMaterial(AStaticMeshActor* LandmarkActor, EWorld_LandmarkType LandmarkType);
    void InitializeBiomeLandmarkMappings();
    void InitializeDefaultAssetReferences();

    // Performance optimization
    float VisibilityUpdateInterval = 2.0f;
    float MaxLandmarkRenderDistance = 10000.0f;
    bool bUseDistanceBasedLOD = true;
};