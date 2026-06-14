#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UMaterialInterface>> TerrainMaterials;

    FWorld_BiomeData()
    {
        BiomeName = TEXT("Default");
        BiomeColor = FLinearColor::White;
        Temperature = 20.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeTransitions();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FString> GetAvailableBiomes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeAtLocation(const FVector& WorldLocation, const FString& BiomeName);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TMap<FString, FWorld_BiomeData> BiomeDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    int32 BiomeResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bEnableBiomeTransitions;

private:
    void CreateDefaultBiomes();
    FWorld_BiomeData BlendBiomes(const FWorld_BiomeData& BiomeA, const FWorld_BiomeData& BiomeB, float BlendFactor);
    FString GetDominantBiome(const FVector& WorldLocation);
};