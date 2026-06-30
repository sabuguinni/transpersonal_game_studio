#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    RuinPillar      UMETA(DisplayName = "Ruin Pillar"),
    RuinWall        UMETA(DisplayName = "Ruin Wall"),
    RuinFoundation  UMETA(DisplayName = "Ruin Foundation"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    StoneMound      UMETA(DisplayName = "Stone Mound")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasVegetationGrowth = true;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructureComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructureComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasVegetationGrowth;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(EArch_StructureType InType, float InWeathering);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EArch_StructureType GetStructureType() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetWeatheringLevel() const;
};

UCLASS(ClassGroup = (TranspersonalGame), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CretaceousRuin : public AActor
{
    GENERATED_BODY()

public:
    AArch_CretaceousRuin();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* RuinMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture",
        meta = (AllowPrivateAccess = "true"))
    UArch_StructureComponent* StructureComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float CollapseRadius;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatherAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsStructureIntact() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetStructureCenter() const;

protected:
    virtual void BeginPlay() override;

private:
    bool bStructureIntact;
};
