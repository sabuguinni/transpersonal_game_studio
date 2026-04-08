#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "GameFramework/Character.h"
#include "CharacterSystem.generated.h"

/**
 * Base Character System for Transpersonal Game
 * Handles MetaHuman integration, character variation, and survival mechanics
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist - Paleontologist"),
    Survivor        UMETA(DisplayName = "Survivor - Other Human"),
    Memory          UMETA(DisplayName = "Memory - Flashback Character"),
    Corpse          UMETA(DisplayName = "Corpse - Found Remains")
};

UENUM(BlueprintType)
enum class EClothingCondition : uint8
{
    Fresh           UMETA(DisplayName = "Fresh - Just Arrived"),
    Worn            UMETA(DisplayName = "Worn - Days Survived"),
    Damaged         UMETA(DisplayName = "Damaged - Weeks Survived"),
    Tattered        UMETA(DisplayName = "Tattered - Months Survived"),
    Primitive       UMETA(DisplayName = "Primitive - Makeshift Clothing")
};

UENUM(BlueprintType)
enum class EPhysicalCondition : uint8
{
    Healthy         UMETA(DisplayName = "Healthy"),
    Tired           UMETA(DisplayName = "Tired"),
    Injured         UMETA(DisplayName = "Injured"),
    Starving        UMETA(DisplayName = "Starving"),
    Sick            UMETA(DisplayName = "Sick"),
    Dying           UMETA(DisplayName = "Dying")
};

USTRUCT(BlueprintType)
struct FCharacterVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString MetaHumanPresetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Muscle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 HairStyleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    EClothingCondition ClothingState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    EPhysicalCondition PhysicalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float DirtLevel; // 0.0 = Clean, 1.0 = Filthy

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ScarLevel; // 0.0 = No scars, 1.0 = Heavily scarred

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 DaysSurvived;

    FCharacterVariation()
    {
        MetaHumanPresetID = TEXT("Default");
        SkinTone = 0.5f;
        BodyWeight = 0.5f;
        Muscle = 0.5f;
        HairStyleID = 0;
        HairColor = FLinearColor::Black;
        EyeColor = FLinearColor::Blue;
        ClothingState = EClothingCondition::Fresh;
        PhysicalState = EPhysicalCondition::Healthy;
        DirtLevel = 0.0f;
        ScarLevel = 0.0f;
        DaysSurvived = 0;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ABaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABaseCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    ECharacterArchetype CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    FCharacterVariation CharacterVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    FText CharacterBiography;

    // MetaHuman Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    class USkeletalMeshComponent* MetaHumanBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    class USkeletalMeshComponent* MetaHumanHead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    class USkeletalMeshComponent* MetaHumanHair;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    class USkeletalMeshComponent* MetaHumanClothing;

public:
    UFUNCTION(BlueprintCallable, Category = "Character System")
    void ApplyCharacterVariation(const FCharacterVariation& NewVariation);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void UpdateSurvivalCondition(int32 NewDaysSurvived);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void SetClothingCondition(EClothingCondition NewCondition);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void SetPhysicalCondition(EPhysicalCondition NewCondition);

    UFUNCTION(BlueprintPure, Category = "Character System")
    FCharacterVariation GetCharacterVariation() const { return CharacterVariation; }

    UFUNCTION(BlueprintPure, Category = "Character System")
    ECharacterArchetype GetCharacterType() const { return CharacterType; }

private:
    void SetupMetaHumanComponents();
    void ApplyMaterialParameters();
    void UpdateClothingMaterials();
    void UpdateSkinMaterials();
};

/**
 * Character Factory for procedural character generation
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterFactory : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Character Factory")
    static FCharacterVariation GenerateRandomVariation(ECharacterArchetype ArchetypeHint);

    UFUNCTION(BlueprintCallable, Category = "Character Factory")
    static FCharacterVariation GenerateProtagonistVariation();

    UFUNCTION(BlueprintCallable, Category = "Character Factory")
    static FCharacterVariation GenerateSurvivorVariation(int32 DaysSurvived);

    UFUNCTION(BlueprintCallable, Category = "Character Factory")
    static TArray<FString> GetAvailableMetaHumanPresets();

private:
    static TArray<FString> MetaHumanPresetDatabase;
    static void InitializePresetDatabase();
};