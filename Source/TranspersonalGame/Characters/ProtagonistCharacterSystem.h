#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameFramework/Character.h"
#include "CharacterArchetypes.h"
#include "JurassicCharacterArtist.h"
#include "ProtagonistCharacterSystem.generated.h"

class UMetaHumanIdentity;
class USkeletalMeshComponent;
class UGroomComponent;
class UMaterialInstanceDynamic;

/**
 * Enum for protagonist customization presets
 */
UENUM(BlueprintType)
enum class EProtagonistPreset : uint8
{
    Default_Academic        UMETA(DisplayName = "Academic Paleontologist"),
    Rugged_FieldWorker     UMETA(DisplayName = "Field Research Specialist"),
    Young_Graduate         UMETA(DisplayName = "Graduate Student"),
    Experienced_Professor  UMETA(DisplayName = "Veteran Professor"),
    Custom                 UMETA(DisplayName = "Custom Configuration"),
    
    MAX                    UMETA(Hidden)
};

/**
 * Enum for protagonist survival progression stages
 */
UENUM(BlueprintType)
enum class ESurvivalStage : uint8
{
    FreshArrival          UMETA(DisplayName = "Fresh Arrival - Day 0"),
    EarlyAdaptation       UMETA(DisplayName = "Early Adaptation - Days 1-7"),
    SurvivalLearning      UMETA(DisplayName = "Learning Phase - Days 8-30"),
    Adaptation            UMETA(DisplayName = "Adaptation - Days 31-90"),
    VeteranSurvivor       UMETA(DisplayName = "Veteran Survivor - 90+ Days"),
    
    MAX                   UMETA(Hidden)
};

/**
 * Structure for protagonist physical characteristics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FProtagonistPhysicalTraits
{
    GENERATED_BODY()

    FProtagonistPhysicalTraits()
        : Gender(ECharacterGender::Male)
        , AgeRange(35.0f)
        , Height(175.0f)
        , Build(ECharacterBuild::Medium_Balanced)
        , SkinTone(ESkinTone::Medium)
        , EyeColor(FLinearColor::Blue)
        , HairColor(FLinearColor::Black)
        , HairStyle(TEXT("Academic_Short"))
        , FacialHairStyle(TEXT("Clean_Shaven"))
        , bHasGlasses(true)
        , GlassesStyle(TEXT("Academic_Frames"))
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    ECharacterGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic", meta = (ClampMin = "25.0", ClampMax = "65.0"))
    float AgeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic", meta = (ClampMin = "150.0", ClampMax = "200.0"))
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    ECharacterBuild Build;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    ESkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString HairStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString FacialHairStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories")
    bool bHasGlasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories")
    FString GlassesStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    FFacialFeatureSet FacialFeatures;
};

/**
 * Structure for protagonist survival progression
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FProtagonistSurvivalProgression
{
    GENERATED_BODY()

    FProtagonistSurvivalProgression()
        : CurrentStage(ESurvivalStage::FreshArrival)
        , DaysInWorld(0.0f)
        , PhysicalCondition(1.0f)
        , MentalState(0.8f)
        , ClothingCondition(1.0f)
        , SkinCondition(1.0f)
        , HairCondition(1.0f)
        , bHasBeardGrowth(false)
        , BeardGrowthLevel(0.0f)
        , SunTanLevel(0.0f)
        , DirtAccumulation(0.0f)
        , MuscleDefinition(0.0f)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    ESurvivalStage CurrentStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DaysInWorld;

    // Physical condition changes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PhysicalCondition; // 1.0 = perfect health, 0.0 = near death

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mental", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MentalState; // 1.0 = calm confidence, 0.0 = broken

    // Visual wear and tear
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wear", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClothingCondition; // 1.0 = pristine, 0.0 = rags

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wear", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinCondition; // 1.0 = perfect skin, 0.0 = heavily damaged

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wear", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HairCondition; // 1.0 = well-groomed, 0.0 = unkempt

    // Growth and adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
    bool bHasBeardGrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BeardGrowthLevel;

    // Environmental effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SunTanLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtAccumulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MuscleDefinition; // Gained through survival activities

    // Scars and injuries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injuries")
    TArray<FString> AcquiredScars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injuries")
    TArray<FString> CurrentInjuries;
};

/**
 * Structure for protagonist clothing system
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FProtagonistClothingSystem
{
    GENERATED_BODY()

    FProtagonistClothingSystem()
        : BaseOutfit(TEXT("Academic_Fieldwork"))
        , ShirtCondition(1.0f)
        , PantsCondition(1.0f)
        , BootsCondition(1.0f)
        , JacketCondition(1.0f)
        , bHasHat(false)
        , HatCondition(1.0f)
        , bHasGloves(false)
        , GlovesCondition(1.0f)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base")
    FString BaseOutfit;

    // Individual clothing piece conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ShirtCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PantsCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BootsCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JacketCondition;

    // Accessories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories")
    bool bHasHat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HatCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories")
    bool bHasGloves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float GlovesCondition;

    // Improvised additions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Improvised")
    TArray<FString> ImprovisedClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Improvised")
    TArray<FString> CraftedAccessories;
};

/**
 * Complete protagonist configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FProtagonistConfiguration
{
    GENERATED_BODY()

    FProtagonistConfiguration()
        : CharacterName(TEXT("Dr. Marcus Chen"))
        , Preset(EProtagonistPreset::Default_Academic)
        , ProfessionTitle(TEXT("Paleontologist"))
        , Institution(TEXT("University Research"))
        , Specialization(TEXT("Mesozoic Era Specialist"))
        , bIsPlayerControlled(true)
    {
    }

    // Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EProtagonistPreset Preset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FString ProfessionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FString Institution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FString Specialization;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bIsPlayerControlled;

    // Appearance and progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FProtagonistPhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    FProtagonistSurvivalProgression SurvivalProgression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FProtagonistClothingSystem ClothingSystem;
};

/**
 * Protagonist Character System
 * 
 * Specialized system for creating and managing the player character.
 * Handles visual progression through survival stages, dynamic appearance
 * changes based on time and experiences, and maintains character continuity.
 */
