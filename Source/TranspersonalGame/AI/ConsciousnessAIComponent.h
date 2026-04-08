#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "ConsciousnessAIComponent.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Dormant         UMETA(DisplayName = "Dormant"),
    Awakening       UMETA(DisplayName = "Awakening"),
    Aware           UMETA(DisplayName = "Aware"),
    Enlightened     UMETA(DisplayName = "Enlightened"),
    Transcendent    UMETA(DisplayName = "Transcendent")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Curious         UMETA(DisplayName = "Curious"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Compassionate   UMETA(DisplayName = "Compassionate"),
    Hostile         UMETA(DisplayName = "Hostile"),
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Wise            UMETA(DisplayName = "Wise")
};

USTRUCT(BlueprintType)
struct FConsciousnessProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessState CurrentState = EConsciousnessState::Dormant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float AwarenessLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float SpiritualResonance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float EmpathyLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EEmotionalState EmotionalState = EEmotionalState::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    TArray<FString> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float PlayerRelationship = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessAIComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FConsciousnessProfile ConsciousnessProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float UpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float PlayerDetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float InteractionRadius = 300.0f;

    // Core AI Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness AI")
    void UpdateConsciousnessState();

    UFUNCTION(BlueprintCallable, Category = "Consciousness AI")
    void ProcessPlayerInteraction(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Consciousness AI")
    void RespondToEnvironmentalChange(const FString& ChangeType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Consciousness AI")
    void EvolveConsciousness(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Consciousness AI")
    FString GenerateDialogue();

    UFUNCTION(BlueprintCallable, Category = "Consciousness AI")
    void AddMemory(const FString& Memory);

    UFUNCTION(BlueprintCallable, Category = "Consciousness AI")
    bool ShouldInitiateInteraction() const;

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Consciousness AI")
    bool IsPlayerNearby() const;

    UFUNCTION(BlueprintPure, Category = "Consciousness AI")
    float GetConsciousnessEvolutionRate() const;

    UFUNCTION(BlueprintPure, Category = "Consciousness AI")
    FString GetCurrentMood() const;

private:
    float TimeSinceLastUpdate;
    AActor* DetectedPlayer;
    
    void UpdatePlayerDetection();
    void ProcessConsciousnessEvolution(float DeltaTime);
    void UpdateEmotionalState();
    EConsciousnessState CalculateNextConsciousnessState() const;
    EEmotionalState CalculateEmotionalResponse() const;
};