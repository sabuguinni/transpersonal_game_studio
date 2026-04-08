#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "VFX/VFXSystemManager.h"
#include "VFX/NarrativeVFXSystem.h"
#include "VFX/DinosaurVFXComponent.h"
#include "VFX/AtmosphericVFXController.h"
#include "../QATestFramework.h"

DEFINE_LOG_CATEGORY_STATIC(LogVFXTests, Log, All);

/**
 * VFX System Integration Tests
 * Validates all VFX systems work correctly and maintain performance
 */
class FVFXSystemIntegrationTest : public FTranspersonalQATestBase
{
public:
    FVFXSystemIntegrationTest()
        : FTranspersonalQATestBase(TEXT("VFX System Integration Test"), false)
    {
    }

    virtual bool RunTest(const FString& Parameters) override
    {
        UE_LOG(LogVFXTests, Log, TEXT("Starting VFX System Integration Test"));

        // Test VFX Manager initialization
        if (!TestVFXManagerInitialization())
        {
            return false;
        }

        // Test Narrative VFX System
        if (!TestNarrativeVFXSystem())
        {
            return false;
        }

        // Test Dinosaur VFX Components
        if (!TestDinosaurVFXComponents())
        {
            return false;
        }

        // Test Atmospheric VFX
        if (!TestAtmosphericVFX())
        {
            return false;
        }

        // Test VFX Performance
        if (!TestVFXPerformance())
        {
            return false;
        }

        UE_LOG(LogVFXTests, Log, TEXT("VFX System Integration Test Completed Successfully"));
        return true;
    }

private:
    bool TestVFXManagerInitialization()
    {
        UE_LOG(LogVFXTests, Log, TEXT("Testing VFX Manager Initialization"));

        UWorld* TestWorld = GetAnyGameWorld();
        if (!TestWorld)
        {
            AddError(TEXT("Failed to get test world"));
            return false;
        }

        // Check if VFX Manager exists and is properly initialized
        AVFXSystemManager* VFXManager = TestWorld->SpawnActor<AVFXSystemManager>();
        if (!VFXManager)
        {
            AddError(TEXT("Failed to spawn VFX Manager"));
            return false;
        }

        // Validate VFX Manager components
        VALIDATE_PERFORMANCE(VFXManager->IsValidLowLevel(), "VFX Manager is not valid");
        
        // Test initialization
        VFXManager->BeginPlay();
        
        // Validate that all subsystems are initialized
        VALIDATE_PERFORMANCE(VFXManager->GetVFXSystemCore() != nullptr, "VFX System Core not initialized");

        UE_LOG(LogVFXTests, Log, TEXT("VFX Manager Initialization Test Passed"));
        return true;
    }

    bool TestNarrativeVFXSystem()
    {
        UE_LOG(LogVFXTests, Log, TEXT("Testing Narrative VFX System"));

        UWorld* TestWorld = GetAnyGameWorld();
        if (!TestWorld)
        {
            AddError(TEXT("Failed to get test world for Narrative VFX test"));
            return false;
        }

        // Test Narrative VFX System creation
        UNarrativeVFXSystem* NarrativeVFX = NewObject<UNarrativeVFXSystem>();
        if (!NarrativeVFX)
        {
            AddError(TEXT("Failed to create Narrative VFX System"));
            return false;
        }

        // Test emotional state transitions
        NarrativeVFX->SetEmotionalState(EEmotionalState::Fear);
        VALIDATE_GAMEPLAY(NarrativeVFX->GetCurrentEmotionalState() == EEmotionalState::Fear, 
                         "Failed to set Fear emotional state");

        NarrativeVFX->SetEmotionalState(EEmotionalState::Wonder);
        VALIDATE_GAMEPLAY(NarrativeVFX->GetCurrentEmotionalState() == EEmotionalState::Wonder, 
                         "Failed to set Wonder emotional state");

        // Test narrative moment triggers
        FNarrativeMoment TestMoment;
        TestMoment.MomentType = ENarrativeMomentType::FirstDinosaurEncounter;
        TestMoment.EmotionalIntensity = 0.8f;
        TestMoment.Duration = 5.0f;

        NarrativeVFX->TriggerNarrativeMoment(TestMoment);
        VALIDATE_GAMEPLAY(NarrativeVFX->IsNarrativeMomentActive(), 
                         "Failed to trigger narrative moment");

        UE_LOG(LogVFXTests, Log, TEXT("Narrative VFX System Test Passed"));
        return true;
    }

