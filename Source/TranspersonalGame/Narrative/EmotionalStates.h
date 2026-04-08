#pragma once

#include "CoreMinimal.h"
#include "EmotionalStates.generated.h"

/**
 * Unified Emotional State System for Narrative and Quest Integration
 * 
 * This enum consolidates emotional states from both NarrativeSystem and QuestSystem
 * to ensure consistent communication between systems.
 * 
 * Design Philosophy:
 * - Each state represents a distinct emotional experience
 * - States can transition naturally between each other
 * - Both systems can map their specific needs to these core states
 */
UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    // Basic Emotional States
    Calm            UMETA(DisplayName = "Calm"),
    Anxious         UMETA(DisplayName = "Anxious"),
    Terrified       UMETA(DisplayName = "Terrified"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    
    // Discovery and Wonder States
    Curious         UMETA(DisplayName = "Curious"),
    Wonder          UMETA(DisplayName = "Awe and Wonder"),
    Discovery       UMETA(DisplayName = "Joy of Discovery"),
    
    // Social and Connection States
    Loneliness      UMETA(DisplayName = "Isolation and Loneliness"),
    Connection      UMETA(DisplayName = "Connection with Nature"),
    Empathy         UMETA(DisplayName = "Empathy with Creatures"),
    
    // Achievement and Progress States
    Determined      UMETA(DisplayName = "Determined"),
    Achievement     UMETA(DisplayName = "Accomplishment"),
    Hopeful         UMETA(DisplayName = "Hopeful"),
    
    // Tension and Conflict States
    Fear            UMETA(DisplayName = "Primal Fear"),
    Tension         UMETA(DisplayName = "Survival Tension")
};

/**
 * Emotional intensity levels for more nuanced state tracking
 */
UENUM(BlueprintType)
enum class EEmotionalIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Strong      UMETA(DisplayName = "Strong"),
    Overwhelming UMETA(DisplayName = "Overwhelming")
};

/**
 * Complete emotional context for narrative and quest systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEmotionalContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    EEmotionalState PrimaryState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    EEmotionalIntensity Intensity = EEmotionalIntensity::Moderate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    TArray<EEmotionalState> SecondaryStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    float Duration = 0.0f; // How long this state has been active

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    FString Trigger; // What caused this emotional state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    bool bIsTransitioning = false; // Currently changing states

    FEmotionalContext()
    {
        PrimaryState = EEmotionalState::Calm;
        Intensity = EEmotionalIntensity::Moderate;
        Duration = 0.0f;
        bIsTransitioning = false;
    }
};

/**
 * Utility functions for emotional state management
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEmotionalStateUtilities : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // State Transitions
    UFUNCTION(BlueprintCallable, Category = "Emotional States")
    static bool CanTransitionTo(EEmotionalState FromState, EEmotionalState ToState);

    UFUNCTION(BlueprintCallable, Category = "Emotional States")
    static float GetTransitionDuration(EEmotionalState FromState, EEmotionalState ToState);

    UFUNCTION(BlueprintCallable, Category = "Emotional States")
    static TArray<EEmotionalState> GetNaturalTransitions(EEmotionalState CurrentState);

    // State Analysis
    UFUNCTION(BlueprintCallable, Category = "Emotional States")
    static bool IsPositiveState(EEmotionalState State);

    UFUNCTION(BlueprintCallable, Category = "Emotional States")
    static bool IsHighEnergyState(EEmotionalState State);

    UFUNCTION(BlueprintCallable, Category = "Emotional States")
    static float GetEmotionalWeight(EEmotionalState State);

    // Context Utilities
    UFUNCTION(BlueprintCallable, Category = "Emotional States")
    static FEmotionalContext CreateEmotionalContext(EEmotionalState State, EEmotionalIntensity Intensity, const FString& Trigger);

    UFUNCTION(BlueprintCallable, Category = "Emotional States")
    static bool IsEmotionalContextValid(const FEmotionalContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Emotional States")
    static FEmotionalContext BlendEmotionalContexts(const FEmotionalContext& A, const FEmotionalContext& B, float BlendWeight);
};