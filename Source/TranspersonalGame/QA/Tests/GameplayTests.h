#pragma once

#include "CoreMinimal.h"
#include "QA/QATestFramework.h"
#include "Misc/AutomationTest.h"

/**
 * Gameplay Test Declarations
 * 
 * These tests validate core gameplay mechanics specific to the survival game:
 * - Player survival systems
 * - Crafting and resource gathering
 * - Combat mechanics
 * - Dinosaur domestication
 * - Base building
 * - Inventory management
 */

/**
 * Test player survival mechanics
 */
class TRANSPERSONALGAME_API FPlayerSurvivalTest : public FTranspersonalQATestBase
{
public:
    FPlayerSurvivalTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;

protected:
    bool ValidateHungerSystem();
    bool ValidateThirstSystem();
    bool ValidateHealthSystem();
    bool ValidateTemperatureSystem();
};

/**
 * Test crafting system functionality
 */
class TRANSPERSONALGAME_API FCraftingSystemTest : public FTranspersonalQATestBase
{
public:
    FCraftingSystemTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;

protected:
    bool ValidateRecipeExecution();
    bool ValidateResourceConsumption();
    bool ValidateItemCreation();
};

/**
 * Test combat system mechanics
 */
class TRANSPERSONALGAME_API FCombatSystemTest : public FTranspersonalQATestBase
{
public:
    FCombatSystemTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;

protected:
    bool ValidateDamageCalculation();
    bool ValidateHitDetection();
    bool ValidateWeaponDurability();
    bool ValidatePlayerVsDinosaurCombat();
};

/**
 * Test dinosaur domestication system
 */
class TRANSPERSONALGAME_API FDomesticationSystemTest : public FTranspersonalQATestBase
{
public:
    FDomesticationSystemTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;

protected:
    bool ValidateTrustSystem();
    bool ValidateFeedingMechanics();
    bool ValidateBehaviorChanges();
    bool ValidateLongTermRelationship();
};

/**
 * Test base building mechanics
 */
class TRANSPERSONALGAME_API FBaseBuildingTest : public FTranspersonalQATestBase
{
public:
    FBaseBuildingTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;

protected:
    bool ValidateStructurePlacement();
    bool ValidateStructuralIntegrity();
    bool ValidateResourceRequirements();
    bool ValidateDefensiveStructures();
};

/**
 * Test inventory and resource management
 */
class TRANSPERSONALGAME_API FInventorySystemTest : public FTranspersonalQATestBase
{
public:
    FInventorySystemTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;

protected:
    bool ValidateItemStacking();
    bool ValidateWeightLimits();
    bool ValidateItemDurability();
    bool ValidateStorageContainers();
};

/**
 * Test resource gathering mechanics
 */
class TRANSPERSONALGAME_API FResourceGatheringTest : public FTranspersonalQATestBase
{
public:
    FResourceGatheringTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;

protected:
    bool ValidateToolEfficiency();
    bool ValidateResourceRespawn();
    bool ValidateResourceQuality();
    bool ValidateGatheringAnimations();
};