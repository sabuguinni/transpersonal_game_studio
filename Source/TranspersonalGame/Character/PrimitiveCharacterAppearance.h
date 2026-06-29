#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "PrimitiveCharacterAppearance.generated.h"

// ============================================================
// Agent #09 — Character Artist | PROD_CYCLE_AUTO_20260629_001
// PrimitiveCharacterAppearance — Visual identity component
// Manages character mesh, materials, tribal markings, wear state
// ============================================================

/** Body type variants for primitive survivors */
UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    MaleHunter      UMETA(DisplayName = "Male Hunter"),
    FemaleHunter    UMETA(DisplayName = "Female Hunter"),
    MaleElder       UMETA(DisplayName = "Male Elder"),
    FemaleElder     UMETA(DisplayName = "Female Elder"),
    YoungScout      UMETA(DisplayName = "Young Scout"),
};

/** Clothing wear state driven by survival conditions */
UENUM(BlueprintType)
enum class EChar_ClothingWear : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    Worn            UMETA(DisplayName = "Worn"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Tattered        UMETA(DisplayName = "Tattered"),
    Bare            UMETA(DisplayName = "Bare - No Clothing"),
};

/** Tribal marking style — ochre/charcoal patterns */
UENUM(BlueprintType)
enum class EChar_TribalMarkings : uint8
{
    None            UMETA(DisplayName = "No Markings"),
    HunterStripes   UMETA(DisplayName = "Hunter Stripes"),
    ElderSpirals    UMETA(DisplayName = "Elder Spirals"),
    WarriorDots     UMETA(DisplayName = "Warrior Dots"),
    ScoutLines      UMETA(DisplayName = "Scout Lines"),
};

/** Dirt/blood accumulation state — visual survival feedback */
USTRUCT(BlueprintType)
struct FChar_SkinCondition
{
    GENERATED_BODY()

    /** 0.0 = clean, 1.0 = fully caked in dirt/mud */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLevel = 0.0f;

    /** 0.0 = no blood, 1.0 = heavily bloodied */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BloodLevel = 0.0f;

    /** 0.0 = dry, 1.0 = fully drenched (rain/water) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WetLevel = 0.0f;

    /** Sunburn accumulation from prolonged exposure */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SunburnLevel = 0.0f;
};

/** Complete visual identity definition for a primitive character */
USTRUCT(BlueprintType)
struct FChar_VisualIdentity
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EChar_BodyType BodyType = EChar_BodyType::MaleHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EChar_ClothingWear ClothingWear = EChar_ClothingWear::Worn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EChar_TribalMarkings TribalMarkings = EChar_TribalMarkings::HunterStripes;

    /** Base skin tone (0=very dark, 1=very light) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinTone = 0.4f;

    /** Hair darkness (0=black, 1=light brown) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HairDarkness = 0.1f;

    /** Ochre marking intensity (0=none, 1=vivid) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MarkingIntensity = 0.6f;

    /** Muscle definition (0=lean, 1=heavily muscled) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MuscleDefinition = 0.5f;

    /** Scar accumulation from past encounters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarDensity = 0.2f;
};

/**
 * UPrimitiveCharacterAppearance
 * 
 * Actor component that manages the full visual state of a primitive human character.
 * Drives Dynamic Material Instance parameters for dirt, blood, wet, tribal markings,
 * and clothing wear — creating a living biography written on the character's body.
 * 
 * Attach to: ATranspersonalCharacter or any ACharacter subclass.
 */
