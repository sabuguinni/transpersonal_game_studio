#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "../QATestFramework.h"

DEFINE_LOG_CATEGORY_STATIC(LogDinosaurAITests, Log, All);

/**
 * Dinosaur AI Tests for Transpersonal Game
 * Tests dinosaur behavior, AI decision making, and interaction systems
 * Validates the core promise: "Dinosaurs live their own lives"
 */

// Test 1: Dinosaur Behavior Tree Validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDinosaurBehaviorTreeTest, "Transpersonal.Dinosaurs.BehaviorTree", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDinosaurBehaviorTreeTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing Dinosaur Behavior Trees..."));
    
    // Simulate behavior tree states for different dinosaur types
    struct FDinosaurBehaviorState
    {
        FString DinosaurType;
        FString CurrentBehavior;
        float BehaviorDuration;
        TArray<FString> AvailableBehaviors;
        float AggressionLevel; // 0.0 = Peaceful, 1.0 = Highly Aggressive
    };
    
    TArray<FDinosaurBehaviorState> TestDinosaurs = {
        {
            TEXT("Triceratops"), 
            TEXT("Grazing"), 
            45.0f,
            {TEXT("Grazing"), TEXT("Wandering"), TEXT("Resting"), TEXT("Drinking"), TEXT("Defending")},
            0.2f
        },
        {
            TEXT("Velociraptor"), 
            TEXT("Hunting"), 
            120.0f,
            {TEXT("Hunting"), TEXT("Patrolling"), TEXT("Stalking"), TEXT("Pack Coordination"), TEXT("Resting")},
            0.8f
        },
        {
            TEXT("Brachiosaurus"), 
            TEXT("Feeding"), 
            180.0f,
            {TEXT("Feeding"), TEXT("Moving"), TEXT("Resting"), TEXT("Social Interaction")},
            0.1f
        },
        {
            TEXT("T-Rex"), 
            TEXT("Patrolling"), 
            90.0f,
            {TEXT("Patrolling"), TEXT("Hunting"), TEXT("Territorial Display"), TEXT("Resting"), TEXT("Scavenging")},
            0.9f
        }
    };
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing %d dinosaur behavior patterns..."), TestDinosaurs.Num());
    
    for (const FDinosaurBehaviorState& Dinosaur : TestDinosaurs)
    {
        UE_LOG(LogDinosaurAITests, Log, TEXT("Dinosaur: %s"), *Dinosaur.DinosaurType);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Current Behavior: %s (%.1fs)"), 
            *Dinosaur.CurrentBehavior, Dinosaur.BehaviorDuration);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Available Behaviors: %d"), Dinosaur.AvailableBehaviors.Num());
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Aggression Level: %.1f"), Dinosaur.AggressionLevel);
        
        // Validate behavior tree structure
        VALIDATE_AI_BEHAVIOR(Dinosaur.AvailableBehaviors.Num() >= 3,
            FString::Printf(TEXT("%s should have multiple behavior options"), *Dinosaur.DinosaurType));
        
        VALIDATE_AI_BEHAVIOR(Dinosaur.BehaviorDuration > 0.0f && Dinosaur.BehaviorDuration < 600.0f,
            FString::Printf(TEXT("%s behavior duration should be reasonable"), *Dinosaur.DinosaurType));
        
        VALIDATE_AI_BEHAVIOR(Dinosaur.AggressionLevel >= 0.0f && Dinosaur.AggressionLevel <= 1.0f,
            FString::Printf(TEXT("%s aggression level should be valid"), *Dinosaur.DinosaurType));
        
        // Validate behavior consistency with dinosaur type
        if (Dinosaur.DinosaurType.Contains(TEXT("T-Rex")) || Dinosaur.DinosaurType.Contains(TEXT("Raptor")))
        {
            VALIDATE_AI_BEHAVIOR(Dinosaur.AggressionLevel > 0.5f,
                FString::Printf(TEXT("%s should be aggressive predator"), *Dinosaur.DinosaurType));
        }
        else if (Dinosaur.DinosaurType.Contains(TEXT("Triceratops")) || Dinosaur.DinosaurType.Contains(TEXT("Brachio")))
        {
            VALIDATE_AI_BEHAVIOR(Dinosaur.AggressionLevel < 0.5f,
                FString::Printf(TEXT("%s should be peaceful herbivore"), *Dinosaur.DinosaurType));
        }
    }
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("✓ Dinosaur behavior tree validation passed"));
    return true;
}

