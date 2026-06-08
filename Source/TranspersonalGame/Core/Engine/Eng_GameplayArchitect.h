#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "SharedTypes.h"
#include "Eng_GameplayArchitect.generated.h"

// Forward declarations
class UEng_BiomeSystemManager;
class UEng_ArchitecturalCore;
class UEng_CompilationManager;

/**
 * Eng_GameplayArchitect - Supreme gameplay architecture authority
 * Defines and enforces all gameplay system architectural patterns, integration rules,
 * and cross-system communication protocols for the prehistoric survival game.
 * This is the master architect that ensures all gameplay systems work cohesively.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_GameplayArchitect : public AActor
{
    GENERATED_BODY()

public:
    AEng_GameplayArchitect();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ===== CORE ARCHITECTURAL COMPONENTS =====
    
    /** Root scene component for architectural organization */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    USceneComponent* ArchitecturalRoot;

    /** Visual representation mesh for debugging */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UStaticMeshComponent* ArchitecturalMesh;

    // ===== GAMEPLAY SYSTEM ARCHITECTURE =====

    /** Current gameplay architecture state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Architecture")
    EEng_ArchitecturalState GameplayArchitectureState;

    /** Gameplay system integration level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Architecture")
    EEng_SystemIntegrationLevel SystemIntegrationLevel;

    /** Active gameplay patterns count */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay Architecture")
    int32 ActiveGameplayPatterns;

    /** Cross-system communication channels */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay Architecture")
    int32 CrossSystemChannels;

    // ===== SURVIVAL SYSTEM ARCHITECTURE =====

    /** Survival mechanics integration state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    bool bSurvivalSystemsIntegrated;

    /** Hunger/thirst/stamina architecture validation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    bool bSurvivalStatsArchitectureValid;

    /** Crafting system architectural compliance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    bool bCraftingArchitectureCompliant;

    /** Shelter building system integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Architecture")
    bool bShelterSystemIntegrated;

    // ===== DINOSAUR SYSTEM ARCHITECTURE =====

    /** Dinosaur AI architecture validation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Architecture")
    bool bDinosaurAIArchitectureValid;

    /** Dinosaur behavior tree integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Architecture")
    bool bDinosaurBehaviorTreesIntegrated;

    /** Dinosaur ecology system compliance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Architecture")
    bool bDinosaurEcologyCompliant;

    /** Predator-prey relationship architecture */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Architecture")
    bool bPredatorPreyArchitectureValid;

    // ===== WORLD SYSTEM ARCHITECTURE =====

    /** Biome system architectural integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Architecture")
    bool bBiomeSystemArchitectureIntegrated;

    /** Terrain generation architecture compliance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Architecture")
    bool bTerrainArchitectureCompliant;

    /** Weather system integration state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Architecture")
    bool bWeatherSystemIntegrated;

    /** Day/night cycle architecture validation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Architecture")
    bool bDayNightCycleArchitectureValid;

    // ===== COMBAT SYSTEM ARCHITECTURE =====

    /** Combat mechanics architectural validation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Architecture")
    bool bCombatMechanicsArchitectureValid;

    /** Weapon system integration state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Architecture")
    bool bWeaponSystemIntegrated;

    /** Damage calculation architecture compliance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Architecture")
    bool bDamageArchitectureCompliant;

    /** Tactical combat AI integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Architecture")
    bool bTacticalCombatAIIntegrated;

    // ===== ARCHITECTURAL VALIDATION METHODS =====

    /** Initialize all gameplay architectural systems */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeGameplayArchitecture();

    /** Validate survival system architectural compliance */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSurvivalArchitecture();

    /** Validate dinosaur system architectural integration */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateDinosaurArchitecture();

    /** Validate world system architectural compliance */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateWorldArchitecture();

    /** Validate combat system architectural integration */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateCombatArchitecture();

    /** Perform comprehensive architectural validation */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool PerformComprehensiveArchitecturalValidation();

    /** Get current architectural health score (0-100) */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetArchitecturalHealthScore() const;

    /** Get detailed architectural status report */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FString GetArchitecturalStatusReport() const;

    // ===== SYSTEM INTEGRATION METHODS =====

    /** Integrate survival systems with world systems */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateSurvivalWithWorldSystems();

    /** Integrate dinosaur AI with biome systems */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateDinosaurAIWithBiomeSystems();

    /** Integrate combat systems with survival mechanics */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateCombatWithSurvivalSystems();

    /** Establish cross-system communication channels */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void EstablishCrossSystemCommunication();

    // ===== ARCHITECTURAL GOVERNANCE =====

    /** Enforce architectural compliance across all systems */
    UFUNCTION(BlueprintCallable, Category = "Governance")
    void EnforceArchitecturalCompliance();

    /** Audit system architectural patterns */
    UFUNCTION(BlueprintCallable, Category = "Governance")
    void AuditSystemArchitecturalPatterns();

    /** Generate architectural improvement recommendations */
    UFUNCTION(BlueprintCallable, Category = "Governance")
    TArray<FString> GenerateArchitecturalImprovements();

private:
    /** Internal architectural state tracking */
    float ArchitecturalHealthScore;
    
    /** Last validation timestamp */
    float LastValidationTime;
    
    /** Architectural compliance flags */
    TMap<FString, bool> SystemComplianceFlags;
    
    /** Cross-system integration status */
    TMap<FString, EEng_SystemIntegrationLevel> SystemIntegrationStatus;
};