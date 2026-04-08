#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "MetaHumanCreatorTypes.h"
#include "CharacterSystem.generated.h"

// Enum para tipos de personagens
UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Researcher      UMETA(DisplayName = "Researcher"), 
    Tribal          UMETA(DisplayName = "Tribal"),
    Wanderer        UMETA(DisplayName = "Wanderer"),
    Elder           UMETA(DisplayName = "Elder"),
    Child           UMETA(DisplayName = "Child")
};

// Enum para etnias/origens
UENUM(BlueprintType)
enum class ECharacterEthnicity : uint8
{
    European        UMETA(DisplayName = "European"),
    African         UMETA(DisplayName = "African"),
    Asian           UMETA(DisplayName = "Asian"),
    Indigenous      UMETA(DisplayName = "Indigenous"),
    MiddleEastern   UMETA(DisplayName = "Middle Eastern"),
    Mixed           UMETA(DisplayName = "Mixed Heritage")
};

// Enum para idades
UENUM(BlueprintType)
enum class ECharacterAgeGroup : uint8
{
    Child           UMETA(DisplayName = "Child (8-12)"),
    Teenager        UMETA(DisplayName = "Teenager (13-17)"),
    YoungAdult      UMETA(DisplayName = "Young Adult (18-30)"),
    MiddleAged      UMETA(DisplayName = "Middle Aged (31-50)"),
    Mature          UMETA(DisplayName = "Mature (51-65)"),
    Elder           UMETA(DisplayName = "Elder (65+)")
};

// Struct para dados de aparência
USTRUCT(BlueprintType)
struct FCharacterAppearanceData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demographics")
    ECharacterEthnicity Ethnicity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demographics")
    ECharacterAgeGroup AgeGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demographics")
    bool bIsMale;

    // MetaHuman specific data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class UAnimBlueprint> AnimationBlueprint;

    // Physical characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float HeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float BuildScale;

    // Clothing and accessories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<TSoftObjectPtr<class USkeletalMesh>> ClothingMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<TSoftObjectPtr<class UMaterialInterface>> ClothingMaterials;

    // Survival state indicators
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float DirtinessLevel; // 0.0 = clean, 1.0 = very dirty

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float WearLevel; // 0.0 = new clothes, 1.0 = very worn

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival State")
    float InjuryLevel; // 0.0 = healthy, 1.0 = heavily injured

    // Personality visual cues
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString PersonalityNotes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float ConfidenceLevel; // Affects posture and animations

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float FearLevel; // Affects facial expressions and body language

    FCharacterAppearanceData()
    {
        CharacterName = TEXT("");
        Archetype = ECharacterArchetype::Survivor;
        Ethnicity = ECharacterEthnicity::Mixed;
        AgeGroup = ECharacterAgeGroup::YoungAdult;
        bIsMale = true;
        HeightScale = 1.0f;
        BuildScale = 1.0f;
        DirtinessLevel = 0.3f;
        WearLevel = 0.4f;
        InjuryLevel = 0.1f;
        ConfidenceLevel = 0.5f;
        FearLevel = 0.6f;
    }
};

// Data Asset para configuração do sistema de personagens
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterSystemConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Database")
    TSoftObjectPtr<class UDataTable> CharacterDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman Presets")
    TArray<TSoftObjectPtr<class USkeletalMesh>> MaleMetaHumanPresets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman Presets")
    TArray<TSoftObjectPtr<class USkeletalMesh>> FemaleMetaHumanPresets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clothing Sets")
    TMap<ECharacterArchetype, TArray<TSoftObjectPtr<class USkeletalMesh>>> ArchetypeClothingSets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<class UMaterialInterface> BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<class UMaterialInterface> DirtyClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<class UMaterialInterface> WornClothingMaterial;
};

// Subsystem para gerenciar personagens
UCLASS()
class TRANSPERSONALGAME_API UCharacterManagementSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    FCharacterAppearanceData GetCharacterData(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    TArray<FCharacterAppearanceData> GetCharactersByArchetype(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    FCharacterAppearanceData GenerateRandomCharacter(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void ApplyCharacterAppearance(class ACharacter* Character, const FCharacterAppearanceData& AppearanceData);

private:
    UPROPERTY()
    TSoftObjectPtr<UCharacterSystemConfig> CharacterConfig;

    UPROPERTY()
    TMap<FString, FCharacterAppearanceData> LoadedCharacters;

    void LoadCharacterDatabase();
    FCharacterAppearanceData CreateRandomAppearanceData(ECharacterArchetype Archetype);
};