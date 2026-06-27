#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CretaceousRuinActor.generated.h"

UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
    WallSegment     UMETA(DisplayName = "Wall Segment"),
    Pillar          UMETA(DisplayName = "Pillar"),
    Archway         UMETA(DisplayName = "Archway"),
    BoulderCluster  UMETA(DisplayName = "Boulder Cluster"),
    RuinedFloor     UMETA(DisplayName = "Ruined Floor")
};

USTRUCT(BlueprintType)
struct FArch_RuinData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    EArch_RuinType RuinType = EArch_RuinType::WallSegment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    bool bIsCollapsed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    float CollapseAngle = 0.0f;
};

/**
 * ACretaceousRuinActor
 * Represents ancient Cretaceous-era stone ruins in the prehistoric world.
 * These ruins are environmental storytelling props — weathered limestone
 * structures that hint at a pre-human geological past.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Ruin Actor"))
class TRANSPERSONALGAME_API ACretaceousRuinActor : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousRuinActor();

    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    FArch_RuinData RuinData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture|Components",
        meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* RuinMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture|Components",
        meta = (AllowPrivateAccess = "true"))
    class UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    float InspectionRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    FText RuinDescription;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruin")
    void ApplyWeathering(float WeatherAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruin")
    void SetRuinType(EArch_RuinType NewType);

    UFUNCTION(BlueprintPure, Category = "Architecture|Ruin")
    FArch_RuinData GetRuinData() const { return RuinData; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Ruin")
    void RandomizeWeathering();

private:
    void UpdateMeshForRuinType();
};