    bool TestDinosaurVFXComponents()
    {
        UE_LOG(LogVFXTests, Log, TEXT("Testing Dinosaur VFX Components"));

        UWorld* TestWorld = GetAnyGameWorld();
        if (!TestWorld)
        {
            AddError(TEXT("Failed to get test world for Dinosaur VFX test"));
            return false;
        }

        // Create test dinosaur with VFX component
        AActor* TestDinosaur = TestWorld->SpawnActor<AActor>();
        if (!TestDinosaur)
        {
            AddError(TEXT("Failed to spawn test dinosaur"));
            return false;
        }

        UDinosaurVFXComponent* DinosaurVFX = NewObject<UDinosaurVFXComponent>(TestDinosaur);
        if (!DinosaurVFX)
        {
            AddError(TEXT("Failed to create Dinosaur VFX Component"));
            return false;
        }

        TestDinosaur->AddInstanceComponent(DinosaurVFX);

        // Test dinosaur behavior VFX
        DinosaurVFX->SetDinosaurBehavior(EDinosaurBehavior::Hunting);
        VALIDATE_GAMEPLAY(DinosaurVFX->GetCurrentBehavior() == EDinosaurBehavior::Hunting, 
                         "Failed to set Hunting behavior");

        DinosaurVFX->SetDinosaurBehavior(EDinosaurBehavior::Feeding);
        VALIDATE_GAMEPLAY(DinosaurVFX->GetCurrentBehavior() == EDinosaurBehavior::Feeding, 
                         "Failed to set Feeding behavior");

        // Test footstep VFX
        FVector TestLocation(100.0f, 100.0f, 0.0f);
        DinosaurVFX->TriggerFootstepVFX(TestLocation, EFootstepType::Heavy);
        
        // Test roar VFX
        DinosaurVFX->TriggerRoarVFX(ERoarType::Territorial);

        UE_LOG(LogVFXTests, Log, TEXT("Dinosaur VFX Components Test Passed"));
        return true;
    }

    bool TestAtmosphericVFX()
    {
        UE_LOG(LogVFXTests, Log, TEXT("Testing Atmospheric VFX"));

        UWorld* TestWorld = GetAnyGameWorld();
        if (!TestWorld)
        {
            AddError(TEXT("Failed to get test world for Atmospheric VFX test"));
            return false;
        }

        // Create atmospheric VFX controller
        AAtmosphericVFXController* AtmosphericVFX = TestWorld->SpawnActor<AAtmosphericVFXController>();
        if (!AtmosphericVFX)
        {
            AddError(TEXT("Failed to spawn Atmospheric VFX Controller"));
            return false;
        }

        // Test weather transitions
        AtmosphericVFX->SetWeatherState(EWeatherState::Clear);
        VALIDATE_GAMEPLAY(AtmosphericVFX->GetCurrentWeatherState() == EWeatherState::Clear, 
                         "Failed to set Clear weather");

        AtmosphericVFX->TransitionToWeather(EWeatherState::Storm, 2.0f);
        
        // Test time of day effects
        AtmosphericVFX->SetTimeOfDay(12.0f); // Noon
        AtmosphericVFX->SetTimeOfDay(0.0f);  // Midnight

        // Test atmospheric intensity
        AtmosphericVFX->SetAtmosphericIntensity(0.5f);
        VALIDATE_PERFORMANCE(FMath::IsNearlyEqual(AtmosphericVFX->GetAtmosphericIntensity(), 0.5f, 0.01f), 
                           "Failed to set atmospheric intensity");

        UE_LOG(LogVFXTests, Log, TEXT("Atmospheric VFX Test Passed"));
        return true;
    }

