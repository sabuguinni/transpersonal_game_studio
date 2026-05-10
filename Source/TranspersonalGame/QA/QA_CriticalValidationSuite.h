#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/PostProcessVolume.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

/**
 * Critical validation suite for QA Agent
 * Tests core game systems, compilation status, and integration health
 */

/**
 * Test that validates all critical C++ classes can be loaded and instantiated
 */
DECLARE_SIMPLE_AUTOMATION_TEST(FQA_ClassLoadingValidationTest, "TranspersonalGame.QA.Critical.ClassLoading", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * Test that validates MinPlayableMap has all required actors and components
 */
DECLARE_SIMPLE_AUTOMATION_TEST(FQA_MinPlayableMapValidationTest, "TranspersonalGame.QA.Critical.MinPlayableMap", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * Test that validates character movement and basic gameplay functionality
 */
DECLARE_SIMPLE_AUTOMATION_TEST(FQA_GameplayValidationTest, "TranspersonalGame.QA.Critical.Gameplay", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * Test that validates VFX systems integration and performance
 */
DECLARE_SIMPLE_AUTOMATION_TEST(FQA_VFXIntegrationValidationTest, "TranspersonalGame.QA.Critical.VFXIntegration", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * Test that validates environment and lighting meets Cretaceous period requirements
 */
DECLARE_SIMPLE_AUTOMATION_TEST(FQA_EnvironmentValidationTest, "TranspersonalGame.QA.Critical.Environment", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * Test that validates performance metrics and memory usage
 */
DECLARE_SIMPLE_AUTOMATION_TEST(FQA_PerformanceValidationTest, "TranspersonalGame.QA.Critical.Performance", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * Test that validates asset loading and content browser organization
 */
DECLARE_SIMPLE_AUTOMATION_TEST(FQA_AssetValidationTest, "TranspersonalGame.QA.Critical.Assets", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * Test that validates compilation status and module dependencies
 */
DECLARE_SIMPLE_AUTOMATION_TEST(FQA_CompilationValidationTest, "TranspersonalGame.QA.Critical.Compilation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * Utility class for QA validation operations
 */
UCLASS()
class TRANSPERSONALGAME_API UQA_ValidationUtilities : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Get the current test world (editor or game world)
     */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    static UWorld* GetValidationWorld();

    /**
     * Count actors of a specific class in the world
     */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    static int32 CountActorsOfClass(UWorld* World, UClass* ActorClass);

    /**
     * Validate that an actor has required components
     */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    static bool ValidateActorComponents(AActor* Actor, const TArray<UClass*>& RequiredComponents);

    /**
     * Check if a class can be loaded from its path
     */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    static bool CanLoadClass(const FString& ClassPath);

    /**
     * Get memory usage statistics
     */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    static FString GetMemoryUsageStats();

    /**
     * Validate lighting setup for Cretaceous period
     */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    static bool ValidateCretaceousLighting(UWorld* World);

    /**
     * Check VFX system performance
     */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    static bool ValidateVFXPerformance(UWorld* World);

    /**
     * Generate validation report
     */
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    static FString GenerateValidationReport(const TArray<FString>& TestResults);

private:
    /**
     * Internal helper to check component requirements
     */
    static bool CheckComponentRequirement(AActor* Actor, UClass* ComponentClass);
};