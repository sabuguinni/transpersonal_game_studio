#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchRuinStructure.generated.h"

UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
    WallSegment     UMETA(DisplayName = "Wall Segment"),
    Pillar          UMETA(DisplayName = "Pillar"),
    Archway         UMETA(DisplayName = "Archway"),
    Foundation      UMETA(DisplayName = "Foundation"),
    RubblePile      UMETA(DisplayName = "Rubble Pile"),
    Doorframe       UMETA(DisplayName = "Doorframe")
};

UENUM(BlueprintType)
enum class EArch_RuinMaterial : uint8
{
    Limestone       UMETA(DisplayName = "Limestone"),
    Sandstone       UMETA(DisplayName = "Sandstone"),
    VolcanicRock    UMETA(DisplayName = "Volcanic Rock"),
    MudBrick        UMETA(DisplayName = "Mud Brick")
};

USTRUCT(BlueprintType)
struct FArch_RuinConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Ruin")
    EArch_RuinType RuinType = EArch_RuinType::WallSegment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Ruin")
    EArch_RuinMaterial MaterialType = EArch_RuinMaterial::Limestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Ruin")
    float DegradationLevel = 0.5f;  // 0=pristine, 1=rubble

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Ruin")
    float MossCoverage = 0.7f;      // 0=bare, 1=fully covered

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Ruin")
    bool bHasVegetationGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Ruin")
    FVector StructureScale = FVector(1.0f, 1.0f, 1.0f);
};

/**
 * AArchRuinStructure
 * Cretaceous-era architectural ruin structure for the prehistoric survival game.
 * Represents ancient stone constructions overgrown and degraded over millennia.
 * Agent #7 — Architecture & Interior Agent
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Architecture")
class TRANSPERSONALGAME_API AArchRuinStructure : public AActor
{
    GENERATED_BODY()

public:
    AArchRuinStructure();

    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Primary structural mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arch|Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> StructureMesh;

    /** Vegetation overlay mesh (moss, ferns) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arch|Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> VegetationMesh;

    /** Ruin configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Config")
    FArch_RuinConfig RuinConfig;

    /** World biome location tag */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Placement")
    FName BiomeTag = NAME_None;

    /** Whether this ruin can be used as player shelter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Gameplay")
    bool bProvidesShelter = false;

    /** Shelter quality rating 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arch|Gameplay",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ShelterQuality = 0.3f;

    /** Apply ruin configuration to mesh */
    UFUNCTION(BlueprintCallable, Category = "Arch|Ruin")
    void ApplyRuinConfig();

    /** Get degradation description for UI */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arch|Ruin")
    FString GetDegradationDescription() const;

    /** Check if player is inside shelter bounds */
    UFUNCTION(BlueprintCallable, Category = "Arch|Gameplay")
    bool IsPlayerInShelter(const FVector& PlayerLocation) const;

    /** Get shelter radius based on ruin type */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Arch|Gameplay")
    float GetShelterRadius() const;
};
