#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_MetaHumanIntegration.generated.h"

UENUM(BlueprintType)
enum class EChar_MetaHumanPreset : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout"),
    Custom          UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MetaHumanConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    EChar_MetaHumanPreset PresetType = EChar_MetaHumanPreset::Custom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<TSoftObjectPtr<UMaterialInterface>> FaceMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<TSoftObjectPtr<UMaterialInterface>> BodyMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float AgeModifier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float WeatheringIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanIntegration();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Config")
    FChar_MetaHumanConfig MetaHumanConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    bool bEnableDiversitySystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    TArray<FChar_MetaHumanConfig> DiversityPresets;

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyMetaHumanPreset(EChar_MetaHumanPreset PresetType);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyCustomConfiguration(const FChar_MetaHumanConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void GenerateRandomizedAppearance();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetAgeAndWeathering(float Age, float Weathering);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void UpdateMaterialParameters();

private:
    UPROPERTY()
    class USkeletalMeshComponent* TargetMeshComponent;

    void InitializeDiversityPresets();
    void ApplyMaterialParameters(UMaterialInterface* Material, const FChar_MetaHumanConfig& Config);
    FChar_MetaHumanConfig GetPresetConfiguration(EChar_MetaHumanPreset PresetType);
};