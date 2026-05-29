#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Arch_StructuralSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood"),
    Bone        UMETA(DisplayName = "Bone"),
    Clay        UMETA(DisplayName = "Clay"),
    Hide        UMETA(DisplayName = "Hide")
};

UENUM(BlueprintType)
enum class EArch_WeatheringLevel : uint8
{
    Pristine    UMETA(DisplayName = "Pristine"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Damaged     UMETA(DisplayName = "Damaged"),
    Deteriorated UMETA(DisplayName = "Deteriorated"),
    Collapsed   UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_MaterialType MaterialType = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_WeatheringLevel WeatheringLevel = EArch_WeatheringLevel::Pristine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float MossGrowthRate = 0.05f;

    FArch_StructuralProperties()
    {
        MaterialType = EArch_MaterialType::Stone;
        WeatheringLevel = EArch_WeatheringLevel::Pristine;
        StructuralIntegrity = 100.0f;
        WeatheringRate = 0.1f;
        bHasMossGrowth = false;
        MossGrowthRate = 0.05f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructuralProperties StructuralProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float EnvironmentalDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bSeismicResistant = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float SeismicDamageThreshold = 50.0f;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplySeismicDamage(float SeismicForce);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateMossGrowth(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetWeatheringLevel(EArch_WeatheringLevel NewLevel);

    UFUNCTION(BlueprintPure, Category = "Architecture")
    float GetStructuralIntegrityPercent() const;

    UFUNCTION(BlueprintPure, Category = "Architecture")
    bool IsStructureCollapsed() const;

private:
    void UpdateStructuralIntegrity(float DeltaTime);
    void ProcessEnvironmentalEffects(float DeltaTime);
    float CalculateMaterialResistance() const;
};