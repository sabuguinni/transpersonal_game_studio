#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Char_TribalAppearanceSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Elder           UMETA(DisplayName = "Elder"),
    Child           UMETA(DisplayName = "Child"),
    Chief           UMETA(DisplayName = "Chief"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class EChar_WeatheringLevel : uint8
{
    Fresh           UMETA(DisplayName = "Fresh"),
    Worn            UMETA(DisplayName = "Worn"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Battered        UMETA(DisplayName = "Battered"),
    Ancient         UMETA(DisplayName = "Ancient")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalMarkings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    FLinearColor PrimaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    FLinearColor SecondaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    int32 PatternIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    bool bHasScars;

    FChar_TribalMarkings()
    {
        PrimaryColor = FLinearColor::Red;
        SecondaryColor = FLinearColor::White;
        PatternIndex = 0;
        Intensity = 1.0f;
        bHasScars = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> TorsoMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> LegsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> FeetMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> AccessoriesMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_WeatheringLevel WeatheringLevel;

    FChar_ClothingSet()
    {
        WeatheringLevel = EChar_WeatheringLevel::Fresh;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalAppearanceSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalAppearanceSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core appearance properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    FChar_TribalMarkings FaceMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    FChar_TribalMarkings BodyMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    FChar_ClothingSet ClothingSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    float SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    float HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    float EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    float Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    float BodyMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    float Height;

    // Weathering and survival effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Effects")
    float DirtLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Effects")
    float ScarLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Effects")
    float TanLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Effects")
    bool bHasTattoos;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Effects")
    bool bHasWarPaint;

    // Torch interaction properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Interaction")
    bool bCanLightTorches;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Interaction")
    bool bIsCarryingTorch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Interaction")
    float TorchLightingSkill;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void GenerateRandomTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void ApplyTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void ApplyWeatheringEffects(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void UpdateClothingCondition(EChar_WeatheringLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Tribal Appearance")
    void ApplyTribalMarkings(const FChar_TribalMarkings& Markings, bool bFaceMarkings = true);

    UFUNCTION(BlueprintCallable, Category = "Torch Interaction")
    void SetTorchCarryingState(bool bCarrying);

    UFUNCTION(BlueprintCallable, Category = "Torch Interaction")
    bool CanInteractWithTorch() const;

private:
    // Internal appearance management
    UPROPERTY()
    TWeakObjectPtr<USkeletalMeshComponent> OwnerMeshComponent;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

    void InitializeMaterialParameters();
    void UpdateMaterialParameters();
    void ApplyRoleSpecificAppearance();
    void GenerateTribalMarkings();
    void ApplySurvivalWeathering();

    // Torch lighting integration
    void UpdateTorchLightingEffects();
    void HandleTorchInteraction();

    // Randomization helpers
    float GetRandomFloat(float Min, float Max) const;
    int32 GetRandomInt(int32 Min, int32 Max) const;
    FLinearColor GetRandomTribalColor() const;
};