UCLASS()
class TRANSPERSONALGAME_API UProtagonistCharacterSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Protagonist creation and management
    UFUNCTION(BlueprintCallable, Category = "Protagonist")
    ACharacter* CreateProtagonist(
        const FProtagonistConfiguration& Configuration,
        const FVector& SpawnLocation,
        const FRotator& SpawnRotation = FRotator::ZeroRotator
    );

    UFUNCTION(BlueprintCallable, Category = "Protagonist")
    void UpdateProtagonistAppearance(ACharacter* Protagonist);

    UFUNCTION(BlueprintCallable, Category = "Protagonist")
    void AdvanceSurvivalStage(ACharacter* Protagonist, float DaysElapsed);

    // Preset management
    UFUNCTION(BlueprintCallable, Category = "Presets")
    FProtagonistConfiguration GetPresetConfiguration(EProtagonistPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyPresetToProtagonist(ACharacter* Protagonist, EProtagonistPreset Preset);

    // Visual progression
    UFUNCTION(BlueprintCallable, Category = "Progression")
    void ApplySurvivalWear(ACharacter* Protagonist, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void ApplyEnvironmentalEffects(ACharacter* Protagonist, float SunExposure, float DirtLevel);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void AddSurvivalScar(ACharacter* Protagonist, const FString& ScarDescription);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void UpdateBeardGrowth(ACharacter* Protagonist, float GrowthLevel);

    // Clothing system
    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void UpdateClothingCondition(ACharacter* Protagonist, const FString& ClothingPiece, float Condition);

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void AddImprovisedClothing(ACharacter* Protagonist, const FString& ItemDescription);

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    void RemoveClothingPiece(ACharacter* Protagonist, const FString& ClothingPiece);

    // Character state queries
    UFUNCTION(BlueprintCallable, Category = "State")
    ESurvivalStage GetCurrentSurvivalStage(ACharacter* Protagonist) const;

    UFUNCTION(BlueprintCallable, Category = "State")
    float GetSurvivalDays(ACharacter* Protagonist) const;

    UFUNCTION(BlueprintCallable, Category = "State")
    FString GetCharacterDescription(ACharacter* Protagonist) const;

    UFUNCTION(BlueprintCallable, Category = "State")
    bool IsProtagonistCharacter(ACharacter* Character) const;

protected:
    // Internal systems
    UFUNCTION()
    void InitializeProtagonistPresets();

    UFUNCTION()
    void SetupMetaHumanForProtagonist(ACharacter* Protagonist, const FProtagonistConfiguration& Config);

    UFUNCTION()
    void ApplyPhysicalTraits(ACharacter* Protagonist, const FProtagonistPhysicalTraits& Traits);

    UFUNCTION()
    void UpdateMaterialParameters(ACharacter* Protagonist, const FProtagonistSurvivalProgression& Progression);

    UFUNCTION()
    void UpdateGroomComponents(ACharacter* Protagonist, const FProtagonistSurvivalProgression& Progression);

    UFUNCTION()
    ESurvivalStage CalculateSurvivalStage(float DaysInWorld) const;

private:
    // Preset configurations
    UPROPERTY()
    TMap<EProtagonistPreset, FProtagonistConfiguration> PresetConfigurations;

    // Character tracking
    UPROPERTY()
    TMap<ACharacter*, FProtagonistConfiguration> ProtagonistCharacters;

    // Reference to character artist system
    UPROPERTY()
    class UJurassicCharacterArtist* CharacterArtist;

    // MetaHuman assets
    UPROPERTY()
    TMap<ECharacterGender, TSoftObjectPtr<UMetaHumanIdentity>> ProtagonistMetaHumanPresets;

    // Material templates
    UPROPERTY()
    TMap<FString, UMaterialInterface*> ClothingMaterialTemplates;

    UPROPERTY()
    UMaterialInterface* SkinMaterialTemplate;

    UPROPERTY()
    UMaterialInterface* HairMaterialTemplate;
};