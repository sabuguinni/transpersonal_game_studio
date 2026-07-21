#pragma once

#include "CoreMinimal.h"
#include "Char_TribalWarrior.h"
#include "Components/PointLightComponent.h"
#include "Components/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "../SharedTypes.h"
#include "Char_TribalShaman.generated.h"

/**
 * Tribal shaman character specializing in primitive medicine and rituals
 * Extends tribal warrior with spiritual/healing equipment and abilities
 * Features ritual scars, bone ornaments, and mystical staff
 * Designed for NPC interactions and quest-giving roles
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AChar_TribalShaman : public AChar_TribalWarrior
{
    GENERATED_BODY()

public:
    AChar_TribalShaman();

protected:
    virtual void BeginPlay() override;

    // === SHAMAN-SPECIFIC COMPONENTS ===
    
    /** Ritual staff with carved symbols */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shaman Equipment")
    UStaticMeshComponent* RitualStaffMesh;
    
    /** Medicine pouch with herbs and remedies */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shaman Equipment")
    UStaticMeshComponent* MedicinePouchMesh;
    
    /** Bone ritual mask for ceremonies */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shaman Equipment")
    UStaticMeshComponent* RitualMaskMesh;
    
    /** Feather headdress ornaments */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shaman Equipment")
    UStaticMeshComponent* FeatherHeaddressMesh;
    
    /** Fire light component for torch/ritual fire effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shaman Effects")
    UPointLightComponent* RitualFireLight;
    
    /** Smoke particle system for mystical atmosphere */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shaman Effects")
    UParticleSystemComponent* SmokeParticles;
    
    // === SHAMAN TRAITS ===
    
    /** Healing knowledge level (affects available remedies) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Traits", meta = (ClampMin = "1", ClampMax = "10"))
    int32 HealingKnowledge;
    
    /** Ritual experience (affects ceremony effectiveness) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Traits", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RitualExperience;
    
    /** Spiritual wisdom level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Traits", meta = (ClampMin = "1", ClampMax = "10"))
    int32 SpiritualWisdom;
    
    /** Number of ritual scars (shows experience) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Appearance", meta = (ClampMin = "0", ClampMax = "20"))
    int32 RitualScarCount;
    
    /** Type of ritual mask worn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Appearance")
    EChar_RitualMaskType RitualMaskType;
    
    /** Staff carving pattern */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Equipment")
    EChar_StaffCarvingType StaffCarvingType;
    
    // === MATERIALS ===
    
    /** Ritual scar material for weathered skin */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Materials")
    UMaterialInterface* RitualScarMaterial;
    
    /** Carved wood material for staff */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Materials")
    UMaterialInterface* CarvedWoodMaterial;
    
    /** Feather material for headdress */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Materials")
    UMaterialInterface* FeatherMaterial;
    
    /** Medicine pouch leather material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shaman Materials")
    UMaterialInterface* MedicinePouchMaterial;

public:
    // === SHAMAN FUNCTIONS ===
    
    /** Apply shaman appearance preset */
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyShamanPreset();
    
    /** Perform healing ritual animation and effects */
    UFUNCTION(BlueprintCallable, Category = "Shaman Abilities")
    void PerformHealingRitual();
    
    /** Start/stop ritual fire and smoke effects */
    UFUNCTION(BlueprintCallable, Category = "Shaman Effects")
    void SetRitualEffects(bool bActive);
    
    /** Change ritual mask type */
    UFUNCTION(BlueprintCallable, Category = "Shaman Customization")
    void SetRitualMask(EChar_RitualMaskType NewMaskType);
    
    /** Increase healing knowledge through experience */
    UFUNCTION(BlueprintCallable, Category = "Character Progression")
    void GainHealingExperience(int32 ExperiencePoints);
    
    /** Check if shaman can perform specific healing type */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Shaman Abilities")
    bool CanPerformHealing(EChar_HealingType HealingType) const;

protected:
    // === INTERNAL FUNCTIONS ===
    
    /** Initialize shaman-specific equipment */
    void InitializeShamanEquipment();
    
    /** Configure ritual effects and lighting */
    void SetupRitualEffects();
    
    /** Apply ritual scars to character mesh */
    void ApplyRitualScars();
    
    /** Update staff appearance based on carving type */
    void UpdateStaffAppearance();
    
    /** Configure medicine pouch contents */
    void SetupMedicinePouch();
};

// TODO_ASSET_GENERATION_FAILED: Tribal shaman 3D model generation failed
// Required assets for manual creation:
// - Ritual staff with carved animal symbols and leather wrapping
// - Medicine pouch made from animal hide with herb compartments
// - Bone ritual mask with eye holes and decorative patterns
// - Feather headdress with various bird feathers and bone beads
// - Ritual scar texture overlays for weathered skin material
// - Carved wood material with age and wear patterns
// - Feather material with natural color variations
// - Medicine pouch leather with staining and wear marks