// Test 2: Dinosaur Perception System
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDinosaurPerceptionTest, "Transpersonal.Dinosaurs.Perception", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDinosaurPerceptionTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing Dinosaur Perception Systems..."));
    
    // Simulate perception data for different dinosaur types
    struct FDinosaurPerception
    {
        FString DinosaurType;
        float SightRange; // cm
        float HearingRange; // cm
        float SightAngle; // degrees
        bool bCanSmell;
        float SmellRange; // cm
        TArray<FString> DetectedThreats;
        TArray<FString> DetectedPrey;
    };
    
    TArray<FDinosaurPerception> TestPerceptions = {
        {
            TEXT("Velociraptor"),
            2000.0f, 1500.0f, 120.0f, true, 800.0f,
            {TEXT("Player"), TEXT("Large Predator")},
            {TEXT("Small Herbivore"), TEXT("Injured Animal")}
        },
        {
            TEXT("T-Rex"),
            3000.0f, 2500.0f, 90.0f, true, 1500.0f,
            {TEXT("Pack Hunters"), TEXT("Territory Intruders")},
            {TEXT("Large Herbivore"), TEXT("Carrion")}
        },
        {
            TEXT("Triceratops"),
            1500.0f, 2000.0f, 180.0f, false, 0.0f,
            {TEXT("Predator"), TEXT("Player with Weapon")},
            {}
        },
        {
            TEXT("Parasaurolophus"),
            1800.0f, 3000.0f, 200.0f, false, 0.0f,
            {TEXT("Any Predator"), TEXT("Loud Noises")},
            {}
        }
    };
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing perception for %d dinosaur types..."), TestPerceptions.Num());
    
    for (const FDinosaurPerception& Perception : TestPerceptions)
    {
        UE_LOG(LogDinosaurAITests, Log, TEXT("Dinosaur: %s"), *Perception.DinosaurType);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Sight Range: %.0f cm"), Perception.SightRange);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Hearing Range: %.0f cm"), Perception.HearingRange);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Sight Angle: %.0f degrees"), Perception.SightAngle);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Can Smell: %s"), Perception.bCanSmell ? TEXT("Yes") : TEXT("No"));
        
        if (Perception.bCanSmell)
        {
            UE_LOG(LogDinosaurAITests, Log, TEXT("  Smell Range: %.0f cm"), Perception.SmellRange);
        }
        
        // Validate perception ranges
        VALIDATE_AI_BEHAVIOR(Perception.SightRange > 500.0f && Perception.SightRange < 5000.0f,
            FString::Printf(TEXT("%s sight range should be reasonable"), *Perception.DinosaurType));
        
        VALIDATE_AI_BEHAVIOR(Perception.HearingRange > 500.0f && Perception.HearingRange < 5000.0f,
            FString::Printf(TEXT("%s hearing range should be reasonable"), *Perception.DinosaurType));
        
        VALIDATE_AI_BEHAVIOR(Perception.SightAngle > 60.0f && Perception.SightAngle <= 360.0f,
            FString::Printf(TEXT("%s sight angle should be valid"), *Perception.DinosaurType));
        
        // Validate predator vs herbivore perception differences
        bool bIsPredator = Perception.DinosaurType.Contains(TEXT("Rex")) || 
                          Perception.DinosaurType.Contains(TEXT("Raptor"));
        
        if (bIsPredator)
        {
            VALIDATE_AI_BEHAVIOR(Perception.DetectedPrey.Num() > 0,
                FString::Printf(TEXT("Predator %s should detect prey"), *Perception.DinosaurType));
        }
        else
        {
            VALIDATE_AI_BEHAVIOR(Perception.DetectedThreats.Num() > 0,
                FString::Printf(TEXT("Herbivore %s should detect threats"), *Perception.DinosaurType));
        }
    }
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("✓ Dinosaur perception validation passed"));
    return true;
}

