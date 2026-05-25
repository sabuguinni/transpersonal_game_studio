#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "TribalCharacterComponent.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Hide"),
    Hunter      UMETA(DisplayName = "Hunter Gear"),
    Gatherer    UMETA(DisplayName = "Gatherer Wraps"),
    Warrior     UMETA(DisplayName = "Warrior Armor")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBattleScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFeatherHeadband = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneJewelry = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringLevel = 0.5f;

    FChar_TribalAppearance()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Athletic;
        ClothingStyle = EChar_ClothingStyle::Hunter;
        bHasTribalMarkings = true;
        bHasBattleScars = false;
        bHasFeatherHeadband = true;
        bHasBoneJewelry = true;
        DirtLevel = 0.3f;
        WeatheringLevel = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTribalCharacterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTribalCharacterComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_TribalAppearance CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    UMaterialInterface* BaseMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Character Appearance")
    UMaterialInstanceDynamic* DynamicMaterial;

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void UpdateSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void UpdateBodyBuild(EChar_BodyBuild NewBodyBuild);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void UpdateClothingStyle(EChar_ClothingStyle NewClothingStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetTribalMarkings(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetBattleScars(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetDirtLevel(float NewDirtLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetWeatheringLevel(float NewWeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance", CallInEditor)
    void PreviewRandomAppearance();

private:
    void InitializeMaterial();
    void UpdateMaterialParameters();
    FLinearColor GetSkinToneColor(EChar_SkinTone SkinTone);
    float GetBodyBuildScale(EChar_BodyBuild BodyBuild);
};

#include "TribalCharacterComponent.generated.h"