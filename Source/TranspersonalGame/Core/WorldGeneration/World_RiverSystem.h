#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_RiverSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRiverSystem, Log, All);

/**
 * Data structure for river segment configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverSegmentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    FVector EndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    EWorld_BiomeType BiomeType;

    FWorld_RiverSegmentData()
    {
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector::ZeroVector;
        Width = 500.0f;
        Depth = 100.0f;
        FlowSpeed = 1.0f;
        BiomeType = EWorld_BiomeType::Savanna;
    }
};

/**
 * River system configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverSystemConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    int32 MaxRiverSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float MinRiverWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float MaxRiverWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float RiverDepthMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    bool bGenerateRiverBanks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    bool bGenerateWaterfalls;

    FWorld_RiverSystemConfig()
    {
        MaxRiverSegments = 50;
        MinRiverWidth = 300.0f;
        MaxRiverWidth = 1000.0f;
        RiverDepthMultiplier = 0.2f;
        bGenerateRiverBanks = true;
        bGenerateWaterfalls = false;
    }
};

/**
 * World River System Component
 * Generates and manages realistic river networks across biomes
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_RiverSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_RiverSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // River generation functions
    UFUNCTION(BlueprintCallable, Category = "River System")
    void GenerateRiverNetwork();

    UFUNCTION(BlueprintCallable, Category = "River System")
    void GenerateRiverInBiome(EWorld_BiomeType BiomeType, const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "River System")
    void CreateRiverSegment(const FWorld_RiverSegmentData& SegmentData);

    UFUNCTION(BlueprintCallable, Category = "River System")
    void ClearAllRivers();

    // River configuration
    UFUNCTION(BlueprintCallable, Category = "River System")
    void SetRiverSystemConfig(const FWorld_RiverSystemConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "River System")
    FWorld_RiverSystemConfig GetRiverSystemConfig() const { return RiverConfig; }

    // River queries
    UFUNCTION(BlueprintCallable, Category = "River System")
    bool IsLocationNearRiver(const FVector& Location, float Threshold = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "River System")
    FVector GetNearestRiverLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "River System")
    TArray<FWorld_RiverSegmentData> GetRiverSegmentsInRadius(const FVector& Center, float Radius) const;

protected:
    // River system configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    FWorld_RiverSystemConfig RiverConfig;

    // Generated river data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "River System")
    TArray<FWorld_RiverSegmentData> RiverSegments;

    // River actor references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "River System")
    TArray<AStaticMeshActor*> RiverActors;

    // River generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float RiverGenerationSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float RiverFlowAnimationSpeed;

private:
    // Internal river generation functions
    void GenerateMainRivers();
    void GenerateTributaries();
    void GenerateRiverBanks(const FWorld_RiverSegmentData& SegmentData);
    void ConnectRiverSegments();
    
    // River path calculation
    TArray<FVector> CalculateRiverPath(const FVector& Start, const FVector& End, EWorld_BiomeType BiomeType);
    FVector CalculateRiverDirection(const FVector& CurrentLocation, const FVector& TargetLocation, EWorld_BiomeType BiomeType);
    
    // River mesh creation
    void CreateRiverMesh(const FWorld_RiverSegmentData& SegmentData);
    void UpdateRiverMaterial(AStaticMeshActor* RiverActor, EWorld_BiomeType BiomeType);
    
    // Biome-specific river properties
    FWorld_RiverSegmentData GetBiomeRiverProperties(EWorld_BiomeType BiomeType) const;
    
    // River network validation
    bool ValidateRiverPlacement(const FVector& Location) const;
    void OptimizeRiverNetwork();
    
    // Performance management
    void UpdateRiverLOD();
    void CullDistantRivers();
    
    // River system state
    bool bRiverSystemInitialized;
    float LastRiverUpdateTime;
    int32 ActiveRiverCount;
};