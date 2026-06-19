#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchRuinPillarActor.generated.h"

UENUM(BlueprintType)
enum class EArch_RuinState : uint8
{
    Intact       UMETA(DisplayName = "Intact"),
    Cracked      UMETA(DisplayName = "Cracked"),
    HalfBuried   UMETA(DisplayName = "Half Buried"),
    Collapsed    UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct FArch_RuinData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    EArch_RuinState RuinState = EArch_RuinState::HalfBuried;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    float WeatheringLevel = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    float MossCoverage = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    bool bHasVegetationGrowth = true;
};

/**
 * AArchRuinPillarActor — Cretaceous-era stone ruin pillar
 * Prehistoric architectural remnant placed in the world as environmental storytelling.
 * Represents ancient geological formations or early hominid stone structures.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Arch Ruin Pillar Actor"))
class TRANSPERSONALGAME_API AArchRuinPillarActor : public AActor
{
    GENERATED_BODY()

public:
    AArchRuinPillarActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture|Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* PillarMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture|Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    FArch_RuinData RuinData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Placement")
    float BiomeX = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Placement")
    float BiomeY = 50000.0f;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruin")
    void ApplyWeathering(float WeatherAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruin")
    EArch_RuinState GetRuinState() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Debug")
    void LogRuinStatus();
};