UCLASS(ClassGroup = "Character", meta = (BlueprintSpawnableComponent), DisplayName = "Primitive Character Appearance")
class TRANSPERSONALGAME_API UPrimitiveCharacterAppearance : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrimitiveCharacterAppearance();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Visual Identity ───────────────────────────────────────────────

    /** The character's permanent visual identity (body type, markings, skin tone) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Identity")
    FChar_VisualIdentity VisualIdentity;

    /** Current dynamic skin condition (dirt, blood, wet, sunburn) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Condition")
    FChar_SkinCondition SkinCondition;

    // ─── Material Slots ────────────────────────────────────────────────

    /** Index of the skin material slot on the skeletal mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Materials")
    int32 SkinMaterialSlot = 0;

    /** Index of the clothing material slot on the skeletal mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Materials")
    int32 ClothingMaterialSlot = 1;

    /** Index of the hair material slot on the skeletal mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Materials")
    int32 HairMaterialSlot = 2;

    // ─── Runtime State ─────────────────────────────────────────────────

    /** How long character has been in rain (seconds) — drives WetLevel */
    UPROPERTY(BlueprintReadOnly, Category = "Character|Condition")
    float TimeInRain = 0.0f;

    /** How long character has been in direct sunlight (seconds) — drives SunburnLevel */
    UPROPERTY(BlueprintReadOnly, Category = "Character|Condition")
    float TimeInSun = 0.0f;

    /** Whether character is currently in rain (set by weather system) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Condition")
    bool bInRain = false;

    /** Whether character is currently in direct sunlight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Condition")
    bool bInDirectSunlight = false;

    // ─── Survival-Driven Appearance Changes ────────────────────────────

    /**
     * Called when character takes damage — adds blood splatter to skin condition.
     * @param DamageAmount 0.0-1.0 normalized damage amount
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void OnCharacterDamaged(float DamageAmount);

    /**
     * Called when character crawls through mud/dirt terrain.
     * @param DirtAmount 0.0-1.0 normalized dirt accumulation
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void AccumulateDirt(float DirtAmount);

    /**
     * Called when character enters water or heavy rain — resets dirt, adds wet.
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void OnEnterWater();

    /**
     * Degrade clothing wear state by one level (Pristine → Worn → Damaged → Tattered).
     * Called after major combat encounters or prolonged survival.
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void DegradeClothing();

    /**
     * Apply a new visual identity (used during character creation or respawn).
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    void ApplyVisualIdentity(const FChar_VisualIdentity& NewIdentity);

    /**
     * Force-refresh all Dynamic Material Instance parameters from current state.
     * Call after any manual property change in editor.
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Character|Appearance")
    void RefreshMaterials();

    /**
     * Get a human-readable description of current appearance state (for debug HUD).
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Appearance")
    FString GetAppearanceDebugString() const;

    // ─── Camera & Third-Person Setup ───────────────────────────────────

    /** Recommended camera boom length for this body type */
    UFUNCTION(BlueprintPure, Category = "Character|Camera")
    float GetRecommendedCameraBoomLength() const;

    /** Recommended camera pitch offset for this body type */
    UFUNCTION(BlueprintPure, Category = "Character|Camera")
    float GetRecommendedCameraPitch() const;

private:
    /** Cached Dynamic Material Instances — created from skeletal mesh materials */
    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

    /** Reference to owner's skeletal mesh component */
    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh = nullptr;

    /** Initialize Dynamic Material Instances from owner's skeletal mesh */
    void InitializeDynamicMaterials();

    /** Push current SkinCondition values to all Dynamic Material Instances */
    void UpdateSkinConditionParameters();

    /** Push current VisualIdentity values to all Dynamic Material Instances */
    void UpdateIdentityParameters();

    /** Clamp all condition values to [0,1] range */
    void ClampConditionValues();

    /** Dry rate per second when not in rain */
    static constexpr float DryRate = 0.005f;

    /** Dirt fade rate per second (natural weathering) */
    static constexpr float DirtFadeRate = 0.001f;

    /** Blood dry rate per second */
    static constexpr float BloodDryRate = 0.002f;

    /** Wet accumulation rate per second in rain */
    static constexpr float WetAccumulationRate = 0.05f;

    /** Sunburn accumulation rate per second in direct sun */
    static constexpr float SunburnAccumulationRate = 0.0008f;
};
