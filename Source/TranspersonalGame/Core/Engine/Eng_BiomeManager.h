#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurTypes;

    FEng_BiomeData()
    {
        BiomeName = TEXT("Unknown");
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 15000.0f;
        BiomeType = EBiomeType::Savanna;
        Temperature = 25.0f;
        Humidity = 50.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FEng_BiomeData> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bAutoInitializeBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bDebugVisualization;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeDefaultBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeTypeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeDataAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FVector> GetDistributedSpawnLocations(int32 TotalCount) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsDinosaurAllowedInBiome(const FString& DinosaurType, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void AddBiome(const FEng_BiomeData& NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RemoveBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeData(EBiomeType BiomeType, const FEng_BiomeData& UpdatedData);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintBiomeInfo();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void CreateBiomeVisualizationActors();

private:
    void SetupDefaultBiomeData();
    float CalculateDistanceToNearestBiome(const FVector& Location) const;
    int32 FindBiomeIndex(EBiomeType BiomeType) const;
};