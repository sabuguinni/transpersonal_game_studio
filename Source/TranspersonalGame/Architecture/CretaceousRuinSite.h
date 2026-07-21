#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/BoxComponent.h"
#include "CretaceousRuinSite.generated.h"

/**
 * Enum defining the type of Cretaceous ruin structure.
 * Each type has distinct architectural characteristics and gameplay implications.
 */
UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
    WallSegment     UMETA(DisplayName = "Wall Segment"),
    Pillar          UMETA(DisplayName = "Standing Pillar"),
    Archway         UMETA(DisplayName = "Collapsed Archway"),
    Chamber         UMETA(DisplayName = "Enclosed Chamber"),
    Altar           UMETA(DisplayName = "Central Altar"),
    Rubble          UMETA(DisplayName = "Rubble Pile"),
    Foundation      UMETA(DisplayName = "Foundation Remnant")
};

/**
 * Enum defining the decay state of a ruin element.
 * Affects visual appearance and structural stability gameplay.
 */
UENUM(BlueprintType)
enum class EArch_DecayState : uint8
{
    Intact          UMETA(DisplayName = "Mostly Intact"),
    Weathered       UMETA(DisplayName = "Heavily Weathered"),
    Crumbling       UMETA(DisplayName = "Actively Crumbling"),
    Collapsed       UMETA(DisplayName = "Collapsed"),
    Buried          UMETA(DisplayName = "Partially Buried")
};

/**
 * Struct holding metadata about a ruin element — who built it, when, and what it meant.
 * This is the archaeological record embedded in each structure.
 */
USTRUCT(BlueprintType)
struct FArch_RuinElementData
{
    GENERATED_BODY()

    /** Type of architectural element */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_RuinType RuinType = EArch_RuinType::WallSegment;

    /** Current decay state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_DecayState DecayState = EArch_DecayState::Weathered;

    /** Estimated age in millions of years (geological context) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Lore")
    float EstimatedAgeMYA = 66.0f;

    /** Whether this element can shelter the player from rain/predators */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Gameplay")
    bool bProvidesShel ter = false;

    /** Whether this element is structurally unstable and can collapse */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Gameplay")
    bool bIsUnstable = false;

    /** Climbable surface — player can scale this element */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Gameplay")
    bool bIsClimbable = true;

    /** Discovery XP reward when player first interacts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Gameplay")
    float DiscoveryReward = 25.0f;
};

/**
 * ACretaceousRuinSite — A prehistoric ruin structure in the game world.
 * 
 * These are not mystical or spiritual sites. They are the physical remnants
 * of ancient geological and biological processes — rock formations shaped by
 * erosion, volcanic activity, and the passage of 66+ million years.
 * 
 * Gameplay role: shelter, landmark navigation, resource gathering (stone tools),
 * and environmental storytelling through physical decay patterns.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Architecture")
class TRANSPERSONALGAME_API ACretaceousRuinSite : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousRuinSite();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // =========================================================================
    // COMPONENTS
    // =========================================================================

    /** Root collision box for the entire ruin site */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UBoxComponent* SiteCollision;

    /** Primary structural mesh (main wall or pillar) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* PrimaryStructure;

    /** Secondary structural mesh (rubble, collapsed sections) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* SecondaryStructure;

    /** Atmospheric interior light (simulates light shafts through gaps) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UPointLightComponent* InteriorLight;

    // =========================================================================
    // PROPERTIES
    // =========================================================================

    /** Metadata describing this ruin element */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_RuinElementData RuinData;

    /** Biome region this ruin belongs to (for world generation integration) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|World")
    FName BiomeRegion = FName("CretaceousJungle");

    /** Whether the ruin site has been discovered by the player */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture|Gameplay")
    bool bIsDiscovered = false;

    /** Whether dinosaurs use this site as a nesting/shelter location */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ecology")
    bool bIsDinosaurNestingSite = false;

    /** Stone material quality for crafting (0.0 = rubble, 1.0 = pristine blocks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Resources",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StoneMaterialQuality = 0.7f;

    /** Number of stone tool resources available at this site */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Resources")
    int32 AvailableStoneResources = 5;

    // =========================================================================
    // FUNCTIONS
    // =========================================================================

    /** Called when player enters the ruin site proximity */
    UFUNCTION(BlueprintCallable, Category = "Architecture|Gameplay")
    void OnPlayerEnterSite(AActor* PlayerActor);

    /** Called when player harvests stone from this site */
    UFUNCTION(BlueprintCallable, Category = "Architecture|Resources")
    int32 HarvestStoneResource(int32 RequestedAmount);

    /** Returns true if this site provides shelter from weather */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture|Gameplay")
    bool ProvidesWeatherShelter() const;

    /** Returns the structural integrity as a 0-1 value */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture|Gameplay")
    float GetStructuralIntegrity() const;

    /** Trigger collapse sequence if structure is unstable */
    UFUNCTION(BlueprintCallable, Category = "Architecture|Gameplay")
    void TriggerCollapse();

    /** Get the best vantage point position on this structure */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture|Navigation")
    FVector GetVantagePoint() const;

private:
    /** Internal: apply decay visual state to meshes */
    void ApplyDecayVisuals();

    /** Internal: configure lighting based on ruin type */
    void ConfigureInteriorLighting();

    /** Cached structural integrity value */
    float CachedStructuralIntegrity;
};
