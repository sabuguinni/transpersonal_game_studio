#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "WorldGenerationTypes.h"
#include "World_RiverSystemGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Node")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Node")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Node")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Node")
    float FlowRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Node")
    int32 NodeIndex;

    FWorld_RiverNode()
    {
        Location = FVector::ZeroVector;
        Width = 500.0f;
        Depth = 100.0f;
        FlowRate = 1.0f;
        NodeIndex = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverPath
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Path")
    TArray<FWorld_RiverNode> RiverNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Path")
    EWorld_BiomeType SourceBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Path")
    EWorld_BiomeType TargetBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Path")
    float TotalLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Path")
    FString RiverName;

    FWorld_RiverPath()
    {
        SourceBiome = EWorld_BiomeType::Savana;
        TargetBiome = EWorld_BiomeType::Savana;
        TotalLength = 0.0f;
        RiverName = TEXT("UnnamedRiver");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverSystemConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    int32 MaxRiversPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float MinRiverLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float MaxRiverLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float RiverMeandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    float BridgeSpawnChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    bool bGenerateRiverBanks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    bool bGenerateRiverVegetation;

    FWorld_RiverSystemConfig()
    {
        MaxRiversPerBiome = 3;
        MinRiverLength = 5000.0f;
        MaxRiverLength = 25000.0f;
        RiverMeandering = 0.3f;
        BridgeSpawnChance = 0.2f;
        bGenerateRiverBanks = true;
        bGenerateRiverVegetation = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_RiverSystemGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_RiverSystemGenerator();

protected:
    virtual void BeginPlay() override;

public:
    // River system configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River System")
    FWorld_RiverSystemConfig RiverSystemConfig;

    // Auto-generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bConnectBiomes;

    // Generated river data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generated Data")
    TArray<FWorld_RiverPath> GeneratedRivers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generated Data")
    TArray<AStaticMeshActor*> RiverActors;

    // Main generation functions
    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void GenerateRiverSystemForAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void GenerateRiverSystemForBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    FWorld_RiverPath GenerateRiverPath(const FVector& StartLocation, const FVector& EndLocation, EWorld_BiomeType SourceBiome, EWorld_BiomeType TargetBiome);

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void CreateRiverActorsFromPath(const FWorld_RiverPath& RiverPath);

    // River path calculation
    UFUNCTION(BlueprintCallable, Category = "River Calculation")
    TArray<FWorld_RiverNode> CalculateRiverNodes(const FVector& StartLocation, const FVector& EndLocation, int32 NodeCount);

    UFUNCTION(BlueprintCallable, Category = "River Calculation")
    FVector CalculateRiverMeandering(const FVector& BaseDirection, float MeanderingFactor, int32 NodeIndex);

    UFUNCTION(BlueprintCallable, Category = "River Calculation")
    float CalculateRiverWidth(const FWorld_RiverNode& Node, float BaseWidth, float DistanceFromSource);

    // Inter-biome connections
    UFUNCTION(BlueprintCallable, Category = "Biome Connections")
    void GenerateInterBiomeRivers();

    UFUNCTION(BlueprintCallable, Category = "Biome Connections")
    TArray<FVector> GetBiomeBorderPoints(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Connections")
    FVector FindNearestBiomeBorder(const FVector& Location, EWorld_BiomeType TargetBiome);

    // River features
    UFUNCTION(BlueprintCallable, Category = "River Features")
    void GenerateRiverBanks(const FWorld_RiverPath& RiverPath);

    UFUNCTION(BlueprintCallable, Category = "River Features")
    void GenerateRiverVegetation(const FWorld_RiverPath& RiverPath);

    UFUNCTION(BlueprintCallable, Category = "River Features")
    void GenerateRiverBridges(const FWorld_RiverPath& RiverPath);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FVector GetBiomeCenterLocation(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetBiomeRadius() const { return 15000.0f; }

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FString GenerateRiverName(EWorld_BiomeType SourceBiome, EWorld_BiomeType TargetBiome) const;

    // Cleanup functions
    UFUNCTION(BlueprintCallable, Category = "Management")
    void ClearAllRivers();

    UFUNCTION(BlueprintCallable, Category = "Management")
    void RegenerateRiverSystem();

private:
    // Internal helper functions
    AStaticMeshActor* CreateRiverSegmentActor(const FWorld_RiverNode& StartNode, const FWorld_RiverNode& EndNode);
    void ConfigureRiverSegmentMesh(AStaticMeshActor* RiverActor, const FWorld_RiverNode& StartNode, const FWorld_RiverNode& EndNode);
    FVector CalculateSegmentScale(const FWorld_RiverNode& StartNode, const FWorld_RiverNode& EndNode);
    FRotator CalculateSegmentRotation(const FWorld_RiverNode& StartNode, const FWorld_RiverNode& EndNode);
};