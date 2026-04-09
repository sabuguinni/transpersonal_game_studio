#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"
#include "JurassicCharacterMasterSystemV43.generated.h"

// Forward declarations
class UMetaHumanCharacterComponent;
class UCharacterLightingIntegration;
class UCharacterMaterialSystem;
class UCharacterDiversityManager;

/**
 * Character Archetype Enumeration for Prehistoric Setting
 */
UENUM(BlueprintType)
enum class EJurassicCharacterArchetype : uint8
{
    TribalElder         UMETA(DisplayName = "Tribal Elder"),
    HunterGatherer      UMETA(DisplayName = "Hunter Gatherer"),
    YoungScout          UMETA(DisplayName = "Young Scout"),
    ShamanMystic        UMETA(DisplayName = "Shaman Mystic"),
    CommunityProtector  UMETA(DisplayName = "Community Protector"),
    WiseHealer          UMETA(DisplayName = "Wise Healer"),
    SkillfulCrafter     UMETA(DisplayName = "Skillful Crafter"),
    BraveWarrior        UMETA(DisplayName = "Brave Warrior")
};

/**
 * Character Emotional State for Dynamic Expression
 */
UENUM(BlueprintType)
enum class ECharacterEmotionalState : uint8
{
    SurvivalAlert       UMETA(DisplayName = "Survival Alert"),
    TribalWisdom        UMETA(DisplayName = "Tribal Wisdom"),
    HunterFocus         UMETA(DisplayName = "Hunter Focus"),
    MysticalTrance      UMETA(DisplayName = "Mystical Trance"),
    CommunityBond       UMETA(DisplayName = "Community Bond"),
    FearfulCaution      UMETA(DisplayName = "Fearful Caution"),
    DeterminedResolve   UMETA(DisplayName = "Determined Resolve"),
    PeacefulRest        UMETA(DisplayName = "Peaceful Rest")
};

/**
 * Character Physical Variation Parameters
 */
USTRUCT(BlueprintType)
struct FJurassicCharacterVariation
{
    GENERATED_BODY()

    // Physical characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    float AgeVariation = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    float BodyTypeVariation = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    float FacialFeatureVariation = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    float SkinToneVariation = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    float HeightVariation = 0.3f;

    // Clothing and accessories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing Variation")
    float ClothingWearVariation = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing Variation")
    float AccessoryVariation = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing Variation")
    float WeatheringVariation = 0.8f;

    FJurassicCharacterVariation()
    {
        AgeVariation = 0.5f;
        BodyTypeVariation = 0.5f;
        FacialFeatureVariation = 0.5f;
        SkinToneVariation = 0.5f;
        HeightVariation = 0.3f;
        ClothingWearVariation = 0.7f;
        AccessoryVariation = 0.6f;
        WeatheringVariation = 0.8f;
    }
};

/**
 * Character Archetype Data Structure
 */
USTRUCT(BlueprintType)
struct FJurassicCharacterArchetypeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    EJurassicCharacterArchetype ArchetypeType = EJurassicCharacterArchetype::HunterGatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString ArchetypeName = TEXT("Hunter Gatherer");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString Description = TEXT("Skilled survivor focused on hunting and gathering resources");

    // Age range for this archetype
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demographics")
    FVector2D AgeRange = FVector2D(25.0f, 45.0f);

    // Preferred clothing style
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString ClothingStyle = TEXT("Practical Leather");

    // Distinctive features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> DistinctiveFeatures;

    // Posture and movement style
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FString PostureType = TEXT("Alert Survivor");

    // Material parameter overrides
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<FString, float> MaterialParameters;

    FJurassicCharacterArchetypeData()
    {
        ArchetypeType = EJurassicCharacterArchetype::HunterGatherer;
        ArchetypeName = TEXT("Hunter Gatherer");
        Description = TEXT("Skilled survivor focused on hunting and gathering resources");
        AgeRange = FVector2D(25.0f, 45.0f);
        ClothingStyle = TEXT("Practical Leather");
        PostureType = TEXT("Alert Survivor");
        
        DistinctiveFeatures.Add(TEXT("Muscular Build"));
        DistinctiveFeatures.Add(TEXT("Tool Marks"));
        DistinctiveFeatures.Add(TEXT("Sun Weathered"));
        
        MaterialParameters.Add(TEXT("SkinRoughness"), 0.6f);
        MaterialParameters.Add(TEXT("ClothingWear"), 0.8f);
        MaterialParameters.Add(TEXT("WeatheringIntensity"), 0.7f);
    }
};

