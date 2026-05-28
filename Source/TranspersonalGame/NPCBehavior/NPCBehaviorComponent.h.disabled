#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPCBehaviorTypes.h"
#include "NPCBehaviorComponent.generated.h"

/**
 * NPC BEHAVIOR COMPONENT
 * NPC Behavior Agent #11
 * 
 * Core component that manages NPC personality, needs, memory, relationships,
 * and behavior state. Designed for realistic prehistoric tribal society.
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ═══════════════════════════════════════════════════════════════
    // CORE NPC DATA
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName = TEXT("Unnamed");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_Profession Profession = ENPC_Profession::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    bool bIsMale = true;

    // ═══════════════════════════════════════════════════════════════
    // PERSONALITY SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENPC_PersonalityTrait> PrimaryTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Extraversion = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Agreeableness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Conscientiousness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Neuroticism = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Openness = 0.5f;

    // ═══════════════════════════════════════════════════════════════
    // NEEDS & MOTIVATIONS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    FNPC_Needs CurrentNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float NeedsDecayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float CriticalNeedThreshold = 20.0f;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR STATE
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_BehaviorData BehaviorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FNPC_ScheduleEntry> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorUpdateInterval = 1.0f;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.1f;

    // ═══════════════════════════════════════════════════════════════
    // RELATIONSHIP SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    TArray<FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    float RelationshipDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    float SocialRange = 1000.0f;

    // ═══════════════════════════════════════════════════════════════
    // DIALOGUE SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNPC_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float LastDialogueTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueCooldown = 5.0f;

    // ═══════════════════════════════════════════════════════════════
    // COMBAT BEHAVIOR
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FNPC_CombatData CombatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatDetectionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AlliedCallRange = 800.0f;

    // ═══════════════════════════════════════════════════════════════
    // AI INTEGRATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AIUpdateInterval = 0.5f;

private:
    // Internal timers
    float NeedsUpdateTimer = 0.0f;
    float BehaviorUpdateTimer = 0.0f;
    float MemoryUpdateTimer = 0.0f;
    float RelationshipUpdateTimer = 0.0f;
    float AIUpdateTimer = 0.0f;

public:
    // ═══════════════════════════════════════════════════════════════
    // PUBLIC INTERFACE METHODS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemory(ENPC_MemoryType Type, const FVector& Location, const FString& Description, float Importance = 1.0f, AActor* RelatedActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateRelationship(AActor* TargetActor, float AffinityChange, float TrustChange = 0.0f, float RespectChange = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPC_Relationship* FindRelationship(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FString GetDialogueLine(ENPC_DialogueType Type, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanInteract() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartInteraction(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void EndInteraction();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetNeedValue(const FString& NeedName) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ModifyNeed(const FString& NeedName, float Amount);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool HasPersonalityTrait(ENPC_PersonalityTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void TriggerEmergencyBehavior(ENPC_BehaviorState EmergencyState, AActor* Threat = nullptr);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    TArray<AActor*> FindNearbyAllies(float Range = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void CallForHelp(AActor* Threat);

protected:
    // ═══════════════════════════════════════════════════════════════
    // INTERNAL UPDATE METHODS
    // ═══════════════════════════════════════════════════════════════

    void UpdateNeeds(float DeltaTime);
    void UpdateBehavior(float DeltaTime);
    void UpdateMemories(float DeltaTime);
    void UpdateRelationships(float DeltaTime);
    void UpdateAI(float DeltaTime);

    void ProcessSchedule();
    void EvaluateThreats();
    void HandleCriticalNeeds();

    FNPC_ScheduleEntry* GetCurrentScheduleEntry();
    ENPC_BehaviorState DetermineOptimalBehavior();
    
    void InitializePersonality();
    void InitializeSchedule();
    void InitializeDialogue();

    // Blackboard key management
    void UpdateBlackboardValues();
    void SetBlackboardVector(const FString& KeyName, const FVector& Value);
    void SetBlackboardFloat(const FString& KeyName, float Value);
    void SetBlackboardBool(const FString& KeyName, bool Value);
    void SetBlackboardObject(const FString& KeyName, AActor* Value);
};