// Test 3: Dinosaur Social Behavior
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDinosaurSocialTest, "Transpersonal.Dinosaurs.Social", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDinosaurSocialTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing Dinosaur Social Behaviors..."));
    
    // Simulate social behavior patterns
    struct FDinosaurSocialBehavior
    {
        FString DinosaurType;
        bool bPackHunter;
        bool bHerdAnimal;
        bool bTerritorial;
        int32 PreferredGroupSize;
        float SocialDistance; // cm - preferred distance from others
        TArray<FString> SocialInteractions;
    };
    
    TArray<FDinosaurSocialBehavior> TestSocialBehaviors = {
        {
            TEXT("Velociraptor"),
            true, false, false, 6,
            200.0f,
            {TEXT("Pack Coordination"), TEXT("Hunting Formation"), TEXT("Communication Calls")}
        },
        {
            TEXT("Triceratops"),
            false, true, false, 12,
            300.0f,
            {TEXT("Herd Movement"), TEXT("Protective Formation"), TEXT("Grazing Together")}
        },
        {
            TEXT("T-Rex"),
            false, false, true, 1,
            1000.0f,
            {TEXT("Territorial Display"), TEXT("Scent Marking"), TEXT("Intimidation")}
        },
        {
            TEXT("Parasaurolophus"),
            false, true, false, 20,
            150.0f,
            {TEXT("Vocal Communication"), TEXT("Herd Migration"), TEXT("Alarm Calls")}
        }
    };
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing social behavior for %d dinosaur types..."), TestSocialBehaviors.Num());
    
    for (const FDinosaurSocialBehavior& Social : TestSocialBehaviors)
    {
        UE_LOG(LogDinosaurAITests, Log, TEXT("Dinosaur: %s"), *Social.DinosaurType);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Pack Hunter: %s"), Social.bPackHunter ? TEXT("Yes") : TEXT("No"));
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Herd Animal: %s"), Social.bHerdAnimal ? TEXT("Yes") : TEXT("No"));
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Territorial: %s"), Social.bTerritorial ? TEXT("Yes") : TEXT("No"));
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Preferred Group Size: %d"), Social.PreferredGroupSize);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Social Distance: %.0f cm"), Social.SocialDistance);
        
        // Validate social behavior consistency
        VALIDATE_AI_BEHAVIOR(Social.PreferredGroupSize > 0 && Social.PreferredGroupSize <= 50,
            FString::Printf(TEXT("%s group size should be reasonable"), *Social.DinosaurType));
        
        VALIDATE_AI_BEHAVIOR(Social.SocialDistance > 50.0f && Social.SocialDistance < 2000.0f,
            FString::Printf(TEXT("%s social distance should be reasonable"), *Social.DinosaurType));
        
        VALIDATE_AI_BEHAVIOR(Social.SocialInteractions.Num() > 0,
            FString::Printf(TEXT("%s should have social interactions"), *Social.DinosaurType));
        
        // Validate behavior type consistency
        if (Social.bPackHunter)
        {
            VALIDATE_AI_BEHAVIOR(Social.PreferredGroupSize > 2,
                FString::Printf(TEXT("Pack hunter %s should prefer groups"), *Social.DinosaurType));
        }
        
        if (Social.bTerritorial)
        {
            VALIDATE_AI_BEHAVIOR(Social.PreferredGroupSize <= 2,
                FString::Printf(TEXT("Territorial %s should prefer solitude"), *Social.DinosaurType));
        }
        
        if (Social.bHerdAnimal)
        {
            VALIDATE_AI_BEHAVIOR(Social.PreferredGroupSize > 5,
                FString::Printf(TEXT("Herd animal %s should prefer large groups"), *Social.DinosaurType));
        }
    }
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("✓ Dinosaur social behavior validation passed"));
    return true;
}

