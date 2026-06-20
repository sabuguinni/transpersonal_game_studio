#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchRuinSpawner.generated.h"

UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
    Pillar      UMETA(DisplayName = "Stone Pillar"),
    Wall        UMETA(DisplayName = "Ruined Wall"),
    Altar       UMETA(DisplayName = "Stone Altar"),
    Archway     UMETA(DisplayName = "Collapsed Archway"),
    Foundation  UMETA(DisplayName = "Foundation Slab")
};

USTRUCT(BlueprintType)
struct FArch_RuinElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_RuinType RuinType = EArch_RuinType::Pillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector RelativeOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float RotationYaw = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_RuinSpawner : public AActor
{
    GENERATED_BODY()

public:
    AArch_RuinSpawner();

    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    TArray<FArch_RuinElement> RuinElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    EArch_RuinType PrimaryRuinType = EArch_RuinType::Pillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    float RuinRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    int32 PillarCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    bool bHasCentralAltar = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    float MossIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    float WeatheringAmount = 0.7f;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruins")
    void SpawnRuinFormation();

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruins")
    void ClearRuinFormation();

    UFUNCTION(BlueprintPure, Category = "Architecture|Ruins")
    FVector GetAltarLocation() const;

    UFUNCTION(BlueprintPure, Category = "Architecture|Ruins")
    bool IsRuinComplete() const;

private:
    UPROPERTY()
    TArray<UStaticMeshComponent*> SpawnedComponents;

    void SetupPillarComponent(UStaticMeshComponent* Comp, const FArch_RuinElement& Element);
};
