#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_MetaHumanController.generated.h"

UENUM(BlueprintType)
enum class EChar_CharacterType : uint8
{
    Player          UMETA(DisplayName = "Player Character"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Scout           UMETA(DisplayName = "Scout"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Shaman          UMETA(DisplayName = "Shaman")
};

UENUM(BlueprintType)
enum class EChar_AgeGroup : uint8
{
    Child           UMETA(DisplayName = "Child (8-12)"),
    Teenager        UMETA(DisplayName = "Teenager (13-17)"),
    YoungAdult      UMETA(DisplayName = "Young Adult (18-30)"),
    MiddleAged      UMETA(DisplayName = "Middle Aged (31-45)"),
    Elder           UMETA(DisplayName = "Elder (46+)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    EChar_CharacterType CharacterType = EChar_CharacterType::Player;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    EChar_AgeGroup AgeGroup = EChar_AgeGroup::YoungAdult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    FVector ScaleModifier = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    TObjectPtr<USkeletalMesh> CharacterMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    TArray<TObjectPtr<UMaterialInterface>> CharacterMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    FString CharacterName = TEXT("Unnamed Character");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    FText CharacterDescription = FText::FromString(TEXT("A primitive human surviving in the Cretaceous period"));

    FChar_CharacterVariation()
    {
        CharacterType = EChar_CharacterType::Player;
        AgeGroup = EChar_AgeGroup::YoungAdult;
        ScaleModifier = FVector(1.0f, 1.0f, 1.0f);
        CharacterMesh = nullptr;
        CharacterName = TEXT("Unnamed Character");
        CharacterDescription = FText::FromString(TEXT("A primitive human surviving in the Cretaceous period"));
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanController : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Configuration")
    FChar_CharacterVariation CharacterVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Configuration")
    bool bAutoApplyVariation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Configuration")
    bool bEnableSubsurfaceScattering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Configuration")
    bool bEnableDynamicShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Optimization")
    float SkinSubsurfaceIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Optimization")
    FLinearColor SkinSubsurfaceColor = FLinearColor(1.0f, 0.4f, 0.3f, 1.0f);

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyCharacterVariation(const FChar_CharacterVariation& NewVariation);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetCharacterType(EChar_CharacterType NewType);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetAgeGroup(EChar_AgeGroup NewAgeGroup);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void OptimizeForFireLighting();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ConfigureShadowCasting(bool bCastDynamicShadows, bool bCastStaticShadows);

    UFUNCTION(BlueprintPure, Category = "MetaHuman")
    FChar_CharacterVariation GetCharacterVariation() const { return CharacterVariation; }

    UFUNCTION(BlueprintPure, Category = "MetaHuman")
    EChar_CharacterType GetCharacterType() const { return CharacterVariation.CharacterType; }

    UFUNCTION(BlueprintPure, Category = "MetaHuman")
    EChar_AgeGroup GetAgeGroup() const { return CharacterVariation.AgeGroup; }

protected:
    UFUNCTION()
    void UpdateCharacterMesh();

    UFUNCTION()
    void UpdateCharacterMaterials();

    UFUNCTION()
    void UpdateLightingConfiguration();
};

#include "Char_MetaHumanController.generated.h"