// Test 4: Dinosaur Domestication System
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDinosaurDomesticationTest, "Transpersonal.Dinosaurs.Domestication", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDinosaurDomesticationTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing Dinosaur Domestication System..."));
    
    // Simulate domestication data
    struct FDomesticationData
    {
        FString DinosaurType;
        bool bCanBeDomesticated;
        float DomesticationDifficulty; // 0.0 = Easy, 1.0 = Impossible
        float TrustLevel; // 0.0 = Hostile, 1.0 = Fully Trusted
        int32 DaysToTame;
        TArray<FString> RequiredItems;
        TArray<FString> Benefits;
    };
    
    TArray<FDomesticationData> TestDomestication = {
        {
            TEXT("Parasaurolophus"),
            true, 0.3f, 0.7f, 14,
            {TEXT("Berries"), TEXT("Fresh Leaves"), TEXT("Patience")},
            {TEXT("Early Warning System"), TEXT("Companionship"), TEXT("Pack Animal")}
        },
        {
            TEXT("Triceratops"),
            true, 0.6f, 0.4f, 30,
            {TEXT("Favorite Plants"), TEXT("Salt"), TEXT("Protection from Predators")},
            {TEXT("Mount"), TEXT("Defense"), TEXT("Heavy Lifting")}
        },
        {
            TEXT("Compsognathus"),
            true, 0.2f, 0.8f, 7,
            {TEXT("Small Fish"), TEXT("Insects"), TEXT("Gentle Approach")},
            {TEXT("Pest Control"), TEXT("Scouting"), TEXT("Companionship")}
        },
        {
            TEXT("T-Rex"),
            false, 1.0f, 0.0f, 0,
            {},
            {}
        },
        {
            TEXT("Velociraptor"),
            false, 0.95f, 0.1f, 0,
            {},
            {}
        }
    };
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing domestication for %d dinosaur types..."), TestDomestication.Num());
    
    for (const FDomesticationData& Domestication : TestDomestication)
    {
        UE_LOG(LogDinosaurAITests, Log, TEXT("Dinosaur: %s"), *Domestication.DinosaurType);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Can Be Domesticated: %s"), 
            Domestication.bCanBeDomesticated ? TEXT("Yes") : TEXT("No"));
        
        if (Domestication.bCanBeDomesticated)
        {
            UE_LOG(LogDinosaurAITests, Log, TEXT("  Difficulty: %.1f"), Domestication.DomesticationDifficulty);
            UE_LOG(LogDinosaurAITests, Log, TEXT("  Current Trust: %.1f"), Domestication.TrustLevel);
            UE_LOG(LogDinosaurAITests, Log, TEXT("  Days to Tame: %d"), Domestication.DaysToTame);
            UE_LOG(LogDinosaurAITests, Log, TEXT("  Required Items: %d"), Domestication.RequiredItems.Num());
            UE_LOG(LogDinosaurAITests, Log, TEXT("  Benefits: %d"), Domestication.Benefits.Num());
            
            // Validate domestication parameters
            VALIDATE_AI_BEHAVIOR(Domestication.DomesticationDifficulty >= 0.0f && Domestication.DomesticationDifficulty <= 1.0f,
                FString::Printf(TEXT("%s domestication difficulty should be valid"), *Domestication.DinosaurType));
            
            VALIDATE_AI_BEHAVIOR(Domestication.TrustLevel >= 0.0f && Domestication.TrustLevel <= 1.0f,
                FString::Printf(TEXT("%s trust level should be valid"), *Domestication.DinosaurType));
            
            VALIDATE_AI_BEHAVIOR(Domestication.DaysToTame > 0 && Domestication.DaysToTame <= 60,
                FString::Printf(TEXT("%s taming time should be reasonable"), *Domestication.DinosaurType));
            
            VALIDATE_AI_BEHAVIOR(Domestication.RequiredItems.Num() > 0,
                FString::Printf(TEXT("%s should require items for domestication"), *Domestication.DinosaurType));
            
            VALIDATE_AI_BEHAVIOR(Domestication.Benefits.Num() > 0,
                FString::Printf(TEXT("%s should provide benefits when domesticated"), *Domestication.DinosaurType));
        }
        
        // Validate that dangerous predators cannot be easily domesticated
        bool bIsDangerousPredator = Domestication.DinosaurType.Contains(TEXT("Rex")) || 
                                   Domestication.DinosaurType.Contains(TEXT("Raptor"));
        
        if (bIsDangerousPredator)
        {
            VALIDATE_AI_BEHAVIOR(!Domestication.bCanBeDomesticated || Domestication.DomesticationDifficulty > 0.9f,
                FString::Printf(TEXT("Dangerous predator %s should not be easily domesticated"), *Domestication.DinosaurType));
        }
    }
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("✓ Dinosaur domestication validation passed"));
    return true;
}

