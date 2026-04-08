#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../Core/ConsciousnessSystem.h"
#include "ConsciousnessBasedBehavior.generated.h"

UENUM(BlueprintType)
enum class EBehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Seeking UMETA(DisplayName = "Seeking Knowledge"),
    Contemplating UMETA(DisplayName = "Contemplating"),
    Interacting UMETA(DisplayName = "Social Interaction"),
    Transforming UMETA(DisplayName = "Consciousness Transformation"),
    Guiding UMETA(DisplayName = "Guiding Others"),
    Meditating UMETA(DisplayName = "Meditative State")
};

USTRUCT(BlueprintType)
struct FBehaviorPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBehaviorState BehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredEmotions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    FBehaviorPattern()
    {
        BehaviorType = EBehaviorState::Idle;
        Duration = 5.0f;
        ConsciousnessThreshold = 0.5f;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FInteractionResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessShift;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UnlockedBehaviors;

    FInteractionResponse()
    {
        EmotionalImpact = 0.0f;
        ConsciousnessShift = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessBasedBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessBasedBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior system
    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    void UpdateBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    void SetBehaviorPattern(const FBehaviorPattern& NewPattern);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    EBehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    // Interaction system
    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    FInteractionResponse ProcessPlayerInteraction(const FString& InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    bool CanInteract() const;

    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    void StartGuidingBehavior(AActor* TargetActor);

    // Adaptive learning
    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    void LearnFromInteraction(const FInteractionResponse& Response);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    void AdaptBehaviorToConsciousness();

    // Emotional resonance
    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    void RespondToEmotionalField(const TArray<FString>& NearbyEmotions);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Behavior")
    float CalculateEmotionalResonance(const FString& Emotion) const;

protected:
    // Consciousness system reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consciousness")
    class UConsciousnessSystem* ConsciousnessSystem;

    // Current behavior state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
    EBehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
    float CurrentBehaviorTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
    FBehaviorPattern CurrentPattern;

    // Behavior patterns library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Patterns")
    TArray<FBehaviorPattern> AvailableBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Patterns")
    TArray<FBehaviorPattern> LearnedBehaviors;

    // Interaction settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    float LastInteractionTime;

    // Emotional resonance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Resonance")
    TMap<FString, float> EmotionalAffinities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Resonance")
    float ResonanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Resonance")
    float EmotionalInfluenceRadius;

    // Adaptive learning parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float LearningRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    int32 MaxLearnedBehaviors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning")
    TMap<FString, int32> InteractionHistory;

private:
    // Internal behavior management
    void SelectOptimalBehavior();
    bool EvaluateBehaviorConditions(const FBehaviorPattern& Pattern) const;
    void TransitionToBehavior(EBehaviorState NewState);
    
    // Consciousness integration
    void UpdateConsciousnessInfluence();
    float GetConsciousnessLevel() const;
    TArray<FString> GetActiveEmotions() const;
    
    // Learning algorithms
    void UpdateBehaviorPriorities();
    void CreateNewBehaviorPattern(const FInteractionResponse& Response);
    
    // Utility functions
    AActor* FindNearestPlayer() const;
    TArray<AActor*> FindNearbyNPCs() const;
    void BroadcastEmotionalState();

public:
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBehaviorChanged, EBehaviorState, OldState, EBehaviorState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Behavior Events")
    FOnBehaviorChanged OnBehaviorChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCompleted, FInteractionResponse, Response);
    UPROPERTY(BlueprintAssignable, Category = "Interaction Events")
    FOnInteractionCompleted OnInteractionCompleted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsciousnessShift, float, NewLevel);
    UPROPERTY(BlueprintAssignable, Category = "Consciousness Events")
    FOnConsciousnessShift OnConsciousnessShift;
};