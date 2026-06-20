#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arch_StoneRuinActor.generated.h"

UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
    Pillar      UMETA(DisplayName = "Stone Pillar"),
    Wall        UMETA(DisplayName = "Ruin Wall"),
    Archway     UMETA(DisplayName = "Archway"),
    Foundation  UMETA(DisplayName = "Foundation"),
    Rubble      UMETA(DisplayName = "Rubble Pile")
};

USTRUCT(BlueprintType)
struct FArch_RuinConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    EArch_RuinType RuinType = EArch_RuinType::Pillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    bool bHasFernGrowth = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    float HeightScale = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StoneRuinActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_StoneRuinActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* RuinMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin Configuration")
    FArch_RuinConfig RuinConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin Configuration")
    bool bCastsDynamicShadow = true;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float Level);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EArch_RuinType GetRuinType() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetRuinExtents() const;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    void SetupMeshForRuinType();
};
