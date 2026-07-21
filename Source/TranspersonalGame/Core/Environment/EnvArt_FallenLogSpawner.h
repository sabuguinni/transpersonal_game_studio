#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "SharedTypes.h"
#include "EnvArt_FallenLogSpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LogVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Variation")
    FString VariationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Variation")
    float Length;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Variation")
    float Diameter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Variation")
    float DecayLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Variation")
    bool bHasMoss;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Variation")
    bool bHasLichen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Variation")
    TArray<FString> BiomeCompatibility;

    FEnvArt_LogVariation()
    {
        VariationName = TEXT("DefaultLog");
        Length = 400.0f;
        Diameter = 50.0f;
        DecayLevel = 0.5f;
        bHasMoss = true;
        bHasLichen = false;
        BiomeCompatibility = {TEXT("Forest"), TEXT("Swamp")};
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LogCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Cluster")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Cluster")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Cluster")
    int32 LogCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Cluster")
    TArray<FEnvArt_LogVariation> LogVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Cluster")
    bool bNaturalArrangement;

    FEnvArt_LogCluster()
    {
        CenterLocation = FVector::ZeroVector;
        ClusterRadius = 1000.0f;
        LogCount = 5;
        bNaturalArrangement = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_FallenLogSpawner : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_FallenLogSpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Log Spawning")
    TArray<FEnvArt_LogVariation> AvailableLogTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Log Spawning")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Log Spawning")
    int32 MaxLogsPerCluster;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Log Spawning")
    float MinDistanceBetweenLogs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Log Spawning")
    bool bAlignToTerrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Log Spawning")
    bool bRandomRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Log Spawning")
    FString CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallen Log Spawning")
    TArray<FEnvArt_LogCluster> SpawnedClusters;

    UFUNCTION(BlueprintCallable, Category = "Fallen Log Spawning")
    void SpawnLogCluster(const FVector& Location, const FString& BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Fallen Log Spawning")
    void SpawnSingleLog(const FVector& Location, const FEnvArt_LogVariation& LogType);

    UFUNCTION(BlueprintCallable, Category = "Fallen Log Spawning")
    void ClearAllLogs();

    UFUNCTION(BlueprintCallable, Category = "Fallen Log Spawning")
    FEnvArt_LogVariation GetRandomLogForBiome(const FString& BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Fallen Log Spawning")
    bool IsValidSpawnLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Fallen Log Spawning")
    FVector GetTerrainAlignedPosition(const FVector& BaseLocation);

    UFUNCTION(BlueprintCallable, Category = "Fallen Log Spawning")
    void InitializeLogVariations();

    UFUNCTION(BlueprintCallable, Category = "Fallen Log Spawning")
    void UpdateLogDecay(float DeltaTime);

protected:
    UPROPERTY()
    TArray<AActor*> SpawnedLogActors;

    UPROPERTY()
    UStaticMesh* DefaultLogMesh;

    UPROPERTY()
    UMaterialInterface* LogMaterial;

    UPROPERTY()
    UMaterialInterface* MossyLogMaterial;

    UPROPERTY()
    UMaterialInterface* LichenLogMaterial;

private:
    void LoadLogAssets();
    FVector CalculateLogPlacement(const FVector& ClusterCenter, int32 LogIndex, int32 TotalLogs);
    FRotator CalculateLogRotation(const FVector& Location, bool bAlignToSlope);
    void ApplyLogMaterial(UStaticMeshComponent* MeshComp, const FEnvArt_LogVariation& LogType);
};