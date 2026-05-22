#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Arch_StructuralComponent.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Pillar      UMETA(DisplayName = "Stone Pillar"),
    Archway     UMETA(DisplayName = "Stone Archway"),
    Wall        UMETA(DisplayName = "Stone Wall"),
    Platform    UMETA(DisplayName = "Stone Platform"),
    Ruins       UMETA(DisplayName = "Ancient Ruins")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Limestone   UMETA(DisplayName = "Limestone"),
    Sandstone   UMETA(DisplayName = "Sandstone"),
    Basalt      UMETA(DisplayName = "Basalt"),
    Granite     UMETA(DisplayName = "Granite"),
    Weathered   UMETA(DisplayName = "Weathered Stone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Pillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    EArch_MaterialType MaterialType = EArch_MaterialType::Limestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bHasMoss = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bHasVines = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Age")
    float AgeInYears = 1000.0f;

    FArch_StructureProperties()
    {
        StructureType = EArch_StructureType::Pillar;
        MaterialType = EArch_MaterialType::Limestone;
        WeatheringLevel = 0.5f;
        bHasMoss = true;
        bHasVines = false;
        AgeInYears = 1000.0f;
    }
};

UCLASS(ClassGroup=(Architecture), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralComponent : public UStaticMeshComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureProperties StructureProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stability")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stability")
    bool bCanCollapse = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bIsClimbable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bProvidesShade = true;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetMaterialType(EArch_MaterialType NewMaterial);

    UFUNCTION(BlueprintPure, Category = "Architecture")
    float GetStructuralIntegrity() const { return StructuralIntegrity; }

    UFUNCTION(BlueprintPure, Category = "Architecture")
    bool IsStable() const { return StructuralIntegrity > 25.0f; }

private:
    void UpdateMaterialProperties();
    void UpdateCollisionSettings();
    void ApplyAgeEffects();

    UPROPERTY()
    float LastWeatheringUpdate = 0.0f;
};