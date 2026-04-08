#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "NPCBehaviorSystem.generated.h"

// Forward Declarations
class UBehaviorTree;
class UBlackboardData;
class ADinosaurNPC;

/**
 * Core system managing NPC behavior patterns, memory, and social dynamics
 * Handles autonomous life cycles, domestication progress, and emergent interactions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPCBehaviorSystem : public UObject
{
    GENERATED_BODY()

public:
    UNPCBehaviorSystem();

    // Core System Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeBehaviorSystem(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RegisterNPC(ADinosaurNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UnregisterNPC(ADinosaurNPC* NPC);

    // Behavior State Management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNPCBehaviorState(ADinosaurNPC* NPC, float DeltaTime);

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RecordMemory(ADinosaurNPC* NPC, const FVector& Location, AActor* Subject, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool HasMemoryOf(ADinosaurNPC* NPC, AActor* Subject, float& OutIntensity);

    // Domestication System
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateDomesticationProgress(ADinosaurNPC* NPC, AActor* Player, float PositiveInteraction);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetDomesticationLevel(ADinosaurNPC* NPC, AActor* Player);

protected:
    // Registered NPCs
    UPROPERTY()
    TArray<ADinosaurNPC*> RegisteredNPCs;

    // Behavior Trees for different archetypes
    UPROPERTY(EditDefaultsOnly, Category = "Behavior Trees")
    TMap<FString, UBehaviorTree*> ArchetypeBehaviorTrees;

    // Blackboard assets
    UPROPERTY(EditDefaultsOnly, Category = "Blackboards")
    TMap<FString, UBlackboardData*> ArchetypeBlackboards;

    // World reference
    UPROPERTY()
    UWorld* WorldContext;

private:
    // Internal behavior update functions
    void ProcessDailyRoutines(float DeltaTime);
    void ProcessSocialInteractions(float DeltaTime);
    void ProcessEnvironmentalAwareness(float DeltaTime);
    void ProcessMemoryDecay(float DeltaTime);
};

/**
 * Individual NPC memory entry
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    AActor* Subject;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float EmotionalIntensity;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float TimeStamp;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FString MemoryType; // "threat", "food", "ally", "neutral"

    FNPCMemoryEntry()
    {
        Subject = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        EmotionalIntensity = 0.0f;
        TimeStamp = 0.0f;
        MemoryType = "neutral";
    }
};

/**
 * Domestication progress tracking
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDomesticationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    AActor* PlayerReference;

    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    float TrustLevel; // 0.0 to 1.0

    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    float FearLevel; // 0.0 to 1.0

    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    int32 PositiveInteractions;

    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    int32 NegativeInteractions;

    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    float LastInteractionTime;

    FDomesticationData()
    {
        PlayerReference = nullptr;
        TrustLevel = 0.0f;
        FearLevel = 1.0f;
        PositiveInteractions = 0;
        NegativeInteractions = 0;
        LastInteractionTime = 0.0f;
    }
};