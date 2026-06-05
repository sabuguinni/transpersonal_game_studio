#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousPerformancePropSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousPropData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    bool bIsStorytellingProp;

    FEnvArt_CretaceousPropData()
    {
        PropName = TEXT("DefaultProp");
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        LODDistance = 5000.0f;
        bIsStorytellingProp = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_PerformanceCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVector ClusterCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPropsInCluster;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseInstancedRendering;

    FEnvArt_PerformanceCluster()
    {
        ClusterCenter = FVector::ZeroVector;
        ClusterRadius = 1000.0f;
        MaxPropsInCluster = 50;
        LODDistanceMultiplier = 1.0f;
        bUseInstancedRendering = true;
    }
};

UENUM(BlueprintType)
enum class EEnvArt_PropType : uint8
{
    FallenLog       UMETA(DisplayName = "Fallen Log"),
    Boulder         UMETA(DisplayName = "Boulder"),
    AncientFern     UMETA(DisplayName = "Ancient Fern"),
    WeatheredRock   UMETA(DisplayName = "Weathered Rock"),
    MossStone       UMETA(DisplayName = "Moss Covered Stone"),
    DinosaurBones   UMETA(DisplayName = "Dinosaur Bones"),
    NestSite        UMETA(DisplayName = "Nest Site"),
    FeedingGround   UMETA(DisplayName = "Feeding Ground"),
    WaterSource     UMETA(DisplayName = "Water Source")
};

UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_CretaceousPerformancePropSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_CretaceousPerformancePropSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Prop Management
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Props")
    void InitializePropSystem();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Props")
    void SpawnCretaceousProps();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Props")
    AActor* SpawnPropAtLocation(EEnvArt_PropType PropType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Props")
    void RemovePropAtLocation(const FVector& Location, float SearchRadius = 100.0f);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePropLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CreatePerformanceClusters();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePropVisibility(const FVector& ViewerLocation);

    // Storytelling Props
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void SpawnStorytellingProps();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateEnvironmentalNarrative(const FVector& Location, EEnvArt_PropType StoryType);

    // Validation and Debug
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidatePropDistribution();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void GeneratePerformanceReport();

protected:
    // Prop Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Configuration")
    TArray<FEnvArt_CretaceousPropData> PropDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Configuration")
    TArray<FEnvArt_PerformanceCluster> PerformanceClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Configuration")
    float MaxPropSpawnDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Configuration")
    int32 MaxTotalProps;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseInstancedStaticMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusion;

    // Storytelling Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StorytellingPropDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bEnableEnvironmentalNarrative;

    // Runtime Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedProps;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> StorytellingProps;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    float LastLODUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 CurrentPropCount;

private:
    // Internal Helper Functions
    void LoadPropMeshes();
    void SetupInstancedMeshComponents();
    FVector GetRandomLocationInCluster(const FEnvArt_PerformanceCluster& Cluster);
    bool IsLocationValidForProp(const FVector& Location, EEnvArt_PropType PropType);
    void UpdatePropLOD(AActor* PropActor, float DistanceToViewer);
    void CreatePropCluster(const FEnvArt_PerformanceCluster& ClusterData);

    // Mesh References
    UPROPERTY()
    TMap<EEnvArt_PropType, UStaticMesh*> PropMeshes;

    UPROPERTY()
    TMap<EEnvArt_PropType, UHierarchicalInstancedStaticMeshComponent*> InstancedMeshComponents;

    // Performance Tracking
    float AccumulatedDeltaTime;
    int32 PropsUpdatedThisFrame;
    static constexpr int32 MaxPropsPerFrameUpdate = 10;
};