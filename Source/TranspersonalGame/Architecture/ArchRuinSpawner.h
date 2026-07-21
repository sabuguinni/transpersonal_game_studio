#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchRuinSpawner.generated.h"

UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
    Pillar      UMETA(DisplayName = "Stone Pillar"),
    WallSection UMETA(DisplayName = "Wall Section"),
    Archway     UMETA(DisplayName = "Archway"),
    Boulder     UMETA(DisplayName = "Mossy Boulder"),
    Foundation  UMETA(DisplayName = "Foundation Slab")
};

USTRUCT(BlueprintType)
struct FArch_RuinConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_RuinType RuinType = EArch_RuinType::Pillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector SpawnScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MossIntensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringAmount = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasVines = true;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AArchRuinSpawner : public AActor
{
    GENERATED_BODY()

public:
    AArchRuinSpawner();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    TArray<FArch_RuinConfig> RuinConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    float SpawnRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    int32 MaxRuinCount = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    bool bScatterRandomly = true;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruins", CallInEditor)
    void SpawnRuinCluster();

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruins", CallInEditor)
    void ClearAllRuins();

    UFUNCTION(BlueprintPure, Category = "Architecture|Ruins")
    int32 GetActiveRuinCount() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedRuins;
};
