#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "NPC_InteractionBehaviorSystem.generated.h"

// Forward declarations
class UNPC_SocialMemoryComponent;
class UNPC_EmotionalStateComponent;

UENUM(BlueprintType)
enum class ENPC_InteractionType : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Curious         UMETA(DisplayName = "Curious"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Social          UMETA(DisplayName = "Social"),
    Predatory       UMETA(DisplayName = "Predatory"),
    Submissive      UMETA(DisplayName = "Submissive")
};

UENUM(BlueprintType)
enum class ENPC_InteractionResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Interrupted     UMETA(DisplayName = "Interrupted"),
    Escalated       UMETA(DisplayName = "Escalated"),
    Avoided         UMETA(DisplayName = "Avoided"),
    Remembered      UMETA(DisplayName = "Remembered")
};

USTRUCT(BlueprintType)
struct FNPC_InteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    ENPC_InteractionType InteractionType = ENPC_InteractionType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FVector InteractionLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float Timestamp = 0.0f;

    FNPC_InteractionData()
    {
        InteractionType = ENPC_InteractionType::Neutral;
        InteractionStrength = 1.0f;
        InteractionRange = 1000.0f;
        InteractionDuration = 5.0f;
        bRequiresLineOfSight = true;
        InteractionLocation = FVector::ZeroVector;
        Timestamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_InteractionContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<TWeakObjectPtr<AActor>> NearbyActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float EnvironmentalStress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bIsInTerritory = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bHasPackSupport = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 ThreatLevel = 0;

    FNPC_InteractionContext()
    {
        EnvironmentalStress = 0.0f;
        TimeOfDay = 12.0f;
        bIsInTerritory = false;
        bHasPackSupport = false;
        ThreatLevel = 0;
    }
};

/**
 * Advanced NPC Interaction Behavior System
 * Manages complex behavioral interactions between NPCs, dinosaurs, and players
 * Handles context-aware decision making, social dynamics, and behavioral adaptation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_InteractionBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_InteractionBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core interaction management
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool InitiateInteraction(AActor* TargetActor, ENPC_InteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void ProcessInteraction(const FNPC_InteractionData& InteractionData);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    ENPC_InteractionResult EvaluateInteractionOutcome(const FNPC_InteractionData& InteractionData, const FNPC_InteractionContext& Context);

    // Context analysis
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FNPC_InteractionContext AnalyzeInteractionContext(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool ShouldInitiateInteraction(AActor* TargetActor, ENPC_InteractionType& OutInteractionType);

    // Behavioral adaptation
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void AdaptBehaviorBasedOnInteraction(const FNPC_InteractionData& InteractionData, ENPC_InteractionResult Result);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void UpdateInteractionMemory(AActor* TargetActor, ENPC_InteractionType InteractionType, ENPC_InteractionResult Result);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    float CalculateInteractionPriority(AActor* TargetActor, ENPC_InteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool IsValidInteractionTarget(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void RegisterInteractionCallback(AActor* TargetActor);

protected:
    // Internal processing
    void ProcessPendingInteractions(float DeltaTime);
    void UpdateInteractionStates(float DeltaTime);
    void CleanupExpiredInteractions();

    // Behavioral logic
    ENPC_InteractionType DetermineOptimalInteractionType(AActor* TargetActor, const FNPC_InteractionContext& Context);
    bool ValidateInteractionConditions(const FNPC_InteractionData& InteractionData);
    void ExecuteInteractionBehavior(const FNPC_InteractionData& InteractionData);

    // Component references
    UPROPERTY()
    UNPC_SocialMemoryComponent* SocialMemoryComponent;

    UPROPERTY()
    UNPC_EmotionalStateComponent* EmotionalStateComponent;

private:
    // Configuration
    UPROPERTY(EditAnywhere, Category = "Interaction Settings", meta = (AllowPrivateAccess = "true"))
    float InteractionUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Interaction Settings", meta = (AllowPrivateAccess = "true"))
    float MaxInteractionRange = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Interaction Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxConcurrentInteractions = 3;

    UPROPERTY(EditAnywhere, Category = "Interaction Settings", meta = (AllowPrivateAccess = "true"))
    bool bEnableInteractionLogging = true;

    // Runtime data
    UPROPERTY()
    TArray<FNPC_InteractionData> ActiveInteractions;

    UPROPERTY()
    TArray<FNPC_InteractionData> PendingInteractions;

    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, float> InteractionCooldowns;

    float LastInteractionUpdate = 0.0f;
    int32 InteractionCounter = 0;
};