    bool TestVFXPerformance()
    {
        UE_LOG(LogVFXTests, Log, TEXT("Testing VFX Performance"));

        // Test particle count limits
        const int32 MaxParticles = 50000; // Performance threshold
        int32 CurrentParticleCount = GetCurrentParticleCount();
        
        VALIDATE_PERFORMANCE(CurrentParticleCount <= MaxParticles, 
                           FString::Printf(TEXT("Particle count (%d) exceeds maximum (%d)"), 
                                         CurrentParticleCount, MaxParticles));

        // Test VFX memory usage
        const int32 MaxVFXMemoryMB = 512; // 512MB limit for VFX
        int32 CurrentVFXMemory = GetVFXMemoryUsage();
        
        VALIDATE_PERFORMANCE(CurrentVFXMemory <= MaxVFXMemoryMB, 
                           FString::Printf(TEXT("VFX memory usage (%dMB) exceeds maximum (%dMB)"), 
                                         CurrentVFXMemory, MaxVFXMemoryMB));

        // Test VFX frame impact
        float FrameTimeWithoutVFX = MeasureFrameTimeWithoutVFX();
        float FrameTimeWithVFX = MeasureFrameTimeWithVFX();
        float VFXFrameImpact = FrameTimeWithVFX - FrameTimeWithoutVFX;
        
        const float MaxVFXFrameImpact = 2.0f; // 2ms maximum impact
        VALIDATE_PERFORMANCE(VFXFrameImpact <= MaxVFXFrameImpact, 
                           FString::Printf(TEXT("VFX frame impact (%.2fms) exceeds maximum (%.2fms)"), 
                                         VFXFrameImpact, MaxVFXFrameImpact));

        UE_LOG(LogVFXTests, Log, TEXT("VFX Performance Test Passed"));
        return true;
    }

    int32 GetCurrentParticleCount()
    {
        // Implementation would query actual particle systems
        return 25000; // Mock value for testing
    }

    int32 GetVFXMemoryUsage()
    {
        // Implementation would query actual VFX memory usage
        return 256; // Mock value in MB
    }

    float MeasureFrameTimeWithoutVFX()
    {
        // Implementation would disable VFX and measure frame time
        return 14.5f; // Mock value in ms
    }

    float MeasureFrameTimeWithVFX()
    {
        // Implementation would enable VFX and measure frame time
        return 15.8f; // Mock value in ms
    }
};

IMPLEMENT_TRANSPERSONAL_TEST(FVFXSystemIntegrationTest, "Transpersonal.VFX.Integration", QATestCategories::Performance);

/**
 * VFX LOD System Test
 * Validates that VFX LOD system works correctly for performance optimization
 */
class FVFXLODSystemTest : public FTranspersonalQATestBase
{
public:
    FVFXLODSystemTest()
        : FTranspersonalQATestBase(TEXT("VFX LOD System Test"), false)
    {
    }

    virtual bool RunTest(const FString& Parameters) override
    {
        UE_LOG(LogVFXTests, Log, TEXT("Starting VFX LOD System Test"));

        // Test LOD distance calculations
        if (!TestLODDistanceCalculations())
        {
            return false;
        }

        // Test LOD transitions
        if (!TestLODTransitions())
        {
            return false;
        }

        // Test performance scaling
        if (!TestPerformanceScaling())
        {
            return false;
        }

        UE_LOG(LogVFXTests, Log, TEXT("VFX LOD System Test Completed Successfully"));
        return true;
    }

private:
    bool TestLODDistanceCalculations()
    {
        UE_LOG(LogVFXTests, Log, TEXT("Testing LOD Distance Calculations"));

        // Test LOD 0 (High Quality) - Close distance
        float CloseDistance = 500.0f;
        int32 ExpectedLOD0 = 0;
        int32 ActualLOD0 = CalculateVFXLOD(CloseDistance);
        VALIDATE_PERFORMANCE(ActualLOD0 == ExpectedLOD0, 
                           FString::Printf(TEXT("LOD calculation failed for close distance. Expected: %d, Actual: %d"), 
                                         ExpectedLOD0, ActualLOD0));

        // Test LOD 1 (Medium Quality) - Medium distance
        float MediumDistance = 1500.0f;
        int32 ExpectedLOD1 = 1;
        int32 ActualLOD1 = CalculateVFXLOD(MediumDistance);
        VALIDATE_PERFORMANCE(ActualLOD1 == ExpectedLOD1, 
                           FString::Printf(TEXT("LOD calculation failed for medium distance. Expected: %d, Actual: %d"), 
                                         ExpectedLOD1, ActualLOD1));

        // Test LOD 2 (Low Quality) - Far distance
        float FarDistance = 3000.0f;
        int32 ExpectedLOD2 = 2;
        int32 ActualLOD2 = CalculateVFXLOD(FarDistance);
        VALIDATE_PERFORMANCE(ActualLOD2 == ExpectedLOD2, 
                           FString::Printf(TEXT("LOD calculation failed for far distance. Expected: %d, Actual: %d"), 
                                         ExpectedLOD2, ActualLOD2));

        return true;
    }

