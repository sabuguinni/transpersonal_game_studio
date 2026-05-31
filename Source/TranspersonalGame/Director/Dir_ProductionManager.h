#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Dir_ProductionManager.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    PrototypeDevelopment,
    VerticalSlice,
    Production,
    Polish,
    GoldMaster
};

UENUM(BlueprintType)
enum class EDir_BiomeType : uint8
{
    Savana,
    Pantano,
    Floresta,
    Deserto,
    Montanha
};

USTRUCT(BlueprintType)
struct FDir_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EDir_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActors;

    FDir_BiomeData()
    {
        BiomeType = EDir_BiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        MaxActors = 1000;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float PerformanceScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_ProductionPhase CurrentPhase;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        TotalDinosaurs = 0;
        CompletedSystems = 0;
        PerformanceScore = 100.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
    }
};

/**
 * Production Manager Component - Oversees game development metrics and biome management
 * Ensures actor limits are respected and biome distribution is balanced
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_BiomeData> BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    int32 MaxTotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    int32 MaxDinosaurs;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanSpawnActorInBiome(EDir_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FVector GetRandomLocationInBiome(EDir_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CleanupExcessActors();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const { return ProductionMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_BiomeData> GetBiomeData() const { return BiomeData; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void ValidateActorLimits();

private:
    void InitializeBiomeData();
    void CountActorsInBiomes();
    bool IsActorInBiome(AActor* Actor, const FDir_BiomeData& Biome);
};