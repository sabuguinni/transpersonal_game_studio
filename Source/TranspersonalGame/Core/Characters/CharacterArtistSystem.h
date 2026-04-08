// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "CharacterArtistSystem.generated.h"

/**
 * Character Archetype Definitions
 * Each archetype represents a visual and narrative category of human character
 */
UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    // Primary Characters
    Protagonist         UMETA(DisplayName = "Protagonist - Paleontologist"),
    
    // Memory/Flashback Characters
    Colleague           UMETA(DisplayName = "Research Colleague"),
    Mentor              UMETA(DisplayName = "Academic Mentor"),
    Family              UMETA(DisplayName = "Family Member"),
    Student             UMETA(DisplayName = "Graduate Student"),
    
    // Potential Encounter Characters
    Explorer            UMETA(DisplayName = "Lost Explorer"),
    Researcher          UMETA(DisplayName = "Time-Lost Researcher"),
    Survivor            UMETA(DisplayName = "Previous Survivor"),
    
    // Visual Variation Types
    YoungAdult          UMETA(DisplayName = "Young Adult (20-30)"),
    MiddleAged          UMETA(DisplayName = "Middle Aged (30-50)"),
    Elder               UMETA(DisplayName = "Elder (50+)"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Physical Characteristics for Character Generation
 */
UENUM(BlueprintType)
enum class EBodyType : uint8
{
    Lean                UMETA(DisplayName = "Lean/Athletic"),
    Average             UMETA(DisplayName = "Average Build"),
    Stocky              UMETA(DisplayName = "Stocky/Robust"),
    Tall                UMETA(DisplayName = "Tall/Lanky"),
    MAX                 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EEthnicity : uint8
{
    Caucasian           UMETA(DisplayName = "Caucasian"),
    African             UMETA(DisplayName = "African"),
    Asian               UMETA(DisplayName = "Asian"),
    Hispanic            UMETA(DisplayName = "Hispanic"),
    MiddleEastern       UMETA(DisplayName = "Middle Eastern"),
    Mixed               UMETA(DisplayName = "Mixed Heritage"),
    MAX                 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EClothingStyle : uint8
{
    // Field Work Appropriate
    FieldResearcher     UMETA(DisplayName = "Field Research Gear"),
    CasualOutdoor       UMETA(DisplayName = "Casual Outdoor"),
    AcademicCasual      UMETA(DisplayName = "Academic Casual"),
    
    // Survival Adapted
    SurvivalMade        UMETA(DisplayName = "Survival Crafted"),
    WeatherWorn         UMETA(DisplayName = "Weather Worn"),
    Improvised          UMETA(DisplayName = "Improvised Gear"),
    
    // Memory/Flashback
    FormalAcademic      UMETA(DisplayName = "Formal Academic"),
    LabCoat             UMETA(DisplayName = "Laboratory"),
    ConferenceAttire    UMETA(DisplayName = "Conference Attire"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Character Visual Definition Structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterVisualProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString Biography;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    EBodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    EEthnicity Ethnicity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Height; // In cm, 150-200 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    int32 Age; // 20-70 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EClothingStyle PrimaryClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EClothingStyle SecondaryClothing;

    // MetaHuman Specific Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString MetaHumanPresetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TMap<FString, float> FacialParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TMap<FString, FLinearColor> ColorParameters;

    // Visual Story Elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> VisualStoryMarkers; // Scars, calluses, tan lines, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString PersonalityVisualCues; // Posture, expression tendencies

    FCharacterVisualProfile()
    {
        CharacterName = TEXT("Unnamed Character");
        Biography = TEXT("");
        Archetype = ECharacterArchetype::Protagonist;
        BodyType = EBodyType::Average;
        Ethnicity = EEthnicity::Caucasian;
        Height = 175.0f;
        Age = 35;
        PrimaryClothing = EClothingStyle::FieldResearcher;
        SecondaryClothing = EClothingStyle::SurvivalMade;
        MetaHumanPresetPath = TEXT("");
        PersonalityVisualCues = TEXT("Confident, observant posture");
    }
};

/**
 * Character Art System - Manages all human character creation and visual diversity
 * 
 * This system is responsible for:
 * - Creating unique, believable human characters using MetaHuman Creator
 * - Ensuring visual diversity across all NPCs and the protagonist
 * - Managing clothing and appearance adaptation for survival context
 * - Maintaining visual storytelling through character appearance
 * - Coordinating with animation system for character-specific movement
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterArtistSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterArtistSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character Creation Methods
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    class ACharacter* CreateCharacterFromProfile(const FCharacterVisualProfile& Profile, FVector SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FCharacterVisualProfile GenerateRandomCharacter(ECharacterArchetype TargetArchetype);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void ApplyVisualProfileToCharacter(class ACharacter* Character, const FCharacterVisualProfile& Profile);

    // MetaHuman Integration
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool LoadMetaHumanPreset(const FString& PresetPath, class USkeletalMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyFacialParameters(class USkeletalMeshComponent* MeshComponent, const TMap<FString, float>& Parameters);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyColorParameters(class USkeletalMeshComponent* MeshComponent, const TMap<FString, FLinearColor>& Colors);

    // Clothing and Equipment
    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void ApplyClothingStyle(class ACharacter* Character, EClothingStyle Style, float WearLevel = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void AddSurvivalAdaptations(class ACharacter* Character, int32 DaysSurvived);

    // Visual Storytelling
    UFUNCTION(BlueprintCallable, Category = "Visual Story")
    void ApplyStoryMarkers(class ACharacter* Character, const TArray<FString>& Markers);

    UFUNCTION(BlueprintCallable, Category = "Visual Story")
    void UpdateCharacterCondition(class ACharacter* Character, float HealthPercent, float StressLevel);

    // Character Database Management
    UFUNCTION(BlueprintCallable, Category = "Database")
    void LoadCharacterDatabase();

    UFUNCTION(BlueprintCallable, Category = "Database")
    void SaveCharacterProfile(const FCharacterVisualProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Database")
    FCharacterVisualProfile GetCharacterProfile(const FString& CharacterName);

    // Diversity and Variation
    UFUNCTION(BlueprintCallable, Category = "Diversity")
    void EnsureVisualDiversity(TArray<FCharacterVisualProfile>& Characters);

    UFUNCTION(BlueprintCallable, Category = "Diversity")
    float CalculateVisualSimilarity(const FCharacterVisualProfile& A, const FCharacterVisualProfile& B);

protected:
    // Character Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    class UDataTable* CharacterProfilesTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TMap<FString, FCharacterVisualProfile> LoadedProfiles;

    // MetaHuman Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<ECharacterArchetype, TArray<FString>> MetaHumanPresetsByArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EClothingStyle, class USkeletalMesh*> ClothingMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EClothingStyle, class UMaterialInterface*> ClothingMaterials;

    // Visual Diversity Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    TArray<FCharacterVisualProfile> ActiveCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    float MinimumVisualDifference;

    // Protagonist Specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protagonist")
    FCharacterVisualProfile ProtagonistProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Protagonist")
    bool bProtagonistCreated;

private:
    // Internal character generation methods
    FCharacterVisualProfile GenerateArchetypeVariation(ECharacterArchetype Archetype);
    void ApplyAgeingEffects(FCharacterVisualProfile& Profile);
    void ApplySurvivalWear(FCharacterVisualProfile& Profile, int32 Days);
    TArray<FString> GenerateStoryMarkers(ECharacterArchetype Archetype, int32 Age);
    
    // MetaHuman parameter generation
    TMap<FString, float> GenerateFacialParameters(EEthnicity Ethnicity, int32 Age, EBodyType Body);
    TMap<FString, FLinearColor> GenerateColorParameters(EEthnicity Ethnicity);
    
    // Quality assurance
    bool ValidateCharacterProfile(const FCharacterVisualProfile& Profile);
    void LogCharacterCreation(const FCharacterVisualProfile& Profile);
};

/**
 * Data Table Row Structure for Character Profiles
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterProfileTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FCharacterVisualProfile Profile;

    FCharacterProfileTableRow()
    {
        Profile = FCharacterVisualProfile();
    }
};