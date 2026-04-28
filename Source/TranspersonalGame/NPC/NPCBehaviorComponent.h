#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "NPCBehaviorTypes.h"
#include "NPCBehaviorComponent.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - NPC BEHAVIOR COMPONENT
 * NPC Behavior Agent #11
 * 
 * Core component that manages NPC behavior, personality, needs, memory, and relationships.
 * Attaches to any actor to give it intelligent NPC behavior for prehistoric survival.
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core", meta = (AllowPrivateAccess = "true"))
    FNPC_PersonalityProfile PersonalityProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core", meta = (AllowPrivateAccess = "true"))
    FNPC_BehaviorData BehaviorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core", meta = (AllowPrivateAccess = "true"))
    FNPC_Needs Needs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core", meta = (AllowPrivateAccess = "true"))
    FNPC_Memory Memory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_DialogueData> DialogueOptions;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR SETTINGS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float BehaviorUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float NeedsDecayRate = 1.0f; // How fast needs decay per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float MemoryDecayRate = 0.1f; // How fast memories fade

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    bool bEnableAdvancedAI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    bool bCanInteractWithPlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    bool bCanFormRelationships = true;

    // ═══════════════════════════════════════════════════════════════
    // PATROL AND TERRITORY SETTINGS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float PatrolWaitTime = 5.0f;

    // ═══════════════════════════════════════════════════════════════
    // PRIVATE VARIABLES
    // ═══════════════════════════════════════════════════════════════

private:
    float LastBehaviorUpdate = 0.0f;
    float LastNeedsUpdate = 0.0f;
    float StateTimer = 0.0f;
    bool bIsInitialized = false;

public:
    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR STATE MANAGEMENT
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateMemory(float DeltaTime);

    // ═══════════════════════════════════════════════════════════════
    // PERCEPTION AND AWARENESS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    bool CanSeeActor(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    AActor* FindNearestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    AActor* FindNearestFood() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    FVector FindNearestWaterSource() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    void UpdateThreatLevel();

    // ═══════════════════════════════════════════════════════════════
    // MOVEMENT AND NAVIGATION
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    void MoveToLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    void MoveToActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    void UpdatePatrol();

    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    bool IsAtLocation(const FVector& Location, float Tolerance = 100.0f) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    bool IsInTerritory() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Movement")
    void ReturnToTerritory();

    // ═══════════════════════════════════════════════════════════════
    // RELATIONSHIP MANAGEMENT
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    void AddRelationship(AActor* TargetActor, ENPC_RelationshipType RelationType, float Strength);

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    void UpdateRelationship(AActor* TargetActor, float StrengthChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    FNPC_Relationship* GetRelationship(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    void RemoveRelationship(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    TArray<AActor*> GetAlliesInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    TArray<AActor*> GetEnemiesInRange(float Range) const;

    // ═══════════════════════════════════════════════════════════════
    // DIALOGUE AND INTERACTION
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    bool CanStartDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    FNPC_DialogueData GetDialogueForType(ENPC_DialogueType DialogueType) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void StartDialogue(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void AddDialogueOption(const FNPC_DialogueData& NewDialogue);

    // ═══════════════════════════════════════════════════════════════
    // UTILITY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    void InitializeNPC();

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    void ResetToDefaults();

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    float GetDistanceToActor(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    bool IsActorInRange(AActor* TargetActor, float Range) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    void LogBehaviorState() const;

    // ═══════════════════════════════════════════════════════════════
    // BLUEPRINT EVENTS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnBehaviorStateChanged(ENPC_BehaviorState OldState, ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnThreatDetected(AActor* ThreatActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnThreatLost();

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnNeedCritical(const FString& NeedName);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnDialogueStarted(AActor* InteractingActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnRelationshipChanged(AActor* TargetActor, ENPC_RelationshipType NewType);

    // ═══════════════════════════════════════════════════════════════
    // GETTERS AND SETTERS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Data")
    const FNPC_PersonalityProfile& GetPersonalityProfile() const { return PersonalityProfile; }

    UFUNCTION(BlueprintCallable, Category = "NPC Data")
    const FNPC_BehaviorData& GetBehaviorData() const { return BehaviorData; }

    UFUNCTION(BlueprintCallable, Category = "NPC Data")
    const FNPC_Needs& GetNeeds() const { return Needs; }

    UFUNCTION(BlueprintCallable, Category = "NPC Data")
    const FNPC_Memory& GetMemory() const { return Memory; }

    UFUNCTION(BlueprintCallable, Category = "NPC Data")
    void SetPersonalityTrait(ENPC_PersonalityTrait Trait, bool bAdd = true);

    UFUNCTION(BlueprintCallable, Category = "NPC Data")
    bool HasPersonalityTrait(ENPC_PersonalityTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Data")
    void SetProfession(ENPC_Profession NewProfession);

    UFUNCTION(BlueprintCallable, Category = "NPC Data")
    ENPC_Profession GetProfession() const { return PersonalityProfile.Profession; }
};