/**
 * Master Character System for Jurassic Prehistoric Game
 * Integrates MetaHuman technology with prehistoric character archetypes
 * Handles character creation, variation, lighting integration, and emotional states
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UJurassicCharacterMasterSystemV43 : public UActorComponent
{
    GENERATED_BODY()

public:
    UJurassicCharacterMasterSystemV43();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE CHARACTER CREATION ===
    
    /**
     * Create a new character with specified archetype and variation
     */
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    class ACharacter* CreateJurassicCharacter(
        EJurassicCharacterArchetype ArchetypeType,
        const FJurassicCharacterVariation& Variation,
        const FTransform& SpawnTransform
    );

    /**
     * Apply archetype-specific characteristics to character
     */
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void ApplyCharacterArchetype(
        class ACharacter* Character,
        EJurassicCharacterArchetype ArchetypeType,
        const FJurassicCharacterVariation& Variation
    );

    /**
     * Generate random character variation within archetype constraints
     */
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FJurassicCharacterVariation GenerateRandomVariation(EJurassicCharacterArchetype ArchetypeType);

    // === CHARACTER DIVERSITY SYSTEM ===
    
    /**
     * Apply ethnic and physical diversity to character
     */
    UFUNCTION(BlueprintCallable, Category = "Character Diversity")
    void ApplyCharacterDiversity(
        class ACharacter* Character,
        float EthnicVariation = 0.8f,
        float PhysicalVariation = 0.7f
    );

    /**
     * Set character age and apply age-related visual changes
     */
    UFUNCTION(BlueprintCallable, Category = "Character Diversity")
    void SetCharacterAge(class ACharacter* Character, float Age);

    /**
     * Apply weathering and survival marks to character
     */
    UFUNCTION(BlueprintCallable, Category = "Character Diversity")
    void ApplyWeatheringEffects(
        class ACharacter* Character,
        float WeatheringIntensity = 0.7f,
        float SurvivalMarksIntensity = 0.5f
    );

    // === EMOTIONAL EXPRESSION SYSTEM ===
    
    /**
     * Set character's current emotional state
     */
    UFUNCTION(BlueprintCallable, Category = "Character Expression")
    void SetCharacterEmotionalState(
        class ACharacter* Character,
        ECharacterEmotionalState EmotionalState
    );

    /**
     * Update character facial expression based on emotional state
     */
    UFUNCTION(BlueprintCallable, Category = "Character Expression")
    void UpdateFacialExpression(
        class ACharacter* Character,
        ECharacterEmotionalState EmotionalState,
        float BlendWeight = 1.0f
    );

    /**
     * Apply dynamic posture changes based on emotional state
     */
    UFUNCTION(BlueprintCallable, Category = "Character Expression")
    void UpdateCharacterPosture(
        class ACharacter* Character,
        ECharacterEmotionalState EmotionalState
    );

    // === CLOTHING AND MATERIAL SYSTEM ===
    
    /**
     * Apply prehistoric clothing style to character
     */
    UFUNCTION(BlueprintCallable, Category = "Character Clothing")
    void ApplyPrehistoricClothing(
        class ACharacter* Character,
        const FString& ClothingStyle,
        float WearLevel = 0.7f
    );

    /**
     * Update character materials for lighting integration
     */
    UFUNCTION(BlueprintCallable, Category = "Character Materials")
    void UpdateCharacterMaterials(
        class ACharacter* Character,
        const TMap<FString, float>& MaterialParameters
    );

    /**
     * Apply tribal markings and decorations
     */
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyTribalMarkings(
        class ACharacter* Character,
        const FString& MarkingStyle,
        float Intensity = 0.8f
    );

    // === LIGHTING INTEGRATION ===
    
    /**
     * Configure character for optimal lighting in Jurassic environment
     */
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void ConfigureCharacterLighting(
        class ACharacter* Character,
        bool bUseLumenGI = true,
        bool bEnableSubsurfaceScattering = true
    );

    /**
     * Update character materials for current lighting conditions
     */
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateCharacterForLighting(
        class ACharacter* Character,
        float TimeOfDay,
        float WeatherIntensity
    );

    // === ARCHETYPE MANAGEMENT ===
    
    /**
     * Get archetype data for specified type
     */
    UFUNCTION(BlueprintCallable, Category = "Character Archetypes")
    FJurassicCharacterArchetypeData GetArchetypeData(EJurassicCharacterArchetype ArchetypeType);

    /**
     * Register new archetype data
     */
    UFUNCTION(BlueprintCallable, Category = "Character Archetypes")
    void RegisterArchetypeData(
        EJurassicCharacterArchetype ArchetypeType,
        const FJurassicCharacterArchetypeData& ArchetypeData
    );

    /**
     * Get random archetype weighted by rarity
     */
    UFUNCTION(BlueprintCallable, Category = "Character Archetypes")
    EJurassicCharacterArchetype GetRandomArchetype();

protected:
    // === COMPONENT REFERENCES ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UMetaHumanCharacterComponent> MetaHumanComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCharacterLightingIntegration> LightingIntegration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCharacterMaterialSystem> MaterialSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCharacterDiversityManager> DiversityManager;

    // === ARCHETYPE DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype Data")
    TMap<EJurassicCharacterArchetype, FJurassicCharacterArchetypeData> ArchetypeDatabase;

    // === MATERIAL REFERENCES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> BaseClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> BaseHairMaterial;

    // === CLOTHING ASSETS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TMap<FString, TObjectPtr<UStaticMesh>> ClothingMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TMap<FString, TObjectPtr<UMaterialInterface>> ClothingMaterials;

    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DefaultCharacterScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DiversityIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAdvancedLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableEmotionalExpressions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ExpressionBlendSpeed = 2.0f;

private:
    // === INTERNAL METHODS ===
    
    void InitializeArchetypeDatabase();
    void SetupComponentReferences();
    void ApplyArchetypeSpecificMaterials(class ACharacter* Character, EJurassicCharacterArchetype ArchetypeType);
    void ApplyVariationToMesh(class ACharacter* Character, const FJurassicCharacterVariation& Variation);
    void UpdateCharacterLOD(class ACharacter* Character, float Distance);
    
    // === INTERNAL STATE ===
    
    UPROPERTY()
    TMap<TObjectPtr<class ACharacter>, ECharacterEmotionalState> CharacterEmotionalStates;

    UPROPERTY()
    TMap<TObjectPtr<class ACharacter>, FJurassicCharacterVariation> CharacterVariations;

    float LastUpdateTime = 0.0f;
    bool bSystemInitialized = false;
};