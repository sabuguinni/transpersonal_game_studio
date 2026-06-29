#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "ArchRuinStructure.generated.h"

/**
 * Cretaceous-era stone ruin structure for the prehistoric survival world.
 * Represents ancient limestone ruins with moss, ferns, and weathered blocks.
 * Agent #7 — Architecture & Interior Agent
 */

UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
    WallSegment     UMETA(DisplayName = "Wall Segment"),
    Archway         UMETA(DisplayName = "Archway"),
    Column          UMETA(DisplayName = "Column"),
    Platform        UMETA(DisplayName = "Platform"),
    Rubble          UMETA(DisplayName = "Rubble Pile"),
    Altar           UMETA(DisplayName = "Stone Altar")
};

UENUM(BlueprintType)
enum class EArch_WeatherState : uint8
{
    Fresh           UMETA(DisplayName = "Fresh Stone"),
    Mossy           UMETA(DisplayName = "Moss Covered"),
    Overgrown       UMETA(DisplayName = "Heavily Overgrown"),
    Collapsed       UMETA(DisplayName = "Partially Collapsed")
};

USTRUCT(BlueprintType)
struct FArch_RuinConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    EArch_RuinType RuinType = EArch_RuinType::WallSegment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    EArch_WeatherState WeatherState = EArch_WeatherState::Mossy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    float StructuralIntegrity = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    bool bCanCollapse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruin")
    float CollapseThreshold = 0.2f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchRuinStructure : public AActor
{
    GENERATED_BODY()

public:
    AArchRuinStructure();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === COMPONENTS ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* PrimaryMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* RubbleMesh;

    // === CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    FArch_RuinConfig RuinConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    float CurrentIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
    bool bIsCollapsed;

    // === FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruin")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruin")
    void CollapseStructure();

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruin")
    float GetStructuralIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruin")
    EArch_RuinType GetRuinType() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Ruin")
    void SetWeatherState(EArch_WeatherState NewState);

    UFUNCTION(CallInEditor, Category = "Architecture|Debug")
    void DebugPrintRuinInfo();
};