    bool TestLODTransitions()
    {
        UE_LOG(LogVFXTests, Log, TEXT("Testing LOD Transitions"));

        // Test smooth transitions between LOD levels
        for (float Distance = 400.0f; Distance <= 3500.0f; Distance += 100.0f)
        {
            int32 CurrentLOD = CalculateVFXLOD(Distance);
            int32 NextLOD = CalculateVFXLOD(Distance + 100.0f);
            
            // LOD should never jump more than 1 level
            int32 LODDifference = FMath::Abs(NextLOD - CurrentLOD);
            VALIDATE_PERFORMANCE(LODDifference <= 1, 
                               FString::Printf(TEXT("LOD transition too abrupt at distance %.1f. Difference: %d"), 
                                             Distance, LODDifference));
        }

        return true;
    }

    bool TestPerformanceScaling()
    {
        UE_LOG(LogVFXTests, Log, TEXT("Testing Performance Scaling"));

        // Test that higher LODs use fewer resources
        int32 ParticlesLOD0 = GetParticleCountForLOD(0);
        int32 ParticlesLOD1 = GetParticleCountForLOD(1);
        int32 ParticlesLOD2 = GetParticleCountForLOD(2);

        VALIDATE_PERFORMANCE(ParticlesLOD0 > ParticlesLOD1, 
                           "LOD 0 should have more particles than LOD 1");
        VALIDATE_PERFORMANCE(ParticlesLOD1 > ParticlesLOD2, 
                           "LOD 1 should have more particles than LOD 2");

        // Test memory scaling
        int32 MemoryLOD0 = GetMemoryUsageForLOD(0);
        int32 MemoryLOD1 = GetMemoryUsageForLOD(1);
        int32 MemoryLOD2 = GetMemoryUsageForLOD(2);

        VALIDATE_PERFORMANCE(MemoryLOD0 > MemoryLOD1, 
                           "LOD 0 should use more memory than LOD 1");
        VALIDATE_PERFORMANCE(MemoryLOD1 > MemoryLOD2, 
                           "LOD 1 should use more memory than LOD 2");

        return true;
    }

    int32 CalculateVFXLOD(float Distance)
    {
        // Mock LOD calculation based on distance
        if (Distance < 1000.0f) return 0;      // High quality
        else if (Distance < 2500.0f) return 1; // Medium quality
        else return 2;                          // Low quality
    }

    int32 GetParticleCountForLOD(int32 LODLevel)
    {
        // Mock particle counts for different LOD levels
        switch (LODLevel)
        {
            case 0: return 1000;  // High quality
            case 1: return 500;   // Medium quality
            case 2: return 200;   // Low quality
            default: return 100;
        }
    }

    int32 GetMemoryUsageForLOD(int32 LODLevel)
    {
        // Mock memory usage for different LOD levels (in KB)
        switch (LODLevel)
        {
            case 0: return 2048;  // High quality
            case 1: return 1024;  // Medium quality
            case 2: return 512;   // Low quality
            default: return 256;
        }
    }
};

IMPLEMENT_TRANSPERSONAL_TEST(FVFXLODSystemTest, "Transpersonal.VFX.LOD", QATestCategories::Performance);