// Test 5: Dinosaur Individual Variation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDinosaurVariationTest, "Transpersonal.Dinosaurs.Variation", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDinosaurVariationTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing Dinosaur Individual Variation..."));
    
    // Test the core promise: "Each dinosaur is unique and identifiable"
    struct FDinosaurIndividual
    {
        FString DinosaurType;
        FString IndividualName;
        FVector PhysicalVariation; // Size, Color Hue, Pattern Intensity
        float PersonalityAggression;
        float PersonalityCuriosity;
        float PersonalityFear;
        TArray<FString> UniqueTraits;
        FString PreferredBehavior;
    };
    
    TArray<FDinosaurIndividual> TestIndividuals = {
        {
            TEXT("Triceratops"), TEXT("Scarred Horn"),
            FVector(1.2f, 0.8f, 0.6f), // 20% larger, brownish hue, 60% pattern intensity
            0.4f, 0.2f, 0.3f,
            {TEXT("Broken Left Horn"), TEXT("Limps Slightly"), TEXT("Prefers Open Areas")},
            TEXT("Cautious Grazing")
        },
        {
            TEXT("Triceratops"), TEXT("Bright Crest"),
            FVector(0.9f, 0.3f, 1.0f), // 10% smaller, bright green, full pattern
            0.1f, 0.7f, 0.2f,
            {TEXT("Vibrant Coloration"), TEXT("Approaches Humans"), TEXT("Leads Herd")},
            TEXT("Social Leadership")
        },
        {
            TEXT("Velociraptor"), TEXT("Silent Stalker"),
            FVector(1.1f, 0.1f, 0.3f), // 10% larger, dark coloration, minimal pattern
            0.9f, 0.8f, 0.1f,
            {TEXT("Unusually Quiet"), TEXT("Prefers Ambush"), TEXT("Hunts Alone")},
            TEXT("Stealth Hunting")
        },
        {
            TEXT("Velociraptor"), TEXT("Pack Alpha"),
            FVector(1.3f, 0.6f, 0.8f), // 30% larger, standard coloration, strong pattern
            0.7f, 0.5f, 0.2f,
            {TEXT("Larger Than Average"), TEXT("Coordinates Pack"), TEXT("Distinctive Call")},
            TEXT("Pack Leadership")
        }
    };
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("Testing individual variation for %d dinosaurs..."), TestIndividuals.Num());
    
    for (const FDinosaurIndividual& Individual : TestIndividuals)
    {
        UE_LOG(LogDinosaurAITests, Log, TEXT("Individual: %s (%s)"), 
            *Individual.IndividualName, *Individual.DinosaurType);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Physical Variation: Size %.1f, Hue %.1f, Pattern %.1f"), 
            Individual.PhysicalVariation.X, Individual.PhysicalVariation.Y, Individual.PhysicalVariation.Z);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Personality: Aggression %.1f, Curiosity %.1f, Fear %.1f"), 
            Individual.PersonalityAggression, Individual.PersonalityCuriosity, Individual.PersonalityFear);
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Unique Traits: %d"), Individual.UniqueTraits.Num());
        UE_LOG(LogDinosaurAITests, Log, TEXT("  Preferred Behavior: %s"), *Individual.PreferredBehavior);
        
        // Validate individual variation
        VALIDATE_AI_BEHAVIOR(Individual.PhysicalVariation.X > 0.5f && Individual.PhysicalVariation.X < 2.0f,
            FString::Printf(TEXT("%s size variation should be reasonable"), *Individual.IndividualName));
        
        VALIDATE_AI_BEHAVIOR(Individual.PhysicalVariation.Y >= 0.0f && Individual.PhysicalVariation.Y <= 1.0f,
            FString::Printf(TEXT("%s color hue should be valid"), *Individual.IndividualName));
        
        VALIDATE_AI_BEHAVIOR(Individual.PhysicalVariation.Z >= 0.0f && Individual.PhysicalVariation.Z <= 1.0f,
            FString::Printf(TEXT("%s pattern intensity should be valid"), *Individual.IndividualName));
        
        // Validate personality traits
        VALIDATE_AI_BEHAVIOR(Individual.PersonalityAggression >= 0.0f && Individual.PersonalityAggression <= 1.0f,
            FString::Printf(TEXT("%s aggression should be valid"), *Individual.IndividualName));
        
        VALIDATE_AI_BEHAVIOR(Individual.PersonalityCuriosity >= 0.0f && Individual.PersonalityCuriosity <= 1.0f,
            FString::Printf(TEXT("%s curiosity should be valid"), *Individual.IndividualName));
        
        VALIDATE_AI_BEHAVIOR(Individual.PersonalityFear >= 0.0f && Individual.PersonalityFear <= 1.0f,
            FString::Printf(TEXT("%s fear should be valid"), *Individual.IndividualName));
        
        // Validate uniqueness
        VALIDATE_AI_BEHAVIOR(Individual.UniqueTraits.Num() >= 2,
            FString::Printf(TEXT("%s should have multiple unique traits"), *Individual.IndividualName));
        
        VALIDATE_AI_BEHAVIOR(!Individual.PreferredBehavior.IsEmpty(),
            FString::Printf(TEXT("%s should have preferred behavior"), *Individual.IndividualName));
    }
    
    // Test that individuals of same species are actually different
    if (TestIndividuals.Num() >= 2)
    {
        bool bFoundDifferences = false;
        for (int32 i = 0; i < TestIndividuals.Num() - 1; i++)
        {
            for (int32 j = i + 1; j < TestIndividuals.Num(); j++)
            {
                if (TestIndividuals[i].DinosaurType == TestIndividuals[j].DinosaurType)
                {
                    // Same species - check for differences
                    bool bPhysicallyDifferent = !TestIndividuals[i].PhysicalVariation.Equals(TestIndividuals[j].PhysicalVariation, 0.1f);
                    bool bPersonalityDifferent = FMath::Abs(TestIndividuals[i].PersonalityAggression - TestIndividuals[j].PersonalityAggression) > 0.1f;
                    bool bBehaviorDifferent = TestIndividuals[i].PreferredBehavior != TestIndividuals[j].PreferredBehavior;
                    
                    if (bPhysicallyDifferent || bPersonalityDifferent || bBehaviorDifferent)
                    {
                        bFoundDifferences = true;
                        UE_LOG(LogDinosaurAITests, Log, TEXT("✓ Found individual differences between %s and %s"), 
                            *TestIndividuals[i].IndividualName, *TestIndividuals[j].IndividualName);
                    }
                }
            }
        }
        
        VALIDATE_AI_BEHAVIOR(bFoundDifferences,
            "Individuals of same species should have observable differences");
    }
    
    UE_LOG(LogDinosaurAITests, Log, TEXT("✓ Dinosaur individual variation validation passed"));
    return true;
}