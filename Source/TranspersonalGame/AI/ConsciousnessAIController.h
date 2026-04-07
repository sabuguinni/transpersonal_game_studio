#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "../Core/ConsciousnessComponent.h"
#include "ConsciousnessAIController.generated.h"

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
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Agitated        UMETA(DisplayName = "Agitated"),
    Compassionate   UMETA(DisplayName = "Compassionate")
};

USTRUCT(BlueprintType)
struct FConsciousnessMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    FConsciousnessMemory()
    {
        Location = FVector::ZeroVector;
        ConsciousnessLevel = 0.0f;
        Timestamp = 0.0f;
        EventDescription = TEXT("");
    }
};

UCLASS()
class TRANSPERSONALGAME_API AConsciousnessAIController : public AAIController
{
    GENERATED_BODY()

public:
    AConsciousnessAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Consciousness Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consciousness")
    class UConsciousnessComponent* ConsciousnessComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Consciousness State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessState CurrentConsciousnessState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EEmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float AwarenessRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float EmotionalStability;

    // Memory System
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FConsciousnessMemory> ConsciousnessMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Behavior Trees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* DormantBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* AwareBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* EnlightenedBehaviorTree;

public:
    // Consciousness Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateConsciousnessLevel(float DeltaLevel);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void SetConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void SetEmotionalState(EEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    float GetConsciousnessLevel() const { return ConsciousnessLevel; }

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    EConsciousnessState GetConsciousnessState() const { return CurrentConsciousnessState; }

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    EEmotionalState GetEmotionalState() const { return CurrentEmotionalState; }

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddConsciousnessMemory(FVector Location, float ConsciousnessLevel, const FString& EventDescription);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FConsciousnessMemory> GetNearbyMemories(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemoryDecay(float DeltaTime);

    // Perception Functions
    UFUNCTION(BlueprintCallable, Category = "Perception")
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void DetectConsciousnessFields();

    UFUNCTION(BlueprintCallable, Category = "Perception")
    bool CanSenseConsciousness(AActor* Actor, float& OutConsciousnessLevel);

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToConsciousnessChange(float OldLevel, float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void HandlePlayerInteraction(class ATranspersonalCharacter* Player);

protected:
    // Internal Functions
    void InitializeConsciousness();
    void UpdateConsciousnessState(float DeltaTime);
    void ProcessEmotionalResponse(float DeltaTime);
    void UpdateBlackboardValues();
    
    // Consciousness thresholds
    UPROPERTY(EditAnywhere, Category = "Consciousness")
    float AwakeningThreshold = 25.0f;
    
    UPROPERTY(EditAnywhere, Category = "Consciousness")
    float AwarenessThreshold = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Consciousness")
    float EnlightenmentThreshold = 75.0f;
    
    UPROPERTY(EditAnywhere, Category = "Consciousness")
    float TranscendenceThreshold = 95.0f;

private:
    float LastConsciousnessUpdate;
    float EmotionalResponseTimer;
    bool bIsProcessingConsciousnessChange;
};