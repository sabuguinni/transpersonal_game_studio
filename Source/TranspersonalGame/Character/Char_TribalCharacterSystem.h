#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Char_TribalCharacterSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter")
};

UENUM(BlueprintType)
enum class EChar_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Fear        UMETA(DisplayName = "Fear"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Curious     UMETA(DisplayName = "Curious"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Alert       UMETA(DisplayName = "Alert"),
    Joyful      UMETA(DisplayName = "Joyful")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 TribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFeathers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneJewelry;

    FChar_TribalAppearance()
    {
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        BodyScale = 1.0f;
        TribalMarkings = 0;
        bHasFeathers = false;
        bHasBoneJewelry = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Strength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Agility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Wisdom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Endurance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SocialStanding;

    FChar_TribalStats()
    {
        Strength = 50.0f;
        Agility = 50.0f;
        Wisdom = 50.0f;
        Endurance = 50.0f;
        SocialStanding = 50.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Character Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FChar_TribalStats Stats;

    // Emotional System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    EChar_EmotionalState CurrentEmotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionDuration;

    // Equipment and Tools
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<UStaticMeshComponent*> EquippedTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    UStaticMeshComponent* PrimaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    UStaticMeshComponent* SecondaryTool;

    // Social Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<AChar_TribalCharacter*> KnownCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsTribalLeader;

    // Character Functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void UpdateEmotionalState(EChar_EmotionalState NewEmotion, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyAppearanceSettings();

    UFUNCTION(BlueprintCallable, Category = "Character")
    void EquipTool(UStaticMesh* ToolMesh, bool bIsPrimaryWeapon);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void InteractWithCharacter(AChar_TribalCharacter* OtherCharacter);

    UFUNCTION(BlueprintCallable, Category = "Character")
    FString GetCharacterDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool CanPerformAction(const FString& ActionName) const;

protected:
    // Internal character management
    void UpdateCharacterMesh();
    void UpdateEquipment();
    void ProcessEmotionalDecay(float DeltaTime);
    void UpdateSocialRelationships();

private:
    // Character customization components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* TribalMarkingsComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* FeatherComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* JewelryComponent;

    // Timers and state tracking
    float EmotionTimer;
    float LastInteractionTime;
    bool bAppearanceNeedsUpdate;
};